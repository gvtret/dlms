#pragma once

#include "dlms/llc/llc_error.hpp"

#include <cstdint>

namespace dlms {
namespace llc {

/**
 * @brief DLMS/COSEM LLC data direction for standard client/server traffic.
 */
enum class LlcDirection
{
  /// Client request sent to a server.
  ClientToServer,
  /// Server response sent to a client.
  ServerToClient
};

/**
 * @brief Three-byte DLMS/COSEM LLC header.
 */
struct LlcHeader
{
  /// Destination LLC service access point.
  std::uint8_t dsap;
  /// Source LLC service access point.
  std::uint8_t ssap;
  /// LLC quality/control byte. DLMS/COSEM HDLC profile uses 0x00 in v1.
  std::uint8_t control;
};

/**
 * @brief Build the standard DLMS/COSEM LLC header for a direction.
 *
 * @param direction Client/server transfer direction.
 * @return Standard LLC header for that direction.
 */
LlcHeader MakeLlcHeader(LlcDirection direction);

/**
 * @brief Check whether the header is one of the standard DLMS/COSEM headers.
 *
 * Broadcast Destination LSAP is not considered a standard header by this
 * helper. Use `ValidateLlcHeader` for policy-aware validation.
 *
 * @param header Header to check.
 * @return true for `E6 E6 00` or `E6 E7 00`.
 */
bool IsKnownDlmsLlcHeader(const LlcHeader& header);

/**
 * @brief Validate an LLC header using the current v1 policy.
 *
 * @param header Header to validate.
 * @param allowBroadcastDestination Whether Destination LSAP 0xFF is accepted.
 * @return Ok when the header is valid for the policy, otherwise a status code.
 */
LlcStatus ValidateLlcHeader(
  const LlcHeader& header,
  bool allowBroadcastDestination);

} // namespace llc
} // namespace dlms
