#include "dlms/client/client_data.hpp"

#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"

#include <cstddef>

namespace dlms {
namespace client {
namespace {

constexpr std::size_t kMaximumDataDepth = 8u;

ClientStatus MapDataStatus(dlms::apdu::ApduStatus status)
{
  switch (status) {
  case dlms::apdu::ApduStatus::Ok:
    return ClientStatus::Ok;
  case dlms::apdu::ApduStatus::InvalidArgument:
  case dlms::apdu::ApduStatus::InvalidTag:
  case dlms::apdu::ApduStatus::InvalidData:
  case dlms::apdu::ApduStatus::InvalidLength:
  case dlms::apdu::ApduStatus::InvalidBer:
  case dlms::apdu::ApduStatus::InvalidAxdr:
  case dlms::apdu::ApduStatus::InvalidChoice:
  case dlms::apdu::ApduStatus::InvalidInvokeId:
  case dlms::apdu::ApduStatus::InvalidDescriptor:
  case dlms::apdu::ApduStatus::InvalidConformance:
  case dlms::apdu::ApduStatus::NeedMoreData:
  case dlms::apdu::ApduStatus::UnsupportedDataType:
  case dlms::apdu::ApduStatus::UnsupportedApdu:
  case dlms::apdu::ApduStatus::UnsupportedAcseField:
  case dlms::apdu::ApduStatus::UnsupportedXdlmsService:
  case dlms::apdu::ApduStatus::UnsupportedFeature:
    return ClientStatus::InvalidArgument;
  case dlms::apdu::ApduStatus::OutputBufferTooSmall:
  case dlms::apdu::ApduStatus::PduTooLarge:
  case dlms::apdu::ApduStatus::InternalError:
    return ClientStatus::InternalError;
  }

  return ClientStatus::InternalError;
}

ClientStatus EncodeData(
  const dlms::apdu::DlmsData& data,
  std::vector<std::uint8_t>& encodedData)
{
  encodedData.clear();
  std::vector<std::uint8_t> buffer(16u);
  for (;;) {
    dlms::apdu::ApduWriter writer(&buffer[0], buffer.size());
    const dlms::apdu::ApduStatus status =
      dlms::apdu::EncodeDlmsData(data, writer);
    if (status == dlms::apdu::ApduStatus::Ok) {
      encodedData.assign(buffer.begin(), buffer.begin() + writer.WrittenSize());
      return ClientStatus::Ok;
    }
    if (status != dlms::apdu::ApduStatus::OutputBufferTooSmall) {
      return MapDataStatus(status);
    }
    buffer.resize(buffer.size() * 2u);
  }
}

ClientStatus DecodeData(
  const std::vector<std::uint8_t>& encodedData,
  dlms::apdu::DlmsData& data)
{
  data = {};
  const std::uint8_t* input = encodedData.empty() ? 0 : &encodedData[0];
  return MapDataStatus(
    dlms::apdu::DecodeDlmsData(
      input,
      encodedData.size(),
      kMaximumDataDepth,
      data));
}

dlms::apdu::DlmsData EmptyData(dlms::apdu::DlmsDataType type)
{
  dlms::apdu::DlmsData data = {};
  data.type = type;
  return data;
}

ClientStatus DecodeType(
  const std::vector<std::uint8_t>& encodedData,
  dlms::apdu::DlmsDataType expectedType,
  dlms::apdu::DlmsData& data)
{
  const ClientStatus status = DecodeData(encodedData, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  if (data.type != expectedType) {
    data = {};
    return ClientStatus::InvalidArgument;
  }
  return ClientStatus::Ok;
}

} // namespace

ClientStatus EncodeDlmsBoolean(
  bool value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data = EmptyData(dlms::apdu::DlmsDataType::Boolean);
  data.booleanValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsBoolean(
  const std::vector<std::uint8_t>& encodedData,
  bool& value)
{
  value = false;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::Boolean, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = data.booleanValue;
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsInteger(
  std::int8_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data = EmptyData(dlms::apdu::DlmsDataType::Integer);
  data.signedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsInteger(
  const std::vector<std::uint8_t>& encodedData,
  std::int8_t& value)
{
  value = 0;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::Integer, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::int8_t>(data.signedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsLong(
  std::int16_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data = EmptyData(dlms::apdu::DlmsDataType::Long);
  data.signedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsLong(
  const std::vector<std::uint8_t>& encodedData,
  std::int16_t& value)
{
  value = 0;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::Long, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::int16_t>(data.signedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsDoubleLong(
  std::int32_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data = EmptyData(dlms::apdu::DlmsDataType::DoubleLong);
  data.signedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsDoubleLong(
  const std::vector<std::uint8_t>& encodedData,
  std::int32_t& value)
{
  value = 0;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::DoubleLong, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::int32_t>(data.signedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsUnsigned(
  std::uint8_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data = EmptyData(dlms::apdu::DlmsDataType::Unsigned);
  data.unsignedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsUnsigned(
  const std::vector<std::uint8_t>& encodedData,
  std::uint8_t& value)
{
  value = 0u;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::Unsigned, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::uint8_t>(data.unsignedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsLongUnsigned(
  std::uint16_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data =
    EmptyData(dlms::apdu::DlmsDataType::LongUnsigned);
  data.unsignedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsLongUnsigned(
  const std::vector<std::uint8_t>& encodedData,
  std::uint16_t& value)
{
  value = 0u;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::LongUnsigned, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::uint16_t>(data.unsignedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsDoubleLongUnsigned(
  std::uint32_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data =
    EmptyData(dlms::apdu::DlmsDataType::DoubleLongUnsigned);
  data.unsignedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsDoubleLongUnsigned(
  const std::vector<std::uint8_t>& encodedData,
  std::uint32_t& value)
{
  value = 0u;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(
      encodedData,
      dlms::apdu::DlmsDataType::DoubleLongUnsigned,
      data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::uint32_t>(data.unsignedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsEnum(
  std::uint8_t value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data = EmptyData(dlms::apdu::DlmsDataType::Enum);
  data.unsignedValue = value;
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsEnum(
  const std::vector<std::uint8_t>& encodedData,
  std::uint8_t& value)
{
  value = 0u;
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::Enum, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  value = static_cast<std::uint8_t>(data.unsignedValue);
  return ClientStatus::Ok;
}

ClientStatus EncodeDlmsOctetString(
  const std::vector<std::uint8_t>& value,
  std::vector<std::uint8_t>& encodedData)
{
  dlms::apdu::DlmsData data =
    EmptyData(dlms::apdu::DlmsDataType::OctetString);
  data.bytes.data = value.empty() ? 0 : &value[0];
  data.bytes.size = value.size();
  return EncodeData(data, encodedData);
}

ClientStatus DecodeDlmsOctetString(
  const std::vector<std::uint8_t>& encodedData,
  std::vector<std::uint8_t>& value)
{
  value.clear();
  dlms::apdu::DlmsData data = {};
  const ClientStatus status =
    DecodeType(encodedData, dlms::apdu::DlmsDataType::OctetString, data);
  if (status != ClientStatus::Ok) {
    return status;
  }
  if (data.bytes.size == 0u) {
    return ClientStatus::Ok;
  }
  value.assign(data.bytes.data, data.bytes.data + data.bytes.size);
  return ClientStatus::Ok;
}

} // namespace client
} // namespace dlms
