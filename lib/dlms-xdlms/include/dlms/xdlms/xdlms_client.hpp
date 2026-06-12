#pragma once

#include "dlms/association/association_client_interface.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/xdlms/xdlms_association_state_interface.hpp"
#include "dlms/xdlms/xdlms_security_processor_interface.hpp"
#include "dlms/xdlms/xdlms_types.hpp"

#include <memory>

namespace dlms {
namespace association {
class AssociationClient;
}
namespace security {
class CipheredApduProcessor;
}
namespace xdlms {

class XdlmsClient
{
public:
  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    IXdlmsAssociationState& association);

  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    IXdlmsAssociationState& association,
    IXdlmsSecurityProcessor& security);

  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association);

  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association);

  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association,
    IXdlmsSecurityProcessor& security);

  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::IAssociationClient& association,
    IXdlmsSecurityProcessor& security);

  XdlmsClient(
    dlms::profile::IApduChannel& channel,
    dlms::association::AssociationClient& association,
    dlms::security::CipheredApduProcessor& security);

  XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    GetResult& result);

  XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    const SelectiveAccessDescriptor& selectiveAccess,
    GetResult& result);

  XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    const ServiceOptions& options,
    GetResult& result);

  XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    const SelectiveAccessDescriptor& selectiveAccess,
    const ServiceOptions& options,
    GetResult& result);

  XdlmsStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData,
    SetResult& result);

  XdlmsStatus Set(
    const CosemAttributeDescriptor& descriptor,
    const std::vector<std::uint8_t>& encodedData,
    const ServiceOptions& options,
    SetResult& result);

  XdlmsStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    ActionResult& result);

  XdlmsStatus Action(
    const CosemMethodDescriptor& descriptor,
    bool hasParameter,
    const std::vector<std::uint8_t>& encodedParameter,
    const ServiceOptions& options,
    ActionResult& result);

private:
  XdlmsClient(const XdlmsClient&);
  XdlmsClient& operator=(const XdlmsClient&);

  XdlmsStatus Get(
    const CosemAttributeDescriptor& descriptor,
    const SelectiveAccessDescriptor* selectiveAccess,
    const ServiceOptions& options,
    GetResult& result);

  dlms::profile::IApduChannel& channel_;
  std::unique_ptr<IXdlmsAssociationState> ownedAssociation_;
  IXdlmsAssociationState* association_;
  std::unique_ptr<IXdlmsSecurityProcessor> ownedSecurity_;
  IXdlmsSecurityProcessor* security_;
  InvokeIdAllocator invokeIds_;
};

} // namespace xdlms
} // namespace dlms
