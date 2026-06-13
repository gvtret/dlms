#include "dlms/client/client.hpp"
#include "dlms/client/client_data.hpp"
#include "dlms/cosem/simple_objects.hpp"

#include "dlms/association/association_client_interface.hpp"

#include <cstdint>
#include <vector>

namespace {

class ExampleApduChannel : public dlms::profile::IApduChannel
{
public:
  dlms::profile::ProfileStatus Open()
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return true;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView)
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>&)
  {
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer)
  {
    return dlms::profile::ProfileStatus::Ok;
  }
};

class ExampleAssociation : public dlms::association::IAssociationClient
{
public:
  ExampleAssociation()
    : state_(dlms::association::AssociationState::Closed)
    , result_(dlms::association::EmptyAssociationResult())
  {
  }

  dlms::association::AssociationStatus Open()
  {
    state_ = dlms::association::AssociationState::Open;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus Close()
  {
    state_ = dlms::association::AssociationState::Closed;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus Establish()
  {
    state_ = dlms::association::AssociationState::Associated;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationStatus Release()
  {
    state_ = dlms::association::AssociationState::Closed;
    return dlms::association::AssociationStatus::Ok;
  }

  dlms::association::AssociationState State() const
  {
    return state_;
  }

  bool IsAssociated() const
  {
    return state_ == dlms::association::AssociationState::Associated;
  }

  const dlms::association::AssociationResult& Result() const
  {
    return result_;
  }

private:
  dlms::association::AssociationState state_;
  dlms::association::AssociationResult result_;
};

bool SameName(
  const dlms::xdlms::CosemLogicalName& lhs,
  const dlms::xdlms::CosemLogicalName& rhs)
{
  for (std::size_t i = 0u; i < lhs.Size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

dlms::xdlms::CosemLogicalName XName(
  std::uint8_t a,
  std::uint8_t b,
  std::uint8_t c,
  std::uint8_t d,
  std::uint8_t e,
  std::uint8_t f)
{
  return dlms::xdlms::CosemLogicalName(a, b, c, d, e, f);
}

dlms::cosem::CosemLogicalName CName(
  std::uint8_t a,
  std::uint8_t b,
  std::uint8_t c,
  std::uint8_t d,
  std::uint8_t e,
  std::uint8_t f)
{
  return dlms::cosem::CosemLogicalName(a, b, c, d, e, f);
}

dlms::client::DlmsDateTime ExampleDateTime(std::uint8_t minute)
{
  dlms::client::DlmsDateTime value;
  value.date.year = 2024u;
  value.date.month = 6u;
  value.date.dayOfMonth = 13u;
  value.date.dayOfWeek = 5u;
  value.time.hour = 10u;
  value.time.minute = minute;
  value.time.second = 0u;
  value.time.hundredths = 0u;
  value.deviation = 180;
  value.clockStatus = 0u;
  return value;
}

dlms::cosem::CosemCaptureObject CaptureObject(
  std::uint16_t classId,
  const dlms::cosem::CosemLogicalName& logicalName,
  std::uint8_t attributeId)
{
  dlms::cosem::CosemCaptureObject object;
  object.object.classId = classId;
  object.object.version = 0u;
  object.object.logicalName = logicalName;
  object.attributeId = attributeId;
  object.dataIndex = 0u;
  return object;
}

dlms::cosem::CosemByteBuffer ProfileRow(
  const dlms::client::DlmsDateTime& timestamp,
  std::uint16_t value)
{
  dlms::cosem::CosemByteBuffer timestampBytes;
  dlms::cosem::CosemByteBuffer valueBytes;
  dlms::client::EncodeDlmsDateTime(timestamp, timestampBytes);
  dlms::client::EncodeDlmsLongUnsigned(value, valueBytes);

  dlms::cosem::CosemByteBuffer row;
  row.push_back(0x02u);
  row.push_back(0x02u);
  row.insert(row.end(), timestampBytes.begin(), timestampBytes.end());
  row.insert(row.end(), valueBytes.begin(), valueBytes.end());
  return row;
}

class ExampleXdlmsService : public dlms::client::IClientXdlmsService
{
public:
  dlms::xdlms::XdlmsStatus Get(
    const dlms::client::CosemAttributeDescriptor& descriptor,
    dlms::xdlms::GetResult& result)
  {
    result = dlms::xdlms::EmptyGetResult();
    result.invokeId = 1u;
    result.hasAccessResult = true;
    result.accessResult = 0u;

    if (descriptor.classId == 8u &&
        descriptor.attributeId == 2u &&
        SameName(descriptor.instanceId, XName(0, 0, 1, 0, 0, 255))) {
      result.hasData = true;
      dlms::client::EncodeDlmsDateTime(ExampleDateTime(15u), result.data);
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    if (descriptor.classId == 3u &&
        descriptor.attributeId == 2u &&
        SameName(descriptor.instanceId, XName(1, 0, 1, 8, 0, 255))) {
      result.hasData = true;
      dlms::client::EncodeDlmsLongUnsigned(12345u, result.data);
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    result.accessResult = 3u;
    return dlms::xdlms::XdlmsStatus::Ok;
  }

  dlms::xdlms::XdlmsStatus Get(
    const dlms::client::CosemAttributeDescriptor& descriptor,
    const dlms::client::SelectiveAccessDescriptor& selectiveAccess,
    dlms::xdlms::GetResult& result)
  {
    result = dlms::xdlms::EmptyGetResult();
    result.invokeId = 2u;
    result.hasAccessResult = true;
    result.accessResult = 0u;

    if (descriptor.classId != 7u ||
        descriptor.attributeId != 2u ||
        !SameName(descriptor.instanceId, XName(1, 0, 99, 1, 0, 255)) ||
        selectiveAccess.selector !=
          dlms::cosem::ProfileGenericRangeAccessSelector()) {
      result.accessResult = 3u;
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    dlms::cosem::CosemProfileGenericRangeDescriptor range;
    if (dlms::cosem::DecodeProfileGenericRangeDescriptor(
          selectiveAccess.encodedParameters,
          range) != dlms::cosem::CosemStatus::Ok ||
        range.selectedValues.empty()) {
      result.accessResult = 3u;
      return dlms::xdlms::XdlmsStatus::Ok;
    }

    std::vector<dlms::cosem::CosemByteBuffer> rows;
    rows.push_back(ProfileRow(ExampleDateTime(0u), 100u));
    rows.push_back(ProfileRow(ExampleDateTime(15u), 120u));
    result.hasData = true;
    result.data = dlms::cosem::EncodeProfileGenericBuffer(rows);
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
};

bool ResultOk(const dlms::client::ClientGetResult& result)
{
  return result.hasData &&
         (!result.hasAccessResult || result.accessResult == 0u);
}

} // namespace

int main()
{
  ExampleApduChannel channel;
  ExampleAssociation association;
  ExampleXdlmsService xdlms;
  dlms::client::DlmsClient client(channel, association, xdlms);

  if (client.Connect() != dlms::client::ClientStatus::Ok ||
      client.OpenAssociation() != dlms::client::ClientStatus::Ok) {
    return 1;
  }

  dlms::client::ClientGetResult result;
  if (client.ReadAttribute(8u, XName(0, 0, 1, 0, 0, 255), 2u, result) !=
      dlms::client::ClientStatus::Ok ||
      !ResultOk(result)) {
    return 1;
  }

  dlms::client::DlmsDateTime clock;
  if (dlms::client::DecodeDlmsDateTime(result.encodedData, clock) !=
      dlms::client::ClientStatus::Ok ||
      clock.date.year != 2024u ||
      clock.time.minute != 15u) {
    return 1;
  }

  if (client.ReadAttribute(3u, XName(1, 0, 1, 8, 0, 255), 2u, result) !=
      dlms::client::ClientStatus::Ok ||
      !ResultOk(result)) {
    return 1;
  }

  std::uint16_t activeEnergy = 0u;
  if (dlms::client::DecodeDlmsLongUnsigned(
        result.encodedData,
        activeEnergy) != dlms::client::ClientStatus::Ok ||
      activeEnergy != 12345u) {
    return 1;
  }

  dlms::cosem::CosemProfileGenericRangeDescriptor range;
  range.restrictingObject =
    CaptureObject(8u, CName(0, 0, 1, 0, 0, 255), 2u);
  dlms::client::EncodeDlmsDateTime(ExampleDateTime(0u), range.fromValue);
  dlms::client::EncodeDlmsDateTime(ExampleDateTime(30u), range.toValue);
  range.selectedValues.push_back(
    CaptureObject(3u, CName(1, 0, 1, 8, 0, 255), 2u));

  dlms::client::SelectiveAccessDescriptor selection =
    dlms::xdlms::EmptySelectiveAccessDescriptor();
  selection.selector = dlms::cosem::ProfileGenericRangeAccessSelector();
  selection.encodedParameters =
    dlms::cosem::EncodeProfileGenericRangeDescriptor(range);

  if (client.ReadAttribute(
        7u,
        XName(1, 0, 99, 1, 0, 255),
        2u,
        selection,
        result) != dlms::client::ClientStatus::Ok ||
      !ResultOk(result)) {
    return 1;
  }

  std::vector<dlms::cosem::CosemByteBuffer> rows;
  if (dlms::cosem::DecodeProfileGenericBuffer(result.encodedData, rows) !=
      dlms::cosem::CosemStatus::Ok ||
      rows.size() != 2u) {
    return 1;
  }

  if (client.ReleaseAssociation() != dlms::client::ClientStatus::Ok) {
    return 1;
  }
  return client.Close() == dlms::client::ClientStatus::Ok ? 0 : 1;
}
