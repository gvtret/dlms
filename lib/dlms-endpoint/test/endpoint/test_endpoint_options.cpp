#include "dlms/endpoint/endpoint.hpp"

#include <gtest/gtest.h>

namespace {

dlms::endpoint::EndpointTransportOptions ValidTcpTransport()
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Tcp;
  options.host = "127.0.0.1";
  options.port = 4059u;
  return options;
}

} // namespace

TEST(EndpointOptions, DefaultsAreNoSecurityWrapperTcpShape)
{
  const dlms::endpoint::ClientEndpointOptions options =
    dlms::endpoint::DefaultClientEndpointOptions();

  EXPECT_EQ(dlms::endpoint::EndpointTransportKind::Tcp,
            options.transport.kind);
  EXPECT_EQ(dlms::endpoint::EndpointProfileKind::Wrapper,
            options.profile.kind);
  EXPECT_EQ(16u, options.profile.clientSap);
  EXPECT_EQ(1u, options.profile.serverSap);
  EXPECT_FALSE(options.profile.hdlcUseSession);
  EXPECT_EQ(dlms::endpoint::EndpointAuthenticationKind::None,
            options.security.authentication);
  EXPECT_EQ(0u, options.security.invocationCounter);
  EXPECT_EQ(0u, options.security.peerSystemTitleSize);
  EXPECT_EQ(0u, options.security.authenticationKeySize);
  EXPECT_EQ(0u, options.security.globalUnicastEncryptionKeySize);
  EXPECT_FALSE(options.security.cipheredApdu);

  const dlms::endpoint::ServerEndpointOptions server =
    dlms::endpoint::DefaultServerEndpointOptions();
  EXPECT_FALSE(server.negotiateAssociation);

  const dlms::endpoint::GatewayEndpointOptions gateway =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  EXPECT_FALSE(gateway.downstream.negotiateAssociation);

  const dlms::endpoint::PushListenerEndpointOptions push =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  EXPECT_FALSE(push.negotiateAssociation);
}

TEST(EndpointOptions, ValidatesTcpUdpAndSerialTransports)
{
  dlms::endpoint::EndpointTransportOptions options = ValidTcpTransport();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointTransportOptions(options));

  options.kind = dlms::endpoint::EndpointTransportKind::Udp;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointTransportOptions(options));

  options.host = "";
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointTransportOptions(options));

  options = dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Serial;
  options.serialDevice = "COM1";
  options.baudRate = 9600u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointTransportOptions(options));

  options.baudRate = 0u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointTransportOptions(options));
}

TEST(EndpointOptions, ValidatesProfileOptions)
{
  dlms::endpoint::EndpointProfileOptions options =
    dlms::endpoint::DefaultEndpointProfileOptions();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointProfileOptions(options));

  options.clientSap = 0u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointProfileOptions(options));

  options = dlms::endpoint::DefaultEndpointProfileOptions();
  options.kind = dlms::endpoint::EndpointProfileKind::Hdlc;
  options.hdlcUseSession = true;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointProfileOptions(options));

  options.clientSap = 128u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointProfileOptions(options));

  options.kind = dlms::endpoint::EndpointProfileKind::Wrapper;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointProfileOptions(options));

  options = dlms::endpoint::DefaultEndpointProfileOptions();
  options.kind = dlms::endpoint::EndpointProfileKind::Hdlc;
  options.serverSap = 0x4000u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointProfileOptions(options));

  options.kind = dlms::endpoint::EndpointProfileKind::Wrapper;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointProfileOptions(options));
}

TEST(EndpointOptions, ValidatesSecurityOptions)
{
  const std::uint8_t password[] = { 'p', 'w' };
  const std::uint8_t title[] = { 'S', 'Y', 'S', 'T', 'I', 'T', 'L', 'E' };
  const std::uint8_t key[] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F};

  dlms::endpoint::EndpointSecurityOptions options =
    dlms::endpoint::DefaultEndpointSecurityOptions();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.authentication =
    dlms::endpoint::EndpointAuthenticationKind::LowPassword;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.password = password;
  options.passwordSize = sizeof(password);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options = dlms::endpoint::DefaultEndpointSecurityOptions();
  options.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.systemTitle = title;
  options.systemTitleSize = sizeof(title);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.authenticationKey = key;
  options.authenticationKeySize = sizeof(key);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.peerSystemTitle = title;
  options.peerSystemTitleSize = sizeof(title) - 1u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.peerSystemTitleSize = sizeof(title);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.cipheredApdu = true;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.globalUnicastEncryptionKey = key;
  options.globalUnicastEncryptionKeySize = sizeof(key) - 1u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));

  options.globalUnicastEncryptionKeySize = sizeof(key);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateEndpointSecurityOptions(options));
}

TEST(EndpointOptions, ValidatesComposedEndpointOptions)
{
  dlms::endpoint::ClientEndpointOptions client =
    dlms::endpoint::DefaultClientEndpointOptions();
  client.transport = ValidTcpTransport();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateClientEndpointOptions(client));

  client.transport.kind = dlms::endpoint::EndpointTransportKind::Udp;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::UnsupportedProfile,
            dlms::endpoint::ValidateClientEndpointOptions(client));
  client.transport = ValidTcpTransport();

  dlms::endpoint::ServerEndpointOptions server =
    dlms::endpoint::DefaultServerEndpointOptions();
  server.transport = ValidTcpTransport();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateServerEndpointOptions(server));

  dlms::endpoint::PushListenerEndpointOptions push =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  push.transport = ValidTcpTransport();
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidatePushListenerEndpointOptions(push));

  dlms::endpoint::GatewayEndpointOptions gateway =
    dlms::endpoint::DefaultGatewayEndpointOptions();
  gateway.downstream = server;
  gateway.upstream = client;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::ValidateGatewayEndpointOptions(gateway));

  gateway.upstream.transport.host = 0;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::ValidateGatewayEndpointOptions(gateway));
}
