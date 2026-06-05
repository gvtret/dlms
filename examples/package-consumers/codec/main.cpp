#include "dlms/apdu/apdu_types.hpp"
#include "dlms/hdlc/hdlc_address.hpp"
#include "dlms/llc/llc_header.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
  const dlms::apdu::ByteView empty = {
    static_cast<const std::uint8_t*>(0),
    static_cast<std::size_t>(0)
  };

  if (empty.data != 0 || empty.size != 0) {
    return 1;
  }

  dlms::hdlc::HdlcAddress clientAddress;
  const dlms::hdlc::HdlcStatus status =
    dlms::hdlc::DlmsHdlcAddress::MakeClientAddress(16, clientAddress);

  const dlms::llc::LlcHeader header =
    dlms::llc::MakeLlcHeader(dlms::llc::LlcDirection::ClientToServer);

  return status == dlms::hdlc::HdlcStatus::Ok &&
      dlms::llc::IsKnownDlmsLlcHeader(header)
    ? 0
    : 1;
}
