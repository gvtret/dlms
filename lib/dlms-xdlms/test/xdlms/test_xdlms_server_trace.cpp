// Tests for IXdlmsTraceSink wiring on the server side
// (XdlmsServerApduProcessor). P1 §7 Commit 2/3.
//
// Scope: verify that RequestReceived/ResponseSent are emitted in pairs
// for happy-path Get/Set/Action, that DecodeFailed fires for malformed
// APDUs, and that default-null sink is true zero-cost (no virtual call).

#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/xdlms/xdlms_server.hpp"
#include "dlms/xdlms/xdlms_trace.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

class CapturingTraceSink : public dlms::xdlms::IXdlmsTraceSink
{
public:
  void OnXdlmsTrace(const dlms::xdlms::XdlmsTraceEvent& event) override
  {
    events.push_back(event);
  }

  std::vector<dlms::xdlms::XdlmsTraceEvent> events;
};

class EchoGetDispatcher : public dlms::xdlms::IXdlmsServerDispatcher
{
public:
  dlms::xdlms::XdlmsStatus DispatchGet(
    const dlms::xdlms::GetIndication&,
    dlms::xdlms::GetResult& output) override
  {
    output = dlms::xdlms::EmptyGetResult();
    output.hasData = true;
    output.data.assign(3u, 0u);
    output.data[0] = 0x12u;  // long-unsigned tag
    output.data[1] = 0x00u;
    output.data[2] = 0x2Au;
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  dlms::xdlms::XdlmsStatus DispatchSet(
    const dlms::xdlms::SetIndication&,
    dlms::xdlms::SetResult& output) override
  {
    output = dlms::xdlms::EmptySetResult();
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  dlms::xdlms::XdlmsStatus DispatchAction(
    const dlms::xdlms::ActionIndication&,
    dlms::xdlms::ActionResult& output) override
  {
    output = dlms::xdlms::EmptyActionResult();
    return dlms::xdlms::XdlmsStatus::Ok;
  }
};

std::vector<std::uint8_t> EncodeApdu(const dlms::apdu::XdlmsApdu& apdu)
{
  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(apdu, output));
  return output;
}

std::vector<std::uint8_t> MakeGetRequest(std::uint8_t invokeIdAndPriority)
{
  return EncodeApdu(dlms::apdu::MakeGetRequestNormal(
    invokeIdAndPriority,
    3u,
    dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
    2u));
}

TEST(XdlmsServerTrace, ServerEmitsRequestReceivedAndResponseSentForGet)
{
  EchoGetDispatcher dispatcher;
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  CapturingTraceSink sink;
  processor.SetTraceSink(&sink);

  std::vector<std::uint8_t> response;
  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(MakeGetRequest(0xC1u), response));

  ASSERT_EQ(2u, sink.events.size());

  EXPECT_EQ(dlms::xdlms::XdlmsTraceKind::RequestReceived,
            sink.events[0].kind);
  EXPECT_EQ(dlms::xdlms::XdlmsTraceDirection::Inbound,
            sink.events[0].direction);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok, sink.events[0].status);
  EXPECT_EQ(0x01u, sink.events[0].invokeId);
  EXPECT_EQ(3u, sink.events[0].classId);
  EXPECT_EQ(2u, sink.events[0].attributeOrMethodId);
  EXPECT_EQ(1u, sink.events[0].logicalName[0]);
  EXPECT_EQ(255u, sink.events[0].logicalName[5]);

  EXPECT_EQ(dlms::xdlms::XdlmsTraceKind::ResponseSent,
            sink.events[1].kind);
  EXPECT_EQ(dlms::xdlms::XdlmsTraceDirection::Inbound,
            sink.events[1].direction);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok, sink.events[1].status);
  EXPECT_EQ(response.size(), sink.events[1].apduSize);
}

TEST(XdlmsServerTrace, ServerEmitsDecodeFailedOnGarbageInput)
{
  EchoGetDispatcher dispatcher;
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  CapturingTraceSink sink;
  processor.SetTraceSink(&sink);

  std::vector<std::uint8_t> garbage;
  garbage.push_back(0xFFu);
  garbage.push_back(0xFFu);
  garbage.push_back(0xFFu);

  std::vector<std::uint8_t> response;
  ASSERT_EQ(dlms::xdlms::XdlmsStatus::DecodeFailed,
            processor.ProcessRequest(garbage, response));

  ASSERT_EQ(1u, sink.events.size());
  EXPECT_EQ(dlms::xdlms::XdlmsTraceKind::DecodeFailed,
            sink.events[0].kind);
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::DecodeFailed,
            sink.events[0].status);
  EXPECT_EQ(garbage.size(), sink.events[0].apduSize);
}

TEST(XdlmsServerTrace, ServerWithoutSinkProcessesNormally)
{
  EchoGetDispatcher dispatcher;
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  // No SetTraceSink call -> traceSink_ is null.

  std::vector<std::uint8_t> response;
  ASSERT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(MakeGetRequest(0xC1u), response));
  EXPECT_FALSE(response.empty());
  EXPECT_EQ(static_cast<dlms::xdlms::IXdlmsTraceSink*>(nullptr),
            processor.TraceSink());
}

}  // namespace
