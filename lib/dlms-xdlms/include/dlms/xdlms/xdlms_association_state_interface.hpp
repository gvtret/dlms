#pragma once

#include <cstdint>

namespace dlms {
namespace xdlms {

class IXdlmsAssociationState
{
public:
  virtual ~IXdlmsAssociationState();

  virtual bool IsAssociated() const = 0;

  // Opaque, non-secret 64-bit logging seed used to construct
  // cross-layer trace correlation ids (P1 §2). Default 0 means
  // "no association context"; consumers receive
  // kNoConversationId == 0 in that case. Never derived from
  // keys / system titles / HLS challenges.
  virtual std::uint64_t ConversationSeed() const noexcept { return 0; }
};

} // namespace xdlms
} // namespace dlms
