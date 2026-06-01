#include "dlms/endpoint/endpoint.hpp"

#include <cstdint>
#include <vector>

namespace {

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
  explicit ExampleApduChannel(const std::vector<std::uint8_t>& received)
    : open_(false)
    , received_(received)
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

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView)
  {
    return dlms::profile::ProfileStatus::UnsupportedFeature;
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
    for (std::size_t i = 0u; i < received_.size(); ++i) {
      output.data[i] = received_[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = received_.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  const std::vector<std::uint8_t>& Received() const
  {
    return received_;
  }

private:
  bool open_;
  std::vector<std::uint8_t> received_;
};

class ExamplePushHandler : public dlms::endpoint::IPushIndicationHandler
{
public:
  dlms::endpoint::EndpointStatus OnPushApdu(
    const std::vector<std::uint8_t>& apdu)
  {
    lastApdu_ = apdu;
    return dlms::endpoint::EndpointStatus::Ok;
  }

  const std::vector<std::uint8_t>& LastApdu() const
  {
    return lastApdu_;
  }

private:
  std::vector<std::uint8_t> lastApdu_;
};

} // namespace

int main()
{
  const std::uint8_t kPushApdu[] = {0x0Fu, 0x01u, 0x00u};
  const std::vector<std::uint8_t> pushApdu(
    kPushApdu,
    kPushApdu + sizeof(kPushApdu));

  ExampleApduChannel channel(pushApdu);
  ExamplePushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);
  if (endpoint.Open() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  if (endpoint.RunOnce() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  if (handler.LastApdu() != channel.Received()) {
    return 1;
  }
  return endpoint.Close() == dlms::endpoint::EndpointStatus::Ok ? 0 : 1;
}
