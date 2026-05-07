#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/action.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/cosem/cosem.hpp"
#include "dlms/server/server.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

class IntegrationDataObject : public dlms::cosem::ICosemObject
{
public:
  explicit IntegrationDataObject(
    const dlms::cosem::CosemByteBuffer& value,
    dlms::cosem::AttributeAccessMode attributeAccess =
      dlms::cosem::AttributeAccessMode::ReadOnly)
    : value_(value)
  {
    descriptor_.key.classId = 3u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(2u, attributeAccess);
    rights_.SetMethodAccess(1u, dlms::cosem::MethodAccessMode::Access);
    rights_.SetMethodAccess(2u, dlms::cosem::MethodAccessMode::NoAccess);
    actionData_.push_back(0x12u);
    actionData_.push_back(0x24u);
    actionData_.push_back(0x68u);
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
    ++invokeCount_;
    lastInvokeMethodId_ = methodId;
    lastInvokeParameter_ = input;
    if (methodId != 1u) {
      return dlms::cosem::CosemStatus::MethodNotFound;
    }
    output = actionData_;
    return dlms::cosem::CosemStatus::Ok;
  }

  std::size_t InvokeCount() const
  {
    return invokeCount_;
  }

  std::uint8_t LastInvokeMethodId() const
  {
    return lastInvokeMethodId_;
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
  std::size_t invokeCount_ = 0u;
  std::uint8_t lastInvokeMethodId_ = 0u;
  dlms::cosem::CosemByteBuffer lastInvokeParameter_;
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

std::vector<std::uint8_t> EncodeRequest()
{
  dlms::apdu::XdlmsApdu request = dlms::apdu::MakeGetRequestNormal(
    0x85u,
    3u,
    dlms::apdu::LogicalName(1, 0, 1, 8, 0, 255),
    2u);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

std::vector<std::uint8_t> EncodeSetRequest(
  std::uint8_t invokeIdAndPriority,
  const dlms::apdu::DlmsData& value)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::SetRequest;
  request.setRequestAny.choice = dlms::apdu::SetRequestChoice::Normal;
  request.setRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.setRequestAny.normal.descriptor.classId = 3u;
  request.setRequestAny.normal.descriptor.logicalName[0] = 1u;
  request.setRequestAny.normal.descriptor.logicalName[1] = 0u;
  request.setRequestAny.normal.descriptor.logicalName[2] = 1u;
  request.setRequestAny.normal.descriptor.logicalName[3] = 8u;
  request.setRequestAny.normal.descriptor.logicalName[4] = 0u;
  request.setRequestAny.normal.descriptor.logicalName[5] = 255u;
  request.setRequestAny.normal.descriptor.attributeId = 2u;
  request.setRequestAny.normal.hasSelection = false;
  request.setRequestAny.data = value;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

std::vector<std::uint8_t> EncodeActionRequest(
  std::uint8_t invokeIdAndPriority,
  std::uint8_t methodId,
  const dlms::apdu::DlmsData& parameter)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  request.actionRequestAny.choice = dlms::apdu::ActionRequestChoice::Normal;
  request.actionRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.actionRequestAny.normal.descriptor.classId = 3u;
  request.actionRequestAny.normal.descriptor.logicalName[0] = 1u;
  request.actionRequestAny.normal.descriptor.logicalName[1] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[2] = 1u;
  request.actionRequestAny.normal.descriptor.logicalName[3] = 8u;
  request.actionRequestAny.normal.descriptor.logicalName[4] = 0u;
  request.actionRequestAny.normal.descriptor.logicalName[5] = 255u;
  request.actionRequestAny.normal.descriptor.methodId = methodId;
  request.actionRequestAny.normal.hasInvocationParameter = true;
  request.actionRequestAny.normal.invocationParameter = parameter;

  std::vector<std::uint8_t> output;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::EncodeXdlmsApdu(request, output));
  return output;
}

dlms::apdu::DlmsData MakeLongUnsignedData(std::uint16_t value)
{
  dlms::apdu::DlmsData data;
  data.type = dlms::apdu::DlmsDataType::LongUnsigned;
  data.unsignedValue = value;
  return data;
}

dlms::apdu::XdlmsApdu DecodeResponse(
  const std::vector<std::uint8_t>& response)
{
  dlms::apdu::XdlmsApdu apdu;
  EXPECT_EQ(dlms::apdu::ApduStatus::Ok,
            dlms::apdu::DecodeXdlmsApdu(
              response.empty() ? 0 : &response[0],
              response.size(),
              apdu));
  return apdu;
}

} // namespace

