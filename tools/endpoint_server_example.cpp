#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"

#include <cstdint>
#include <memory>
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
  std::vector<std::uint8_t> sent_;
};

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;

  std::uint8_t buffer[16] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  dlms::apdu::EncodeDlmsData(data, writer);
  return std::vector<std::uint8_t>(buffer, buffer + writer.WrittenSize());
}

} // namespace

int main()
{
  ExampleApduChannel channel;
  dlms::cosem::LogicalDevice logicalDevice(1u, "example");
  logicalDevice.RegisterObject(
    std::shared_ptr<dlms::cosem::CosemRegisterObject>(
      new dlms::cosem::CosemRegisterObject(
        dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255),
        EncodeLongUnsigned(2300u),
        dlms::cosem::CosemByteBuffer(),
        dlms::cosem::AttributeAccessMode::ReadOnly)));

  dlms::endpoint::ServerEndpoint endpoint(channel, logicalDevice);
  return endpoint.Open() == dlms::endpoint::EndpointStatus::Ok ? 0 : 1;
}
