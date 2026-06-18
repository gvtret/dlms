// Tests for IC 70 (Disconnect Control) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 §4.5.8 / DLMS UA Blue Book Ed. 12.1 §4.5.8.
//
// All three dynamic attributes are typed:
//   output_state   (id 2) : bool
//   control_state  (id 3) : enum {Disconnected=0, Connected=1,
//                                  ReadyForReconnection=2}
//   control_mode   (id 4) : enum {Mode0..Mode6}
//
// Methods 1 (remote_disconnect) and 2 (remote_reconnect) implement the
// state-machine transitions described in §4.5.8.3 directly inside the
// built-in object; backends own only the physical relay (via the
// `SetOutputState` / `SetControlState` hooks).

#include <cstdint>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemDisconnectControlObject;

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 96u, 3u, 10u, 255u);
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
  bool outputState,
  Object::ControlState controlState,
  Object::ControlMode controlMode,
  dlms::cosem::AttributeAccessMode controlModeAccess =
    dlms::cosem::AttributeAccessMode::ReadAndWrite)
{
  return Object(MakeName(), outputState, controlState, controlMode,
                controlModeAccess);
}

}  // namespace

TEST(CosemDisconnectControlObject, DescriptorAndAccessRights)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);

  EXPECT_EQ(70u, object.Descriptor().key.classId);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);

  const auto rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly, rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly, rights.AttributeAccess(2u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly, rights.AttributeAccess(3u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(4u));
}

TEST(CosemDisconnectControlObject, ReadAttributeEncodesTypedAxdr)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x03u, 0x01u}), out);  // boolean true

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x01u}), out);  // enum Connected=1

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x02u}), out);  // enum Mode2

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(5u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(0u, out));
}

TEST(CosemDisconnectControlObject, OutputStateFalseEncodesAsBooleanZero)
{
  Object object = MakeObject(false, Object::ControlState::Disconnected,
                             Object::ControlMode::Mode0);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x03u, 0x00u}), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x00u}), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x00u}), out);
}

TEST(CosemDisconnectControlObject, WriteControlModeParsesAndValidatesEnum)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);

  for (std::uint8_t raw = 0u; raw <= 6u; ++raw) {
    const dlms::cosem::CosemByteBuffer input = BytesFromList({0x16u, raw});
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.WriteAttribute(4u, input))
      << "raw=" << static_cast<unsigned>(raw);
    EXPECT_EQ(static_cast<std::uint8_t>(object.GetControlMode()), raw);
  }

  // out-of-range enum (7..255) → InvalidArgument, stored value preserved.
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0x03u})));
  ASSERT_EQ(Object::ControlMode::Mode3, object.GetControlMode());

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0x07u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0xFFu})));
  EXPECT_EQ(Object::ControlMode::Mode3, object.GetControlMode());
}

TEST(CosemDisconnectControlObject, WriteControlModeRejectsMalformedAxdr)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);
  // empty
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, dlms::cosem::CosemByteBuffer()));
  // wrong tag (boolean instead of enum)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x03u, 0x01u})));
  // trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x16u, 0x02u, 0xFFu})));
  // truncated
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x16u})));

  EXPECT_EQ(Object::ControlMode::Mode2, object.GetControlMode());
}

TEST(CosemDisconnectControlObject, WriteControlModeReadOnlyRejected)
{
  Object readOnly(MakeName(), true, Object::ControlState::Connected,
                  Object::ControlMode::Mode2,
                  dlms::cosem::AttributeAccessMode::ReadOnly);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(4u, BytesFromList({0x16u, 0x05u})));
  EXPECT_EQ(Object::ControlMode::Mode2, readOnly.GetControlMode());
}

TEST(CosemDisconnectControlObject, WriteAttributeRejectsReadOnlyAttributes)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);
  const dlms::cosem::CosemByteBuffer input = BytesFromList({0x16u, 0x05u});
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, input));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, input));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(3u, input));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, input));
}

TEST(CosemDisconnectControlObject, RemoteDisconnectInMode0IsUnsupported)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode0);
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(1u, dlms::cosem::CosemByteBuffer(), out));
  EXPECT_TRUE(out.empty());
  EXPECT_TRUE(object.OutputState());
  EXPECT_EQ(Object::ControlState::Connected, object.GetControlState());
}

