#include "dlms/association/association_types.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/profile/profile_types.hpp"
#include "dlms/transport/transport_trace.hpp"
#include "dlms/xdlms/xdlms_correlation.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

using dlms::xdlms::MakeConversationId;
using dlms::xdlms::kNoConversationId;

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

} // namespace
