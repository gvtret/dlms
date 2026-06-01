#pragma once

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace llc {

/// Standard DLMS/COSEM Destination LSAP for LLC frames carried over HDLC.
static const std::uint8_t kLlcDsapDlms = 0xE6;
/// Standard Source LSAP for client-to-server DLMS/COSEM LLC frames.
static const std::uint8_t kLlcSsapClient = 0xE6;
/// Standard Source LSAP for server-to-client DLMS/COSEM LLC frames.
static const std::uint8_t kLlcSsapServer = 0xE7;
/// Broadcast Destination LSAP accepted only by explicit decode policy.
static const std::uint8_t kLlcDsapBroadcast = 0xFF;
/// DLMS/COSEM LLC control byte supported by this v1 implementation.
static const std::uint8_t kLlcControlDefault = 0x00;
/// Number of octets in the LLC header: DSAP, SSAP, and control.
static const std::size_t kLlcHeaderSize = 3;

} // namespace llc
} // namespace dlms
