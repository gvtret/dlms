#include "dlms/endpoint/endpoint.hpp"

#include <gtest/gtest.h>

namespace {

dlms::endpoint::ClientEndpointOptions WrapperTcpOptions()
{
  dlms::endpoint::ClientEndpointOptions options =
    dlms::endpoint::DefaultClientEndpointOptions();
  options.transport.kind = dlms::endpoint::EndpointTransportKind::Tcp;
  options.transport.host = "127.0.0.1";
  options.transport.port = 1u;
  options.transport.timeoutMs = 1u;
  options.profile.kind = dlms::endpoint::EndpointProfileKind::Wrapper;
  options.profile.clientSap = 16u;
  options.profile.serverSap = 1u;
  return options;
}

dlms::endpoint::ClientAttributeDescriptor AttributeDescriptor()
{
  dlms::endpoint::ClientAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 1u;
  descriptor.instanceId =
    dlms::xdlms::CosemLogicalName(0, 0, 1, 0, 0, 255);
  descriptor.attributeId = 2u;
  return descriptor;
}

dlms::endpoint::ClientMethodDescriptor MethodDescriptor()
{
  dlms::endpoint::ClientMethodDescriptor descriptor =
    dlms::xdlms::EmptyCosemMethodDescriptor();
  descriptor.classId = 1u;
  descriptor.instanceId =
    dlms::xdlms::CosemLogicalName(0, 0, 1, 0, 0, 255);
  descriptor.methodId = 1u;
  return descriptor;
}

} // namespace

TEST(ClientEndpoint, StartsClosedAndCloseIsIdempotent)
{
  dlms::endpoint::ClientEndpoint endpoint(WrapperTcpOptions());

  EXPECT_FALSE(endpoint.IsOpen());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ClientEndpoint, ServicesRequireOpenEndpoint)
{
  dlms::endpoint::ClientEndpoint endpoint(WrapperTcpOptions());

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.Get(AttributeDescriptor(), output));
  EXPECT_TRUE(output.empty());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.Set(AttributeDescriptor(), std::vector<std::uint8_t>()));

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.Action(
              MethodDescriptor(),
              false,
              std::vector<std::uint8_t>(),
              output));
  EXPECT_TRUE(output.empty());
}

TEST(ClientEndpoint, OpenRejectsInvalidOptionsBeforeNetworkUse)
{
  dlms::endpoint::ClientEndpointOptions options = WrapperTcpOptions();
  options.transport.host = "";
  dlms::endpoint::ClientEndpoint endpoint(options);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ClientEndpoint, OpenMapsConnectionFailure)
{
  dlms::endpoint::ClientEndpoint endpoint(WrapperTcpOptions());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ClientEndpoint, SupportsPasswordAssociationOptionShapes)
{
  const std::uint8_t password[] =
    { 'H', 'i', 'P', 'a', 's', 's', 'w', 'o', 'r', 'd' };

  dlms::endpoint::ClientEndpointOptions options = WrapperTcpOptions();
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  options.security.password = password;
  options.security.passwordSize = sizeof(password);

  dlms::endpoint::ClientEndpoint low(options);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed, low.Open());

  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighPassword;
  dlms::endpoint::ClientEndpoint high(options);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed, high.Open());
}

TEST(ClientEndpoint, GmacRequiresDedicatedKeyPhase)
{
  const std::uint8_t systemTitle[] =
    { 'S', 'Y', 'S', 'T', 'I', 'T', 'L', 'E' };
  const std::uint8_t authenticationKey[] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F};
  dlms::endpoint::ClientEndpointOptions options = WrapperTcpOptions();
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  options.security.systemTitle = systemTitle;
  options.security.systemTitleSize = sizeof(systemTitle);

  dlms::endpoint::ClientEndpoint endpoint(options);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            endpoint.Open());

  options.security.authenticationKey = authenticationKey;
  options.security.authenticationKeySize = sizeof(authenticationKey);
  dlms::endpoint::ClientEndpoint keyed(options);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed, keyed.Open());

  options.security.cipheredApdu = true;
  dlms::endpoint::ClientEndpoint missingCipherKey(options);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            missingCipherKey.Open());

  options.security.globalUnicastEncryptionKey = authenticationKey;
  options.security.globalUnicastEncryptionKeySize = sizeof(authenticationKey);
  dlms::endpoint::ClientEndpoint ciphered(options);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed, ciphered.Open());
}

TEST(ClientEndpoint, OpenAfterFailedOpenIsIdempotentAndRetries)
{
  // Reuse port=1 (connect refused) options.
  dlms::endpoint::ClientEndpoint endpoint(WrapperTcpOptions());

  // First open: connect refused -> ProfileFailed, endpoint stays closed.
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());

  // Close after failed open is a no-op and returns Ok (no stale state).
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());

  // Second open retries the underlying connect; same failure surfaces again.
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ClientEndpoint, CloseAfterFailedOpenLeavesNoStateBehind)
{
  dlms::endpoint::ClientEndpoint endpoint(WrapperTcpOptions());

  // Failed open at the connect stage.
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());

  // Multiple Close()s are all no-ops and never return non-Ok.
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());

  // Services still fail with InvalidState, not InternalError.
  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            endpoint.Get(AttributeDescriptor(), output));
  EXPECT_TRUE(output.empty());
}

TEST(ClientEndpoint, OpenIsIdempotentAfterValidationFailure)
{
  // Bad host -> validation failure before any network use.
  dlms::endpoint::ClientEndpointOptions options = WrapperTcpOptions();
  options.transport.host = "";
  dlms::endpoint::ClientEndpoint endpoint(options);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());

  // Same failure on the second Open(); no state was created.
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            endpoint.Open());
  EXPECT_FALSE(endpoint.IsOpen());

  // Close stays a no-op.
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, endpoint.Close());
  EXPECT_FALSE(endpoint.IsOpen());
}

TEST(ClientEndpoint, DestructorClosesAfterFailedOpenWithoutLeak)
{
  // No state-assertions here beyond the open failure; the test is that
  // destruction does not throw / abort / leak after a failed open. Under
  // ASan a leak in the failed-Open path would surface as a regression.
  for (int i = 0; i < 3; ++i) {
    dlms::endpoint::ClientEndpoint endpoint(WrapperTcpOptions());
    EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
              endpoint.Open());
    EXPECT_FALSE(endpoint.IsOpen());
    // Endpoint goes out of scope here.
  }
}

TEST(ClientEndpoint, MapsClientStatusesToEndpointStatuses)
{
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::Ok));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::InvalidArgument));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::NotAssociated));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::TransportFailed,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::TransportOpenFailed));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ProfileFailed,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::ChannelOpenFailed));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::AssociationFailed,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::AssociationFailed));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::ServiceFailed,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::ServiceRejected));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::SecurityFailed,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::SecurityFailed));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::UnsupportedProfile,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::UnsupportedFeature));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InternalError,
            dlms::endpoint::MapClientStatus(
              dlms::client::ClientStatus::InternalError));
}
