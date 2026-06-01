#include "dlms/association/association_types.hpp"

#include "dlms/apdu/initiate.hpp"

namespace dlms {
namespace association {

AssociationOptions DefaultAssociationOptions()
{
  const dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();

  AssociationOptions options;
  options.applicationContext = ApplicationContext::LogicalNameNoCiphering;
  options.authenticationMode = AuthenticationMode::None;
  options.lowLevelSecurityCredential.clear();
  options.highLevelSecurity = 0;
  options.callingApplicationTitle.clear();
  options.traceSink = 0;
  options.hasProposedQualityOfService = request.hasProposedQualityOfService;
  options.proposedQualityOfService = request.proposedQualityOfService;
  options.proposedDlmsVersionNumber = request.proposedDlmsVersionNumber;
  options.proposedConformance = request.proposedConformance;
  options.clientMaxReceivePduSize = request.clientMaxReceivePduSize;
  return options;
}

AssociationServerOptions DefaultAssociationServerOptions()
{
  const dlms::apdu::InitiateRequest request =
    dlms::apdu::MakeDefaultInitiateRequest();

  AssociationServerOptions options;
  options.applicationContext = ApplicationContext::LogicalNameNoCiphering;
  options.authenticationMode = AuthenticationMode::None;
  options.lowLevelSecurityCredential.clear();
  options.highLevelSecurity = 0;
  options.respondingApplicationTitle.clear();
  options.negotiatedDlmsVersionNumber = request.proposedDlmsVersionNumber;
  options.negotiatedConformance = request.proposedConformance;
  options.serverMaxReceivePduSize = request.clientMaxReceivePduSize;
  options.vaaName = 0x0007u;
  return options;
}

AssociationResult EmptyAssociationResult()
{
  AssociationResult result;
  result.negotiatedDlmsVersionNumber = 0;
  result.negotiatedConformance.bytes[0] = 0;
  result.negotiatedConformance.bytes[1] = 0;
  result.negotiatedConformance.bytes[2] = 0;
  result.serverMaxReceivePduSize = 0;
  result.vaaName = 0;
  result.hasAareResult = false;
  result.aareResult = 0;
  result.hasAareDiagnostic = false;
  result.aareDiagnostic = 0;
  result.highLevelSecurityServerChallenge.clear();
  result.callingApplicationTitle.clear();
  result.respondingApplicationTitle.clear();
  return result;
}

} // namespace association
} // namespace dlms
