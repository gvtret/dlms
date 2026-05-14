#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/association/association_client.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/xdlms/xdlms_client.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <deque>
#include <vector>

namespace {

class FakeApduChannel : public dlms::profile::IApduChannel
{
public:
  FakeApduChannel()
    : open(false)
    , sendCalls(0)
    , receiveCalls(0)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    open = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls;
    sent.assign(apdu.data, apdu.data + apdu.size);
    sentHistory.push_back(sent);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++receiveCalls;
    if (!receiveQueue.empty()) {
      apdu = receiveQueue.front();
      receiveQueue.pop_front();
    } else {
      apdu = nextReceive;
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    ++receiveCalls;
    if (output.size < nextReceive.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0; i < nextReceive.size(); ++i) {
      output.data[i] = nextReceive[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = nextReceive.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  bool open;
  int sendCalls;
  int receiveCalls;
  std::vector<std::uint8_t> sent;
  std::vector<std::vector<std::uint8_t> > sentHistory;
  std::vector<std::uint8_t> nextReceive;
  std::deque<std::vector<std::uint8_t> > receiveQueue;
};

std::vector<std::uint8_t> EncodedLongUnsigned(std::uint16_t value);

class ServerBackedApduChannel : public dlms::profile::IApduChannel
{
public:
  explicit ServerBackedApduChannel(
    dlms::xdlms::XdlmsServerApduProcessor& processor)
    : processor_(processor)
    , open(false)
    , sendCalls(0)
    , receiveCalls(0)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    open = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls;
    sent.assign(apdu.data, apdu.data + apdu.size);
    sentHistory.push_back(sent);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++receiveCalls;
    if (!receiveQueue.empty()) {
      apdu = receiveQueue.front();
      receiveQueue.pop_front();
      return dlms::profile::ProfileStatus::Ok;
    }

    const dlms::xdlms::XdlmsStatus status =
      processor_.ProcessRequest(sent, apdu);
    return status == dlms::xdlms::XdlmsStatus::Ok
      ? dlms::profile::ProfileStatus::Ok
      : dlms::profile::ProfileStatus::InvalidFrame;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    std::vector<std::uint8_t> apdu;
    const dlms::profile::ProfileStatus status = ReceiveApdu(apdu);
    if (status != dlms::profile::ProfileStatus::Ok) {
      return status;
    }
    if (output.size < apdu.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0; i < apdu.size(); ++i) {
      output.data[i] = apdu[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = apdu.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::xdlms::XdlmsServerApduProcessor& processor_;
  bool open;
  int sendCalls;
  int receiveCalls;
  std::vector<std::uint8_t> sent;
  std::vector<std::vector<std::uint8_t> > sentHistory;
  std::deque<std::vector<std::uint8_t> > receiveQueue;
};

class ActionOnlyHandler : public dlms::xdlms::IXdlmsServerHandler
{
public:
  ActionOnlyHandler()
    : setCalls(0)
    , actionCalls(0)
    , lastSet(dlms::xdlms::EmptySetIndication())
    , lastAction(dlms::xdlms::EmptyActionIndication())
  {
  }

  dlms::xdlms::XdlmsStatus HandleGet(
    const dlms::xdlms::GetIndication& indication,
    dlms::xdlms::GetResult& result)
  {
    (void)indication;
    (void)result;
    return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
  }

  dlms::xdlms::XdlmsStatus HandleSet(
    const dlms::xdlms::SetIndication& indication,
    dlms::xdlms::SetResult& result)
  {
    ++setCalls;
    lastSet = indication;
    result.accessResult = 0u;
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  dlms::xdlms::XdlmsStatus HandleAction(
    const dlms::xdlms::ActionIndication& indication,
    dlms::xdlms::ActionResult& result)
  {
    ++actionCalls;
    lastAction = indication;
    result.actionResult = 0u;
    result.hasData = true;
    result.data = EncodedLongUnsigned(0x2468u);
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  int setCalls;
  int actionCalls;
  dlms::xdlms::SetIndication lastSet;
  dlms::xdlms::ActionIndication lastAction;
};

std::vector<std::uint8_t> MakeAareBytes()
{
  const std::uint8_t kAare[] = {
    0x61, 0x4E, 0x80, 0x02, 0x02, 0x84, 0xA1, 0x09,
    0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
    0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05,
    0xA1, 0x03, 0x02, 0x01, 0x0E, 0x88, 0x02, 0x07,
    0x80, 0x89, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
    0x02, 0x02, 0xAA, 0x12, 0x80, 0x10, 0xC6, 0x69,
    0x73, 0x51, 0xFF, 0x4A, 0xEC, 0x29, 0xCD, 0xBA,
    0xAB, 0xF2, 0xFB, 0xE3, 0x46, 0x7C, 0xBE, 0x10,
    0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04,
    0x00, 0x40, 0x18, 0x1D, 0x02, 0x00, 0x00, 0x07};

  return std::vector<std::uint8_t>(kAare, kAare + sizeof(kAare));
}

std::vector<std::uint8_t> MakeGetResponse()
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::GetResponse;
  response.getResponse.invokeIdAndPriority = 0x81u;
  response.getResponse.resultChoice = dlms::apdu::GetDataResultChoice::Data;
  response.getResponse.data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  response.getResponse.data.unsignedValue = 0x09F1u;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> MakeGetBlockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber,
  bool lastBlock,
  const std::vector<std::uint8_t>& rawData)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::GetResponse;
  response.getResponseAny.choice =
    dlms::apdu::GetResponseChoice::WithDataBlock;
  response.getResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.getResponseAny.dataBlock.lastBlock = lastBlock;
  response.getResponseAny.dataBlock.blockNumber = blockNumber;
  response.getResponseAny.dataBlock.rawData.data =
    rawData.empty() ? 0 : &rawData[0];
  response.getResponseAny.dataBlock.rawData.size = rawData.size();

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> EncodedLongUnsigned(std::uint16_t value)
{
  std::vector<std::uint8_t> data;
  data.push_back(0x12u);
  data.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
  data.push_back(static_cast<std::uint8_t>(value & 0xFFu));
  return data;
}

std::vector<std::uint8_t> MakeSetAckBlockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::SetResponse;
  response.setResponseAny.choice = dlms::apdu::SetResponseChoice::DataBlock;
  response.setResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.setResponseAny.blockNumber = blockNumber;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> MakeSetLastBlockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber,
  std::uint8_t result)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::SetResponse;
  response.setResponseAny.choice =
    dlms::apdu::SetResponseChoice::LastDataBlock;
  response.setResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.setResponseAny.blockNumber = blockNumber;
  response.setResponseAny.result = result;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> MakeActionBlockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber,
  bool lastBlock,
  const std::vector<std::uint8_t>& rawData)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::ActionResponse;
  response.actionResponseAny.choice =
    dlms::apdu::ActionResponseChoice::WithPblock;
  response.actionResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponseAny.dataBlock.lastBlock = lastBlock;
  response.actionResponseAny.dataBlock.blockNumber = blockNumber;
  response.actionResponseAny.dataBlock.rawData.data =
    rawData.empty() ? 0 : &rawData[0];
  response.actionResponseAny.dataBlock.rawData.size = rawData.size();

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> MakeActionNextPblockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::ActionResponse;
  response.actionResponseAny.choice =
    dlms::apdu::ActionResponseChoice::NextPblock;
  response.actionResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponseAny.blockNumber = blockNumber;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> MakeActionResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint8_t result)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::ActionResponse;
  response.actionResponseAny.choice =
    dlms::apdu::ActionResponseChoice::Normal;
  response.actionResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponseAny.normal.result = result;
  response.actionResponseAny.normal.hasReturnParameter = true;
  response.actionResponseAny.normal.returnParameter.type =
    dlms::apdu::DlmsDataType::LongUnsigned;
  response.actionResponseAny.normal.returnParameter.unsignedValue = 0x2468u;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

std::vector<std::uint8_t> ActionPayload(
  std::uint8_t result,
  const std::vector<std::uint8_t>& encodedData)
{
  std::vector<std::uint8_t> output;
  output.push_back(result);
  output.push_back(encodedData.empty() ? 0u : 1u);
  if (!encodedData.empty()) {
    output.push_back(0u);
  }
  output.insert(output.end(), encodedData.begin(), encodedData.end());
  return output;
}

dlms::xdlms::CosemAttributeDescriptor MakeDescriptor()
{
  dlms::xdlms::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 7u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 99, 1, 0, 255);
  descriptor.attributeId = 7u;
  return descriptor;
}

dlms::xdlms::CosemMethodDescriptor MakeMethodDescriptor()
{
  dlms::xdlms::CosemMethodDescriptor descriptor =
    dlms::xdlms::EmptyCosemMethodDescriptor();
  descriptor.classId = 7u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 99, 1, 0, 255);
  descriptor.methodId = 1u;
  return descriptor;
}

} // namespace

TEST(XdlmsIntegration, AssociationAndNormalGetShareApduChannel)
{
  FakeApduChannel channel;
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.nextReceive = MakeAareBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());
  EXPECT_EQ(1, channel.sendCalls);
  EXPECT_EQ(1, channel.receiveCalls);

