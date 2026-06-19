// Tests for IC 41 (TCP-UDP Setup) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 (2021) §4.9.1 / DLMS UA Blue Book Ed. 12.1 §4.9.1.
//
// All five dynamic attributes are typed (no CHOICE):
//   2 tcp_udp_port        : long-unsigned (0..65535)
//   3 ip_reference        : octet-string(6) = logical name of an IP setup
//   4 mss                 : long-unsigned, range [40, 65535], default 576
//   5 nb_of_sim_conn      : unsigned, min 1
//   6 inactivity_time_out : long-unsigned seconds, default 180 (0 = off)
//
// The class defines no methods; InvokeMethod always returns
// MethodNotFound.

#include <cstdint>
#include <initializer_list>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemTcpUdpSetupObject;

dlms::cosem::CosemLogicalName MakeName()
{
  // 0-0-25-0-0-255 is the canonical "TCP-UDP setup" instance logical
  // name in the Blue Book annex; the actual bytes are irrelevant for
  // these tests but the choice matches real-world data.
  return dlms::cosem::CosemLogicalName(0u, 0u, 25u, 0u, 0u, 255u);
}

dlms::cosem::CosemLogicalName MakeIpRef()
{
  // 0-0-25-1-0-255 is the canonical "IPv4 setup" instance logical name.
  return dlms::cosem::CosemLogicalName(0u, 0u, 25u, 1u, 0u, 255u);
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
                /*tcp_udp_port*/ 4059u,
                MakeIpRef(),
                /*mss*/ 576u,
                /*nb_of_sim_conn*/ 1u,
                /*inactivity_time_out*/ 180u,
                access);
}

}  // namespace

TEST(CosemTcpUdpSetupObject, DescriptorAndAccessRights)
{
  Object object = MakeObject();
  EXPECT_EQ(41u, object.Descriptor().key.classId);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);

  const auto rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id = 2u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id))
      << "id=" << static_cast<unsigned>(id);
  }
}

TEST(CosemTcpUdpSetupObject, ReadAttributeEncodesTypedAxdr)
{
  Object object = MakeObject();

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  // long-unsigned 4059 = 0x0FDB
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x0Fu, 0xDBu}), out);

  // ip_reference is encoded as a 6-byte octet-string (logical name).
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(EncodedLogicalName(MakeIpRef()), out);

  // long-unsigned 576 = 0x0240
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x02u, 0x40u}), out);

  // unsigned 1
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x01u}), out);

  // long-unsigned 180 = 0x00B4
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(BytesFromList({0x12u, 0x00u, 0xB4u}), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(0u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
}

TEST(CosemTcpUdpSetupObject, CtorClampsOutOfRangeInputs)
{
  Object object(MakeName(),
                /*tcp_udp_port*/ 0u,         // pass-through (no Blue Book min)
                MakeIpRef(),
                /*mss*/ 10u,                 // < 40 → normalized to default 576
                /*nb_of_sim_conn*/ 0u,       // < 1  → normalized to 1
                /*inactivity_time_out*/ 0u,  // pass-through (0 = disabled)
                dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(0u, object.TcpUdpPort());
  EXPECT_EQ(576u, object.Mss());
  EXPECT_EQ(1u, object.NbOfSimConn());
  EXPECT_EQ(0u, object.InactivityTimeOut());
}

TEST(CosemTcpUdpSetupObject, CtorAcceptsBoundaryValues)
{
  // mss = exactly 40 (lower bound)
  Object lo(MakeName(), 0u, MakeIpRef(), 40u, 1u, 0u,
            dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(40u, lo.Mss());

  // mss = 65535 (upper bound), nb_of_sim_conn = 255
  Object hi(MakeName(), 65535u, MakeIpRef(), 65535u, 255u, 65535u,
            dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(65535u, hi.TcpUdpPort());
  EXPECT_EQ(65535u, hi.Mss());
  EXPECT_EQ(255u, hi.NbOfSimConn());
  EXPECT_EQ(65535u, hi.InactivityTimeOut());
}

TEST(CosemTcpUdpSetupObject, WriteTcpUdpPortDecodesLongUnsigned)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0x10u, 0x00u})));
  EXPECT_EQ(0x1000u, object.TcpUdpPort());

  // wrong tag
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x11u, 0x01u})));
  // truncated
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, BytesFromList({0x12u, 0x10u})));
  // trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x12u, 0x10u, 0x00u, 0x00u})));
  // value preserved on rejection
  EXPECT_EQ(0x1000u, object.TcpUdpPort());
}

