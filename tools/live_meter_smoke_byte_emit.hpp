#ifndef DLMS_TOOLS_LIVE_METER_SMOKE_BYTE_EMIT_HPP
#define DLMS_TOOLS_LIVE_METER_SMOKE_BYTE_EMIT_HPP

// Wire-byte hex dump policy for the live meter smoke tool.
//
// Centralised here so the redaction policy is one piece of
// code that lives.cpp uses and that the redaction unit test
// can exercise without instantiating the full console sinks
// or touching std::cout.
//
// Default behaviour: skip the bytes payload entirely. The
// caller must set DLMS_LIVE_TRACE_WIRE_BYTES=1 in the
// environment to opt back into a raw hex dump. The intent is
// to keep HLS challenges, GMAC tags, and any ciphered or
// clear protected APDU payload off the operator console
// during a routine live smoke run.

#include "dlms/profile/profile_types.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ios>
#include <ostream>

namespace dlms_live_smoke {

inline bool WireBytesTraceEnabled()
{
  const char* value = std::getenv("DLMS_LIVE_TRACE_WIRE_BYTES");
  return value != 0 && std::strcmp(value, "1") == 0;
}

inline void WriteHexBytes(
  std::ostream& out,
  const std::uint8_t* bytes,
  std::size_t size)
{
  if (bytes == 0 || size == 0u) {
    return;
  }

  std::ios::fmtflags flags = out.flags();
  const char fill = out.fill();
  out << std::hex << std::setfill('0');
  for (std::size_t i = 0u; i < size; ++i) {
    if (i != 0u) {
      out << ' ';
    }
    out << std::setw(2) << static_cast<unsigned>(bytes[i]);
  }
  out.flags(flags);
  out.fill(fill);
}

inline void EmitWrapperBytesIfEnabled(
  std::ostream& out,
  const dlms::profile::WrapperTcpTraceEvent& event)
{
  const bool isWire =
    event.kind == dlms::profile::WrapperTcpTraceKind::WireWrite ||
    event.kind == dlms::profile::WrapperTcpTraceKind::WireRead;
  if (!isWire || event.bytes == 0 || event.byteSize == 0u) {
    return;
  }
  if (!WireBytesTraceEnabled()) {
    return;
  }
  out << " bytes=";
  WriteHexBytes(out, event.bytes, event.byteSize);
}

inline void EmitHdlcBytesIfEnabled(
  std::ostream& out,
  const dlms::profile::HdlcProfileTraceEvent& event)
{
  const bool isWire =
    event.kind == dlms::profile::HdlcProfileTraceKind::WireWrite ||
    event.kind == dlms::profile::HdlcProfileTraceKind::WireRead;
  if (!isWire || event.bytes == 0 || event.byteSize == 0u) {
    return;
  }
  if (!WireBytesTraceEnabled()) {
    return;
  }
  out << " bytes=";
  WriteHexBytes(out, event.bytes, event.byteSize);
}

}  // namespace dlms_live_smoke

#endif  // DLMS_TOOLS_LIVE_METER_SMOKE_BYTE_EMIT_HPP
