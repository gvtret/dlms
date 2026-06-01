#include "dlms/cosem/cosem_status.hpp"

namespace dlms {
namespace cosem {

const char* CosemStatusName(CosemStatus status)
{
  switch (status) {
  case CosemStatus::Ok:
    return "Ok";
  case CosemStatus::InvalidArgument:
    return "InvalidArgument";
  case CosemStatus::DuplicateObject:
    return "DuplicateObject";
  case CosemStatus::ObjectNotFound:
    return "ObjectNotFound";
  case CosemStatus::AttributeNotFound:
    return "AttributeNotFound";
  case CosemStatus::MethodNotFound:
    return "MethodNotFound";
  case CosemStatus::AccessDenied:
    return "AccessDenied";
  case CosemStatus::OutputBufferTooSmall:
    return "OutputBufferTooSmall";
  case CosemStatus::UnsupportedFeature:
    return "UnsupportedFeature";
  case CosemStatus::ObjectError:
    return "ObjectError";
  case CosemStatus::InternalError:
    return "InternalError";
  }

  return "Unknown";
}

} // namespace cosem
} // namespace dlms
