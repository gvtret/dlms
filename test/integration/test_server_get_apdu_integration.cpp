#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
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
  explicit IntegrationDataObject(const dlms::cosem::CosemByteBuffer& value)
    : value_(value)
  {
    descriptor_.key.classId = 3u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(
      2u, dlms::cosem::AttributeAccessMode::ReadOnly);
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
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::AccessDenied;
  }

  dlms::cosem::CosemStatus InvokeMethod(
    std::uint8_t,
    const dlms::cosem::CosemByteBuffer&,
    dlms::cosem::CosemByteBuffer&)
  {
    return dlms::cosem::CosemStatus::MethodNotFound;
  }

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  dlms::cosem::CosemByteBuffer value_;
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
