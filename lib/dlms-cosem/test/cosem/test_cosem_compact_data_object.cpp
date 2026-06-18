// Tests for IC 62 (Compact Data) — per-IC file (rule P2.4).
//
// IEC 62056-6-2 ED4 §5.2.2 / DLMS UA Blue Book Ed. 12.1 §4.5.7.
//
// Typed attributes:
//   template_id    (id 4) : std::uint8_t
//   capture_method (id 6) : enum {Inactive=0, Invoke=1, InvokeAndStore=2}
//
// `compact_buffer` (id 2), `capture_objects` (id 3) and
// `template_description` (id 5) remain opaque CosemByteBuffer — they
// depend on the discriminated-union infra shared with IC 7 (Profile
// Generic) and will be migrated when that lands.
//
// Method 1 `reset` is implemented in-place (clears `compact_buffer`).
// Method 2 `capture` returns UnsupportedFeature pending typed
// capture_object_definition machinery.

#include <cstdint>

#include "dlms/cosem/simple_objects.hpp"

#include <gtest/gtest.h>

namespace {

using Object = dlms::cosem::CosemCompactDataObject;

dlms::cosem::CosemLogicalName MakeName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 66u, 0u, 1u, 255u);
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

dlms::cosem::CosemByteBuffer SampleBuffer()
{
  return BytesFromList({0x09u, 0x04u, 0xDEu, 0xADu, 0xBEu, 0xEFu});
}

dlms::cosem::CosemByteBuffer SampleCaptureObjects()
{
  // array(1) of structure(4): long-unsigned 3 (Register),
  // octet-string(6) 1.0.32.7.0.255, integer 2, long-unsigned 0
  return BytesFromList({
    0x01u, 0x01u,
      0x02u, 0x04u,
        0x12u, 0x00u, 0x03u,
        0x09u, 0x06u,
          0x01u, 0x00u, 0x20u, 0x07u, 0x00u, 0xFFu,
        0x0Fu, 0x02u,
        0x12u, 0x00u, 0x00u});
}

dlms::cosem::CosemByteBuffer SampleTemplateDescription()
{
  return BytesFromList({0x09u, 0x02u, 0x12u, 0x00u});
}

Object MakeObject(
  std::uint8_t templateId = 1u,
  Object::CaptureMethod captureMethod = Object::CaptureMethod::Invoke,
  dlms::cosem::AttributeAccessMode access =
    dlms::cosem::AttributeAccessMode::ReadAndWrite)
{
  return Object(MakeName(), SampleBuffer(), SampleCaptureObjects(),
                templateId, SampleTemplateDescription(), captureMethod,
                access);
}

}  // namespace

TEST(CosemCompactDataObject, DescriptorAndAccessRights)
{
  Object object = MakeObject();
  EXPECT_EQ(62u, object.Descriptor().key.classId);
  EXPECT_EQ(1u, object.Descriptor().key.version);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);

  const auto rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u})
    EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
              rights.AttributeAccess(id));
}

TEST(CosemCompactDataObject, ReadAttributeEncodesTypedAxdr)
{
  Object object = MakeObject(7u, Object::CaptureMethod::InvokeAndStore);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(MakeName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(SampleBuffer(), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(SampleCaptureObjects(), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(BytesFromList({0x11u, 0x07u}), out);  // unsigned 7

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(5u, out));
  EXPECT_EQ(SampleTemplateDescription(), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(6u, out));
  EXPECT_EQ(BytesFromList({0x16u, 0x02u}), out);  // enum InvokeAndStore

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(7u, out));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(0u, out));
}

TEST(CosemCompactDataObject, WriteTemplateIdParsesUnsignedAxdr)
{
  Object object = MakeObject();
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x11u, 0xA5u})));
  EXPECT_EQ(0xA5u, object.TemplateId());

  // boundary values
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x11u, 0x00u})));
  EXPECT_EQ(0u, object.TemplateId());
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, BytesFromList({0x11u, 0xFFu})));
  EXPECT_EQ(0xFFu, object.TemplateId());
}

TEST(CosemCompactDataObject, WriteTemplateIdRejectsMalformedAxdr)
{
  Object object = MakeObject(1u);
  // empty
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, dlms::cosem::CosemByteBuffer()));
  // wrong tag (long-unsigned instead of unsigned)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x12u, 0x00u, 0x05u})));
  // trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x11u, 0x05u, 0xFFu})));
  // truncated
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x11u})));
  EXPECT_EQ(1u, object.TemplateId());
}

TEST(CosemCompactDataObject, WriteCaptureMethodParsesAndValidatesEnum)
{
  Object object = MakeObject(1u, Object::CaptureMethod::Invoke);

  for (std::uint8_t raw : {0u, 1u, 2u}) {
    EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
              object.WriteAttribute(6u, BytesFromList({0x16u, raw})))
      << "raw=" << static_cast<unsigned>(raw);
    EXPECT_EQ(static_cast<std::uint8_t>(object.GetCaptureMethod()), raw);
  }

  // out-of-range enum (3..255) → InvalidArgument, stored value preserved.
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(6u, BytesFromList({0x16u, 0x01u})));
  ASSERT_EQ(Object::CaptureMethod::Invoke, object.GetCaptureMethod());

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, BytesFromList({0x16u, 0x03u})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, BytesFromList({0x16u, 0xFFu})));
  EXPECT_EQ(Object::CaptureMethod::Invoke, object.GetCaptureMethod());
}

