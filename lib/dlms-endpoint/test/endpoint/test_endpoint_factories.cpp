#include "dlms/endpoint/endpoint.hpp"

#include "dlms/transport/tcp_stream_transport.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <thread>

namespace {

dlms::endpoint::EndpointTransportOptions TcpOptions()
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Tcp;
  options.host = "127.0.0.1";
  options.port = 4059u;
  return options;
}

dlms::endpoint::EndpointTransportOptions TcpListenerOptions()
{
  dlms::endpoint::EndpointTransportOptions options = TcpOptions();
  options.port = 0u;
  options.timeoutMs = 500u;
  return options;
}

dlms::endpoint::EndpointTransportOptions UdpOptions()
{
  dlms::endpoint::EndpointTransportOptions options = TcpOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Udp;
  return options;
}

dlms::endpoint::EndpointTransportOptions UdpListenerOptions()
{
  dlms::endpoint::EndpointTransportOptions options = UdpOptions();
  options.port = 0u;
  options.timeoutMs = 500u;
  return options;
}

dlms::endpoint::EndpointTransportOptions UdpSenderOptions(
  std::uint16_t remotePort)
{
  dlms::endpoint::EndpointTransportOptions options = UdpOptions();
  options.port = remotePort;
  options.timeoutMs = 500u;
  return options;
}

dlms::endpoint::EndpointTransportOptions SerialOptions()
{
  dlms::endpoint::EndpointTransportOptions options =
    dlms::endpoint::DefaultEndpointTransportOptions();
  options.kind = dlms::endpoint::EndpointTransportKind::Serial;
  options.serialDevice = "COM1";
  options.baudRate = 9600u;
  return options;
}

dlms::transport::TcpStreamTransportOptions TcpClientOptions(
  std::uint16_t port)
{
  dlms::transport::TcpStreamTransportOptions options;
  options.host = "127.0.0.1";
  options.port = port;
  options.connectTimeout.milliseconds = 500u;
  options.readTimeout.milliseconds = 500u;
  options.writeTimeout.milliseconds = 500u;
  return options;
}

void ExpectAcceptsLoopbackChannel(
  dlms::endpoint::EndpointProfileKind profileKind)
{
  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  profile.kind = profileKind;

  dlms::endpoint::EndpointListenerBundle bundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(
              TcpListenerOptions(),
              profile,
              bundle));
  ASSERT_TRUE(bundle.Listener() != 0);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok, bundle.Listener()->Open());
  ASSERT_NE(0u, bundle.Listener()->LocalPort());

  dlms::transport::TcpStreamTransport client(
    TcpClientOptions(bundle.Listener()->LocalPort()));
  ASSERT_EQ(dlms::transport::TransportStatus::Ok, client.Open());

  std::unique_ptr<dlms::profile::IApduChannel> channel;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            bundle.Listener()->Accept(channel));
  ASSERT_TRUE(channel.get() != 0);
  EXPECT_TRUE(channel->IsOpen());

  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, channel->Close());
  EXPECT_EQ(dlms::transport::TransportStatus::Ok, client.Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok, bundle.Listener()->Close());
}

} // namespace

TEST(EndpointFactories, CreatesTransportBundlesWithoutOpeningThem)
{
  dlms::endpoint::EndpointTransportBundle transport;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(TcpOptions(), transport));
  EXPECT_TRUE(transport.ByteStream() != 0);
  EXPECT_TRUE(transport.Datagram() == 0);
  EXPECT_FALSE(transport.ByteStream()->IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(UdpOptions(), transport));
  EXPECT_TRUE(transport.ByteStream() == 0);
  EXPECT_TRUE(transport.Datagram() != 0);
  EXPECT_FALSE(transport.Datagram()->IsOpen());

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              SerialOptions(),
              transport));
  EXPECT_TRUE(transport.ByteStream() != 0);
  EXPECT_TRUE(transport.Datagram() == 0);
  EXPECT_FALSE(transport.ByteStream()->IsOpen());
}

TEST(EndpointFactories, RejectsInvalidTransportOptions)
{
  dlms::endpoint::EndpointTransportOptions options = TcpOptions();
  options.host = 0;

  dlms::endpoint::EndpointTransportBundle transport;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::CreateEndpointTransport(options, transport));
  EXPECT_TRUE(transport.ByteStream() == 0);
  EXPECT_TRUE(transport.Datagram() == 0);
}

