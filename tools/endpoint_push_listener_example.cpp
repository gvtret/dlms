#include "dlms/endpoint/endpoint.hpp"

#include <cstdint>
#include <vector>

namespace {

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
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

private:
  bool open_ = false;
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

private:
  std::vector<std::uint8_t> lastApdu_;
};

} // namespace

int main()
{
  ExampleApduChannel channel;
  ExamplePushHandler handler;
  dlms::endpoint::PushListenerEndpoint endpoint(channel, handler);
  return endpoint.Open() == dlms::endpoint::EndpointStatus::Ok ? 0 : 1;
}
