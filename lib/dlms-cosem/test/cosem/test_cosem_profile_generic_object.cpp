// Tests for IC 7 (Profile Generic) — focused, per-IC file (rule P2.4).
//
// Scope: behaviour of the CosemProfileGenericObject built-in.
// AXDR codec round-trips for capture_objects / buffer / range /
// entry descriptors live in test_simple_objects.cpp until they
// migrate alongside the typed-value refactor of IC 7.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

dlms::cosem::CosemLogicalName MakeName(std::uint8_t c)
{
  return dlms::cosem::CosemLogicalName(1u, 0u, c, 8u, 0u, 255u);
}

dlms::cosem::CosemByteBuffer MakeRow(std::uint8_t seed)
{
  // structure { unsigned: seed }
  dlms::cosem::CosemByteBuffer row;
  row.push_back(0x02u);   // structure
  row.push_back(0x01u);   // length = 1
  row.push_back(0x11u);   // unsigned
  row.push_back(seed);
  return row;
}

dlms::cosem::CosemCaptureObject MakeCapture()
{
  dlms::cosem::CosemCaptureObject capture;
  capture.object.classId = 3u;
  capture.object.version = 0u;
  capture.object.logicalName = MakeName(3u);
  capture.attributeId = 2u;
  capture.dataIndex = 0u;
  return capture;
}

void AppendDoubleLongUnsigned(
  dlms::cosem::CosemByteBuffer& bytes,
  std::uint32_t value)
{
  bytes.push_back(0x06u);
  bytes.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
  bytes.push_back(static_cast<std::uint8_t>(value & 0xffu));
}

std::uint32_t ReadEntriesInUse(
  const dlms::cosem::CosemProfileGenericObject& object)
{
  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.ReadAttribute(7u, output));
  dlms::cosem::CosemByteBuffer expected;
  AppendDoubleLongUnsigned(expected, 0u);
  // Decode the trailing 4 bytes of the double-long-unsigned tag.
  if (output.size() != 5u || output[0] != 0x06u) {
    return 0xFFFFFFFFu;
  }
  return (static_cast<std::uint32_t>(output[1]) << 24)
       | (static_cast<std::uint32_t>(output[2]) << 16)
       | (static_cast<std::uint32_t>(output[3]) << 8)
       |  static_cast<std::uint32_t>(output[4]);
}

dlms::cosem::CosemProfileGenericObject MakeObject(
  const std::vector<dlms::cosem::CosemByteBuffer>& rows)
{
  std::vector<dlms::cosem::CosemCaptureObject> captures;
  captures.push_back(MakeCapture());
  return dlms::cosem::CosemProfileGenericObject(
    MakeName(7u),
    rows,
    captures,
    60u,
    100u);
}

}  // namespace

TEST(CosemProfileGenericObject, ResetClearsBufferAndReportsZeroEntries)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  rows.push_back(MakeRow(0x10u));
  rows.push_back(MakeRow(0x11u));
  rows.push_back(MakeRow(0x12u));

  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);
  ASSERT_EQ(3u, object.BufferRows().size());
  ASSERT_EQ(3u, ReadEntriesInUse(object));

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  methodOutput.push_back(0xFFu);  // pre-populated to verify clear

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(1u, methodInput, methodOutput));
  EXPECT_TRUE(methodOutput.empty());
  EXPECT_TRUE(object.BufferRows().empty());
  EXPECT_EQ(0u, ReadEntriesInUse(object));
}

TEST(CosemProfileGenericObject, ResetIsIdempotentOnEmptyBuffer)
{
  std::vector<dlms::cosem::CosemByteBuffer> empty;
  dlms::cosem::CosemProfileGenericObject object = MakeObject(empty);
  ASSERT_TRUE(object.BufferRows().empty());

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(1u, methodInput, methodOutput));
  EXPECT_TRUE(object.BufferRows().empty());
  EXPECT_EQ(0u, ReadEntriesInUse(object));
}

TEST(CosemProfileGenericObject, CaptureMethodIsUnsupportedFeature)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  rows.push_back(MakeRow(0x20u));
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  methodOutput.push_back(0xAAu);

  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, methodInput, methodOutput));
  EXPECT_TRUE(methodOutput.empty());
  // Buffer untouched by capture() hook.
  EXPECT_EQ(1u, object.BufferRows().size());
  EXPECT_EQ(1u, ReadEntriesInUse(object));
}

TEST(CosemProfileGenericObject, GetBufferByRangeIsUnsupportedFeature)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  methodOutput.push_back(0xBBu);

  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(3u, methodInput, methodOutput));
  EXPECT_TRUE(methodOutput.empty());
}

TEST(CosemProfileGenericObject, GetBufferByIndexIsUnsupportedFeature)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  methodOutput.push_back(0xCCu);

  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(4u, methodInput, methodOutput));
  EXPECT_TRUE(methodOutput.empty());
}

TEST(CosemProfileGenericObject, UnknownMethodReturnsMethodNotFound)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  methodOutput.push_back(0xDDu);

  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(5u, methodInput, methodOutput));
  EXPECT_TRUE(methodOutput.empty());

  EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
            object.InvokeMethod(0u, methodInput, methodOutput));
}

TEST(CosemProfileGenericObject, EntriesInUseTracksBufferSize)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  for (std::uint8_t i = 0u; i < 7u; ++i) {
    rows.push_back(MakeRow(static_cast<std::uint8_t>(0x30u + i)));
  }
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);
  EXPECT_EQ(7u, ReadEntriesInUse(object));

  dlms::cosem::CosemByteBuffer methodInput;
  dlms::cosem::CosemByteBuffer methodOutput;
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(1u, methodInput, methodOutput));
  EXPECT_EQ(0u, ReadEntriesInUse(object));
}

TEST(CosemProfileGenericObject, WriteAttributeReturnsAccessDeniedForKnownIds)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);

  dlms::cosem::CosemByteBuffer input;
  for (std::uint8_t attr = 1u; attr <= 8u; ++attr) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              object.WriteAttribute(attr, input))
      << "attribute id " << static_cast<int>(attr);
  }
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(9u, input));
}

TEST(CosemProfileGenericObject, ReadAttributeRejectsUnknownIds)
{
  std::vector<dlms::cosem::CosemByteBuffer> rows;
  dlms::cosem::CosemProfileGenericObject object = MakeObject(rows);

  dlms::cosem::CosemByteBuffer output;
  output.push_back(0xEEu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(9u, output));
  EXPECT_TRUE(output.empty());

  output.push_back(0xEEu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(0u, output));
  EXPECT_TRUE(output.empty());
}
