#include "dlms/association/association_client_interface.hpp"
#include "dlms/security/key_store.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

int main()
{
  const dlms::xdlms::ServiceOptions options =
    dlms::xdlms::DefaultServiceOptions();

  return options.confirmed && options.allowBlockTransfer ? 0 : 1;
}
