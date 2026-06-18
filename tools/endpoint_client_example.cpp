#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

namespace {

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;

  std::uint8_t buffer[16] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  if (dlms::apdu::EncodeDlmsData(data, writer) !=
      dlms::apdu::ApduStatus::Ok) {
    return std::vector<std::uint8_t>();
  }
  return std::vector<std::uint8_t>(buffer, buffer + writer.WrittenSize());
}

dlms::endpoint::ClientAttributeDescriptor RegisterValueDescriptor()
{
  dlms::endpoint::ClientAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 3u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  descriptor.attributeId = 2u;
  return descriptor;
}

} // namespace

int main()
{
  dlms::cosem::LogicalDevice logicalDevice(1u, "example");
  if (logicalDevice.RegisterObject(
        std::shared_ptr<dlms::cosem::CosemRegisterObject>(
          new dlms::cosem::CosemRegisterObject(
            dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
            EncodeLongUnsigned(2300u),
            dlms::cosem::types::ScalerUnit(),
            dlms::cosem::AttributeAccessMode::ReadOnly))) !=
      dlms::cosem::CosemStatus::Ok) {
    return 1;
  }

  dlms::endpoint::ServerEndpointOptions serverOptions =
    dlms::endpoint::DefaultServerEndpointOptions();
  serverOptions.transport.host = "127.0.0.1";
  serverOptions.transport.port = 0u;
  serverOptions.profile.kind = dlms::endpoint::EndpointProfileKind::Wrapper;
  serverOptions.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::None;
  serverOptions.negotiateAssociation = true;

  dlms::endpoint::EndpointListenerBundle listener;
  if (dlms::endpoint::CreateEndpointListener(
        serverOptions.transport,
        serverOptions.profile,
        listener) != dlms::endpoint::EndpointStatus::Ok ||
      listener.Listener() == 0) {
    return 1;
  }
  if (listener.Listener()->Open() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }

  dlms::endpoint::ClientEndpointOptions options =
    dlms::endpoint::DefaultClientEndpointOptions();
  options.transport.host = "127.0.0.1";
  options.transport.port = listener.Listener()->LocalPort();
  options.profile.kind = dlms::endpoint::EndpointProfileKind::Wrapper;
  options.security.authentication =
    dlms::endpoint::EndpointAuthenticationKind::None;

  if (dlms::endpoint::ValidateClientEndpointOptions(options) !=
      dlms::endpoint::EndpointStatus::Ok) {
    listener.Listener()->Close();
    return 1;
  }

  dlms::endpoint::EndpointStatus serverStatus =
    dlms::endpoint::EndpointStatus::InternalError;
  std::thread serverThread([&]() {
    std::unique_ptr<dlms::profile::IApduChannel> channel;
    serverStatus = listener.Listener()->Accept(channel);
    if (serverStatus != dlms::endpoint::EndpointStatus::Ok ||
        channel.get() == 0) {
      return;
    }

    dlms::endpoint::ServerEndpoint serverEndpoint(
      *channel,
      serverOptions,
      logicalDevice);
    serverStatus = serverEndpoint.Open();
    if (serverStatus == dlms::endpoint::EndpointStatus::Ok) {
      serverStatus = serverEndpoint.RunOnce();
    }
    if (serverStatus == dlms::endpoint::EndpointStatus::Ok) {
      serverStatus = serverEndpoint.RunOnce();
    }
    const dlms::endpoint::EndpointStatus closeStatus = serverEndpoint.Close();
    if (serverStatus == dlms::endpoint::EndpointStatus::Ok) {
      serverStatus = closeStatus;
    }
  });

  dlms::endpoint::ClientEndpoint client(options);

  std::vector<std::uint8_t> responseData;
  const dlms::endpoint::EndpointStatus openStatus = client.Open();
  dlms::endpoint::EndpointStatus getStatus =
    dlms::endpoint::EndpointStatus::InternalError;
  if (openStatus == dlms::endpoint::EndpointStatus::Ok) {
    getStatus = client.Get(RegisterValueDescriptor(), responseData);
  }
  const dlms::endpoint::EndpointStatus closeStatus = client.Close();

  serverThread.join();
  const dlms::endpoint::EndpointStatus listenerCloseStatus =
    listener.Listener()->Close();

  if (openStatus != dlms::endpoint::EndpointStatus::Ok ||
      getStatus != dlms::endpoint::EndpointStatus::Ok ||
      closeStatus != dlms::endpoint::EndpointStatus::Ok ||
      listenerCloseStatus != dlms::endpoint::EndpointStatus::Ok ||
      serverStatus != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  return responseData == EncodeLongUnsigned(2300u) ? 0 : 1;
}
