// Tests for IC 23 (IEC HDLC Setup) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.7.2.2.2 / DLMS UA Blue Book Ed. 12.1.
//
// All seven dynamic attributes are typed:
//   comm_speed                       (id 2) : enum 0..9 (Baud300..Baud115200)
//   window_size_transmit             (id 3) : unsigned 1..7
//   window_size_receive              (id 4) : unsigned 1..7
//   max_info_field_length_transmit   (id 5) : long-unsigned 32..2030
//   max_info_field_length_receive    (id 6) : long-unsigned 32..2030
//   inter_octet_time_out             (id 7) : long-unsigned 20..6000 (ms)
//   inactivity_time_out              (id 8) : long-unsigned (s), 0 = disabled
//   device_address                   (id 9) : long-unsigned 0x0010..0x3FFD
//                                             (read-only; SetDeviceAddress)

#include <cstdint>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemIecHdlcSetupObject;

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 22u, 0u, 0u, 255u);
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

Object MakeObject(
  dlms::cosem::AttributeAccessMode access =
    dlms::cosem::AttributeAccessMode::ReadAndWrite)
{
  return Object(MakeName(),
                Object::CommSpeed::Baud9600,
                /*window_tx*/ 1u,
                /*window_rx*/ 1u,
                /*max_info_tx*/ 128u,
                /*max_info_rx*/ 128u,
                /*inter_octet*/ 25u,
                /*inactivity*/ 120u,
                /*device_address*/ 0x0010u,
                access);
}

}  // namespace

TEST(CosemIecHdlcSetupObject, DescriptorAndAccessRights)
{
  Object object = MakeObject();
  EXPECT_EQ(23u, object.Descriptor().key.classId);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);

  const auto rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 8u; ++id) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "id=" << static_cast<unsigned>(id);
  }
  // device_address is always read-only; SetDeviceAddress is the path.
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(9u));
}

TEST(CosemIecHdlcSetupObject, ReadAttributeEncodesTypedAxdr)
{
  Object object = MakeObject();

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  // enum 5 (Baud9600)
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x05u}), out);
  // unsigned 1
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x01u}), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x01u}), out);
  // long-unsigned 128
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x80u}), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x80u}), out);
  // long-unsigned 25 ms
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(7u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x19u}), out);
  // long-unsigned 120 s
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(8u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x78u}), out);
  // long-unsigned 0x0010
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0x10u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(0u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(10u, out));
}

TEST(CosemIecHdlcSetupObject, CtorClampsAndNormalizesOutOfRangeInputs)
{
  Object object(MakeName(),
                static_cast<Object::CommSpeed>(42u),  // → Baud9600
                /*window_tx*/ 0u,                     // → 1
                /*window_rx*/ 99u,                    // → 7
                /*max_info_tx*/ 10u,                  // → 32
                /*max_info_rx*/ 9999u,                // → 2030
                /*inter_octet*/ 0u,                   // → 20
                /*inactivity*/ 0u,                    // pass-through (0 = off)
                /*device_address*/ 0x0001u,           // → 0x0010
                dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(Object::CommSpeed::Baud9600, object.GetCommSpeed());
  EXPECT_EQ(1u, object.WindowSizeTransmit());
  EXPECT_EQ(7u, object.WindowSizeReceive());
  EXPECT_EQ(32u, object.MaxInfoFieldLengthTransmit());
  EXPECT_EQ(2030u, object.MaxInfoFieldLengthReceive());
  EXPECT_EQ(20u, object.InterOctetTimeOut());
  EXPECT_EQ(0u, object.InactivityTimeOut());
  EXPECT_EQ(0x0010u, object.DeviceAddress());
}

TEST(CosemIecHdlcSetupObject, WriteCommSpeedAcceptsValidEnumOnly)
{
  Object object = MakeObject();
  for (std::uint8_t raw = 0u; raw <= 9u; ++raw) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.WriteAttribute(2u, BytesFromList({0x16u, raw})))
      << "raw=" << static_cast<unsigned>(raw);
    EXPECT_EQ(static_cast<std::uint8_t>(object.GetCommSpeed()), raw);
  }
  // out-of-range enum (10..255) → InvalidArgument
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 10u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0xFFu})));
  // wrong tag
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u, 0x05u})));
  // trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x16u, 0x05u, 0x00u})));
}

TEST(CosemIecHdlcSetupObject, WriteWindowSizeAcceptsRange1To7)
{
  Object object = MakeObject();
  for (std::uint8_t id : {3u, 4u}) {
    for (std::uint8_t raw = 1u; raw <= 7u; ++raw) {
      EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
                object.WriteAttribute(id, BytesFromList({0x11u, raw})))
        << "id=" << static_cast<unsigned>(id)
        << " raw=" << static_cast<unsigned>(raw);
    }
    // 0 and 8 → InvalidArgument
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x11u, 0x00u})));
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x11u, 0x08u})));
    // wrong tag (enum instead of unsigned)
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x16u, 0x01u})));
  }
}

TEST(CosemIecHdlcSetupObject, WriteMaxInfoFieldAcceptsRange32To2030)
{
  Object object = MakeObject();
  for (std::uint8_t id : {5u, 6u}) {
    // lower bound
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.WriteAttribute(id, BytesFromList({0x12u, 0x00u, 0x20u})));
    // upper bound 2030 = 0x07EE
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.WriteAttribute(id, BytesFromList({0x12u, 0x07u, 0xEEu})));
    // below
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x12u, 0x00u, 0x1Fu})));
    // above (2031 = 0x07EF)
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x12u, 0x07u, 0xEFu})));
    // wrong tag
    EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
              object.WriteAttribute(id, BytesFromList({0x11u, 0x80u})));
  }
  EXPECT_EQ(2030u, object.MaxInfoFieldLengthTransmit());
  EXPECT_EQ(2030u, object.MaxInfoFieldLengthReceive());
}

