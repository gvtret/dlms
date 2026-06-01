#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status codes returned by the C ABI.
 *
 * C callers must compare named constants, not numeric values.
 */
typedef enum dlms_llc_status_t
{
  /// Operation completed successfully.
  DLMS_LLC_STATUS_OK,
  /// More input bytes are required to decode a complete LPDU.
  DLMS_LLC_STATUS_NEED_MORE_DATA,
  /// Caller-provided output buffer is too small.
  DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL,
  /// A pointer, size, or argument combination is invalid.
  DLMS_LLC_STATUS_INVALID_ARGUMENT,
  /// The 3-byte LLC header is malformed.
  DLMS_LLC_STATUS_INVALID_HEADER,
  /// Destination LSAP is not accepted by the current policy.
  DLMS_LLC_STATUS_INVALID_DSAP,
  /// Source LSAP is not supported.
  DLMS_LLC_STATUS_INVALID_SSAP,
  /// LLC control byte is not supported.
  DLMS_LLC_STATUS_INVALID_CONTROL,
  /// LPDU length is invalid for the requested operation.
  DLMS_LLC_STATUS_INVALID_LPDU_LENGTH,
  /// LSDU exceeds configured or representable limits.
  DLMS_LLC_STATUS_LSDU_TOO_LARGE,
  /// Encoding broadcast Destination LSAP is forbidden.
  DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN,
  /// Address/SAP value is recognized as unsupported.
  DLMS_LLC_STATUS_UNSUPPORTED_ADDRESS,
  /// Control byte value is recognized as unsupported.
  DLMS_LLC_STATUS_UNSUPPORTED_CONTROL,
  /// Requested feature is outside the implemented profile.
  DLMS_LLC_STATUS_UNSUPPORTED_FEATURE,
  /// Internal error or allocation failure.
  DLMS_LLC_STATUS_INTERNAL_ERROR
} dlms_llc_status_t;

/**
 * @brief Three-byte DLMS/COSEM LLC header for the C ABI.
 */
typedef struct dlms_llc_header_t
{
  /// Destination LLC service access point.
  uint8_t dsap;
  /// Source LLC service access point.
  uint8_t ssap;
  /// LLC control byte. This implementation supports 0x00 for v1.
  uint8_t control;
} dlms_llc_header_t;

/**
 * @brief Non-owning decoded LPDU view for the C ABI.
 *
 * `lsdu` points into the input buffer passed to `dlms_llc_decode_lpdu`.
 */
typedef struct dlms_llc_lpdu_view_t
{
  /// Decoded LLC header.
  dlms_llc_header_t header;
  /// Non-owning pointer to LSDU bytes inside the decoder input buffer.
  const uint8_t* lsdu;
  /// Number of bytes available through `lsdu`.
  size_t lsdu_size;
} dlms_llc_lpdu_view_t;

/**
 * @brief Encode an LLC LPDU into a caller-provided output buffer.
 *
 * `lsdu` may be NULL only when `lsdu_size` is zero. Broadcast Destination LSAP
 * is rejected for encoding.
 */
dlms_llc_status_t dlms_llc_encode_lpdu(
  const dlms_llc_header_t* header,
  const uint8_t* lsdu,
  size_t lsdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);

/**
 * @brief Encode a client-to-server DLMS/COSEM APDU using header E6 E6 00.
 */
dlms_llc_status_t dlms_llc_encode_request(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);

/**
 * @brief Encode a server-to-client DLMS/COSEM APDU using header E6 E7 00.
 */
dlms_llc_status_t dlms_llc_encode_response(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);

/**
 * @brief Decode an LLC LPDU from a caller-provided input buffer.
 *
 * On success, `lpdu->lsdu` points into `input`. Destination LSAP 0xFF is
 * accepted only when `allow_broadcast_destination` is nonzero.
 */
dlms_llc_status_t dlms_llc_decode_lpdu(
  const uint8_t* input,
  size_t input_size,
  int allow_broadcast_destination,
  dlms_llc_lpdu_view_t* lpdu);

/**
 * @brief Validate an LLC header using the v1 DLMS/COSEM policy.
 *
 * Destination LSAP 0xFF is accepted only when
 * `allow_broadcast_destination` is nonzero.
 */
dlms_llc_status_t dlms_llc_validate_header(
  const dlms_llc_header_t* header,
  int allow_broadcast_destination);

#ifdef __cplusplus
} // extern "C"
#endif
