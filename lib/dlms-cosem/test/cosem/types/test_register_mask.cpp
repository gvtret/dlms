// SPDX-License-Identifier: BSD-2-Clause
#include "dlms/cosem/types/register_mask.hpp"

#include <gtest/gtest.h>

namespace dlms::cosem::types {
namespace {

CosemByteBuffer Bytes(std::initializer_list<std::uint8_t> bytes)
{
  return CosemByteBuffer(bytes.begin(), bytes.end());
}

TEST(RegisterMask, DefaultsToEmptyNameAndIndices)
{
  RegisterMask m;
  EXPECT_TRUE(m.MaskName().empty());
  EXPECT_TRUE(m.IndexList().empty());
}

TEST(RegisterMask, ConstructorStoresFields)
{
  RegisterMask m(Bytes({0x41u, 0x42u}), {1u, 2u, 3u});
  EXPECT_EQ(m.MaskName(), Bytes({0x41u, 0x42u}));
  EXPECT_EQ(m.IndexList(), (std::vector<std::uint16_t>{1u, 2u, 3u}));
}

TEST(RegisterMask, MutatorsReplaceFields)
{
  RegisterMask m;
  m.SetMaskName(Bytes({0x01u}));
  m.SetIndexList({5u, 7u});
  EXPECT_EQ(m.MaskName(), Bytes({0x01u}));
  EXPECT_EQ(m.IndexList(), (std::vector<std::uint16_t>{5u, 7u}));
}

TEST(RegisterMask, EqualityComparesBothFields)
{
  RegisterMask a(Bytes({0x01u}), {1u, 2u});
  RegisterMask b(Bytes({0x01u}), {1u, 2u});
  RegisterMask c(Bytes({0x02u}), {1u, 2u});
  RegisterMask d(Bytes({0x01u}), {1u, 3u});
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);
}

}  // namespace
}  // namespace dlms::cosem::types
