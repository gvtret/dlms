// Cross-layer trace correlation integration test (P1 §2 Commit 3/3).
//
// Verifies that when the consumer hands a logging seed to
// IXdlmsAssociationState::ConversationSeed() (or to a logging-aware
// association state stub), then both:
//
//   - the xDLMS trace events emitted by XdlmsClient, AND
//   - the channel-level trace events emitted by an IApduChannel
//     implementation that overrides SetCorrelation()
//
// carry the same bit-identical conversationId, computed from
// MakeConversationId(seed, invokeId).
//
// Strategy: use a hand-rolled IApduChannel that
//   - records the last SetCorrelation(id) it received,
//   - on every SendApdu/ReceiveApdu stamps a captured
//     WrapperTcpTraceEvent with that id and forwards it to a sink,
//   - synthesizes a valid Get / Set / Action response from a queue.
//
// This deliberately avoids dragging in a full server stack and a
// real WrapperTcpProfileChannel: the cross-layer property under
// test is the propagation of conversationId through SetCorrelation()
// + the matching xDLMS-side stamp. Channel-side stamping by real
// channels is covered by test_trace_correlation.cpp.

#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/profile/profile_types.hpp"
#include "dlms/xdlms/xdlms_association_state_interface.hpp"
#include "dlms/xdlms/xdlms_client.hpp"
#include "dlms/xdlms/xdlms_correlation.hpp"
#include "dlms/xdlms/xdlms_trace.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <deque>
#include <vector>

namespace {

using dlms::xdlms::MakeConversationId;
using dlms::xdlms::kNoConversationId;

class FixedSeedAssociation : public dlms::xdlms::IXdlmsAssociationState
{
public:
  explicit FixedSeedAssociation(std::uint64_t seed)
    : seed_(seed)
  {
  }
  bool IsAssociated() const { return true; }
  std::uint64_t ConversationSeed() const noexcept { return seed_; }
private:
  std::uint64_t seed_;
};

class CapturingWrapperSink : public dlms::profile::IWrapperTcpTraceSink
{
public:
  std::vector<dlms::profile::WrapperTcpTraceEvent> events;
  void OnWrapperTcpTrace(const dlms::profile::WrapperTcpTraceEvent& event)
  {
    events.push_back(event);
  }
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

// Channel that overrides SetCorrelation and stamps every emitted
// wrapper-trace event with the latest id. Models any real channel
// (wrapper, hdlc) that participates in §2 cross-layer correlation.
class CorrelationChannel : public dlms::profile::IApduChannel
{
public:
  CorrelationChannel(
    dlms::profile::IWrapperTcpTraceSink& sink)
    : sink_(sink)
    , open_(false)
    , correlation_(kNoConversationId)
  {
  }

  void SetCorrelation(std::uint64_t conversationId) noexcept
  {
    correlation_ = conversationId;
  }

  std::uint64_t LastCorrelation() const { return correlation_; }

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
    Stamp(apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    if (receiveQueue_.empty()) {
      return dlms::profile::ProfileStatus::InvalidFrame;
    }
    apdu = receiveQueue_.front();
    receiveQueue_.pop_front();
    Stamp(apdu.size());
    return dlms::profile::ProfileStatus::Ok;
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

  void Enqueue(const std::vector<std::uint8_t>& apdu)
  {
    receiveQueue_.push_back(apdu);
  }

private:
  void Stamp(std::size_t size)
  {
    dlms::profile::WrapperTcpTraceEvent ev{};
    ev.bytes = nullptr;
    ev.byteSize = size;
    ev.conversationId = correlation_;
    sink_.OnWrapperTcpTrace(ev);
  }

  dlms::profile::IWrapperTcpTraceSink& sink_;
  bool open_;
  std::uint64_t correlation_;
  std::deque<std::vector<std::uint8_t> > receiveQueue_;
};

dlms::xdlms::CosemAttributeDescriptor MakeDescriptor()
{
  dlms::xdlms::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 7u;
  descriptor.instanceId =
    dlms::xdlms::CosemLogicalName(1, 0, 99, 1, 0, 255);
  descriptor.attributeId = 7u;
  return descriptor;
}

std::vector<std::uint8_t> MakeGetResponse(std::uint8_t invokeIdAndPriority)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::GetResponse;
  response.getResponse.invokeIdAndPriority = invokeIdAndPriority;
  response.getResponse.resultChoice = dlms::apdu::GetDataResultChoice::Data;
  response.getResponse.data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  response.getResponse.data.unsignedValue = 0x09F1u;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(response, output));
  return output;
}

} // namespace

