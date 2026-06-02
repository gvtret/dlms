#pragma once

#include "dlms/client/client_options.hpp"
#include "dlms/client/client_status.hpp"
#include "dlms/client/client_xdlms_service_interface.hpp"

#include "dlms/association/association_client_interface.hpp"
#include "dlms/profile/apdu_channel.hpp"

#include <memory>
#include <vector>

namespace dlms {
namespace security {
class CipheredApduProcessor;
}
namespace association {
class AssociationClient;
}
namespace xdlms {
class IXdlmsSecurityProcessor;
}
namespace client {

class DlmsClientOwnedState;

enum class ClientState
{
  Disconnected,
  Connected,
  Associated
};

class DlmsClient
{
public:
  explicit DlmsClient(const DlmsClientOptions& options);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association,
    IClientXdlmsService& xdlms);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association,
    IClientXdlmsService& xdlms);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association,
    dlms::security::CipheredApduProcessor& security);

  DlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association,
    dlms::xdlms::IXdlmsSecurityProcessor& security);

  ~DlmsClient();

  ClientStatus Connect();
  ClientStatus OpenAssociation();
  ClientStatus ReleaseAssociation();
  ClientStatus Close();

  ClientState State() const;
  bool IsConnected() const;
  bool IsAssociated() const;

  ClientStatus Get(
    const CosemAttributeDescriptor& descriptor,
    std::vector<std::uint8_t>& encodedData);

  ClientStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData);

  ClientStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter);

private:
  DlmsClient(const DlmsClient&);
  DlmsClient& operator=(const DlmsClient&);

  std::unique_ptr<DlmsClientOwnedState> owned_;
  dlms::profile::IApduChannel& channel_;
  dlms::association::IAssociationClient& association_;
  IClientXdlmsService* xdlms_;
  ClientState state_;
  ClientStatus constructionStatus_;
  bool hlsAuthentication_;
  bool ownsHdlcDataLinkSession_;
};

const char* ClientStateName(ClientState state);

} // namespace client
} // namespace dlms
