#include "dlms/client/client.hpp"

#include "dlms/association/association_client.hpp"

#include <cstdint>
#include <vector>

namespace {

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
  ExampleApduChannel()
    : open_(false)
    , received_()
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

  void SetReceived(const std::vector<std::uint8_t>& apdu)
  {
    received_ = apdu;
  }

private:
  bool open_;
  std::vector<std::uint8_t> received_;
};

std::vector<std::uint8_t> MakeAareBytes()
{
  const std::uint8_t kAare[] = {
    0x61, 0x4E, 0x80, 0x02, 0x02, 0x84, 0xA1, 0x09,
    0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
    0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05,
    0xA1, 0x03, 0x02, 0x01, 0x0E, 0x88, 0x02, 0x07,
    0x80, 0x89, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
    0x02, 0x02, 0xAA, 0x12, 0x80, 0x10, 0xC6, 0x69,
    0x73, 0x51, 0xFF, 0x4A, 0xEC, 0x29, 0xCD, 0xBA,
    0xAB, 0xF2, 0xFB, 0xE3, 0x46, 0x7C, 0xBE, 0x10,
    0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04,
    0x00, 0x40, 0x18, 0x1D, 0x02, 0x00, 0x00, 0x07};
  return std::vector<std::uint8_t>(kAare, kAare + sizeof(kAare));
}

std::vector<std::uint8_t> MakeRlreBytes()
{
  const std::uint8_t kRlre[] = {0x63, 0x00};
  return std::vector<std::uint8_t>(kRlre, kRlre + sizeof(kRlre));
}

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value)
{
  std::vector<std::uint8_t> output;
  output.push_back(0x12u);
  output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
  output.push_back(static_cast<std::uint8_t>(value & 0xFFu));
  return output;
}

dlms::client::CosemAttributeDescriptor MakeDescriptor()
{
  dlms::client::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 3u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  descriptor.attributeId = 2u;
  return descriptor;
}

class ExampleXdlmsService : public dlms::client::IClientXdlmsService
{
public:
  ExampleXdlmsService()
    : getCalls_(0)
  {
  }

  dlms::xdlms::XdlmsStatus Get(
    const dlms::client::CosemAttributeDescriptor&,
    dlms::xdlms::GetResult& result)
  {
    ++getCalls_;
    result = dlms::xdlms::EmptyGetResult();
    result.invokeId = 1u;
    result.hasData = true;
    result.data = EncodeLongUnsigned(2300u);
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  dlms::xdlms::XdlmsStatus Set(
    const dlms::client::CosemAttributeDescriptor&,
    const std::vector<std::uint8_t>&,
    dlms::xdlms::SetResult& result)
  {
    result = dlms::xdlms::EmptySetResult();
    return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
  }

  dlms::xdlms::XdlmsStatus Action(
    const dlms::client::CosemMethodDescriptor&,
    bool,
    const std::vector<std::uint8_t>&,
    dlms::xdlms::ActionResult& result)
  {
    result = dlms::xdlms::EmptyActionResult();
    return dlms::xdlms::XdlmsStatus::UnsupportedFeature;
  }

  int GetCalls() const
  {
    return getCalls_;
  }

private:
  int getCalls_;
};

} // namespace

int main()
{
  ExampleApduChannel channel;
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());
  ExampleXdlmsService xdlms;
  dlms::client::DlmsClient client(channel, association, xdlms);

  if (client.Connect() != dlms::client::ClientStatus::Ok) {
    return 1;
  }
  channel.SetReceived(MakeAareBytes());
  if (client.OpenAssociation() != dlms::client::ClientStatus::Ok) {
    return 1;
  }

  std::vector<std::uint8_t> output;
  if (client.Get(MakeDescriptor(), output) != dlms::client::ClientStatus::Ok) {
    return 1;
  }
  if (output != EncodeLongUnsigned(2300u) || xdlms.GetCalls() != 1) {
    return 1;
  }

  channel.SetReceived(MakeRlreBytes());
  if (client.ReleaseAssociation() != dlms::client::ClientStatus::Ok) {
    return 1;
  }
  return client.Close() == dlms::client::ClientStatus::Ok ? 0 : 1;
}
