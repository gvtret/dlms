#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/association/association_client.hpp"
#include "dlms/cosem/cosem.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/security/ciphered_apdu_processor.hpp"
#include "dlms/security/in_memory_invocation_counter_store.hpp"
#include "dlms/security/in_memory_key_store.hpp"
#include "dlms/server/server.hpp"
#include "dlms/xdlms/xdlms_client.hpp"
#include "dlms/xdlms/xdlms_server.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

class ServerBackedApduChannel : public dlms::profile::IApduChannel
{
public:
  ServerBackedApduChannel()
    : processor_(0)
    , open_(false)
    , sendCalls_(0)
    , receiveCalls_(0)
  {
  }

  void AttachServer(dlms::xdlms::XdlmsServerApduProcessor& processor)
  {
    processor_ = &processor;
  }

  void SetNextReceive(const std::vector<std::uint8_t>& apdu)
  {
    nextReceive_ = apdu;
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
    lastSent_.assign(apdu.data, apdu.data + apdu.size);
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(std::vector<std::uint8_t>& apdu)
  {
    ++receiveCalls_;
    if (!nextReceive_.empty()) {
      apdu = nextReceive_;
      nextReceive_.clear();
      return dlms::profile::ProfileStatus::Ok;
    }
    if (processor_ == 0) {
      return dlms::profile::ProfileStatus::InvalidFrame;
    }
    std::vector<std::uint8_t> response;
    if (processor_->ProcessRequest(lastSent_, response) !=
        dlms::xdlms::XdlmsStatus::Ok) {
      return dlms::profile::ProfileStatus::InvalidFrame;
    }
    lastResponse_ = response;
    apdu = response;
    return dlms::profile::ProfileStatus::Ok;
  }

  dlms::profile::ProfileStatus ReceiveApdu(
    dlms::profile::ProfileMutableBuffer output)
  {
    std::vector<std::uint8_t> apdu;
    const dlms::profile::ProfileStatus status = ReceiveApdu(apdu);
    if (status != dlms::profile::ProfileStatus::Ok) {
      return status;
    }
    if (output.size < apdu.size()) {
      return dlms::profile::ProfileStatus::OutputBufferTooSmall;
    }
    for (std::size_t i = 0u; i < apdu.size(); ++i) {
      output.data[i] = apdu[i];
    }
    if (output.writtenSize != 0) {
      *output.writtenSize = apdu.size();
    }
    return dlms::profile::ProfileStatus::Ok;
  }

  int SendCalls() const
  {
    return sendCalls_;
  }

  int ReceiveCalls() const
  {
    return receiveCalls_;
  }

  const std::vector<std::uint8_t>& LastSent() const
  {
    return lastSent_;
  }

  const std::vector<std::uint8_t>& LastResponse() const
  {
    return lastResponse_;
  }

private:
  dlms::xdlms::XdlmsServerApduProcessor* processor_;
  bool open_;
  int sendCalls_;
  int receiveCalls_;
  std::vector<std::uint8_t> nextReceive_;
  std::vector<std::uint8_t> lastSent_;
  std::vector<std::uint8_t> lastResponse_;
};

class ReadCountingDataObject : public dlms::cosem::ICosemObject
{
public:
  explicit ReadCountingDataObject(const dlms::cosem::CosemByteBuffer& value)
    : value_(value)
    , readCount_(0u)
  {
    descriptor_.key.classId = 3u;
    descriptor_.key.version = 0u;
    descriptor_.key.logicalName =
      dlms::cosem::CosemLogicalName(1, 0, 1, 8, 0, 255);
    rights_.SetAttributeAccess(
      2u,
      dlms::cosem::AttributeAccessMode::ReadOnly);
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
    ++readCount_;
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
    return dlms::cosem::CosemStatus::AccessDenied;
  }