TEST(EndpointFactories, CreatesProfileBundlesForMatchingTransports)
{
  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  dlms::endpoint::EndpointTransportBundle transport;
  dlms::endpoint::EndpointProfileBundle bundle;

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(TcpOptions(), transport));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              profile,
              transport,
              bundle));
  EXPECT_TRUE(bundle.Channel() != 0);
  EXPECT_TRUE(bundle.HdlcDataLink() == 0);
  EXPECT_FALSE(bundle.Channel()->IsOpen());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(UdpOptions(), transport));
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              profile,
              transport,
              bundle));
  EXPECT_TRUE(bundle.Channel() != 0);
  EXPECT_TRUE(bundle.HdlcDataLink() == 0);
  EXPECT_FALSE(bundle.Channel()->IsOpen());

  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              SerialOptions(),
              transport));
  profile.kind = dlms::endpoint::EndpointProfileKind::Hdlc;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              profile,
              transport,
              bundle));
  EXPECT_TRUE(bundle.Channel() != 0);
  EXPECT_TRUE(bundle.HdlcDataLink() != 0);
  EXPECT_FALSE(bundle.Channel()->IsOpen());
}

TEST(EndpointFactories, RejectsMismatchedProfileTransport)
{
  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  profile.kind = dlms::endpoint::EndpointProfileKind::Hdlc;

  dlms::endpoint::EndpointTransportBundle transport;
  dlms::endpoint::EndpointProfileBundle bundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(UdpOptions(), transport));

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidState,
            dlms::endpoint::CreateEndpointProfile(
              profile,
              transport,
              bundle));
  EXPECT_TRUE(bundle.Channel() == 0);
}

TEST(EndpointFactories, CreatesTcpProfileListenerBundleWithoutOpeningIt)
{
  dlms::endpoint::EndpointListenerBundle bundle;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(
              TcpListenerOptions(),
              dlms::endpoint::DefaultEndpointProfileOptions(),
              bundle));
  EXPECT_TRUE(bundle.Listener() != 0);
  EXPECT_FALSE(bundle.Listener()->IsOpen());
  EXPECT_EQ(0u, bundle.Listener()->LocalPort());
}

TEST(EndpointFactories, CreatesUdpPushProfileListenerBundleWithoutOpeningIt)
{
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.transport = UdpListenerOptions();

  dlms::endpoint::EndpointListenerBundle bundle;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(options, bundle));
  EXPECT_TRUE(bundle.Listener() != 0);
  EXPECT_FALSE(bundle.Listener()->IsOpen());
  EXPECT_EQ(0u, bundle.Listener()->LocalPort());
}

TEST(EndpointFactories, RejectsNonTcpProfileListeners)
{
  dlms::endpoint::EndpointListenerBundle bundle;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::UnsupportedProfile,
            dlms::endpoint::CreateEndpointListener(
              UdpOptions(),
              dlms::endpoint::DefaultEndpointProfileOptions(),
              bundle));
  EXPECT_TRUE(bundle.Listener() == 0);

  EXPECT_EQ(dlms::endpoint::EndpointStatus::UnsupportedProfile,
            dlms::endpoint::CreateEndpointListener(
              SerialOptions(),
              dlms::endpoint::DefaultEndpointProfileOptions(),
              bundle));
  EXPECT_TRUE(bundle.Listener() == 0);
}

TEST(EndpointFactories, RejectsHdlcUdpPushProfileListener)
{
  dlms::endpoint::PushListenerEndpointOptions options =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  options.transport = UdpListenerOptions();
  options.profile.kind = dlms::endpoint::EndpointProfileKind::Hdlc;

  dlms::endpoint::EndpointListenerBundle bundle;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::UnsupportedProfile,
            dlms::endpoint::CreateEndpointListener(options, bundle));
  EXPECT_TRUE(bundle.Listener() == 0);
}

TEST(EndpointFactories, TcpProfileListenerAcceptsWrapperChannel)
{
  ExpectAcceptsLoopbackChannel(dlms::endpoint::EndpointProfileKind::Wrapper);
}

TEST(EndpointFactories, TcpProfileListenerAcceptsHdlcChannel)
{
  ExpectAcceptsLoopbackChannel(dlms::endpoint::EndpointProfileKind::Hdlc);
}

