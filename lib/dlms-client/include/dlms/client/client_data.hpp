#pragma once

#include "dlms/client/client_status.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace client {

ClientStatus EncodeDlmsBoolean(
  bool value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsBoolean(
  const std::vector<std::uint8_t>& encodedData,
  bool& value);

ClientStatus EncodeDlmsInteger(
  std::int8_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsInteger(
  const std::vector<std::uint8_t>& encodedData,
  std::int8_t& value);

ClientStatus EncodeDlmsLong(
  std::int16_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsLong(
  const std::vector<std::uint8_t>& encodedData,
  std::int16_t& value);

ClientStatus EncodeDlmsDoubleLong(
  std::int32_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsDoubleLong(
  const std::vector<std::uint8_t>& encodedData,
  std::int32_t& value);

ClientStatus EncodeDlmsUnsigned(
  std::uint8_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsUnsigned(
  const std::vector<std::uint8_t>& encodedData,
  std::uint8_t& value);

ClientStatus EncodeDlmsLongUnsigned(
  std::uint16_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsLongUnsigned(
  const std::vector<std::uint8_t>& encodedData,
  std::uint16_t& value);

ClientStatus EncodeDlmsDoubleLongUnsigned(
  std::uint32_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsDoubleLongUnsigned(
  const std::vector<std::uint8_t>& encodedData,
  std::uint32_t& value);

ClientStatus EncodeDlmsEnum(
  std::uint8_t value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsEnum(
  const std::vector<std::uint8_t>& encodedData,
  std::uint8_t& value);

ClientStatus EncodeDlmsOctetString(
  const std::vector<std::uint8_t>& value,
  std::vector<std::uint8_t>& encodedData);
ClientStatus DecodeDlmsOctetString(
  const std::vector<std::uint8_t>& encodedData,
  std::vector<std::uint8_t>& value);

} // namespace client
} // namespace dlms
