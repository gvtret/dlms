#include "dlms/association/association_types.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/profile/hdlc_profile_channel.hpp"
#include "dlms/profile/profile_types.hpp"
#include "dlms/profile/wrapper_tcp_profile_channel.hpp"
#include "dlms/transport/fake_transport.hpp"
#include "dlms/transport/transport_trace.hpp"
#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/xdlms/xdlms_correlation.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

using dlms::xdlms::MakeConversationId;
using dlms::xdlms::kNoConversationId;
using dlms::profile::ProfileByteView;

// --- Default initialization: every event must zero its conversationId. ---

TEST(TraceCorrelationField, WrapperTcpDefaultInitIsNoConversationId)
{
  dlms::profile::WrapperTcpTraceEvent ev{};
  EXPECT_EQ(ev.conversationId, kNoConversationId);
}

TEST(TraceCorrelationField, HdlcProfileDefaultInitIsNoConversationId)
{
  dlms::profile::HdlcProfileTraceEvent ev{};
  EXPECT_EQ(ev.conversationId, kNoConversationId);
}

TEST(TraceCorrelationField, TransportDefaultCtorIsNoConversationId)
{
  dlms::transport::TransportTraceEvent ev;
  EXPECT_EQ(ev.conversationId, kNoConversationId);
}

TEST(TraceCorrelationField, AssociationDefaultInitIsNoConversationId)
{
  dlms::association::AssociationTraceEvent ev{};
  EXPECT_EQ(ev.conversationId, kNoConversationId);
}

// --- Round-trip: assigning the field keeps the value. ---

TEST(TraceCorrelationField, RoundTripsAcrossAllFourEvents)
{
  const std::uint64_t id = MakeConversationId(0x9876543210FEDCB0ULL, 0x0A);

  dlms::profile::WrapperTcpTraceEvent w{};
  w.conversationId = id;
  EXPECT_EQ(w.conversationId, id);

  dlms::profile::HdlcProfileTraceEvent h{};
  h.conversationId = id;
  EXPECT_EQ(h.conversationId, id);

  dlms::transport::TransportTraceEvent t;
  t.conversationId = id;
  EXPECT_EQ(t.conversationId, id);

  dlms::association::AssociationTraceEvent a{};
  a.conversationId = id;
  EXPECT_EQ(a.conversationId, id);
}

// --- IApduChannel::SetCorrelation default impl is a callable no-op. ---

class MinimalChannel : public dlms::profile::IApduChannel
{
public:
  dlms::profile::ProfileStatus Open() { return dlms::profile::ProfileStatus::Ok; }
  dlms::profile::ProfileStatus Close() { return dlms::profile::ProfileStatus::Ok; }
  bool IsOpen() const { return false; }
  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>&)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
  dlms::profile::ProfileStatus ReceiveApdu(dlms::profile::ProfileMutableBuffer)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
};

TEST(TraceCorrelationField, ApduChannelDefaultSetCorrelationIsNoexceptNoop)
{
  MinimalChannel ch;
  // The default impl must compile, must be reachable through the base
  // pointer, must accept arbitrary 64-bit values, and must be noexcept.
  dlms::profile::IApduChannel& base = ch;
  static_assert(noexcept(base.SetCorrelation(0)),
                "SetCorrelation must be noexcept");
  base.SetCorrelation(0);
  base.SetCorrelation(kNoConversationId);
  base.SetCorrelation(MakeConversationId(0xCAFEBABEDEADBEE0ULL, 0x0F));
  base.SetCorrelation(static_cast<std::uint64_t>(-1));
  SUCCEED();
}

// --- Channels that override SetCorrelation see the value. ---

class CapturingChannel : public dlms::profile::IApduChannel
{
public:
  std::uint64_t lastCorrelation = kNoConversationId;
  void SetCorrelation(std::uint64_t conversationId) noexcept
  {
    lastCorrelation = conversationId;
  }
  dlms::profile::ProfileStatus Open() { return dlms::profile::ProfileStatus::Ok; }
  dlms::profile::ProfileStatus Close() { return dlms::profile::ProfileStatus::Ok; }
  bool IsOpen() const { return false; }
  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>&)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
  dlms::profile::ProfileStatus ReceiveApdu(dlms::profile::ProfileMutableBuffer)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
};

TEST(TraceCorrelationField, ApduChannelOverrideReceivesValue)
{
  CapturingChannel ch;
  dlms::profile::IApduChannel& base = ch;
  const std::uint64_t id = MakeConversationId(0x1111222233334440ULL, 0x06);
  base.SetCorrelation(id);
  EXPECT_EQ(ch.lastCorrelation, id);
  base.SetCorrelation(kNoConversationId);
  EXPECT_EQ(ch.lastCorrelation, kNoConversationId);
}

// --- End-to-end: WrapperTcpProfileChannel stamps every emitted event. ---

class CapturingWrapperSink : public dlms::profile::IWrapperTcpTraceSink
{
public:
  std::vector<dlms::profile::WrapperTcpTraceEvent> events;
  void OnWrapperTcpTrace(const dlms::profile::WrapperTcpTraceEvent& event)
  {
    events.push_back(event);
  }
};