TEST(CosemCompactDataObject, WriteCaptureMethodRejectsMalformedAxdr)
{
  Object object = MakeObject(1u, Object::CaptureMethod::Invoke);
  // empty
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, dlms::cosem::CosemByteBuffer()));
  // wrong tag (boolean instead of enum)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, BytesFromList({0x03u, 0x01u})));
  // trailing garbage
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, BytesFromList({0x16u, 0x01u, 0xFFu})));
  // truncated
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(6u, BytesFromList({0x16u})));

  EXPECT_EQ(Object::CaptureMethod::Invoke, object.GetCaptureMethod());
}

TEST(CosemCompactDataObject, OpaqueAttributesPassThroughOnWrite)
{
  Object object = MakeObject();
  const dlms::cosem::CosemByteBuffer rawBuffer =
    BytesFromList({0x09u, 0x02u, 0xCAu, 0xFEu});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, rawBuffer));
  EXPECT_EQ(rawBuffer, object.Buffer());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, rawBuffer));
  EXPECT_EQ(rawBuffer, object.CaptureObjects());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(5u, rawBuffer));
  EXPECT_EQ(rawBuffer, object.TemplateDescription());
}

TEST(CosemCompactDataObject, WriteAttributeRejectsReadOnlyAccessMode)
{
  Object readOnly = MakeObject(1u, Object::CaptureMethod::Invoke,
                               dlms::cosem::AttributeAccessMode::ReadOnly);
  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x11u, 0x42u});
  for (std::uint8_t id : {2u, 3u, 4u, 5u, 6u})
    EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
              readOnly.WriteAttribute(id, in));
  EXPECT_EQ(1u, readOnly.TemplateId());
  EXPECT_EQ(Object::CaptureMethod::Invoke, readOnly.GetCaptureMethod());

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            readOnly.WriteAttribute(1u, in));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            readOnly.WriteAttribute(99u, in));
}

TEST(CosemCompactDataObject, ResetMethodClearsCompactBufferOnly)
{
  Object object = MakeObject(7u, Object::CaptureMethod::Invoke);
  ASSERT_FALSE(object.Buffer().empty());

  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.InvokeMethod(1u, dlms::cosem::CosemByteBuffer(), out));
  EXPECT_TRUE(out.empty());
  EXPECT_TRUE(object.Buffer().empty());

  // configuration preserved
  EXPECT_EQ(SampleCaptureObjects(), object.CaptureObjects());
  EXPECT_EQ(7u, object.TemplateId());
  EXPECT_EQ(SampleTemplateDescription(), object.TemplateDescription());
  EXPECT_EQ(Object::CaptureMethod::Invoke, object.GetCaptureMethod());
}

TEST(CosemCompactDataObject, CaptureMethodReturnsUnsupportedFeature)
{
  Object object = MakeObject();
  dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
  EXPECT_EQ(dlms::cosem::CosemStatus::UnsupportedFeature,
            object.InvokeMethod(2u, dlms::cosem::CosemByteBuffer(), out));
  EXPECT_TRUE(out.empty());
  // buffer untouched
  EXPECT_EQ(SampleBuffer(), object.Buffer());
}

TEST(CosemCompactDataObject, UnknownMethodIdsReturnMethodNotFound)
{
  Object object = MakeObject();
  for (std::uint8_t methodId : {0u, 3u, 4u, 99u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xBBu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(methodId, dlms::cosem::CosemByteBuffer(),
                                  out))
      << "method=" << static_cast<unsigned>(methodId);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemCompactDataObject, IsValidCaptureMethodHelper)
{
  EXPECT_TRUE(Object::IsValidCaptureMethod(0u));
  EXPECT_TRUE(Object::IsValidCaptureMethod(1u));
  EXPECT_TRUE(Object::IsValidCaptureMethod(2u));
  EXPECT_FALSE(Object::IsValidCaptureMethod(3u));
  EXPECT_FALSE(Object::IsValidCaptureMethod(255u));
}

TEST(CosemCompactDataObject, CtorNormalizesInvalidCaptureMethodToInactive)
{
  // Safe-fallback: any out-of-range raw value collapses to Inactive
  // (the spec default).
  Object object(MakeName(), SampleBuffer(), SampleCaptureObjects(), 1u,
                SampleTemplateDescription(),
                static_cast<Object::CaptureMethod>(0xFFu),
                dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(Object::CaptureMethod::Inactive, object.GetCaptureMethod());
}

TEST(CosemCompactDataObject, NormalizesVersionAboveMax)
{
  Object object(MakeName(), SampleBuffer(), SampleCaptureObjects(), 1u,
                SampleTemplateDescription(), Object::CaptureMethod::Invoke,
                dlms::cosem::AttributeAccessMode::ReadAndWrite, 99u);
  EXPECT_EQ(Object::MaxSupportedVersion, object.Descriptor().key.version);
}
