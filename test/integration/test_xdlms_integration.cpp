#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/association/association_client.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/xdlms/xdlms_client.hpp"

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

dlms::xdlms::CosemAttributeDescriptor MakeDescriptor()
{
  dlms::xdlms::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 7u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 99, 1, 0, 255);
  descriptor.attributeId = 7u;
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
