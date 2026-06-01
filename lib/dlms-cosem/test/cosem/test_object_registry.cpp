#include "dlms/cosem/cosem.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace {

class TestObject : public dlms::cosem::ICosemObject
{
public:
  explicit TestObject(const dlms::cosem::CosemObjectKey& key)
    : readCount(0u)
    , writeCount(0u)
    , invokeCount(0u)
    , readStatus(dlms::cosem::CosemStatus::Ok)
    , writeStatus(dlms::cosem::CosemStatus::Ok)
    , invokeStatus(dlms::cosem::CosemStatus::Ok)
  {
    descriptor_.key = key;
    readData.push_back(0x11u);
    invokeData.push_back(0x22u);
  }

  dlms::cosem::CosemObjectDescriptor Descriptor() const
  {
    return descriptor_;
  }

  dlms::cosem::CosemAccessRights AccessRights() const
  {
    return rights;
  }

  dlms::cosem::CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    dlms::cosem::CosemByteBuffer& output) const
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }

    ++readCount;
    if (readStatus != dlms::cosem::CosemStatus::Ok) {
      return readStatus;
    }

    output = readData;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const dlms::cosem::CosemByteBuffer& input)
  {
    if (attributeId != 3u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }

    ++writeCount;
    if (writeStatus != dlms::cosem::CosemStatus::Ok) {
      return writeStatus;
    }

    writtenData = input;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const dlms::cosem::CosemByteBuffer& input,
    dlms::cosem::CosemByteBuffer& output)
  {
    if (methodId != 1u) {
      return dlms::cosem::CosemStatus::MethodNotFound;
    }

    ++invokeCount;
    lastMethodInput = input;
    if (invokeStatus != dlms::cosem::CosemStatus::Ok) {
      return invokeStatus;
    }

    output = invokeData;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemAccessRights rights;
  mutable std::size_t readCount;
  std::size_t writeCount;
  std::size_t invokeCount;
  dlms::cosem::CosemStatus readStatus;
  dlms::cosem::CosemStatus writeStatus;
  dlms::cosem::CosemStatus invokeStatus;
  dlms::cosem::CosemByteBuffer readData;
  dlms::cosem::CosemByteBuffer invokeData;
  dlms::cosem::CosemByteBuffer writtenData;
  dlms::cosem::CosemByteBuffer lastMethodInput;

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
};

dlms::cosem::CosemObjectKey MakeKey(
  std::uint16_t classId,
  std::uint8_t version,
  std::uint8_t c)
{
  dlms::cosem::CosemObjectKey key;
  key.classId = classId;
  key.version = version;
  key.logicalName = dlms::cosem::CosemLogicalName(1, 0, c, 8, 0, 255);
  return key;
}

std::shared_ptr<dlms::cosem::ICosemObject> MakeObject(
  const dlms::cosem::CosemObjectKey& key)
{
  return std::shared_ptr<dlms::cosem::ICosemObject>(
    new TestObject(key));
}

std::shared_ptr<TestObject> MakeTestObject(
  const dlms::cosem::CosemObjectKey& key)
{
  return std::shared_ptr<TestObject>(new TestObject(key));
}

dlms::cosem::CosemAttributeDescriptor MakeAttribute(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t attributeId)
{
  dlms::cosem::CosemAttributeDescriptor descriptor;
  descriptor.object = key;
  descriptor.attributeId = attributeId;
  return descriptor;
}

dlms::cosem::CosemMethodDescriptor MakeMethod(
  const dlms::cosem::CosemObjectKey& key,
  std::uint8_t methodId)
{
  dlms::cosem::CosemMethodDescriptor descriptor;
  descriptor.object = key;
  descriptor.methodId = methodId;
  return descriptor;
}

TEST(CosemTypes, LogicalNameStoresSixBytes)
{
  const dlms::cosem::CosemLogicalName name(0, 0, 40, 0, 0, 255);

  ASSERT_EQ(6u, name.Size());
  EXPECT_EQ(0u, name[0]);
  EXPECT_EQ(0u, name[1]);
  EXPECT_EQ(40u, name[2]);
  EXPECT_EQ(0u, name[3]);
  EXPECT_EQ(0u, name[4]);
  EXPECT_EQ(255u, name[5]);
  EXPECT_EQ(0u, name[6]);
  EXPECT_FALSE(name.IsEmpty());
}