TEST(CosemDisconnectControlObject, RemoteDisconnectFromConnectedInMode2)
{
  // §4.5.8.3.1: any mode > 0 → state = Disconnected, output = false.
  for (std::uint8_t modeRaw = 1u; modeRaw <= 6u; ++modeRaw) {
    Object object = MakeObject(true, Object::ControlState::Connected,
                               static_cast<Object::ControlMode>(modeRaw));
    dlms::cosem::CosemByteBuffer out;
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.InvokeMethod(1u, dlms::cosem::CosemByteBuffer(), out))
      << "mode=" << static_cast<unsigned>(modeRaw);
    EXPECT_TRUE(out.empty());
    EXPECT_FALSE(object.OutputState());
    EXPECT_EQ(Object::ControlState::Disconnected, object.GetControlState());
  }
}

TEST(CosemDisconnectControlObject, RemoteReconnectInMode0IsUnsupported)
{
  Object object = MakeObject(false, Object::ControlState::Disconnected,
                             Object::ControlMode::Mode0);
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, dlms::cosem::CosemByteBuffer(), out));
  EXPECT_TRUE(out.empty());
  EXPECT_FALSE(object.OutputState());
  EXPECT_EQ(Object::ControlState::Disconnected, object.GetControlState());
}

TEST(CosemDisconnectControlObject, RemoteReconnectInModes1356TransitionsToReady)
{
  // §4.5.8.3.2: modes 1, 3, 5, 6 → ReadyForReconnection, output stays false
  // (a subsequent manual or local reconnect closes the relay).
  for (std::uint8_t modeRaw : {1u, 3u, 5u, 6u}) {
    Object object = MakeObject(false, Object::ControlState::Disconnected,
                               static_cast<Object::ControlMode>(modeRaw));
    dlms::cosem::CosemByteBuffer out;
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.InvokeMethod(2u, dlms::cosem::CosemByteBuffer(), out))
      << "mode=" << static_cast<unsigned>(modeRaw);
    EXPECT_FALSE(object.OutputState());
    EXPECT_EQ(Object::ControlState::ReadyForReconnection,
              object.GetControlState());
  }
}

TEST(CosemDisconnectControlObject, RemoteReconnectInModes24ClosesRelay)
{
  // §4.5.8.3.2: modes 2, 4 → Connected, output = true.
  for (std::uint8_t modeRaw : {2u, 4u}) {
    Object object = MakeObject(false, Object::ControlState::Disconnected,
                               static_cast<Object::ControlMode>(modeRaw));
    dlms::cosem::CosemByteBuffer out;
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.InvokeMethod(2u, dlms::cosem::CosemByteBuffer(), out))
      << "mode=" << static_cast<unsigned>(modeRaw);
    EXPECT_TRUE(object.OutputState());
    EXPECT_EQ(Object::ControlState::Connected, object.GetControlState());
  }
}

TEST(CosemDisconnectControlObject, UnknownMethodIdsReturnMethodNotFound)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);
  for (std::uint8_t methodId : {0u, 3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xBBu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(methodId, dlms::cosem::CosemByteBuffer(), out))
      << "method=" << static_cast<unsigned>(methodId);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemDisconnectControlObject, SettersUpdateObservableState)
{
  Object object = MakeObject(true, Object::ControlState::Connected,
                             Object::ControlMode::Mode2);
  object.SetOutputState(false);
  object.SetControlState(Object::ControlState::ReadyForReconnection);

  EXPECT_FALSE(object.OutputState());
  EXPECT_EQ(Object::ControlState::ReadyForReconnection, object.GetControlState());

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x03u, 0x00u}), out);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x02u}), out);
}

TEST(CosemDisconnectControlObject, IsValidStaticHelpers)
{
  for (std::uint8_t raw = 0u; raw <= 6u; ++raw)
    EXPECT_TRUE(Object::IsValidControlMode(raw)) << "raw=" << static_cast<unsigned>(raw);
  EXPECT_FALSE(Object::IsValidControlMode(7u));
  EXPECT_FALSE(Object::IsValidControlMode(255u));

  for (std::uint8_t raw = 0u; raw <= 2u; ++raw)
    EXPECT_TRUE(Object::IsValidControlState(raw));
  EXPECT_FALSE(Object::IsValidControlState(3u));
  EXPECT_FALSE(Object::IsValidControlState(255u));
}

TEST(CosemDisconnectControlObject, NormalizesVersionAboveMax)
{
  Object object(MakeName(), true, Object::ControlState::Connected,
                Object::ControlMode::Mode2,
                dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);
}