  channel.nextReceive = MakeGetResponse();
  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::GetResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Get(MakeDescriptor(), result));
  ASSERT_EQ(2, channel.sendCalls);
  ASSERT_EQ(2, channel.receiveCalls);
  ASSERT_EQ(13u, channel.sent.size());
  EXPECT_EQ(0xC0u, channel.sent[0]);
  EXPECT_EQ(0x01u, channel.sent[1]);
  EXPECT_EQ(0x81u, channel.sent[2]);
  EXPECT_EQ(1u, result.invokeId);
  ASSERT_TRUE(result.hasData);
  ASSERT_EQ(3u, result.data.size());
  EXPECT_EQ(0x12u, result.data[0]);
  EXPECT_EQ(0x09u, result.data[1]);
  EXPECT_EQ(0xF1u, result.data[2]);
}

TEST(XdlmsIntegration, NormalGetCollectsResponseDataBlocks)
{
  FakeApduChannel channel;
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.nextReceive = MakeAareBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  channel.receiveQueue.push_back(
    MakeGetBlockResponse(0x81u, 1u, false, EncodedLongUnsigned(0x1111u)));
  channel.receiveQueue.push_back(
    MakeGetBlockResponse(0x81u, 2u, true, EncodedLongUnsigned(0x2222u)));

  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::GetResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Get(MakeDescriptor(), result));

  ASSERT_EQ(3u, channel.sentHistory.size());
  dlms::apdu::XdlmsApdu nextRequest;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[2][0],
              channel.sentHistory[2].size(),
              nextRequest));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetRequest, nextRequest.kind);
  EXPECT_EQ(dlms::apdu::GetRequestChoice::Next,
            nextRequest.getRequestAny.choice);
  EXPECT_EQ(1u, nextRequest.getRequestAny.blockNumber);

  std::vector<std::uint8_t> expected = EncodedLongUnsigned(0x1111u);
  const std::vector<std::uint8_t> second = EncodedLongUnsigned(0x2222u);
  expected.insert(expected.end(), second.begin(), second.end());
  EXPECT_EQ(expected, result.data);
}

