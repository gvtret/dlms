#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/client/client.hpp"
#include "dlms/cosem/cosem.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/server/server.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace {

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value);

class IntegrationDataObject : public dlms::cosem::ICosemObject
{
public:
  explicit IntegrationDataObject(
    const dlms::cosem::CosemByteBuffer& value,
    dlms::cosem::AttributeAccessMode attributeAccess =
      dlms::cosem::AttributeAccessMode::ReadOnly)
    : value_(value)
    , invokeCount_(0u)
  {
    descriptor_.key.classId = 3u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(2u, attributeAccess);
    rights_.SetMethodAccess(1u, dlms::cosem::MethodAccessMode::Access);
    actionData_ = EncodeLongUnsigned(0x2468u);
  }

  dlms::cosem::CosemObjectDescriptor Descriptor() const
  {
    return descriptor_;
  }

  dlms::cosem::CosemAccessRights AccessRights() const
  {
    return rights_;
  }

  dlms::cosem::CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    dlms::cosem::CosemByteBuffer& output) const
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    output = value_;
    return dlms::cosem::CosemStatus::Ok;
  }

  dlms::cosem::CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const dlms::cosem::CosemByteBuffer& input)
  {
    if (attributeId != 2u) {
      return dlms::cosem::CosemStatus::AttributeNotFound;
    }
    value_ = input;
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
    ++invokeCount_;
    lastInvokeParameter_ = input;
    output = actionData_;
    return dlms::cosem::CosemStatus::Ok;
  }

  std::size_t InvokeCount() const
  {
    return invokeCount_;
  }

  dlms::cosem::CosemByteBuffer LastInvokeParameter() const
  {
    return lastInvokeParameter_;
  }

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  dlms::cosem::CosemByteBuffer value_;
  dlms::cosem::CosemByteBuffer actionData_;
  std::size_t invokeCount_;
  dlms::cosem::CosemByteBuffer lastInvokeParameter_;
};

class ClientServerApduChannel : public dlms::profile::IApduChannel
{
public:
  explicit ClientServerApduChannel(
    dlms::xdlms::XdlmsServerApduProcessor& processor)
    : processor_(processor)
    , open_(false)
    , sendCalls_(0u)
  {
  }

