#include "dlms/client/client.hpp"
#include "dlms/client/client_data.hpp"

#include "dlms/association/association_client_interface.hpp"

#include <cstdint>
#include <vector>

namespace {

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
  dlms::profile::ProfileStatus Open()
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return true;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView)
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>&)
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
};

class ExampleAssociation : public dlms::association::IAssociationClient
{
public:
  ExampleAssociation()
    : state_(dlms::association::AssociationState::Closed)
    , result_(dlms::association::EmptyAssociationResult())
  {
  }

  dlms::association::AssociationStatus Open()
  {
    state_ = dlms::association::AssociationState::Open;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus Close()
  {
    state_ = dlms::association::AssociationState::Closed;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus Establish()
  {
    state_ = dlms::association::AssociationState::Associated;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus Release()
  {
    state_ = dlms::association::AssociationState::Closed;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationState State() const
  {
    return state_;
  }

  bool IsAssociated() const
  {
    return state_ == dlms::association::AssociationState::Associated;
  }

  const dlms::association::AssociationResult& Result() const
  {
    return result_;
  }

private:
  dlms::association::AssociationState state_;
  dlms::association::AssociationResult result_;
};

class ExampleXdlmsService : public dlms::client::IClientXdlmsService
{
public:
  dlms::xdlms::XdlmsStatus Get(
    const dlms::client::CosemAttributeDescriptor& descriptor,
    dlms::xdlms::GetResult& result)
  {
    result = dlms::xdlms::EmptyGetResult();
    if (descriptor.classId != 3u || descriptor.attributeId != 2u) {
      result.invokeId = 1u;
      result.hasAccessResult = true;
      result.accessResult = 3u;
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    result.invokeId = 1u;
    result.hasData = true;
    result.hasAccessResult = true;
    result.accessResult = 0u;
    dlms::client::EncodeDlmsLongUnsigned(2300u, result.data);
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  dlms::xdlms::XdlmsStatus Set(
    const dlms::client::CosemAttributeDescriptor&,
    const std::vector<std::uint8_t>&,
    dlms::xdlms::SetResult& result)
  {
    result = dlms::xdlms::EmptySetResult();
    return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
  }

  dlms::xdlms::XdlmsStatus Action(
    const dlms::client::CosemMethodDescriptor&,
    bool,
    const std::vector<std::uint8_t>&,
    dlms::xdlms::ActionResult& result)
  {
    result = dlms::xdlms::EmptyActionResult();
    return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
  }
};

} // namespace

int main()
{
  ExampleApduChannel channel;
  ExampleAssociation association;
  ExampleXdlmsService xdlms;
  dlms::client::DlmsClient client(channel, association, xdlms);

  if (client.Connect() != dlms::client::ClientStatus::Ok) {
    return 1;
  }
  if (client.OpenAssociation() != dlms::client::ClientStatus::Ok) {
    return 1;
  }

  dlms::client::ClientGetResult result;
  const dlms::xdlms::CosemLogicalName activeEnergy(1, 0, 1, 8, 0, 255);
  if (client.ReadAttribute(3u, activeEnergy, 2u, result) !=
      dlms::client::ClientStatus::Ok) {
    return 1;
  }
  if (result.hasAccessResult && result.accessResult != 0u) {
    return 1;
  }
  if (!result.hasData) {
    return 1;
  }

  std::uint16_t value = 0u;
  if (dlms::client::DecodeDlmsLongUnsigned(result.encodedData, value) !=
      dlms::client::ClientStatus::Ok) {
    return 1;
  }
  if (value != 2300u) {
    return 1;
  }

  if (client.ReleaseAssociation() != dlms::client::ClientStatus::Ok) {
    return 1;
  }
  return client.Close() == dlms::client::ClientStatus::Ok ? 0 : 1;
}
