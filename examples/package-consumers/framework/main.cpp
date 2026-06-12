#include "dlms/apdu/apdu_types.hpp"
#include "dlms/client/client_xdlms_service_interface.hpp"
#include "dlms/cosem/logical_device_interface.hpp"
#include "dlms/endpoint/apdu_channel_listener.hpp"
#include "dlms/endpoint/gateway_interfaces.hpp"
#include "dlms/server/server_service_interface.hpp"
#include "dlms/transport/transport_status.hpp"
#include "dlms/xdlms/xdlms_association_state_interface.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

int main()
{
  static_assert(std::is_polymorphic<dlms::client::IClientXdlmsService>::value,
    "client xDLMS service interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::cosem::ILogicalDevice>::value,
    "COSEM logical-device interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::endpoint::IApduChannelListener>::value,
    "endpoint APDU listener interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::endpoint::IGatewayPolicy>::value,
    "endpoint gateway policy interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::server::IServerService>::value,
    "server service interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::xdlms::IXdlmsAssociationState>::value,
    "xDLMS association-state interface must be polymorphic");

  const dlms::apdu::ByteView empty = {
    static_cast<const std::uint8_t*>(0),
    static_cast<std::size_t>(0)
  };

  return empty.data == 0 && empty.size == 0 &&
      dlms::transport::ToString(dlms::transport::TransportStatus::Ok)[0] == 'O'
    ? 0
    : 1;
}
