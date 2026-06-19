// SPDX-License-Identifier: Apache-2.0
//
// Per-IC tests for CosemGprsModemSetupObject (class_id=45, version=0)
// per IEC 62056-6-2 ED4 (2021) §4.7.7 and DLMS UA Blue Book Ed. 12.1
// §4.7.7. IC 45 holds three dynamic attributes: APN (octet-string),
// PIN_code (long-unsigned) and quality_of_service (structure of two
// qos_element substructures, each carrying five unsigned bytes).
// The class defines no specific methods.

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "dlms/cosem/cosem_status.hpp"
#include "dlms/cosem/cosem_types.hpp"
#include "dlms/cosem/simple_objects.hpp"
#include "dlms/cosem/types/quality_of_service.hpp"

namespace {

dlms::cosem::CosemByteBuffer BytesFromList(
  std::initializer_list<std::uint8_t> bytes)
{
  return dlms::cosem::CosemByteBuffer(bytes.begin(), bytes.end());
}

dlms::cosem::CosemByteBuffer LongUnsigned(std::uint16_t value)
{
  return BytesFromList({
    0x12u,
    static_cast<std::uint8_t>((value >> 8) & 0xFFu),
    static_cast<std::uint8_t>(value & 0xFFu)});
}

dlms::cosem::CosemByteBuffer EncodedLogicalName(
  const dlms::cosem::CosemLogicalName& name)
{
  return BytesFromList({
    0x09u, 0x06u,
    name[0], name[1], name[2], name[3], name[4], name[5]});
}

dlms::cosem::CosemByteBuffer EncodedApn(
  const std::vector<std::uint8_t>& apn)
{
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x09u);
  out.push_back(static_cast<std::uint8_t>(apn.size()));
  out.insert(out.end(), apn.begin(), apn.end());
  return out;
}

std::vector<std::uint8_t> ApnInternet()
{
  return {'i', 'n', 't', 'e', 'r', 'n', 'e', 't'};
}

dlms::cosem::types::QualityOfService SampleQos()
{
  return dlms::cosem::types::QualityOfService(
    dlms::cosem::types::QosElement(1u, 2u, 3u, 4u, 5u),
    dlms::cosem::types::QosElement(10u, 20u, 30u, 40u, 50u));
}

dlms::cosem::CosemByteBuffer EncodedQos(
  const dlms::cosem::types::QualityOfService& qos)
{
  // structure(2) { structure(5){5x unsigned}, structure(5){5x unsigned} }
  dlms::cosem::CosemByteBuffer out;
  out.push_back(0x02u);
  out.push_back(0x02u);
  auto append_element = [&](const dlms::cosem::types::QosElement& e) {
    out.push_back(0x02u);
    out.push_back(0x05u);
    out.push_back(0x11u); out.push_back(e.Precedence());
    out.push_back(0x11u); out.push_back(e.Delay());
    out.push_back(0x11u); out.push_back(e.Reliability());
    out.push_back(0x11u); out.push_back(e.PeakThroughput());
    out.push_back(0x11u); out.push_back(e.MeanThroughput());
  };
  append_element(qos.Default());
  append_element(qos.Requested());
  return out;
}

dlms::cosem::CosemLogicalName SampleName()
{
  return dlms::cosem::CosemLogicalName(0u, 0u, 2u, 2u, 0u, 255u);
}

dlms::cosem::CosemGprsModemSetupObject MakeObject(
  dlms::cosem::AttributeAccessMode access)
{
  return dlms::cosem::CosemGprsModemSetupObject(
    SampleName(),
    ApnInternet(),
    /*pin_code*/ 1234u,
    SampleQos(),
    access);
}

} // namespace

TEST(CosemGprsModemSetupObject, DescriptorAndAccessRights)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(45u, object.Descriptor().key.classId);
  EXPECT_EQ(0u, object.Descriptor().key.version);
  EXPECT_EQ(
    dlms::cosem::CosemGprsModemSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
  EXPECT_EQ(SampleName(), object.Descriptor().key.logicalName);

  const dlms::cosem::CosemAccessRights rights = object.AccessRights();
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadOnly,
            rights.AttributeAccess(1u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(2u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(3u));
  EXPECT_EQ(dlms::cosem::AttributeAccessMode::ReadAndWrite,
            rights.AttributeAccess(4u));
}

TEST(CosemGprsModemSetupObject, TypedGettersReflectCtor)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(ApnInternet(), object.Apn());
  EXPECT_EQ(1234u, object.PinCode());
  EXPECT_EQ(SampleQos(), object.QualityOfService());
}

TEST(CosemGprsModemSetupObject, ReadAttributeEmitsTypedAxdr)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(1u, out));
  EXPECT_EQ(EncodedLogicalName(SampleName()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(EncodedApn(ApnInternet()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(3u, out));
  EXPECT_EQ(LongUnsigned(1234u), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedQos(SampleQos()), out);

  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.ReadAttribute(99u, out));
}

TEST(CosemGprsModemSetupObject, ReadEmptyApnEmitsZeroLengthOctetString)
{
  dlms::cosem::CosemGprsModemSetupObject object(
    SampleName(),
    std::vector<std::uint8_t>{},
    /*pin_code*/ 0u,
    dlms::cosem::types::QualityOfService(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite);

  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(2u, out));
  EXPECT_EQ(BytesFromList({0x09u, 0x00u}), out);
}

TEST(CosemGprsModemSetupObject, WriteApnDecodesOctetString)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const std::vector<std::uint8_t> next = {'i', 'o', 't'};
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, EncodedApn(next)));
  EXPECT_EQ(next, object.Apn());

  // Empty APN is a legal octet-string value.
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(2u, BytesFromList({0x09u, 0x00u})));
  EXPECT_TRUE(object.Apn().empty());
}

