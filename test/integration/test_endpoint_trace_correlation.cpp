// Endpoint-side cross-layer correlation integration test (P1 §7 closing).
//
// Verifies that when a real GET flows through ServerEndpoint:
//
//   inbound bytes -> IApduChannel::ReceiveApdu
//     -> XdlmsServerApduProcessor::ProcessRequest
//          (computes conversationId, calls channel.SetCorrelation)
//     -> TracingXdlmsServerDispatcher::DispatchGet
//          (reads channel.CurrentConversationId, stamps event)
//     -> ICosemObject::ReadAttribute
//     -> XdlmsServerApduProcessor emits ResponseSent
//     -> IApduChannel::SendApdu
//
// the same non-zero conversationId appears in:
//   - the channel side (channel.lastCorrelation),
//   - both IXdlmsTraceSink events (RequestReceived + ResponseSent),
//   - the IServerDispatchTraceSink GetDispatched event.
//
// This pins the two production-side gaps that closing-pack 0.102.0+
// fills: TracingXdlmsServerDispatcher reading CurrentConversationId,
// and ServerEndpoint composition wiring the APDU channel into the
// processor.

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/cosem/cosem.hpp"
#include "dlms/endpoint/endpoint.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/server/server_dispatch_trace.hpp"
#include "dlms/xdlms/xdlms_correlation.hpp"
#include "dlms/xdlms/xdlms_trace.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

using dlms::xdlms::MakeConversationId;
using dlms::xdlms::kNoConversationId;

class CorrelationFakeChannel : public dlms::profile::IApduChannel
{
public:
  CorrelationFakeChannel()
    : open_(false)
    , correlation_(kNoConversationId)
    , correlationAtSend_(kNoConversationId)
    , sendCalls_(0u)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open_ = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    open_ = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const { return open_; }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls_;
    correlationAtSend_ = correlation_;
    sent_.assign(apdu.data, apdu.data + apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    apdu = nextReceive_;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    if (output.size < nextReceive_.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0u; i < nextReceive_.size(); ++i) {
      output.data[i] = nextReceive_[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = nextReceive_.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  void SetCorrelation(std::uint64_t conversationId) noexcept
  {
    correlation_ = conversationId;
  }

  std::uint64_t CurrentConversationId() const noexcept
  {
    return correlation_;
  }

  void Prime(const std::vector<std::uint8_t>& apdu) { nextReceive_ = apdu; }
  std::uint64_t LastCorrelation() const { return correlation_; }
  std::uint64_t CorrelationAtSend() const { return correlationAtSend_; }
  std::size_t SendCalls() const { return sendCalls_; }

private:
  bool open_;
  std::uint64_t correlation_;
  std::uint64_t correlationAtSend_;
  std::size_t sendCalls_;
  std::vector<std::uint8_t> nextReceive_;
  std::vector<std::uint8_t> sent_;
};

class CapturingXdlmsSink : public dlms::xdlms::IXdlmsTraceSink
{
public:
  std::vector<dlms::xdlms::XdlmsTraceEvent> events;
  void OnXdlmsTrace(const dlms::xdlms::XdlmsTraceEvent& event)
  {
    events.push_back(event);
  }
};

class CapturingDispatchSink : public dlms::server::IServerDispatchTraceSink
{
public:
  std::vector<dlms::server::ServerDispatchTraceEvent> events;
  void OnServerDispatchTrace(
    const dlms::server::ServerDispatchTraceEvent& event)
  {
    events.push_back(event);
  }
};

dlms::cosem::CosemByteBuffer EncodeLongUnsigned(std::uint16_t value)
{
  std::uint8_t buffer[8];
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeDlmsData(data, writer));
  return dlms::cosem::CosemByteBuffer(buffer, buffer + writer.WrittenSize());
}

std::shared_ptr<dlms::cosem::CosemRegisterObject> MakeRegisterObject(
  std::uint16_t value)
{
  return std::shared_ptr<dlms::cosem::CosemRegisterObject>(
    new dlms::cosem::CosemRegisterObject(
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
      EncodeLongUnsigned(value),
      dlms::cosem::types::ScalerUnit(),
      dlms::cosem::AttributeAccessMode::ReadOnly));
}

std::vector<std::uint8_t> EncodeGetRequest(std::uint8_t invokeIdAndPriority)
{
  const dlms::apdu::XdlmsApdu request =
    dlms::apdu::MakeGetRequestNormal(
      invokeIdAndPriority,
      3u,
      dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
      2u);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

} // namespace

TEST(EndpointTraceCorrelation,
     SameConversationIdAcrossChannelXdlmsAndDispatchForGet)
{
  CorrelationFakeChannel channel;
  const std::uint8_t invokeIdAndPriority = 0x85u;
  channel.Prime(EncodeGetRequest(invokeIdAndPriority));

  CapturingXdlmsSink xdlmsSink;
  CapturingDispatchSink dispatchSink;

  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(MakeRegisterObject(0x1234u)));

  dlms::endpoint::ServerEndpointOptions options =
    dlms::endpoint::DefaultServerEndpointOptions();
  options.xdlmsTraceSink = &xdlmsSink;
  options.serverDispatchTraceSink = &dispatchSink;

  dlms::endpoint::ServerEndpoint endpoint(channel, options, logicalDevice);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Open());
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.RunOnce());

  // With no association seed source wired into the endpoint,
  // ConversationSeed() == 0 and
  // MakeConversationId(0, invokeId) == invokeId & 0x0F.
  const std::uint64_t expectedId =
    MakeConversationId(0u, invokeIdAndPriority & 0x0Fu);
  ASSERT_NE(kNoConversationId, expectedId);

  // Channel saw SetCorrelation with the right id, and that id was
  // still latched when SendApdu was called for the GET response.
  EXPECT_EQ(expectedId, channel.LastCorrelation());
  EXPECT_EQ(expectedId, channel.CorrelationAtSend());
  EXPECT_GE(channel.SendCalls(), 1u);

  // Both xDLMS trace events for this request carry the same id.
  ASSERT_GE(xdlmsSink.events.size(), 2u);
  bool sawRequest = false;
  bool sawResponse = false;
  for (std::size_t i = 0u; i < xdlmsSink.events.size(); ++i) {
    const dlms::xdlms::XdlmsTraceEvent& e = xdlmsSink.events[i];
    if (e.kind == dlms::xdlms::XdlmsTraceKind::RequestReceived) {
      sawRequest = true;
      EXPECT_EQ(expectedId, e.conversationId);
    } else if (e.kind == dlms::xdlms::XdlmsTraceKind::ResponseSent) {
      sawResponse = true;
      EXPECT_EQ(expectedId, e.conversationId);
    }
  }
  EXPECT_TRUE(sawRequest);
  EXPECT_TRUE(sawResponse);

  // Dispatch sink saw exactly one GetDispatched event with the same id.
  ASSERT_EQ(1u, dispatchSink.events.size());
  EXPECT_EQ(dlms::server::ServerDispatchTraceKind::GetDispatched,
            dispatchSink.events[0].kind);
  EXPECT_EQ(expectedId, dispatchSink.events[0].conversationId);
}