TEST(CosemTypes, EmptyDescriptorIsInvalid)
{
  const dlms::cosem::CosemObjectDescriptor descriptor =
    dlms::cosem::EmptyCosemObjectDescriptor();

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            dlms::cosem::ValidateObjectDescriptor(descriptor));
}

TEST(CosemTypes, ObjectKeyUsesClassVersionAndLogicalName)
{
  const dlms::cosem::CosemObjectKey base = MakeKey(1u, 0u, 1u);
  dlms::cosem::CosemObjectKey same = MakeKey(1u, 0u, 1u);
  dlms::cosem::CosemObjectKey differentVersion = MakeKey(1u, 1u, 1u);
  dlms::cosem::CosemObjectKey differentName = MakeKey(1u, 0u, 2u);

  EXPECT_EQ(base, same);
  EXPECT_NE(base, differentVersion);
  EXPECT_NE(base, differentName);
  EXPECT_TRUE(base < differentVersion);
  EXPECT_TRUE(base < differentName);
}

TEST(ObjectRegistry, RejectsNullInvalidAndDuplicateObjects)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey validKey = MakeKey(1u, 0u, 1u);

  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            registry.Register(std::shared_ptr<dlms::cosem::ICosemObject>()));
  EXPECT_EQ(dlms::cosem::CosemStatus::InvalidArgument,
            registry.Register(MakeObject(dlms::cosem::EmptyCosemObjectKey())));
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.Register(MakeObject(validKey)));
  EXPECT_EQ(dlms::cosem::CosemStatus::DuplicateObject,
            registry.Register(MakeObject(validKey)));
  EXPECT_EQ(1u, registry.Size());
}

TEST(ObjectRegistry, FindsRegisteredObjectByKey)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey key = MakeKey(3u, 0u, 10u);
  const std::shared_ptr<dlms::cosem::ICosemObject> object = MakeObject(key);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  EXPECT_TRUE(registry.Contains(key));
  EXPECT_EQ(object.get(), registry.Find(key));
  EXPECT_EQ(0, registry.Find(MakeKey(3u, 0u, 11u)));
}

TEST(ObjectRegistry, ReturnsDescriptorsInKeyOrder)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey second = MakeKey(7u, 0u, 2u);
  const dlms::cosem::CosemObjectKey first = MakeKey(1u, 0u, 1u);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(MakeObject(second)));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(MakeObject(first)));

  const std::vector<dlms::cosem::CosemObjectDescriptor> descriptors =
    registry.Descriptors();

  ASSERT_EQ(2u, descriptors.size());
  EXPECT_EQ(first, descriptors[0].key);
  EXPECT_EQ(second, descriptors[1].key);
}

TEST(CosemStatus, NamesStableValues)
{
  EXPECT_STREQ(
    "DuplicateObject",
    dlms::cosem::CosemStatusName(dlms::cosem::CosemStatus::DuplicateObject));
}

TEST(CosemAccessRights, DistinguishesPublicAndAuthenticatedAccess)
{
  dlms::cosem::CosemAccessRights rights;
  rights.SetAttributeAccess(
    2u, dlms::cosem::AttributeAccessMode::AuthenticatedReadOnly);
  rights.SetAttributeAccess(
    3u, dlms::cosem::AttributeAccessMode::WriteOnly);
  rights.SetMethodAccess(
    1u, dlms::cosem::MethodAccessMode::AuthenticatedAccess);

  EXPECT_FALSE(rights.CanReadAttribute(
    2u, dlms::cosem::PublicAccessContext()));
  EXPECT_TRUE(rights.CanReadAttribute(
    2u, dlms::cosem::AuthenticatedAccessContext()));
  EXPECT_TRUE(rights.CanWriteAttribute(
    3u, dlms::cosem::PublicAccessContext()));
  EXPECT_FALSE(rights.CanInvokeMethod(
    1u, dlms::cosem::PublicAccessContext()));
  EXPECT_TRUE(rights.CanInvokeMethod(
    1u, dlms::cosem::AuthenticatedAccessContext()));
  EXPECT_FALSE(rights.CanReadAttribute(
    99u, dlms::cosem::AuthenticatedAccessContext()));
}