TEST(ServerGetApduIntegration, GetRequestApduReadsCosemObject)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);

  std::vector<std::uint8_t> response;
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(EncodeRequest(), response));

  const dlms::apdu::XdlmsApdu decoded = DecodeResponse(response);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::GetResponse, decoded.kind);
  EXPECT_EQ(dlms::apdu::GetResponseChoice::Normal,
            decoded.getResponseAny.choice);
  EXPECT_EQ(0x85u, decoded.getResponseAny.invokeIdAndPriority);
  EXPECT_EQ(dlms::apdu::GetDataResultChoice::Data,
            decoded.getResponseAny.result.choice);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            decoded.getResponseAny.result.data.type);
  EXPECT_EQ(0x1234u, decoded.getResponseAny.result.data.unsignedValue);
}

TEST(ServerSetApduIntegration, SetRequestApduWritesCosemObject)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(
      EncodeLongUnsigned(0x1234u),
      dlms::cosem::AttributeAccessMode::ReadAndWrite));

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);

  std::vector<std::uint8_t> response;
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(
              EncodeSetRequest(0x86u, MakeLongUnsignedData(0x4321u)),
              response));

  const dlms::apdu::XdlmsApdu decoded = DecodeResponse(response);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, decoded.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            decoded.setResponseAny.choice);
  EXPECT_EQ(0x86u, decoded.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, decoded.setResponseAny.result);

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(EncodeLongUnsigned(0x4321u), stored);
}

TEST(ServerSetApduIntegration, SetRequestApduReportsAccessDenied)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);

  std::vector<std::uint8_t> response;
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(
              EncodeSetRequest(0x87u, MakeLongUnsignedData(0x4321u)),
              response));

  const dlms::apdu::XdlmsApdu decoded = DecodeResponse(response);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::SetResponse, decoded.kind);
  EXPECT_EQ(dlms::apdu::SetResponseChoice::Normal,
            decoded.setResponseAny.choice);
  EXPECT_EQ(0x87u, decoded.setResponseAny.invokeIdAndPriority);
  EXPECT_EQ(3u, decoded.setResponseAny.result);

  dlms::cosem::CosemByteBuffer stored;
  EXPECT_EQ(dlms::cosem::CosemStatus::Ok, object->ReadAttribute(2u, stored));
  EXPECT_EQ(EncodeLongUnsigned(0x1234u), stored);
}

TEST(ServerActionApduIntegration, ActionRequestApduInvokesCosemObject)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);

  std::vector<std::uint8_t> response;
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(
              EncodeActionRequest(0x88u, 1u, MakeLongUnsignedData(0x4321u)),
              response));

  const dlms::apdu::XdlmsApdu decoded = DecodeResponse(response);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, decoded.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            decoded.actionResponseAny.choice);
  EXPECT_EQ(0x88u, decoded.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(0u, decoded.actionResponseAny.normal.result);
  EXPECT_TRUE(decoded.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(dlms::apdu::DlmsDataType::LongUnsigned,
            decoded.actionResponseAny.normal.returnParameter.type);
  EXPECT_EQ(0x2468u,
            decoded.actionResponseAny.normal.returnParameter.unsignedValue);
  EXPECT_EQ(1u, object->InvokeCount());
  EXPECT_EQ(1u, object->LastInvokeMethodId());
  EXPECT_EQ(EncodeLongUnsigned(0x4321u), object->LastInvokeParameter());
}

TEST(ServerActionApduIntegration, ActionRequestApduReportsAccessDenied)
{
  dlms::server::ServerContext context;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<IntegrationDataObject> object(
    new IntegrationDataObject(EncodeLongUnsigned(0x1234u)));

  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  context.AttachLogicalDevice(&logicalDevice);
  context.SetAssociated(true);

  dlms::server::DlmsServer server(context);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(dispatcher);

  std::vector<std::uint8_t> response;
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            processor.ProcessRequest(
              EncodeActionRequest(0x89u, 2u, MakeLongUnsignedData(0x4321u)),
              response));

  const dlms::apdu::XdlmsApdu decoded = DecodeResponse(response);
  EXPECT_EQ(dlms::apdu::XdlmsApduKind::ActionResponse, decoded.kind);
  EXPECT_EQ(dlms::apdu::ActionResponseChoice::Normal,
            decoded.actionResponseAny.choice);
  EXPECT_EQ(0x89u, decoded.actionResponseAny.invokeIdAndPriority);
  EXPECT_EQ(3u, decoded.actionResponseAny.normal.result);
  EXPECT_FALSE(decoded.actionResponseAny.normal.hasReturnParameter);
  EXPECT_EQ(0u, object->InvokeCount());
}
