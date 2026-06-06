#include "dlms/security/suite0_key_wrap.hpp"

#include <gtest/gtest.h>

namespace {

dlms::security::SecurityKey MakeKey(
  dlms::security::SecurityKeyRole role,
  const std::uint8_t* bytes,
  std::size_t size)
{
  dlms::security::SecurityKey key =
    dlms::security::EmptySecurityKey(role);
  key.size = size;
  for (std::size_t i = 0u; i < size; ++i) {
    key.bytes[i] = bytes[i];
  }
  return key;
}

dlms::security::SecurityByteView View(const std::vector<std::uint8_t>& bytes)
{
  dlms::security::SecurityByteView view;
  view.data = bytes.empty() ? 0 : &bytes[0];
  view.size = bytes.size();
  return view;
}

std::vector<std::uint8_t> Bytes(const std::uint8_t* data, std::size_t size)
{
  return std::vector<std::uint8_t>(data, data + size);
}

} // namespace

TEST(Suite0KeyWrap, WrapsAndUnwrapsRfc3394Vector)
{
  const std::uint8_t kekBytes[] = {
    0x00u, 0x01u, 0x02u, 0x03u,
    0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu,
    0x0Cu, 0x0Du, 0x0Eu, 0x0Fu};
  const std::uint8_t plainBytes[] = {
    0x00u, 0x11u, 0x22u, 0x33u,
    0x44u, 0x55u, 0x66u, 0x77u,
    0x88u, 0x99u, 0xAAu, 0xBBu,
    0xCCu, 0xDDu, 0xEEu, 0xFFu};
  const std::uint8_t wrappedBytes[] = {
    0x1Fu, 0xA6u, 0x8Bu, 0x0Au,
    0x81u, 0x12u, 0xB4u, 0x47u,
    0xAEu, 0xF3u, 0x4Bu, 0xD8u,
    0xFBu, 0x5Au, 0x7Bu, 0x82u,
    0x9Du, 0x3Eu, 0x86u, 0x23u,
    0x71u, 0xD2u, 0xCFu, 0xE5u};

  const dlms::security::SecurityKey kek =
    MakeKey(
      dlms::security::SecurityKeyRole::KeyEncryption,
      kekBytes,
      sizeof(kekBytes));
  const std::vector<std::uint8_t> plain =
    Bytes(plainBytes, sizeof(plainBytes));
  const std::vector<std::uint8_t> expectedWrapped =
    Bytes(wrappedBytes, sizeof(wrappedBytes));

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(kek, View(plain), wrapped));
  EXPECT_EQ(expectedWrapped, wrapped);

  std::vector<std::uint8_t> unwrapped;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Unwrap(kek, View(wrapped), unwrapped));
  EXPECT_EQ(plain, unwrapped);
}

TEST(Suite0KeyWrap, RejectsInvalidSizes)
{
  const std::uint8_t kekBytes[16] = {};
  const dlms::security::SecurityKey kek =
    MakeKey(
      dlms::security::SecurityKeyRole::KeyEncryption,
      kekBytes,
      sizeof(kekBytes));
  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> output(1u, 0xAAu);

  std::vector<std::uint8_t> tooSmallPlain(15u, 0x11u);
  EXPECT_EQ(dlms::security::SecurityStatus::InvalidArgument,
            keyWrap.Wrap(kek, View(tooSmallPlain), output));
  EXPECT_TRUE(output.empty());

  std::vector<std::uint8_t> tooSmallWrapped(23u, 0x11u);
  output.push_back(0xAAu);
  EXPECT_EQ(dlms::security::SecurityStatus::InvalidArgument,
            keyWrap.Unwrap(kek, View(tooSmallWrapped), output));
  EXPECT_TRUE(output.empty());
}

TEST(Suite0KeyWrap, RejectsTamperedWrappedKey)
{
  const std::uint8_t kekBytes[16] = {};
  const dlms::security::SecurityKey kek =
    MakeKey(
      dlms::security::SecurityKeyRole::KeyEncryption,
      kekBytes,
      sizeof(kekBytes));
  std::vector<std::uint8_t> plain(16u, 0x22u);

  dlms::security::Suite0KeyWrap keyWrap;
  std::vector<std::uint8_t> wrapped;
  ASSERT_EQ(dlms::security::SecurityStatus::Ok,
            keyWrap.Wrap(kek, View(plain), wrapped));

  wrapped[0] ^= 0x01u;
  std::vector<std::uint8_t> unwrapped(1u, 0xAAu);
  EXPECT_EQ(dlms::security::SecurityStatus::AuthenticationFailed,
            keyWrap.Unwrap(kek, View(wrapped), unwrapped));
  EXPECT_TRUE(unwrapped.empty());
}