TEST(CosemTcpUdpSetupObject, WriteIpReferenceDecodesOctetString6)
{
  Object object = MakeObject();
  const dlms::cosem::CosemLogicalName newRef(0u, 0u, 25u, 2u, 0u, 255u);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, EncodedLogicalName(newRef)));
  EXPECT_EQ(newRef, object.IpReference());

  // wrong length (5 bytes instead of 6)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              3u, BytesFromList({0x09u, 0x05u, 0u, 0u, 25u, 2u, 0u})));
  // wrong tag
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              3u, BytesFromList({0x0Au, 0x06u, 0u, 0u, 25u, 2u, 0u, 255u})));
  // value preserved on rejection
  EXPECT_EQ(newRef, object.IpReference());
}

TEST(CosemTcpUdpSetupObject, WriteMssEnforcesRange)
{
  Object object = MakeObject();
  // 40 is the lower bound (accepted)
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x12u, 0x00u, 0x28u})));
  EXPECT_EQ(40u, object.Mss());

  // 39 is below the bound (rejected, value preserved)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x12u, 0x00u, 0x27u})));
  EXPECT_EQ(40u, object.Mss());

  // 0 is rejected
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x12u, 0x00u, 0x00u})));
  EXPECT_EQ(40u, object.Mss());

  // 65535 is accepted (upper bound = full u16)
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x12u, 0xFFu, 0xFFu})));
  EXPECT_EQ(65535u, object.Mss());

  // wrong tag → rejected
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x11u, 0x80u})));
  EXPECT_EQ(65535u, object.Mss());
}

TEST(CosemTcpUdpSetupObject, WriteNbOfSimConnEnforcesMinOne)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, BytesFromList({0x11u, 0x05u})));
  EXPECT_EQ(5u, object.NbOfSimConn());

  // 0 is rejected; value preserved
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, BytesFromList({0x11u, 0x00u})));
  EXPECT_EQ(5u, object.NbOfSimConn());

  // 255 is accepted (upper bound = full u8)
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, BytesFromList({0x11u, 0xFFu})));
  EXPECT_EQ(255u, object.NbOfSimConn());

  // wrong tag
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(5u, BytesFromList({0x12u, 0x00u, 0x05u})));
}

TEST(CosemTcpUdpSetupObject, WriteInactivityTimeOutDecodesLongUnsigned)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(6u, BytesFromList({0x12u, 0x00u, 0x00u})));
  EXPECT_EQ(0u, object.InactivityTimeOut());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(6u, BytesFromList({0x12u, 0x01u, 0x2Cu})));
  EXPECT_EQ(300u, object.InactivityTimeOut());

  // malformed → rejected, value preserved
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, BytesFromList({0x12u, 0x01u})));
  EXPECT_EQ(300u, object.InactivityTimeOut());
}

TEST(CosemTcpUdpSetupObject, WriteLogicalNameAlwaysDenied)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(MakeName())));
}

TEST(CosemTcpUdpSetupObject, WriteUnknownAttributeReportsNotFound)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(7u, BytesFromList({0x12u, 0x00u, 0x00u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(0u, BytesFromList({0x12u, 0x00u, 0x00u})));
}

TEST(CosemTcpUdpSetupObject, ReadOnlyAccessRejectsWrites)
{
  Object object = MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);
  for (std::uint8_t id = 2u; id <= 6u; ++id) {
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              object.WriteAttribute(id, BytesFromList({0x12u, 0x00u, 0x00u})))
      << "id=" << static_cast<unsigned>(id);
  }
}

TEST(CosemTcpUdpSetupObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  Object object = MakeObject();
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0xFFu);  // pre-fill to verify the call clears it
  for (std::uint8_t id : {0u, 1u, 2u, 255u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(static_cast<std::uint8_t>(id),
                                  dlms::cosem::CosemByteBuffer{}, out));
    EXPECT_TRUE(out.empty()) << "id=" << id;
  }
}

TEST(CosemTcpUdpSetupObject, VersionAboveMaxNormalized)
{
  Object object(MakeName(), 4059u, MakeIpRef(), 576u, 1u, 180u,
                dlms::cosem::AttributeAccessMode::ReadAndWrite,
                /*version*/ 99u);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);
}

TEST(CosemTcpUdpSetupObject, StaticValidators)
{
  EXPECT_FALSE(Object::IsValidMss(0u));
  EXPECT_FALSE(Object::IsValidMss(39u));
  EXPECT_TRUE(Object::IsValidMss(40u));
  EXPECT_TRUE(Object::IsValidMss(576u));
  EXPECT_TRUE(Object::IsValidMss(65535u));

  EXPECT_FALSE(Object::IsValidNbOfSimConn(0u));
  EXPECT_TRUE(Object::IsValidNbOfSimConn(1u));
  EXPECT_TRUE(Object::IsValidNbOfSimConn(255u));
}