TEST(EndpointFactories, UdpPushProfileListenerAcceptsWrapperChannel)
{
  dlms::endpoint::PushListenerEndpointOptions listenerOptions =
    dlms::endpoint::DefaultPushListenerEndpointOptions();
  listenerOptions.transport = UdpListenerOptions();

  dlms::endpoint::EndpointListenerBundle listenerBundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(
              listenerOptions,
              listenerBundle));
  ASSERT_TRUE(listenerBundle.Listener() != 0);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            listenerBundle.Listener()->Open());
  ASSERT_NE(0u, listenerBundle.Listener()->LocalPort());

  dlms::endpoint::EndpointTransportBundle senderTransport;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              UdpSenderOptions(listenerBundle.Listener()->LocalPort()),
              senderTransport));
  dlms::endpoint::EndpointProfileBundle senderProfile;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              listenerOptions.profile,
              senderTransport,
              senderProfile));
  ASSERT_TRUE(senderProfile.Channel() != 0);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            senderProfile.Channel()->Open());

  std::unique_ptr<dlms::profile::IApduChannel> accepted;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            listenerBundle.Listener()->Accept(accepted));
  ASSERT_TRUE(accepted.get() != 0);
  EXPECT_TRUE(accepted->IsOpen());

  const std::uint8_t apdu[] = { 0x0f, 0x01, 0x22 };
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            senderProfile.Channel()->SendApdu(
              dlms::profile::ProfileByteView{ apdu, sizeof(apdu) }));

  std::vector<std::uint8_t> received;
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            accepted->ReceiveApdu(received));
  EXPECT_EQ(std::vector<std::uint8_t>(apdu, apdu + sizeof(apdu)), received);

  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, accepted->Close());
  EXPECT_TRUE(listenerBundle.Listener()->IsOpen());
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok,
            senderProfile.Channel()->Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            listenerBundle.Listener()->Close());
}

TEST(EndpointFactories, TcpProfileListenerAcceptsHdlcSessionChannel)
{
  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  profile.kind = dlms::endpoint::EndpointProfileKind::Hdlc;
  profile.hdlcUseSession = true;

  dlms::endpoint::EndpointListenerBundle listenerBundle;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointListener(
              TcpListenerOptions(),
              profile,
              listenerBundle));
  ASSERT_TRUE(listenerBundle.Listener() != 0);
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            listenerBundle.Listener()->Open());
  ASSERT_NE(0u, listenerBundle.Listener()->LocalPort());

  dlms::endpoint::EndpointTransportOptions clientOptions = TcpOptions();
  clientOptions.port = listenerBundle.Listener()->LocalPort();

  dlms::endpoint::EndpointTransportBundle clientTransport;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointTransport(
              clientOptions,
              clientTransport));
  dlms::endpoint::EndpointProfileBundle clientProfile;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointProfile(
              profile,
              clientTransport,
              clientProfile));
  ASSERT_TRUE(clientProfile.Channel() != 0);
  ASSERT_TRUE(clientProfile.HdlcDataLink() != 0);
  ASSERT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Open());

  std::unique_ptr<dlms::profile::IApduChannel> accepted;
  ASSERT_EQ(dlms::endpoint::EndpointStatus::Ok,
            listenerBundle.Listener()->Accept(accepted));
  ASSERT_TRUE(accepted.get() != 0);

  dlms::profile::ProfileStatus serverOpen =
    dlms::profile::ProfileStatus::InternalError;
  std::thread serverOpenThread([&accepted, &serverOpen]() {
    serverOpen = accepted->Open();
  });
  const dlms::profile::ProfileStatus clientConnect =
    clientProfile.HdlcDataLink()->ConnectDataLink();
  serverOpenThread.join();
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, clientConnect);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, serverOpen);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, accepted->Open());

  const std::uint8_t apdu[] = { 0xc0u, 0x01u, 0x81u, 0x00u };
  std::vector<std::uint8_t> received;
  dlms::profile::ProfileStatus serverReceive =
    dlms::profile::ProfileStatus::InternalError;
  std::thread serverReceiveThread([&accepted, &received, &serverReceive]() {
    serverReceive = accepted->ReceiveApdu(received);
  });
  const dlms::profile::ProfileStatus clientSend =
    clientProfile.Channel()->SendApdu(
      dlms::profile::ProfileByteView{ apdu, sizeof(apdu) });
  serverReceiveThread.join();
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, clientSend);
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, serverReceive);
  EXPECT_EQ(std::vector<std::uint8_t>(apdu, apdu + sizeof(apdu)), received);

  EXPECT_EQ(dlms::profile::ProfileStatus::Ok, accepted->Close());
  EXPECT_EQ(dlms::profile::ProfileStatus::Ok,
            clientProfile.Channel()->Close());
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            listenerBundle.Listener()->Close());
}

