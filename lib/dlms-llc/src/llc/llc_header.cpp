#include "dlms/llc/llc_header.hpp"

#include "dlms/llc/llc_types.hpp"

namespace dlms {
namespace llc {

namespace {

bool IsKnownSourceLsap(std::uint8_t ssap)
{
  return ssap == kLlcSsapClient || ssap == kLlcSsapServer;
}

} // namespace

LlcHeader MakeLlcHeader(LlcDirection direction)
{
  LlcHeader header;
  header.dsap = kLlcDsapDlms;
  header.control = kLlcControlDefault;

  switch (direction) {
    case LlcDirection::ClientToServer:
      header.ssap = kLlcSsapClient;
      break;
    case LlcDirection::ServerToClient:
      header.ssap = kLlcSsapServer;
      break;
  }

  return header;
}

bool IsKnownDlmsLlcHeader(const LlcHeader& header)
{
  return header.dsap == kLlcDsapDlms &&
         header.control == kLlcControlDefault &&
         IsKnownSourceLsap(header.ssap);
}

LlcStatus ValidateLlcHeader(
  const LlcHeader& header,
  bool allowBroadcastDestination)
{
  if (header.control != kLlcControlDefault) {
    return LlcStatus::UnsupportedControl;
  }

  if (header.dsap == kLlcDsapBroadcast) {
    if (!allowBroadcastDestination) {
      return LlcStatus::BroadcastEncodeForbidden;
    }
  } else if (header.dsap != kLlcDsapDlms) {
    return LlcStatus::InvalidDsap;
  }

  if (!IsKnownSourceLsap(header.ssap)) {
    return LlcStatus::InvalidSsap;
  }

  return LlcStatus::Ok;
}

} // namespace llc
} // namespace dlms
