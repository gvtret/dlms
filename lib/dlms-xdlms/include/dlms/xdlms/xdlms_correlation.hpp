#pragma once

#include <cstdint>

namespace dlms {
namespace xdlms {

/**
 * @brief Build a non-secret 64-bit conversation id from an association
 *        logging seed and the current xDLMS invoke-id.
 *
 * The conversation id is a diagnostic correlator used to stitch trace
 * events from independent layers (transport, wrapper, hdlc, association)
 * into a single logical request. See `docs/trace_correlation_design.md`
 * for the full contract.
 *
 * Invariants:
 *   - The low 4 bits always equal `invokeId & 0x0F`, so a human reader
 *     can still spot the invoke-id directly in a hex dump of the id.
 *   - Two distinct `associationSeed` values always produce distinct
 *     conversation ids for the same invoke-id (the high 60 bits come
 *     entirely from the seed).
 *   - Two distinct invoke-ids on the same association always produce
 *     distinct conversation ids (the low 4 bits differ).
 *
 * `associationSeed` is a per-association logging salt with **no**
 * security role. It is not the system-title and not the HLS challenge.
 * The result is never placed on the wire.
 */
constexpr std::uint64_t MakeConversationId(
  std::uint64_t associationSeed,
  std::uint8_t invokeId) noexcept
{
  return (associationSeed & ~static_cast<std::uint64_t>(0x0F)) |
         (static_cast<std::uint64_t>(invokeId) & 0x0Fu);
}

/**
 * @brief Sentinel "no correlation context" conversation id.
 *
 * Trace events constructed before correlation is wired (or by code paths
 * that do not own an association) carry this value. Consumers should
 * treat it as "unknown" rather than as a real id.
 */
constexpr std::uint64_t kNoConversationId = 0;

} // namespace xdlms
} // namespace dlms
