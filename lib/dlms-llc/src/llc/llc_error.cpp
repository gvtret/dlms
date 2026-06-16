#include "dlms/llc/llc_error.hpp"

namespace dlms {
namespace llc {

const char* LlcStatusName(LlcStatus status)
{
  switch (status) {
    case LlcStatus::Ok:
      return "Ok";
    case LlcStatus::NeedMoreData:
      return "NeedMoreData";
    case LlcStatus::OutputBufferTooSmall:
      return "OutputBufferTooSmall";
    case LlcStatus::InvalidArgument:
      return "InvalidArgument";
    case LlcStatus::InvalidHeader:
      return "InvalidHeader";
    case LlcStatus::InvalidDsap:
      return "InvalidDsap";
    case LlcStatus::InvalidSsap:
      return "InvalidSsap";
    case LlcStatus::InvalidControl:
      return "InvalidControl";
    case LlcStatus::InvalidLpduLength:
      return "InvalidLpduLength";
    case LlcStatus::LsduTooLarge:
      return "LsduTooLarge";
    case LlcStatus::BroadcastEncodeForbidden:
      return "BroadcastEncodeForbidden";
    case LlcStatus::UnsupportedAddress:
      return "UnsupportedAddress";
    case LlcStatus::UnsupportedControl:
      return "UnsupportedControl";
    case LlcStatus::UnsupportedFeature:
      return "UnsupportedFeature";
    case LlcStatus::InternalError:
      return "InternalError";
  }

  return "Unknown";
}

} // namespace llc
} // namespace dlms
