#pragma once

namespace dlms {
namespace llc {

/**
 * @brief Status codes returned by the LLC codec public API.
 *
 * Public/runtime API paths must return these status codes instead of throwing
 * exceptions. Callers must treat the enum names as the contract, not the
 * underlying integer representation.
 */
enum class LlcStatus
{
  /// Operation completed successfully.
  Ok,

  /// More input bytes are required to decode a complete LPDU.
  NeedMoreData,
  /// The caller-provided output buffer is too small.
  OutputBufferTooSmall,

  /// A pointer, size, or argument combination is invalid.
  InvalidArgument,
  /// The 3-byte LLC header is malformed.
  InvalidHeader,
  /// Destination LSAP is not accepted by the current policy.
  InvalidDsap,
  /// Source LSAP is not supported.
  InvalidSsap,
  /// LLC control byte is not supported.
  InvalidControl,
  /// LPDU length is invalid for the requested operation.
  InvalidLpduLength,

  /// LSDU exceeds configured or representable limits.
  LsduTooLarge,
  /// Encoding broadcast Destination LSAP is forbidden by the v1 API.
  BroadcastEncodeForbidden,

  /// Address/SAP value is recognized as unsupported.
  UnsupportedAddress,
  /// Control byte value is recognized as unsupported.
  UnsupportedControl,
  /// Requested feature is outside the implemented profile.
  UnsupportedFeature,

  /// Internal error or allocation failure in convenience APIs.
  InternalError
};

} // namespace llc
} // namespace dlms