TEST(EndpointFactories, CreatesSecurityBundleFromEndpointOptions)
{
  const std::uint8_t title[] =
    { 'M', 'Y', 'S', 'Y', 'S', 'T', 'L', 'E' };
  const std::uint8_t peerTitle[] =
    { 'C', 'L', 'I', 'E', 'N', 'T', '0', '1' };
  const std::uint8_t authenticationKey[] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F};

  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  dlms::endpoint::EndpointSecurityOptions security =
    dlms::endpoint::DefaultEndpointSecurityOptions();
  dlms::endpoint::EndpointSecurityBundle bundle;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointSecurity(
              profile,
              security,
              bundle));
  EXPECT_EQ(dlms::endpoint::EndpointAuthenticationKind::None,
            bundle.authentication);
  EXPECT_FALSE(bundle.requiresPassword);
  EXPECT_FALSE(bundle.requiresCiphering);
  EXPECT_EQ(dlms::security::SecurityPolicy::None, bundle.context.policy);

  security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  security.systemTitle = title;
  security.systemTitleSize = sizeof(title);
  security.peerSystemTitle = peerTitle;
  security.peerSystemTitleSize = sizeof(peerTitle);
  security.authenticationKey = authenticationKey;
  security.authenticationKeySize = sizeof(authenticationKey);
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointSecurity(
              profile,
              security,
              bundle));
  EXPECT_FALSE(bundle.requiresCiphering);
  EXPECT_EQ(dlms::security::SecurityPolicy::Authenticated,
            bundle.context.policy);

  security.globalUnicastEncryptionKey = authenticationKey;
  security.globalUnicastEncryptionKeySize = sizeof(authenticationKey);
  security.cipheredApdu = true;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::Ok,
            dlms::endpoint::CreateEndpointSecurity(
              profile,
              security,
              bundle));
  EXPECT_TRUE(bundle.requiresCiphering);
  EXPECT_EQ(dlms::security::SecurityPolicy::AuthenticatedAndEncrypted,
            bundle.context.policy);
  EXPECT_EQ(profile.clientSap, bundle.context.clientSap);
  EXPECT_EQ(profile.serverSap, bundle.context.serverSap);
  EXPECT_EQ('M', bundle.context.localSystemTitle[0]);
  EXPECT_EQ('E', bundle.context.localSystemTitle[7]);
  EXPECT_EQ('C', bundle.context.remoteSystemTitle[0]);
  EXPECT_EQ('1', bundle.context.remoteSystemTitle[7]);
}

TEST(EndpointFactories, SecurityBundleErrorsClearOutput)
{
  dlms::endpoint::EndpointProfileOptions profile =
    dlms::endpoint::DefaultEndpointProfileOptions();
  dlms::endpoint::EndpointSecurityOptions security =
    dlms::endpoint::DefaultEndpointSecurityOptions();
  dlms::endpoint::EndpointSecurityBundle bundle;
  bundle.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  bundle.context = dlms::security::EmptySecurityContext();
  bundle.context.policy =
    dlms::security::SecurityPolicy::AuthenticatedAndEncrypted;
  bundle.requiresPassword = true;
  bundle.requiresCiphering = true;

  profile.clientSap = 0u;
  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::CreateEndpointSecurity(
              profile,
              security,
              bundle));
  EXPECT_EQ(dlms::endpoint::EndpointAuthenticationKind::None,
            bundle.authentication);
  EXPECT_EQ(dlms::security::SecurityPolicy::None, bundle.context.policy);
  EXPECT_FALSE(bundle.requiresPassword);
  EXPECT_FALSE(bundle.requiresCiphering);

  profile = dlms::endpoint::DefaultEndpointProfileOptions();
  security.authentication =
    static_cast<dlms::endpoint::EndpointAuthenticationKind>(99);
  bundle.authentication =
    dlms::endpoint::EndpointAuthenticationKind::HighGmac;
  bundle.context.policy =
    dlms::security::SecurityPolicy::AuthenticatedAndEncrypted;
  bundle.requiresPassword = true;
  bundle.requiresCiphering = true;

  EXPECT_EQ(dlms::endpoint::EndpointStatus::InvalidArgument,
            dlms::endpoint::CreateEndpointSecurity(
              profile,
              security,
              bundle));
  EXPECT_EQ(dlms::endpoint::EndpointAuthenticationKind::None,
            bundle.authentication);
  EXPECT_EQ(dlms::security::SecurityPolicy::None, bundle.context.policy);
  EXPECT_FALSE(bundle.requiresPassword);
  EXPECT_FALSE(bundle.requiresCiphering);
}
