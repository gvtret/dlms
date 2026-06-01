#pragma once

#include "dlms/llc/llc_error.hpp"
#include "dlms/llc/llc_header.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dlms {
namespace llc {

/**
 * @brief Non-owning view of a decoded LLC LPDU.
 *
 * `lsduData` points into the input buffer passed to `DecodeLpduFromBuffer`.
 * The caller must keep that input buffer alive while using the view.
 */
struct LlcLpdu
{
  /// Decoded 3-byte LLC header.
  LlcHeader header;
  /// Non-owning pointer to LSDU bytes inside the decoder input buffer.
  const std::uint8_t* lsduData;
  /// Number of bytes available through `lsduData`.
  std::size_t lsduSize;
};

/**
 * @brief Owning decoded LLC LPDU container.
 */
struct LlcLpduBuffer
{
  /// Decoded 3-byte LLC header.
  LlcHeader header;
  /// Owned copy of the decoded LSDU bytes.
  std::vector<std::uint8_t> lsdu;
};

/**
 * @brief Encode an LLC LPDU into a caller-provided output buffer.
 *
 * `lsdu` may be null only when `lsduSize` is zero. `output` and `writtenSize`
 * must be valid. Broadcast destination LSAP is rejected for encoding.
 *
 * @param header LLC header to serialize.
 * @param lsdu LSDU/APDU bytes to append after the header.
 * @param lsduSize Number of bytes available through `lsdu`.
 * @param output Caller-provided destination buffer.
 * @param outputSize Number of bytes available through `output`.
 * @param writtenSize Set to the encoded LPDU size on success, or zero on
 * failure.
 * @return `LlcStatus::Ok` on success, otherwise a status describing the
 * validation or buffer failure.
 */
LlcStatus EncodeLpduToBuffer(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);

/**
 * @brief Encode an LLC LPDU into an owned byte vector.
 *
 * Clears `output` before encoding. On success, `output` contains the full LPDU.
 * On failure, `output` remains empty.
 *
 * @param header LLC header to serialize.
 * @param lsdu LSDU/APDU bytes to append after the header.
 * @param lsduSize Number of bytes available through `lsdu`.
 * @param output Owned destination buffer.
 * @return `LlcStatus::Ok` on success, otherwise a status describing the
 * validation, allocation, or buffer failure.
 */
LlcStatus EncodeLpdu(
  const LlcHeader& header,
  const std::uint8_t* lsdu,
  std::size_t lsduSize,
  std::vector<std::uint8_t>& output);

/**
 * @brief Decode an LLC LPDU from a caller-provided input buffer.
 *
 * The returned `lpdu` is a view into `input`. Broadcast destination LSAP is
 * accepted only when `allowBroadcastDestination` is true.
 *
 * @param input Encoded LPDU bytes.
 * @param inputSize Number of bytes available through `input`.
 * @param allowBroadcastDestination Whether Destination LSAP 0xFF is accepted.
 * @param lpdu Output view into `input`.
 * @return `LlcStatus::Ok` on success, `LlcStatus::NeedMoreData` when the
 * 3-byte LLC header is incomplete, otherwise a validation status.
 */
LlcStatus DecodeLpduFromBuffer(
  const std::uint8_t* input,
  std::size_t inputSize,
  bool allowBroadcastDestination,
  LlcLpdu& lpdu);

/**
 * @brief Decode an LLC LPDU into an owned container.
 *
 * Clears `lpdu.lsdu` before decoding. On success, `lpdu.lsdu` owns a copy of
 * the decoded LSDU bytes.
 *
 * @param input Encoded LPDU bytes.
 * @param inputSize Number of bytes available through `input`.
 * @param allowBroadcastDestination Whether Destination LSAP 0xFF is accepted.
 * @param lpdu Output LPDU container.
 * @return `LlcStatus::Ok` on success, otherwise a decode, validation, or
 * allocation status.
 */
LlcStatus DecodeLpdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  bool allowBroadcastDestination,
  LlcLpduBuffer& lpdu);

/**
 * @brief Encode a client-to-server DLMS/COSEM APDU as an LLC LPDU.
 *
 * Uses the standard `E6 E6 00` LLC header.
 *
 * @param apdu APDU bytes to place in the LSDU field.
 * @param apduSize Number of bytes available through `apdu`.
 * @param output Owned destination buffer for the encoded LPDU.
 * @return `LlcStatus::Ok` on success, otherwise an encode status.
 */
LlcStatus EncodeDlmsRequest(
  const std::uint8_t* apdu,
  std::size_t apduSize,
  std::vector<std::uint8_t>& output);

/**
 * @brief Encode a server-to-client DLMS/COSEM APDU as an LLC LPDU.
 *
 * Uses the standard `E6 E7 00` LLC header.
 *
 * @param apdu APDU bytes to place in the LSDU field.
 * @param apduSize Number of bytes available through `apdu`.
 * @param output Owned destination buffer for the encoded LPDU.
 * @return `LlcStatus::Ok` on success, otherwise an encode status.
 */
LlcStatus EncodeDlmsResponse(
  const std::uint8_t* apdu,
  std::size_t apduSize,
  std::vector<std::uint8_t>& output);

} // namespace llc
} // namespace dlms