TEST(CosemGprsModemSetupObject, WriteApnRejectsWrongTagAndTruncation)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // long-unsigned tag (0x12) is not an octet-string.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, LongUnsigned(0u)));
  // Length says 4 but only 2 bytes follow.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x09u, 0x04u, 'a', 'b'})));
  // Trailing garbage after the declared length.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              2u, BytesFromList({0x09u, 0x01u, 'a', 'b'})));
  // Empty input.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(2u, dlms::cosem::CosemByteBuffer{}));

  EXPECT_EQ(ApnInternet(), object.Apn());
}

TEST(CosemGprsModemSetupObject, WritePinCodeDecodesLongUnsigned)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, LongUnsigned(0u)));
  EXPECT_EQ(0u, object.PinCode());

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(3u, LongUnsigned(0xFFFFu)));
  EXPECT_EQ(0xFFFFu, object.PinCode());
}

TEST(CosemGprsModemSetupObject, WritePinCodeRejectsWrongTagAndTrailingBytes)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // unsigned (0x11) instead of long-unsigned (0x12)
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, BytesFromList({0x11u, 0x05u})));
  // Truncated long-unsigned (missing low byte).
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, BytesFromList({0x12u, 0x00u})));
  // Trailing garbage after the long-unsigned encoding.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              3u, BytesFromList({0x12u, 0x00u, 0x01u, 0xFFu})));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(3u, dlms::cosem::CosemByteBuffer{}));

  EXPECT_EQ(1234u, object.PinCode());
}

TEST(CosemGprsModemSetupObject, WriteQualityOfServiceRoundTrips)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::types::QualityOfService next(
    dlms::cosem::types::QosElement(7u, 8u, 9u, 10u, 11u),
    dlms::cosem::types::QosElement(255u, 0u, 128u, 64u, 32u));

  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            object.WriteAttribute(4u, EncodedQos(next)));
  EXPECT_EQ(next, object.QualityOfService());

  // Round-trip the new value through ReadAttribute.
  dlms::cosem::CosemByteBuffer out;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object.ReadAttribute(4u, out));
  EXPECT_EQ(EncodedQos(next), out);
}

TEST(CosemGprsModemSetupObject, WriteQualityOfServiceRejectsMalformedInput)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  // Wrong outer tag (structure expected).
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, BytesFromList({0x01u, 0x02u})));
  // Outer structure field count != 2.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              4u, BytesFromList({0x02u, 0x01u, 0x02u, 0x05u,
                                 0x11u, 0u, 0x11u, 0u, 0x11u, 0u,
                                 0x11u, 0u, 0x11u, 0u})));
  // Inner qos_element with only 4 unsigned fields.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              4u, BytesFromList({
                0x02u, 0x02u,
                0x02u, 0x04u,
                  0x11u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u,
                0x02u, 0x05u,
                  0x11u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u})));
  // Inner field uses enum (0x16) instead of unsigned (0x11).
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(
              4u, BytesFromList({
                0x02u, 0x02u,
                0x02u, 0x05u,
                  0x16u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u,
                0x02u, 0x05u,
                  0x11u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u, 0x11u, 0u})));
  // Trailing garbage after a complete encoding.
  dlms::cosem::CosemByteBuffer trailing = EncodedQos(SampleQos());
  trailing.push_back(0xAAu);
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, trailing));
  // Empty input.
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            object.WriteAttribute(4u, dlms::cosem::CosemByteBuffer{}));

  EXPECT_EQ(SampleQos(), object.QualityOfService());
}

TEST(CosemGprsModemSetupObject, WriteLogicalNameAlwaysDenied)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(1u, EncodedLogicalName(SampleName())));
}

TEST(CosemGprsModemSetupObject, WriteUnknownAttributeNotFound)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            object.WriteAttribute(99u, LongUnsigned(0u)));
}

TEST(CosemGprsModemSetupObject, ReadOnlyRejectsAllWrites)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadOnly);

  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(2u, EncodedApn({'x'})));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(3u, LongUnsigned(99u)));
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            object.WriteAttribute(4u, EncodedQos(SampleQos())));

  EXPECT_EQ(ApnInternet(), object.Apn());
  EXPECT_EQ(1234u, object.PinCode());
  EXPECT_EQ(SampleQos(), object.QualityOfService());
}

TEST(CosemGprsModemSetupObject, InvokeMethodAlwaysReturnsMethodNotFound)
{
  dlms::cosem::CosemGprsModemSetupObject object =
    MakeObject(dlms::cosem::AttributeAccessMode::ReadAndWrite);

  const dlms::cosem::CosemByteBuffer in = BytesFromList({0x0Fu, 0x00u});
  for (std::uint8_t method : {0u, 1u, 2u, 99u, 255u}) {
    dlms::cosem::CosemByteBuffer out = BytesFromList({0xAAu});
    EXPECT_EQ(dlms::cosem::CosemStatus::MethodNotFound,
              object.InvokeMethod(
                static_cast<std::uint8_t>(method), in, out))
      << "method id " << static_cast<unsigned>(method);
    EXPECT_TRUE(out.empty());
  }
}

TEST(CosemGprsModemSetupObject, VersionAboveMaxNormalized)
{
  dlms::cosem::CosemGprsModemSetupObject object(
    SampleName(),
    ApnInternet(),
    /*pin_code*/ 0u,
    SampleQos(),
    dlms::cosem::AttributeAccessMode::ReadAndWrite,
    /*version*/ 99u);
  EXPECT_EQ(
    dlms::cosem::CosemGprsModemSetupObject::MaxSupportedVersion,
    object.Descriptor().key.version);
}