std::vector<std::uint8_t> EncodeWpduForCorrelation(
  const std::vector<std::uint8_t>& apdu)
{
  dlms::wrapper::WrapperFrame frame;
  frame.sourcePort = dlms::wrapper::kManagementLogicalDevice;
  frame.destinationPort = dlms::wrapper::kPublicClient;
  frame.data = apdu.empty() ? 0 : &apdu[0];
  frame.dataSize = apdu.size();
  std::vector<std::uint8_t> wpdu;
  EXPECT_EQ(dlms::wrapper::WrapperStatus::Ok,
            dlms::wrapper::EncodeWpdu(
              frame,
              dlms::wrapper::DefaultWrapperCodecLimits(),
              wpdu));
  return wpdu;
}

TEST(TraceCorrelationField, WrapperTcpChannelStampsSendAndReceiveEvents)
{
  dlms::transport::FakeByteStream stream;

  CapturingWrapperSink sink;
  dlms::profile::ApduChannelOptions options =
    dlms::profile::DefaultApduChannelOptions();
  options.wrapperTcpTraceSink = &sink;
  options.localWrapperPort = dlms::wrapper::kPublicClient;
  options.remoteWrapperPort = dlms::wrapper::kManagementLogicalDevice;

  dlms::profile::WrapperTcpProfileChannel channel(stream, options);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.Open());

  // Seed the peer-to-us read buffer with a valid WPDU.
  const std::uint8_t apduBytes[] = {0x61, 0x29, 0xa1, 0x09};
  const std::vector<std::uint8_t> apdu(
    apduBytes, apduBytes + sizeof(apduBytes));
  const std::vector<std::uint8_t> peerWpdu = EncodeWpduForCorrelation(apdu);
  stream.ScriptRead(peerWpdu);

  const std::uint64_t id =
    MakeConversationId(0xABCDEF0123456780ULL, 0x05);

  // Before SetCorrelation: events should carry kNoConversationId.
  const std::uint8_t outApdu[] = {0x60, 0x01};
  ProfileByteView outView;
  outView.data = outApdu;
  outView.size = sizeof(outApdu);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.SendApdu(outView));
  ASSERT_FALSE(sink.events.empty());
  EXPECT_EQ(sink.events.back().conversationId, kNoConversationId)
    << "events emitted before SetCorrelation must use the zero default";

  // After SetCorrelation: every subsequent event must carry id.
  channel.SetCorrelation(id);
  const std::size_t baseline = sink.events.size();

  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.SendApdu(outView));
  std::vector<std::uint8_t> received;
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.ReceiveApdu(received));
  EXPECT_EQ(received, apdu);

  ASSERT_GT(sink.events.size(), baseline);
  for (std::size_t i = baseline; i < sink.events.size(); ++i) {
    EXPECT_EQ(sink.events[i].conversationId, id)
      << "event #" << i << " emitted after SetCorrelation must carry id";
  }

  // After clearing: events go back to zero.
  channel.SetCorrelation(kNoConversationId);
  const std::size_t afterClear = sink.events.size();
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.SendApdu(outView));
  ASSERT_GT(sink.events.size(), afterClear);
  for (std::size_t i = afterClear; i < sink.events.size(); ++i) {
    EXPECT_EQ(sink.events[i].conversationId, kNoConversationId)
      << "event #" << i << " emitted after clearing must drop the id";
  }
}

// --- End-to-end: HdlcProfileChannel stamps every emitted event. ---

class CapturingHdlcSink : public dlms::profile::IHdlcProfileTraceSink
{
public:
  std::vector<dlms::profile::HdlcProfileTraceEvent> events;
  void OnHdlcProfileTrace(const dlms::profile::HdlcProfileTraceEvent& event)
  {
    events.push_back(event);
  }
};

TEST(TraceCorrelationField, HdlcChannelStampsWireWriteEvents)
{
  // We exercise just the WireWrite path: SendApdu builds an HDLC UI
  // frame and emits one WireWrite event. That is enough to pin the
  // SetCorrelation -> emitted event stamping contract without dragging
  // in a full session negotiation.
  dlms::transport::FakeByteStream stream;

  CapturingHdlcSink sink;
  dlms::profile::ApduChannelOptions options =
    dlms::profile::DefaultApduChannelOptions();
  options.hdlcProfileTraceSink = &sink;
  options.hdlcClientAddress = 0x10u;
  options.hdlcLogicalDeviceAddress = 0x01u;
  options.hdlcPhysicalDeviceAddress = 0x11u;
  options.hdlcUseSession = false;

  dlms::profile::HdlcProfileChannel channel(stream, options);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.Open());

  const std::uint8_t apduBytes[] = {0x60, 0x01};
  ProfileByteView view;
  view.data = apduBytes;
  view.size = sizeof(apduBytes);

  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.SendApdu(view));
  ASSERT_FALSE(sink.events.empty());
  EXPECT_EQ(sink.events.back().conversationId, kNoConversationId);

  const std::uint64_t id =
    MakeConversationId(0xDEADBEEFCAFEBA00ULL, 0x0A);
  channel.SetCorrelation(id);

  const std::size_t baseline = sink.events.size();
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.SendApdu(view));
  ASSERT_GT(sink.events.size(), baseline);
  for (std::size_t i = baseline; i < sink.events.size(); ++i) {
    EXPECT_EQ(sink.events[i].conversationId, id);
  }
}

} // namespace
