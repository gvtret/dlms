#pragma once

#include "dlms/xdlms/xdlms_status.hpp"
#include "dlms/xdlms/xdlms_security_processor_interface.hpp"
#include "dlms/xdlms/xdlms_trace.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dlms {
namespace profile {
class IApduChannel;
}
namespace xdlms {
class IXdlmsAssociationState;
}
}

namespace dlms {
namespace security {
class CipheredApduProcessor;
}
namespace xdlms {

struct GetIndication
{
  std::uint8_t invokeId;
  ServiceOptions options;
  CosemAttributeDescriptor descriptor;
};

struct SetIndication
{
  std::uint8_t invokeId;
  ServiceOptions options;
  CosemAttributeDescriptor descriptor;
  std::vector<std::uint8_t> data;
};

struct ActionIndication
{
  std::uint8_t invokeId;
  ServiceOptions options;
  CosemMethodDescriptor descriptor;
  bool hasParameter;
  std::vector<std::uint8_t> parameter;
};

struct ActionRequestBlockState
{
  bool active;
  std::uint8_t invokeId;
  ServiceOptions options;
  CosemMethodDescriptor descriptor;
  std::uint32_t nextBlockNumber;
  std::vector<std::uint8_t> data;
};

struct GetResponseBlockState
{
  bool active;
  std::uint8_t invokeId;
  ServiceOptions options;
  std::uint32_t nextBlockNumber;
  std::size_t offset;
  std::vector<std::uint8_t> data;
};

struct SetRequestBlockState
{
  bool active;
  std::uint8_t invokeId;
  ServiceOptions options;
  CosemAttributeDescriptor descriptor;
  std::uint32_t nextBlockNumber;
  std::vector<std::uint8_t> data;
};

class IXdlmsServerHandler
{
public:
  virtual ~IXdlmsServerHandler();

  virtual XdlmsStatus HandleGet(
    const GetIndication& indication,
    GetResult& result) = 0;

  virtual XdlmsStatus HandleSet(
    const SetIndication& indication,
    SetResult& result);

  virtual XdlmsStatus HandleAction(
    const ActionIndication& indication,
    ActionResult& result);
};

class IXdlmsServerDispatcher
{
public:
  virtual ~IXdlmsServerDispatcher();

  virtual XdlmsStatus DispatchGet(
    const GetIndication& indication,
    GetResult& result) = 0;

  virtual XdlmsStatus DispatchSet(
    const SetIndication& indication,
    SetResult& result) = 0;

  virtual XdlmsStatus DispatchAction(
    const ActionIndication& indication,
    ActionResult& result) = 0;
};

class XdlmsServerDispatcher : public IXdlmsServerDispatcher
{
public:
  explicit XdlmsServerDispatcher(IXdlmsServerHandler& handler);

  XdlmsStatus DispatchGet(
    const GetIndication& indication,
    GetResult& result);

  XdlmsStatus DispatchSet(
    const SetIndication& indication,
    SetResult& result);

  XdlmsStatus DispatchAction(
    const ActionIndication& indication,
    ActionResult& result);

private:
  IXdlmsServerHandler& handler_;
};

class XdlmsServerApduProcessor
{
public:
  explicit XdlmsServerApduProcessor(IXdlmsServerDispatcher& dispatcher);
  XdlmsServerApduProcessor(
    IXdlmsServerDispatcher& dispatcher,
    const ServiceOptions& options);
  XdlmsServerApduProcessor(
    IXdlmsServerDispatcher& dispatcher,
    IXdlmsSecurityProcessor& security);
  XdlmsServerApduProcessor(
    IXdlmsServerDispatcher& dispatcher,
    IXdlmsSecurityProcessor& security,
    const ServiceOptions& options);
  XdlmsServerApduProcessor(
    IXdlmsServerDispatcher& dispatcher,
    dlms::security::CipheredApduProcessor& security);
  XdlmsServerApduProcessor(
    IXdlmsServerDispatcher& dispatcher,
    dlms::security::CipheredApduProcessor& security,
    const ServiceOptions& options);

  XdlmsStatus ProcessRequest(
    const std::vector<std::uint8_t>& requestApdu,
    std::vector<std::uint8_t>& responseApdu);

  void SetTraceSink(IXdlmsTraceSink* sink) noexcept { traceSink_ = sink; }
  IXdlmsTraceSink* TraceSink() const noexcept { return traceSink_; }

  // Optional: pin the inbound APDU channel and the conversation seed
  // source so that ProcessRequest can publish a correlation id matching
  // the one the client used. After successful decode of an APDU, the
  // processor computes `MakeConversationId(seed, invokeId)` and calls
  // `channel->SetCorrelation(...)` so that any trace events emitted on
  // the outbound response (SendApdu) carry the same id. The same id is
  // stamped on every server-side `IXdlmsTraceSink` event. Both setters
  // accept `nullptr` to clear (default state). ABI-safe append.
  void SetApduChannel(dlms::profile::IApduChannel* channel) noexcept { channel_ = channel; }
  dlms::profile::IApduChannel* ApduChannel() const noexcept { return channel_; }
  void SetConversationSeedSource(const IXdlmsAssociationState* state) noexcept { seedSource_ = state; }
  const IXdlmsAssociationState* ConversationSeedSource() const noexcept { return seedSource_; }

private:
  IXdlmsServerDispatcher& dispatcher_;
  std::unique_ptr<IXdlmsSecurityProcessor> ownedSecurity_;
  IXdlmsSecurityProcessor* security_;
  ServiceOptions options_;
  GetResponseBlockState getBlocks_;
  SetRequestBlockState setBlocks_;
  ActionRequestBlockState actionBlocks_;
  IXdlmsTraceSink* traceSink_;
  dlms::profile::IApduChannel* channel_;
  const IXdlmsAssociationState* seedSource_;
};

GetIndication EmptyGetIndication();
SetIndication EmptySetIndication();
ActionIndication EmptyActionIndication();
GetResponseBlockState EmptyGetResponseBlockState();
ActionRequestBlockState EmptyActionRequestBlockState();
SetRequestBlockState EmptySetRequestBlockState();
XdlmsStatus ValidateInvokeId(std::uint8_t invokeId);

} // namespace xdlms
} // namespace dlms