TEST(CosemIecHdlcSetupObject, WriteInterOctetTimeOutAcceptsRange20To6000)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(7u, BytesFromList({0x12u, 0x00u, 0x14u})));
  // 6000 = 0x1770
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(7u, BytesFromList({0x12u, 0x17u, 0x70u})));
  EXPECT_EQ(6000u, object.InterOctetTimeOut());
  // 19 below, 6001 above
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(7u, BytesFromList({0x12u, 0x00u, 0x13u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(7u, BytesFromList({0x12u, 0x17u, 0x71u})));
}

TEST(CosemIecHdlcSetupObject, WriteInactivityTimeOutAcceptsFullRange)
{
  Object object = MakeObject();
  // 0 means disabled per Blue Book
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(8u, BytesFromList({0x12u, 0x00u, 0x00u})));
  EXPECT_EQ(0u, object.InactivityTimeOut());
  // 65535
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(8u, BytesFromList({0x12u, 0xFFu, 0xFFu})));
  EXPECT_EQ(0xFFFFu, object.InactivityTimeOut());
  // wrong tag
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(8u, BytesFromList({0x11u, 0x10u})));
}

TEST(CosemIecHdlcSetupObject, ReadOnlyAttributesRejectWrites)
{
  Object object = MakeObject();
  const dlms::cosem::CosemByteBuffer payload =
    BytesFromList({0x12u, 0x01u, 0x00u});
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, payload));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(9u, payload));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, payload));
}

TEST(CosemIecHdlcSetupObject, ReadOnlyCallerCannotWriteMutableAttributes)
{
  Object readOnly =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 8u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(id, BytesFromList({0x16u, 0x05u})))
      << "id=" << static_cast<unsigned>(id);
  }
  EXPECT_EQ(Object::CommSpeed::Baud9600, readOnly.GetCommSpeed());
  EXPECT_EQ(120u, readOnly.InactivityTimeOut());
}

TEST(CosemIecHdlcSetupObject, NoMethodsDefined)
{
  Object object = MakeObject();
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {1u, 2u, 3u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(method, in, out))
      << "method=" << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemIecHdlcSetupObject, SetDeviceAddressValidatesRange)
{
  Object object = MakeObject();
  EXPECT_TRUE(object.SetDeviceAddress(0x0010u));
  EXPECT_EQ(0x0010u, object.DeviceAddress());
  EXPECT_TRUE(object.SetDeviceAddress(0x3FFDu));
  EXPECT_EQ(0x3FFDu, object.DeviceAddress());

  EXPECT_FALSE(object.SetDeviceAddress(0x000Fu));
  EXPECT_EQ(0x3FFDu, object.DeviceAddress());
  EXPECT_FALSE(object.SetDeviceAddress(0x3FFEu));
  EXPECT_EQ(0x3FFDu, object.DeviceAddress());
  EXPECT_FALSE(object.SetDeviceAddress(0xFFFFu));
  EXPECT_EQ(0x3FFDu, object.DeviceAddress());

  // device_address read still reflects current value.
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(9u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x3Fu, 0xFDu}), out);
}

TEST(CosemIecHdlcSetupObject, IsValidStaticHelpers)
{
  for (std::uint8_t raw = 0u; raw <= 9u; ++raw)
    EXPECT_TRUE(Object::IsValidCommSpeed(raw))
      << "raw=" << static_cast<unsigned>(raw);
  EXPECT_FALSE(Object::IsValidCommSpeed(10u));
  EXPECT_FALSE(Object::IsValidCommSpeed(0xFFu));

  for (std::uint8_t raw = 1u; raw <= 7u; ++raw)
    EXPECT_TRUE(Object::IsValidWindowSize(raw));
  EXPECT_FALSE(Object::IsValidWindowSize(0u));
  EXPECT_FALSE(Object::IsValidWindowSize(8u));

  EXPECT_TRUE(Object::IsValidMaxInfoFieldLength(32u));
  EXPECT_TRUE(Object::IsValidMaxInfoFieldLength(2030u));
  EXPECT_FALSE(Object::IsValidMaxInfoFieldLength(31u));
  EXPECT_FALSE(Object::IsValidMaxInfoFieldLength(2031u));

  EXPECT_TRUE(Object::IsValidInterOctetTimeOut(20u));
  EXPECT_TRUE(Object::IsValidInterOctetTimeOut(6000u));
  EXPECT_FALSE(Object::IsValidInterOctetTimeOut(19u));
  EXPECT_FALSE(Object::IsValidInterOctetTimeOut(6001u));

  EXPECT_TRUE(Object::IsValidDeviceAddress(0x0010u));
  EXPECT_TRUE(Object::IsValidDeviceAddress(0x3FFDu));
  EXPECT_FALSE(Object::IsValidDeviceAddress(0x000Fu));
  EXPECT_FALSE(Object::IsValidDeviceAddress(0x3FFEu));
}

TEST(CosemIecHdlcSetupObject, NormalizesVersionAboveMax)
{
  Object object(MakeName(),
                Object::CommSpeed::Baud9600,
                1u, 1u, 128u, 128u, 25u, 120u, 0x0010u,
                dlms::cosem::AttributeAccessMode::ReadAndWrite,
                /*version*/ 99u);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);
}