TEST(XdlmsIntegration, NormalSetSendsRequestDataBlocks)
{
  FakeApduChannel channel;
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.nextReceive = MakeAareBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  channel.receiveQueue.push_back(MakeSetAckBlockResponse(0x81u, 1u));
  channel.receiveQueue.push_back(MakeSetLastBlockResponse(0x81u, 2u, 0u));

  dlms::xdlms::ServiceOptions options =
    dlms::xdlms::DefaultServiceOptions();
  options.maxSetBlockPayloadBytes = 2u;

  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::SetResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Set(
              MakeDescriptor(),
              EncodedLongUnsigned(0x4321u),
              options,
              result));

  ASSERT_EQ(3u, channel.sentHistory.size());
  dlms::apdu::XdlmsApdu firstBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[1][0],
              channel.sentHistory[1].size(),
              firstBlock));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetRequest, firstBlock.kind);
  EXPECT_EQ(dlms::apdu::SetRequestChoice::WithFirstDataBlock,
            firstBlock.setRequestAny.choice);
  EXPECT_FALSE(firstBlock.setRequestAny.dataBlock.lastBlock);
  EXPECT_EQ(1u, firstBlock.setRequestAny.dataBlock.blockNumber);

  dlms::apdu::XdlmsApdu finalBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[2][0],
              channel.sentHistory[2].size(),
              finalBlock));
  EXPECT_EQ(dlms::apdu::SetRequestChoice::WithDataBlock,
            finalBlock.setRequestAny.choice);
  EXPECT_TRUE(finalBlock.setRequestAny.dataBlock.lastBlock);
  EXPECT_EQ(2u, finalBlock.setRequestAny.dataBlock.blockNumber);
  EXPECT_EQ(1u, result.invokeId);
  EXPECT_EQ(0u, result.accessResult);
}

