#include "dlms/endpoint/endpoint.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"

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

class ExampleUpstream : public dlms::endpoint::IGatewayUpstream
{
public:
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

  dlms::endpoint::EndpointStatus Get(
    const dlms::endpoint::ClientAttributeDescriptor&,
    std::vector<std::uint8_t>& encodedData)
  {
    encodedData = EncodeLongUnsigned(2300u);
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Set(
    const dlms::endpoint::ClientAttributeDescriptor&,
    const std::vector<std::uint8_t>&)
  {
    return dlms::endpoint::EndpointStatus::Ok;
  }

  dlms::endpoint::EndpointStatus Action(
    const dlms::endpoint::ClientMethodDescriptor&,
    bool,
    const std::vector<std::uint8_t>&,
    std::vector<std::uint8_t>& encodedReturnParameter)
  {
    encodedReturnParameter.clear();
    return dlms::endpoint::EndpointStatus::Ok;
  }

private:
  bool open_ = false;
};

class AllowAllPolicy : public dlms::endpoint::IGatewayPolicy
{
public:
  bool AllowGet(const dlms::endpoint::ClientAttributeDescriptor&) const
  {
    return true;
  }

  bool AllowSet(const dlms::endpoint::ClientAttributeDescriptor&) const
  {
    return true;
  }

  bool AllowAction(const dlms::endpoint::ClientMethodDescriptor&) const
  {
    return true;
  }
};

} // namespace

int main()
{
  ExampleApduChannel downstream;
  ExampleUpstream upstream;
  AllowAllPolicy policy;
  dlms::endpoint::GatewayEndpoint endpoint(downstream, upstream, policy);
  if (endpoint.Open() != dlms::endpoint::EndpointStatus::Ok) {
    return 1;
  }
  return endpoint.Close() == dlms::endpoint::EndpointStatus::Ok ? 0 : 1;
}
