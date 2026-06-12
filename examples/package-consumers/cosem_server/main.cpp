#include "dlms/cosem/logical_device_interface.hpp"
#include "dlms/server/server_service_interface.hpp"
#include "dlms/server/server_types.hpp"

#include <type_traits>

int main()
{
  static_assert(std::is_polymorphic<dlms::cosem::ILogicalDevice>::value,
    "COSEM logical-device interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::server::IServerService>::value,
    "server service interface must be polymorphic");

  const dlms::server::ServerAssociationContext context =
    dlms::server::EmptyServerAssociationContext();

  return !context.associated ? 0 : 1;
}
