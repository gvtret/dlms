// Tests for IC 61 (Register Table) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 §4.3.7 / DLMS UA Blue Book Ed. 12.1 §4.3.7.
//
// `scaler_unit` (attribute 4) is now typed (`types::ScalerUnit`);
// `table_cell_values` (2) and `table_cell_definition` (3) remain
// opaque AXDR buffers because their column-schema is meter-specific
// and depends on the wired Register / Extended Register / Demand
// Register entries.

#include <cstdint>
#include <vector>

#include "dlms/cosem/simple_objects.hpp"
#include "dlms/cosem/types/scaler_unit.hpp"

#include <gtest/gtest.h>

namespace {

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 61u, 0u, 0u, 255u);
}

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  dlms::cosem::CosemByteBuffer out;
  out.reserve(bytes.size());
  for (std::uint8_t b : bytes) out.push_back(b);
  return out;
}

dlms::cosem::CosemByteBuffer EncodedLogicalName(
  const dlms::cosem::CosemLogicalName& name)
{
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x09u);
  bytes.push_back(0x06u);
  for (std::size_t i = 0u; i < name.Size(); ++i) bytes.push_back(name[i]);
  return bytes;
}

dlms::cosem::CosemByteBuffer SampleTableCellValues()
{
  // array(2){ array(1){long-unsigned 10}, array(1){long-unsigned 20} }
  return BytesFromList({
    0x01u, 0x02u,
      0x01u, 0x01u, 0x12u, 0x00u, 0x0Au,
      0x01u, 0x01u, 0x12u, 0x00u, 0x14u});
}

dlms::cosem::CosemByteBuffer SampleTableCellDefinition()
{
  // structure(3){ long-unsigned 3 (Register), octet-string 6, integer 2 }
  return BytesFromList({
    0x02u, 0x03u,
      0x12u, 0x00u, 0x03u,
      0x09u, 0x06u, 0x01u, 0x00u, 0x01u, 0x08u, 0x00u, 0xFFu,
      0x0Fu, 0x02u});
}

dlms::cosem::CosemByteBuffer EncodedScalerUnit(
  std::int8_t scaler,
  std::uint8_t unit)
{
  // structure(2){ integer scaler, enum unit }
  dlms::cosem::CosemByteBuffer bytes;
  bytes.push_back(0x02u);
  bytes.push_back(0x02u);
  bytes.push_back(0x0Fu);
  bytes.push_back(static_cast<std::uint8_t>(scaler));
  bytes.push_back(0x16u);
  bytes.push_back(unit);
  return bytes;
}

dlms::cosem::CosemRegisterTableObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemRegisterTableObject(
    MakeName(),
    SampleTableCellValues(),
    SampleTableCellDefinition(),
    dlms::cosem::types::ScalerUnit(0, 30u),   // 30 = Wh
    access);
}

}  // namespace

TEST(CosemRegisterTableObject, ExposesAllAttributesWithTypedScalerUnit)
{
  dlms::cosem::CosemRegisterTableObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(61u, object.Descriptor().key.classId);
  EXPECT_EQ(dlms::cosem::CosemRegisterTableObject::MaxSupportedVersion,
            object.Descriptor().key.version);
  EXPECT_EQ(0, object.ScalerUnit().Scaler());
  EXPECT_EQ(30u, object.ScalerUnit().Unit());

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(SampleTableCellValues(), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(SampleTableCellDefinition(), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedScalerUnit(0, 30u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(0u, out));
}

TEST(CosemRegisterTableObject, WriteScalerUnitParsesAxdrStructure)
{
  dlms::cosem::CosemRegisterTableObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // scaler = -3, unit = 27 (W)
  const dlms::cosem::CosemByteBuffer input = EncodedScalerUnit(-3, 27u);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.WriteAttribute(4u, input));
  EXPECT_EQ(-3, object.ScalerUnit().Scaler());
  EXPECT_EQ(27u, object.ScalerUnit().Unit());

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(input, out);
}

TEST(CosemRegisterTableObject, WriteScalerUnitRejectsMalformedInput)
{
  dlms::cosem::CosemRegisterTableObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::types::ScalerUnit original = object.ScalerUnit();

  // empty
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, dlms::cosem::CosemByteBuffer()));
  // wrong tag (array instead of structure)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u,
              BytesFromList({0x01u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu})));
  // wrong field count
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x02u, 0x01u, 0x0Fu, 0x00u})));
  // trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u,
              BytesFromList({0x02u, 0x02u, 0x0Fu, 0x00u, 0x16u, 0x1Eu, 0xFFu})));

  EXPECT_EQ(original.Scaler(), object.ScalerUnit().Scaler());
  EXPECT_EQ(original.Unit(), object.ScalerUnit().Unit());
}

TEST(CosemRegisterTableObject, WriteTableCellDefinitionAcceptedReadOnlyRejected)
{
  dlms::cosem::CosemRegisterTableObject writable =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  const dlms::cosem::CosemByteBuffer replacement =
    BytesFromList({0x12u, 0x00u, 0x05u});

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            writable.WriteAttribute(3u, replacement));
  EXPECT_EQ(replacement, writable.TableCellDefinition());

  dlms::cosem::CosemRegisterTableObject readOnly =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(3u, replacement));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(4u, EncodedScalerUnit(-1, 27u)));
  EXPECT_EQ(SampleTableCellDefinition(), readOnly.TableCellDefinition());
  EXPECT_EQ(0, readOnly.ScalerUnit().Scaler());
  EXPECT_EQ(30u, readOnly.ScalerUnit().Unit());
}

TEST(CosemRegisterTableObject, WriteAttributeRejectsLogicalNameAndValues)
{
  dlms::cosem::CosemRegisterTableObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer input =
    BytesFromList({0x12u, 0x00u, 0x05u});
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, input));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, input));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, input));
}

TEST(CosemRegisterTableObject, SetTableCellValuesUpdatesReadResult)
{
  dlms::cosem::CosemRegisterTableObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer refreshed =
    BytesFromList({0x01u, 0x01u, 0x01u, 0x01u, 0x12u, 0x01u, 0x00u});
  object.SetTableCellValues(refreshed);
  EXPECT_EQ(refreshed, object.TableCellValues());

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(refreshed, out);
}

TEST(CosemRegisterTableObject, MethodsReturnUnsupportedFeatureOrNotFound)
{
  dlms::cosem::CosemRegisterTableObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in =
    BytesFromList({0x12u, 0x00u, 0x01u});

  for (std::uint8_t methodId : {1u, 2u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
              object.InvokeMethod(methodId, in, out))
      << "method id " << static_cast<unsigned>(methodId);
    EXPECT_TRUE(out.empty());
  }
  for (std::uint8_t methodId : {0u, 3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(methodId, in, out))
      << "method id " << static_cast<unsigned>(methodId);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemRegisterTableObject, NormalizesVersionAboveMax)
{
  dlms::cosem::CosemRegisterTableObject object(
    MakeName(),
    SampleTableCellValues(),
    SampleTableCellDefinition(),
    dlms::cosem::types::ScalerUnit(0, 30u),
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    99u);
  EXPECT_EQ(dlms::cosem::CosemRegisterTableObject::MaxSupportedVersion,
            object.Descriptor().key.version);
}
