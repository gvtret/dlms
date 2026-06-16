#include "dlms/wrapper/wrapper_error.hpp"

namespace dlms {
namespace wrapper {

const char* WrapperStatusName(WrapperStatus status)
{
  switch (status) {
    case WrapperStatus::Ok:
      return "Ok";
    case WrapperStatus::NeedMoreData:
      return "NeedMoreData";
    case WrapperStatus::OutputBufferTooSmall:
      return "OutputBufferTooSmall";
    case WrapperStatus::InvalidArgument:
      return "InvalidArgument";
    case WrapperStatus::InvalidVersion:
      return "InvalidVersion";
    case WrapperStatus::InvalidLength:
      return "InvalidLength";
    case WrapperStatus::InvalidSourcePort:
      return "InvalidSourcePort";
    case WrapperStatus::InvalidDestinationPort:
      return "InvalidDestinationPort";
    case WrapperStatus::DataTooLarge:
      return "DataTooLarge";
    case WrapperStatus::FrameTooLarge:
      return "FrameTooLarge";
    case WrapperStatus::UnsupportedFeature:
      return "UnsupportedFeature";
    case WrapperStatus::InternalError:
      return "InternalError";
  }

  return "Unknown";
}

} // namespace wrapper
} // namespace dlms
