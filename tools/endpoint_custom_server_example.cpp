#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"

#include <cstdint>
#include <vector>

namespace {

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
  explicit ExampleApduChannel(const std::vector<std::uint8_t>& request)
    : open_(false)
    , received_(request)
    , sent_()
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open_ = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    open_ = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return open_;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    sent_.assign(apdu.data, apdu.data + apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    apdu = received_;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    if (output.size < received_.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t index = 0u; index < received_.size(); ++index) {
      output.data[index] = received_[index];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = received_.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  const std::vector<std::uint8_t>& Sent() const
  {
    return sent_;
  }

private:
  bool open_;
  std::vector<std::uint8_t> received_;
  std::vector<std::uint8_t> sent_;
};

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

std::vector<std::uint8_t> EncodeGetRequest()
{
  const dlms::apdu::XdlmsApdu request =
    dlms::apdu::MakeGetRequestNormal(
      0x85u,
      3u,
      dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
      2u);

  std::vector<std::uint8_t> output;
  dlms::apdu::EncodeXdlmsApdu(request, output);
  return output;
}

class ExampleServerService : public dlms::server::IServerService
{
public:
  dlms::server::ServerGetResponse HandleGet(
    const dlms::server::ServerGetRequest& request)
  {
    return dlms::server::MakeServerGetDataResponse(
      request.invokeId,
      EncodeLongUnsigned(2300u));
  }

  dlms::server::ServerSetResponse HandleSet(
    const dlms::server::ServerSetRequest& request)
  {
    return dlms::server::MakeServerSetResponse(
      request.invokeId,
      dlms::server::ServerStatus::UnsupportedFeature);
  }

  dlms::server::ServerActionResponse HandleAction(
    const dlms::server::ServerActionRequest& request)
  {
    return dlms::server::MakeServerActionResponse(
      request.invokeId,
      dlms::server::ServerStatus::UnsupportedFeature);
  }
};

} // namespace

int main()
{
  ExampleApduChannel channel(EncodeGetRequest());
  ExampleServerService server;
  dlms::endpoint::ServerEndpoint endpoint(channel, server);

  if (endpoint.Open() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  if (endpoint.RunOnce() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  if (endpoint.Close() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  return channel.Sent().empty() ? 1 : 0;
}
