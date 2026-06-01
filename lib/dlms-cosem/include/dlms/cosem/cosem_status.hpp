#pragma once

namespace dlms {
namespace cosem {

enum class CosemStatus
{
  Ok,
  InvalidArgument,
  DuplicateObject,
  ObjectNotFound,
  AttributeNotFound,
  MethodNotFound,
  AccessDenied,
  OutputBufferTooSmall,
  UnsupportedFeature,
  ObjectError,
  InternalError
};

const char* CosemStatusName(CosemStatus status);

} // namespace cosem
} // namespace dlms