  std::size_t ReadCount() const
  {
    return readCount_;
  }

private:
  dlms::cosem::CosemObjectDescriptor descriptor_;
  dlms::cosem::CosemAccessRights rights_;
  dlms::cosem::CosemByteBuffer value_;
  mutable std::size_t readCount_;
};

std::vector<std::uint8_t> MakeAareBytes()
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

dlms::xdlms::CosemAttributeDescriptor MakeDescriptor()
{
  dlms::xdlms::CosemAttributeDescriptor descriptor =
    dlms::xdlms::EmptyCosemAttributeDescriptor();
  descriptor.classId = 3u;
  descriptor.instanceId = dlms::xdlms::CosemLogicalName(1, 0, 1, 8, 0, 255);
  descriptor.attributeId = 2u;
  return descriptor;
}

dlms::security::SecurityKey MakeKey(
  dlms::security::SecurityKeyRole role,
  std::uint8_t seed)
{
  dlms::security::SecurityKey key = dlms::security::EmptySecurityKey(role);
  key.size = 16u;
  for (std::size_t i = 0u; i < key.size; ++i) {
    key.bytes[i] = static_cast<std::uint8_t>(seed + i);
  }
  return key;
}

void InstallKeys(dlms::security::InMemoryKeyStore& keys)
{
  ASSERT_EQ(
    dlms::security::SecurityStatus::Ok,
    keys.SetKey(
      MakeKey(
        dlms::security::SecurityKeyRole::GlobalUnicastEncryption,
        0x10u)));
  ASSERT_EQ(
    dlms::security::SecurityStatus::Ok,
    keys.SetKey(
      MakeKey(
        dlms::security::SecurityKeyRole::Authentication,
        0x80u)));
}

dlms::security::SecurityContext MakeContext(
  dlms::security::SecurityRole role)
{
  dlms::security::SecurityContext context =
    dlms::security::EmptySecurityContext();
  context.policy = dlms::security::SecurityPolicy::AuthenticatedAndEncrypted;
  context.role = role;
  context.clientSap = 16u;
  context.serverSap = 1u;

  const std::uint8_t clientTitle[8] =
    {0x4du, 0x4du, 0x4du, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u};
  const std::uint8_t serverTitle[8] =
    {0x4du, 0x45u, 0x54u, 0x00u, 0x00u, 0x00u, 0x00u, 0x02u};

  for (std::size_t i = 0u; i < 8u; ++i) {
    context.localSystemTitle[i] =
      role == dlms::security::SecurityRole::Client
        ? clientTitle[i]
        : serverTitle[i];
    context.remoteSystemTitle[i] =
      role == dlms::security::SecurityRole::Client
        ? serverTitle[i]
        : clientTitle[i];
  }

  return context;
}

} // namespace

TEST(CipheredGetIntegration, CipheredGetRoundTripProtectsClientAndServerApdus)
{
  dlms::security::InMemoryKeyStore keys;
  InstallKeys(keys);

  dlms::security::InMemoryInvocationCounterStore clientCounters;
  dlms::security::InMemoryInvocationCounterStore serverCounters;
  clientCounters.SetLocalCounter(1u);
  serverCounters.SetLocalCounter(1u);

  const dlms::security::SecurityContext clientSecurityContext =
    MakeContext(dlms::security::SecurityRole::Client);
  const dlms::security::SecurityContext serverSecurityContext =
    MakeContext(dlms::security::SecurityRole::Server);
  dlms::security::CipheredApduProcessor clientSecurity(
    clientSecurityContext,
    keys,
    clientCounters);
  dlms::security::CipheredApduProcessor serverSecurity(
    serverSecurityContext,
    keys,
    serverCounters);

  dlms::server::ServerContext serverContext;
  dlms::cosem::LogicalDevice logicalDevice(1u, "ld-1");
  const std::shared_ptr<ReadCountingDataObject> object(
    new ReadCountingDataObject(EncodeLongUnsigned(0x1234u)));
  ASSERT_EQ(dlms::cosem::CosemStatus::Ok,
            logicalDevice.RegisterObject(object));
  serverContext.AttachLogicalDevice(&logicalDevice);
  serverContext.SetAssociated(true);

  dlms::server::DlmsServer server(serverContext);
  dlms::server::XdlmsServerAdapter adapter(server);
  dlms::xdlms::XdlmsServerDispatcher dispatcher(adapter);
  dlms::xdlms::XdlmsServerApduProcessor processor(
    dispatcher,
    serverSecurity);

  ServerBackedApduChannel channel;
  channel.SetNextReceive(MakeAareBytes());
  channel.AttachServer(processor);

  dlms::association::AssociationClient association(
    channel,
    dlms::association::DefaultAssociationOptions());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok, association.Open());
  ASSERT_EQ(dlms::association::AssociationStatus::Ok,
            association.Establish());
  ASSERT_TRUE(association.IsAssociated());

  dlms::xdlms::XdlmsClient client(channel, association, clientSecurity);
  dlms::xdlms::GetResult result;
  EXPECT_EQ(dlms::xdlms::XdlmsStatus::Ok,
            client.Get(MakeDescriptor(), result));

  EXPECT_EQ(2, channel.SendCalls());
  EXPECT_EQ(2, channel.ReceiveCalls());
  ASSERT_FALSE(channel.LastSent().empty());
  ASSERT_FALSE(channel.LastResponse().empty());
  EXPECT_EQ(0x30u, channel.LastSent()[0]);
  EXPECT_EQ(0x30u, channel.LastResponse()[0]);
  EXPECT_NE(0xC0u, channel.LastSent()[0]);
  EXPECT_NE(0xC4u, channel.LastResponse()[0]);

  EXPECT_EQ(1u, object->ReadCount());
  ASSERT_TRUE(result.hasData);
  ASSERT_EQ(3u, result.data.size());
  EXPECT_EQ(0x12u, result.data[0]);
  EXPECT_EQ(0x12u, result.data[1]);
  EXPECT_EQ(0x34u, result.data[2]);
}