  dlms::profile::ProfileStatus Open()
  {
    open_ = true;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus Close()
  {
    open_ = false;
    return dlms::profile::ProfileStatus::Ok;
  }

  bool IsOpen() const
  {
    return open_;
  }

  dlms::profile::ProfileStatus SendApdu(dlms::profile::ProfileByteView apdu)
  {
    ++sendCalls_;
    lastResponse_.clear();

    if (sendCalls_ == 1u) {
      lastResponse_ = MakeAareBytes();
      return dlms::profile::ProfileStatus::Ok;
    }

    const std::vector<std::uint8_t> request(apdu.data, apdu.data + apdu.size);
    if (processor_.ProcessRequest(request, lastResponse_) !=
        dlms::xdlms::XdlmsStatus::Ok) {
      return dlms::profile::ProfileStatus::InternalError;
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    apdu = lastResponse_;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    if (output.size < lastResponse_.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0; i < lastResponse_.size(); ++i) {
      output.data[i] = lastResponse_[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = lastResponse_.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

private:
  static std::vector<std::uint8_t> MakeAareBytes()
  {
    const std::uint8_t kAare[] = {
      0x61, 0x4E, 0x80, 0x02, 0x02, 0x84, 0xA1, 0x09,
      0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
      0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05,
      0xA1, 0x03, 0x02, 0x01, 0x0E, 0x88, 0x02, 0x07,
      0x80, 0x89, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
      0x02, 0x02, 0xAA, 0x12, 0x80, 0x10, 0xC6, 0x69,
      0x73, 0x51, 0xFF, 0x4A, 0xEC, 0x29, 0xCD, 0xBA,
      0xAB, 0xF2, 0xFB, 0xE3, 0x46, 0x7C, 0xBE, 0x10,
      0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04,
      0x00, 0x40, 0x18, 0x1D, 0x02, 0x00, 0x00, 0x07};
    return std::vector<std::uint8_t>(kAare, kAare + sizeof(kAare));
  }

  dlms::xdlms::XdlmsServerApduProcessor& processor_;
  bool open_;
  std::size_t sendCalls_;
  std::vector<std::uint8_t> lastResponse_;
};

std::vector<std::uint8_t> EncodeLongUnsigned(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;

  std::uint8_t buffer[16] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeDlmsData(data, writer));
  return std::vector<std::uint8_t>(buffer, buffer + writer.WrittenSize());
}

dlms::client::CosemAttributeDescriptor MakeDescriptor()
{
  dlms::client::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 3u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  descriptor.attributeId = 2u;
  return descriptor;
}

dlms::client::CosemMethodDescriptor MakeMethodDescriptor()
{
  dlms::client::CosemMethodDescriptor descriptor =
    dlms::xdlms::EmptyCosemMethodDescriptor();
  descriptor.classId = 3u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  descriptor.methodId = 1u;
  return descriptor;
}

void AttachObject(
  dlms::server::ServerContext& context,
  dlms::cosem::LogicalDevice& logicalDevice,
  const std::shared_ptr<IntegrationDataObject>& object)
{
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);
}

void OpenClientAssociation(dlms::client::DlmsClient& client)
{
  ASSERT_EQ(dlms::client::ClientStatus::Ok, client.Connect());
  ASSERT_EQ(dlms::client::ClientStatus::Ok, client.OpenAssociation());
}

} // namespace

TEST(ClientGetIntegration, PublicClientReadsMinimalServerObject)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::vector<std::uint8_t> expectedValue =
    EncodeLongUnsigned(0x1234u);
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(expectedValue));

  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  ClientServerApduChannel channel(processor);
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());
  dlms::client::DlmsClient client(channel, association);

  OpenClientAssociation(client);

  std::vector<std::uint8_t> actualValue;
  EXPECT_EQ(dlms::client::ClientStatus::Ok,
            client.Get(MakeDescriptor(), actualValue));
  EXPECT_EQ(expectedValue, actualValue);
}

TEST(ClientSetIntegration, PublicClientWritesMinimalServerObject)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(
      EncodeLongUnsigned(0x1234u),
      dlms::cosem::AttributeAccessMode::ReadAndWrite));
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  ClientServerApduChannel channel(processor);
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());
  dlms::client::DlmsClient client(channel, association);
  OpenClientAssociation(client);

  const std::vector<std::uint8_t> newValue = EncodeLongUnsigned(0x4321u);
  EXPECT_EQ(dlms::client::ClientStatus::Ok,
            client.Set(MakeDescriptor(), newValue));

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(newValue, stored);
}

TEST(ClientSetIntegration, PublicClientReportsServiceRejection)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::vector<std::uint8_t> initialValue = EncodeLongUnsigned(0x1234u);
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(initialValue));
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  ClientServerApduChannel channel(processor);
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());
  dlms::client::DlmsClient client(channel, association);
  OpenClientAssociation(client);

  EXPECT_EQ(dlms::client::ClientStatus::ServiceRejected,
            client.Set(MakeDescriptor(), EncodeLongUnsigned(0x4321u)));

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(initialValue, stored);
}

TEST(ClientActionIntegration, PublicClientInvokesMinimalServerMethod)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));
  AttachObject(context, logicalDevice, object);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);
  ClientServerApduChannel channel(processor);
  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());
  dlms::client::DlmsClient client(channel, association);
  OpenClientAssociation(client);

  const std::vector<std::uint8_t> parameter = EncodeLongUnsigned(0x1357u);
  std::vector<std::uint8_t> returnParameter;
  EXPECT_EQ(dlms::client::ClientStatus::Ok,
            client.Action(
              MakeMethodDescriptor(),
              true,
              parameter,
              returnParameter));
  EXPECT_EQ(EncodeLongUnsigned(0x2468u), returnParameter);
  EXPECT_EQ(1u, object->InvokeCount());
  EXPECT_EQ(parameter, object->LastInvokeParameter());
}
