#include "dlms/profile/profile_types.hpp"
#include "dlms/transport/transport_status.hpp"

int main()
{
  const dlms::profile::ApduChannelOptions options =
    dlms::profile::DefaultApduChannelOptions();

  return options.maximumApduSize > 0 &&
      dlms::transport::ToString(dlms::transport::TransportStatus::Ok)[0] == 'O'
    ? 0
    : 1;
}
