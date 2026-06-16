#pragma once

#include "dlms/profile/profile_types.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace profile {

class IApduChannel
{
public:
  virtual ~IApduChannel() {}

  virtual ProfileStatus Open() = 0;
  virtual ProfileStatus Close() = 0;
  virtual bool IsOpen() const = 0;

  virtual ProfileStatus SendApdu(ProfileByteView apdu) = 0;
  virtual ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu) = 0;
  virtual ProfileStatus ReceiveApdu(ProfileMutableBuffer output) = 0;

  // Optional: set the diagnostic correlation id stamped on every trace
  // event emitted by this channel until the next call. Pass 0 to clear.
  // Default no-op — channels without trace sinks need not implement.
  // See docs/trace_correlation_design.md.
  virtual void SetCorrelation(std::uint64_t /*conversationId*/) noexcept {}

  // Optional: read the correlation id currently stamped on this channel.
  // Returns 0 when no correlation has been set (or for channels that do
  // not participate in trace correlation). Used by server-side trace
  // sinks to publish the conversation id seeded by the xDLMS processor
  // after it has decoded the inbound invoke id. ABI-safe append (new
  // virtual at the end of the interface). See
  // docs/trace_correlation_design.md.
  virtual std::uint64_t CurrentConversationId() const noexcept { return 0u; }
};

} // namespace profile
} // namespace dlms

