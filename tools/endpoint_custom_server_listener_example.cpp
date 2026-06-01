#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace {

struct ChannelState
{
  ChannelState()
    : open(false)
    , received()
    , sent()
  {
  }

  bool open;
  std::vector<std::uint8_t> received;
  std::vector<std::uint8_t> sent;
};

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
  explicit ExampleApduChannel(const std::shared_ptr<ChannelState>& state)
    : state_(state)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    state_->open = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    state_->open = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return state_->open;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    state_->sent.assign(apdu.data, apdu.data + apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    apdu = state_->received;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    if (output.size < state_->received.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t index = 0u; index < state_->received.size(); ++index) {
      output.data[index] = state_->received[index];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = state_->received.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

private:
  std::shared_ptr<ChannelState> state_;
};

class ExampleApduChannelListener
  : public dlms::endpoint::IApduChannelListener
{
public:
  explicit ExampleApduChannelListener(
    const std::shared_ptr<ChannelState>& state)
    : open_(false)
    , state_(state)
  {
  }

  dlms::endpoint::EndpointStatus Open()
  {
    open_ = true;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Close()
  {
    open_ = false;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  bool IsOpen() const
  {
    return open_;
  }

  dlms::endpoint::EndpointStatus Accept(
    std::unique_ptr<dlms::profile::IApduChannel>& channel)
  {
    channel.reset(new ExampleApduChannel(state_));
    return dlms::endpoint::EndpointStatus::Ok;
  }

private:
  bool open_;
  std::shared_ptr<ChannelState> state_;
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
  const std::shared_ptr<ChannelState> channelState(new ChannelState());
  channelState->received = EncodeGetRequest();

  ExampleApduChannelListener listener(channelState);
  ExampleServerService server;
  dlms::endpoint::ServerListenerRuntime runtime(listener, server);

  if (runtime.Open() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  if (runtime.RunOnce() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  if (runtime.Close() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  return channelState->sent.empty() || channelState->open ? 1 : 0;
}