TEST(XdlmsIntegration, NormalActionCollectsResponsePblocks)
{
  FakeApduChannel channel;
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.nextReceive = MakeAareBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  const std::vector<std::uint8_t> payload =
    ActionPayload(0u, EncodedLongUnsigned(0x2468u));
  channel.receiveQueue.push_back(
    MakeActionBlockResponse(
      0x81u,
      1u,
      false,
      std::vector<std::uint8_t>(payload.begin(), payload.begin() + 2)));
  channel.receiveQueue.push_back(
    MakeActionBlockResponse(
      0x81u,
      2u,
      true,
      std::vector<std::uint8_t>(payload.begin() + 2, payload.end())));

  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::ActionResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Action(
              MakeMethodDescriptor(),
              false,
              std::vector<std::uint8_t>(),
              result));

  ASSERT_EQ(3u, channel.sentHistory.size());
  dlms::apdu::XdlmsApdu nextRequest;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[2][0],
              channel.sentHistory[2].size(),
              nextRequest));
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionRequest, nextRequest.kind);
  EXPECT_EQ(dlms::apdu::ActionRequestChoice::NextPblock,
            nextRequest.actionRequestAny.choice);
  EXPECT_EQ(1u, nextRequest.actionRequestAny.blockNumber);

  EXPECT_EQ(1u, result.invokeId);
  EXPECT_EQ(0u, result.actionResult);
  EXPECT_TRUE(result.hasData);
  EXPECT_EQ(EncodedLongUnsigned(0x2468u), result.data);
}

TEST(XdlmsIntegration, NormalActionSendsRequestPblocks)
{
  FakeApduChannel channel;
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.nextReceive = MakeAareBytes();
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  channel.receiveQueue.push_back(MakeActionNextPblockResponse(0x81u, 1u));
  channel.receiveQueue.push_back(MakeActionResponse(0x81u, 0u));

  dlms::xdlms::ServiceOptions options =
    dlms::xdlms::DefaultServiceOptions();
  options.maxActionBlockPayloadBytes = 2u;

  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::ActionResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Action(
              MakeMethodDescriptor(),
              true,
              EncodedLongUnsigned(0x4321u),
              options,
              result));

  ASSERT_EQ(3u, channel.sentHistory.size());
  dlms::apdu::XdlmsApdu firstBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[1][0],
              channel.sentHistory[1].size(),
              firstBlock));
  EXPECT_EQ(dlms::apdu::ActionRequestChoice::WithFirstPblock,
            firstBlock.actionRequestAny.choice);
  EXPECT_EQ(1u, firstBlock.actionRequestAny.dataBlock.blockNumber);
  EXPECT_FALSE(firstBlock.actionRequestAny.dataBlock.lastBlock);
  EXPECT_EQ(2u, firstBlock.actionRequestAny.dataBlock.rawData.size);

  dlms::apdu::XdlmsApdu finalBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[2][0],
              channel.sentHistory[2].size(),
              finalBlock));
  EXPECT_EQ(dlms::apdu::ActionRequestChoice::WithPblock,
            finalBlock.actionRequestAny.choice);
  EXPECT_EQ(2u, finalBlock.actionRequestAny.dataBlock.blockNumber);
  EXPECT_TRUE(finalBlock.actionRequestAny.dataBlock.lastBlock);
  EXPECT_EQ(1u, finalBlock.actionRequestAny.dataBlock.rawData.size);

  EXPECT_EQ(1u, result.invokeId);
  EXPECT_EQ(0u, result.actionResult);
  EXPECT_TRUE(result.hasData);
  EXPECT_EQ(EncodedLongUnsigned(0x2468u), result.data);
}