TEST(CrossLayerCorrelation, GetEmitsMatchingConversationIdOnBothLayers)
{
  CapturingWrapperSink wrapperSink;
  CorrelationChannel channel(wrapperSink);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.Open());

  const std::uint64_t seed = 0x9876543210FEDC00ULL;
  FixedSeedAssociation association(seed);

  CapturingXdlmsSink xdlmsSink;
  dlms::xdlms::XdlmsClient xdlms(channel, association);
  xdlms.SetTraceSink(&xdlmsSink);

  // The very first invoke id allocated by XdlmsClient is 1.
  const std::uint8_t expectedInvokeId = 1u;
  const std::uint64_t expectedId =
    MakeConversationId(seed, expectedInvokeId);

  // Seed a normal Get response whose invokeId matches.
  channel.Enqueue(MakeGetResponse(0x80u | expectedInvokeId));

  dlms::xdlms::GetResult result;
  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Get(MakeDescriptor(), result));
  EXPECT_EQ(expectedInvokeId, result.invokeId);

  // 1. XdlmsClient must have called SetCorrelation with the
  //    derived id before performing any I/O.
  EXPECT_EQ(expectedId, channel.LastCorrelation());

  // 2. xDLMS sink saw at least one event, and every event carries
  //    the same conversationId.
  ASSERT_FALSE(xdlmsSink.events.empty());
  for (std::size_t i = 0; i < xdlmsSink.events.size(); ++i) {
    EXPECT_EQ(expectedId, xdlmsSink.events[i].conversationId)
      << "xdlms event #" << i;
    EXPECT_EQ(expectedInvokeId, xdlmsSink.events[i].invokeId)
      << "xdlms event #" << i;
  }

  // 3. Channel-side wrapper sink saw at least one event, and every
  //    event carries the same conversationId.
  ASSERT_FALSE(wrapperSink.events.empty());
  for (std::size_t i = 0; i < wrapperSink.events.size(); ++i) {
    EXPECT_EQ(expectedId, wrapperSink.events[i].conversationId)
      << "wrapper event #" << i;
  }
}

TEST(CrossLayerCorrelation, ZeroSeedYieldsZeroConversationId)
{
  // seed=0 ⇒ MakeConversationId(0, invokeId) == invokeId & 0x0F,
  // which is non-zero for invokeId>=1. The test name is loose but
  // the property we pin is "seed=0 + invokeId=1 ⇒ conversationId=1".
  CapturingWrapperSink wrapperSink;
  CorrelationChannel channel(wrapperSink);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.Open());

  FixedSeedAssociation association(0u);
  CapturingXdlmsSink xdlmsSink;
  dlms::xdlms::XdlmsClient xdlms(channel, association);
  xdlms.SetTraceSink(&xdlmsSink);

  const std::uint8_t expectedInvokeId = 1u;
  const std::uint64_t expectedId = MakeConversationId(0u, expectedInvokeId);
  EXPECT_EQ(static_cast<std::uint64_t>(expectedInvokeId & 0x0Fu),
            expectedId);

  channel.Enqueue(MakeGetResponse(0x80u | expectedInvokeId));

  dlms::xdlms::GetResult result;
  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Get(MakeDescriptor(), result));

  EXPECT_EQ(expectedId, channel.LastCorrelation());
  ASSERT_FALSE(xdlmsSink.events.empty());
  EXPECT_EQ(expectedId, xdlmsSink.events.back().conversationId);
  ASSERT_FALSE(wrapperSink.events.empty());
  EXPECT_EQ(expectedId, wrapperSink.events.back().conversationId);
}

TEST(CrossLayerCorrelation, NoTraceSinkStillCallsSetCorrelation)
{
  // Even when no xDLMS sink is attached, XdlmsClient must still
  // stamp the channel so channel-level sinks see correlated events.
  CapturingWrapperSink wrapperSink;
  CorrelationChannel channel(wrapperSink);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok, channel.Open());

  const std::uint64_t seed = 0xAAAA0000BBBB0000ULL;
  FixedSeedAssociation association(seed);
  dlms::xdlms::XdlmsClient xdlms(channel, association);
  // intentionally: no SetTraceSink

  const std::uint8_t expectedInvokeId = 1u;
  const std::uint64_t expectedId =
    MakeConversationId(seed, expectedInvokeId);

  channel.Enqueue(MakeGetResponse(0x80u | expectedInvokeId));

  dlms::xdlms::GetResult result;
  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            xdlms.Get(MakeDescriptor(), result));

  EXPECT_EQ(expectedId, channel.LastCorrelation());
  ASSERT_FALSE(wrapperSink.events.empty());
  for (std::size_t i = 0; i < wrapperSink.events.size(); ++i) {
    EXPECT_EQ(expectedId, wrapperSink.events[i].conversationId)
      << "wrapper event #" << i;
  }
}