TEST(ObjectRegistry, ReadsAttributeWhenAccessIsGranted)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey key = MakeKey(1u, 0u, 1u);
  const std::shared_ptr<TestObject> object = MakeTestObject(key);
  object->rights.SetAttributeAccess(
    2u, dlms::cosem::AttributeAccessMode::ReadOnly);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer output;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.ReadAttribute(MakeAttribute(key, 2u), output));
  ASSERT_EQ(1u, output.size());
  EXPECT_EQ(0x11u, output[0]);
  EXPECT_EQ(1u, object->readCount);
}

TEST(ObjectRegistry, DeniesReadBeforeCallingObject)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey key = MakeKey(1u, 0u, 1u);
  const std::shared_ptr<TestObject> object = MakeTestObject(key);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer output;
  output.push_back(0xAAu);
  EXPECT_EQ(dlms::cosem::CosemStatus::AccessDenied,
            registry.ReadAttribute(MakeAttribute(key, 2u), output));
  ASSERT_EQ(1u, output.size());
  EXPECT_EQ(0xAAu, output[0]);
  EXPECT_EQ(0u, object->readCount);
}

TEST(ObjectRegistry, WritesAttributeWhenAccessIsGranted)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey key = MakeKey(1u, 0u, 1u);
  const std::shared_ptr<TestObject> object = MakeTestObject(key);
  object->rights.SetAttributeAccess(
    3u, dlms::cosem::AttributeAccessMode::WriteOnly);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer input;
  input.push_back(0x33u);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.WriteAttribute(MakeAttribute(key, 3u), input));
  EXPECT_EQ(input, object->writtenData);
  EXPECT_EQ(1u, object->writeCount);
}

TEST(ObjectRegistry, InvokesMethodWhenAccessIsGranted)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey key = MakeKey(1u, 0u, 1u);
  const std::shared_ptr<TestObject> object = MakeTestObject(key);
  object->rights.SetMethodAccess(
    1u, dlms::cosem::MethodAccessMode::Access);

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer input;
  dlms::cosem::CosemByteBuffer output;
  input.push_back(0x55u);
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok,
            registry.InvokeMethod(MakeMethod(key, 1u), input, output));
  EXPECT_EQ(input, object->lastMethodInput);
  ASSERT_EQ(1u, output.size());
  EXPECT_EQ(0x22u, output[0]);
}

TEST(ObjectRegistry, ReturnsMissingAndObjectErrors)
{
  dlms::cosem::ObjectRegistry registry;
  const dlms::cosem::CosemObjectKey key = MakeKey(1u, 0u, 1u);
  const std::shared_ptr<TestObject> object = MakeTestObject(key);
  object->rights.SetAttributeAccess(
    2u, dlms::cosem::AttributeAccessMode::ReadOnly);
  object->rights.SetAttributeAccess(
    99u, dlms::cosem::AttributeAccessMode::ReadOnly);
  object->readStatus = dlms::cosem::CosemStatus::ObjectError;

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok, registry.Register(object));

  dlms::cosem::CosemByteBuffer output;
  output.push_back(0xAAu);
  EXPECT_EQ(dlms::cosem::CosemStatus::ObjectNotFound,
            registry.ReadAttribute(MakeAttribute(MakeKey(1u, 0u, 9u), 2u),
                                   output));
  EXPECT_EQ(dlms::cosem::CosemStatus::AttributeNotFound,
            registry.ReadAttribute(MakeAttribute(key, 99u), output));
  EXPECT_EQ(dlms::cosem::CosemStatus::ObjectError,
            registry.ReadAttribute(MakeAttribute(key, 2u), output));
  ASSERT_EQ(1u, output.size());
  EXPECT_EQ(0xAAu, output[0]);
}

} // namespace