TEST(XdlmsIntegration, ActionRequestPblocksReachServerProcessor)
{
  ActionOnlyHandler handler;
  dlms::xdlms::XdlmsServerDispatcher dispatcher(handler);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  ServerBackedApduChannel channel(processor);
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.receiveQueue.push_back(MakeAareBytes());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  dlms::xdlms::ServiceOptions options =
    dlms::xdlms::DefaultServiceOptions();
  options.maxActionBlockPayloadBytes = 2u;

  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::ActionResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Action(
              MakeMethodDescriptor(),
              true,
              EncodedLongUnsigned(0x4321u),
              options,
              result));

  ASSERT_EQ(1, handler.actionCalls);
  EXPECT_TRUE(handler.lastAction.hasParameter);
  EXPECT_EQ(EncodedLongUnsigned(0x4321u), handler.lastAction.parameter);
  EXPECT_EQ(1u, result.invokeId);
  EXPECT_EQ(0u, result.actionResult);
  EXPECT_TRUE(result.hasData);
  EXPECT_EQ(EncodedLongUnsigned(0x2468u), result.data);

  ASSERT_EQ(3u, channel.sentHistory.size());
  dlms::apdu::XdlmsApdu firstBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[1][0],
              channel.sentHistory[1].size(),
              firstBlock));
  EXPECT_EQ(dlms::apdu::ActionRequestChoice::WithFirstPblock,
            firstBlock.actionRequestAny.choice);

  dlms::apdu::XdlmsApdu finalBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[2][0],
              channel.sentHistory[2].size(),
              finalBlock));
  EXPECT_EQ(dlms::apdu::ActionRequestChoice::WithPblock,
            finalBlock.actionRequestAny.choice);
}

TEST(XdlmsIntegration, SetRequestBlocksReachServerProcessor)
{
  ActionOnlyHandler handler;
  dlms::xdlms::XdlmsServerDispatcher dispatcher(handler);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  ServerBackedApduChannel channel(processor);
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());

  channel.receiveQueue.push_back(MakeAareBytes());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  dlms::xdlms::ServiceOptions options =
    dlms::xdlms::DefaultServiceOptions();
  options.maxSetBlockPayloadBytes = 2u;

  dlms::xdlms::XdlmsClient xdlms(channel, association);
  dlms::xdlms::SetResult result;

  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Set(
              MakeDescriptor(),
              EncodedLongUnsigned(0x4321u),
              options,
              result));

  ASSERT_EQ(1, handler.setCalls);
  EXPECT_EQ(EncodedLongUnsigned(0x4321u), handler.lastSet.data);
  EXPECT_EQ(1u, result.invokeId);
  EXPECT_EQ(0u, result.accessResult);

  ASSERT_EQ(3u, channel.sentHistory.size());
  dlms::apdu::XdlmsApdu firstBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[1][0],
              channel.sentHistory[1].size(),
              firstBlock));
  EXPECT_EQ(dlms::apdu::SetRequestChoice::WithFirstDataBlock,
            firstBlock.setRequestAny.choice);

  dlms::apdu::XdlmsApdu finalBlock;
  ASSERT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              &channel.sentHistory[2][0],
              channel.sentHistory[2].size(),
              finalBlock));
  EXPECT_EQ(dlms::apdu::SetRequestChoice::WithDataBlock,
            finalBlock.setRequestAny.choice);
}
