#include "dlms/apdu/apdu_types.hpp"
#include "dlms/endpoint/endpoint.hpp"
#include "dlms/transport/transport_status.hpp"

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

  const char* statusName =
    dlms::transport::ToString(dlms::transport::TransportStatus::Ok);
  return statusName[0] == 'O' ? 0 : 1;
}
