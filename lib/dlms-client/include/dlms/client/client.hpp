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

struct ClientGetResult
{
  ClientStatus status;
  std::uint8_t invokeId;
  bool hasData;
  std::vector<std::uint8_t> encodedData;
  bool hasAccessResult;
  std::uint8_t accessResult;
};

struct ClientSetResult
{
  ClientStatus status;
  std::uint8_t invokeId;
  std::uint8_t accessResult;
};

struct ClientActionResult
{
  ClientStatus status;
  std::uint8_t invokeId;
  std::uint8_t actionResult;
  bool hasData;
  std::vector<std::uint8_t> encodedReturnParameter;
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

  ClientStatus ReadAttribute(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t attributeId,
    ClientGetResult& result);

  ClientStatus ReadAttribute(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t attributeId,
    const SelectiveAccessDescriptor& selectiveAccess,
    ClientGetResult& result);

  ClientStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData);

  ClientStatus WriteAttribute(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t attributeId,
    const std::vector<std::uint8_t>& encodedData,
    ClientSetResult& result);

  ClientStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    std::vector<std::uint8_t>& encodedReturnParameter);

  ClientStatus CallMethod(
    std::uint16_t classId,
    const dlms::xdlms::CosemLogicalName& logicalName,
    std::uint8_t methodId,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    ClientActionResult& result);

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
