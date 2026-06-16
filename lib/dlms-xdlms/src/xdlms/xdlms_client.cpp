#include "dlms/xdlms/xdlms_client.hpp"

#include "dlms/apdu/action.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/association/association_client.hpp"
#include "dlms/security/ciphered_apdu_processor.hpp"
#include "dlms/xdlms/xdlms_association_state.hpp"
#include "dlms/xdlms/xdlms_correlation.hpp"
#include "dlms/xdlms/xdlms_security_processor.hpp"
#include "dlms/xdlms/xdlms_trace.hpp"

namespace dlms {
namespace xdlms {
namespace {

// Trace emission context shared between SendAndReceive and its
// callers. Holds the structured fields the caller already knows
// (kind, invokeId, options, descriptor) so the trace path does not
// re-derive them from the APDU.
struct XdlmsTraceContext
{
  IXdlmsTraceSink* sink;
  std::uint64_t conversationId;
  XdlmsTraceKind requestKind;
  XdlmsTraceKind responseKind;
  std::uint8_t invokeId;
  ServiceOptions options;
  std::uint16_t classId;
  std::uint8_t attributeOrMethodId;
  std::uint8_t logicalName[6];
  bool hasBlockNumber;
  std::uint32_t blockNumber;
};

XdlmsTraceContext MakeTraceContext(
  IXdlmsTraceSink* sink,
  std::uint64_t conversationId,
  XdlmsTraceKind requestKind,
  XdlmsTraceKind responseKind,
  std::uint8_t invokeId,
  const ServiceOptions& options)
{
  XdlmsTraceContext ctx;
  ctx.sink = sink;
  ctx.conversationId = conversationId;
  ctx.requestKind = requestKind;
  ctx.responseKind = responseKind;
  ctx.invokeId = invokeId;
  ctx.options = options;
  ctx.classId = 0u;
  ctx.attributeOrMethodId = 0u;
  for (std::size_t i = 0; i < 6u; ++i) {
    ctx.logicalName[i] = 0u;
  }
  ctx.hasBlockNumber = false;
  ctx.blockNumber = 0u;
  return ctx;
}

void FillDescriptor(
  XdlmsTraceContext& ctx,
  const CosemAttributeDescriptor& descriptor)
{
  ctx.classId = descriptor.classId;
  ctx.attributeOrMethodId = descriptor.attributeId;
  for (std::size_t i = 0; i < descriptor.instanceId.Size(); ++i) {
    ctx.logicalName[i] = descriptor.instanceId[i];
  }
}

void FillDescriptor(
  XdlmsTraceContext& ctx,
  const CosemMethodDescriptor& descriptor)
{
  ctx.classId = descriptor.classId;
  ctx.attributeOrMethodId = descriptor.methodId;
  for (std::size_t i = 0; i < descriptor.instanceId.Size(); ++i) {
    ctx.logicalName[i] = descriptor.instanceId[i];
  }
}

void EmitTrace(
  const XdlmsTraceContext& ctx,
  XdlmsTraceKind kind,
  XdlmsTraceDirection direction,
  XdlmsStatus status,
  std::size_t apduSize,
  std::size_t payloadSize)
{
  if (ctx.sink == 0) {
    return;
  }
  XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
  event.kind = kind;
  event.direction = direction;
  event.status = status;
  event.invokeId = ctx.invokeId;
  event.options = ctx.options;
  event.classId = ctx.classId;
  event.attributeOrMethodId = ctx.attributeOrMethodId;
  for (std::size_t i = 0; i < 6u; ++i) {
    event.logicalName[i] = ctx.logicalName[i];
  }
  event.hasBlockNumber = ctx.hasBlockNumber;
  event.blockNumber = ctx.blockNumber;
  event.apduSize = apduSize;
  event.payloadSize = payloadSize;
  event.conversationId = ctx.conversationId;
  ctx.sink->OnXdlmsTrace(event);
}

std::uint8_t MakeInvokeIdAndPriority(
  std::uint8_t invokeId,
  const ServiceOptions& options)
{
  std::uint8_t value = static_cast<std::uint8_t>(invokeId & 0x0Fu);
  if (options.confirmed) {
    value = static_cast<std::uint8_t>(value | 0x80u);
  }
  if (options.highPriority) {
    value = static_cast<std::uint8_t>(value | 0x40u);
  }
  return value;
}

dlms::apdu::LogicalName ToApduLogicalName(
  const CosemLogicalName& logicalName)
{
  return dlms::apdu::LogicalName(
    logicalName[0],
    logicalName[1],
    logicalName[2],
    logicalName[3],
    logicalName[4],
    logicalName[5]);
}

dlms::apdu::CosemAttributeDescriptor ToApduDescriptor(
  const CosemAttributeDescriptor& descriptor)
{
  dlms::apdu::CosemAttributeDescriptor apduDescriptor;
  apduDescriptor.classId = descriptor.classId;
  for (std::size_t i = 0; i < descriptor.instanceId.Size(); ++i) {
    apduDescriptor.logicalName[i] = descriptor.instanceId[i];
  }
  apduDescriptor.attributeId = descriptor.attributeId;
  return apduDescriptor;
}

dlms::apdu::CosemMethodDescriptor ToApduDescriptor(
  const CosemMethodDescriptor& descriptor)
{
  dlms::apdu::CosemMethodDescriptor apduDescriptor;
  apduDescriptor.classId = descriptor.classId;
  for (std::size_t i = 0; i < descriptor.instanceId.Size(); ++i) {
    apduDescriptor.logicalName[i] = descriptor.instanceId[i];
  }
  apduDescriptor.methodId = descriptor.methodId;
  return apduDescriptor;
}

XdlmsStatus DecodeEncodedData(
  const std::vector<std::uint8_t>& encodedData,
  dlms::apdu::DlmsData& output)
{
  if (encodedData.empty()) {
    return XdlmsStatus::InvalidArgument;
  }

  return dlms::apdu::DecodeDlmsData(
      &encodedData[0],
      encodedData.size(),
      8,
      output) == dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus CopyEncodedData(
  const dlms::apdu::DlmsData& data,
  GetResult& result)
{
  std::uint8_t buffer[2048] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  const dlms::apdu::ApduStatus status =
    dlms::apdu::EncodeDlmsData(data, writer);
  if (status != dlms::apdu::ApduStatus::Ok) {
    return XdlmsStatus::DecodeFailed;
  }

  result.data.assign(buffer, buffer + writer.WrittenSize());
  result.hasData = true;
  return XdlmsStatus::Ok;
}

XdlmsStatus CopyEncodedData(
  const dlms::apdu::DlmsData& data,
  ActionResult& result)
{
  std::uint8_t buffer[2048] = {};
  dlms::apdu::ApduWriter writer(buffer, sizeof(buffer));
  const dlms::apdu::ApduStatus status =
    dlms::apdu::EncodeDlmsData(data, writer);
  if (status != dlms::apdu::ApduStatus::Ok) {
    return XdlmsStatus::DecodeFailed;
  }

  result.data.assign(buffer, buffer + writer.WrittenSize());
  result.hasData = true;
  return XdlmsStatus::Ok;
}

XdlmsStatus SendAndReceive(
  dlms::profile::IApduChannel& channel,
  IXdlmsSecurityProcessor* security,
  const dlms::apdu::XdlmsApdu& request,
  std::vector<std::uint8_t>& decodedResponseBytes,
  dlms::apdu::XdlmsApdu& response,
  const XdlmsTraceContext& trace)
{
  decodedResponseBytes.clear();

  std::vector<std::uint8_t> encodedRequest;
  if (dlms::apdu::EncodeXdlmsApdu(request, encodedRequest) !=
      dlms::apdu::ApduStatus::Ok) {
    EmitTrace(
      trace,
      XdlmsTraceKind::DecodeFailed,
      XdlmsTraceDirection::Outbound,
      XdlmsStatus::EncodeFailed,
      0u,
      0u);
    return XdlmsStatus::EncodeFailed;
  }

  std::vector<std::uint8_t> outboundRequest = encodedRequest;
  if (security != 0) {
    dlms::security::SecurityByteView plain;
    plain.data = encodedRequest.empty() ? 0 : &encodedRequest[0];
    plain.size = encodedRequest.size();
    const dlms::security::SecurityStatus status =
      security->Protect(plain, outboundRequest);
    if (status != dlms::security::SecurityStatus::Ok) {
      EmitTrace(
        trace,
        XdlmsTraceKind::SecurityFailed,
        XdlmsTraceDirection::Outbound,
        XdlmsStatus::SecurityFailed,
        encodedRequest.size(),
        0u);
      return XdlmsStatus::SecurityFailed;
    }
  }

  EmitTrace(
    trace,
    trace.requestKind,
    XdlmsTraceDirection::Outbound,
    XdlmsStatus::Ok,
    outboundRequest.size(),
    encodedRequest.size());

  dlms::profile::ProfileByteView view = {};
  view.data = outboundRequest.empty() ? 0 : &outboundRequest[0];
  view.size = outboundRequest.size();
  if (channel.SendApdu(view) != dlms::profile::ProfileStatus::Ok) {
    return XdlmsStatus::SendFailed;
  }

  std::vector<std::uint8_t> encodedResponse;
  if (channel.ReceiveApdu(encodedResponse) != dlms::profile::ProfileStatus::Ok) {
    return XdlmsStatus::ReceiveFailed;
  }

  std::vector<std::uint8_t> inboundResponse = encodedResponse;
  if (security != 0) {
    dlms::security::SecurityByteView protectedApdu;
    protectedApdu.data = encodedResponse.empty() ? 0 : &encodedResponse[0];
    protectedApdu.size = encodedResponse.size();
    const dlms::security::SecurityStatus status =
      security->Unprotect(protectedApdu, inboundResponse);
    if (status != dlms::security::SecurityStatus::Ok) {
      EmitTrace(
        trace,
        XdlmsTraceKind::SecurityFailed,
        XdlmsTraceDirection::Inbound,
        XdlmsStatus::SecurityFailed,
        encodedResponse.size(),
        0u);
      return XdlmsStatus::SecurityFailed;
    }
  }

  decodedResponseBytes = inboundResponse;
  const dlms::apdu::ApduStatus decodeStatus = dlms::apdu::DecodeXdlmsApdu(
      decodedResponseBytes.empty() ? 0 : &decodedResponseBytes[0],
      decodedResponseBytes.size(),
      response);
  if (decodeStatus != dlms::apdu::ApduStatus::Ok) {
    EmitTrace(
      trace,
      XdlmsTraceKind::DecodeFailed,
      XdlmsTraceDirection::Inbound,
      XdlmsStatus::DecodeFailed,
      encodedResponse.size(),
      inboundResponse.size());
    return XdlmsStatus::DecodeFailed;
  }

  EmitTrace(
    trace,
    trace.responseKind,
    XdlmsTraceDirection::Inbound,
    XdlmsStatus::Ok,
    encodedResponse.size(),
    inboundResponse.size());
  return XdlmsStatus::Ok;
}

class BlockTransferManager
{
public:
  explicit BlockTransferManager(std::size_t maxBytes)
    : maxBytes_(maxBytes)
    , nextBlockNumber_(1u)
  {
  }

  XdlmsStatus AppendBlock(const dlms::apdu::DataBlockG& block)
  {
    if (block.blockNumber != nextBlockNumber_) {
      return XdlmsStatus::DecodeFailed;
    }
    if (block.rawData.size > maxBytes_ ||
        data_.size() > maxBytes_ - block.rawData.size) {
      return XdlmsStatus::DecodeFailed;
    }
    if (block.rawData.size != 0u && block.rawData.data == 0) {
      return XdlmsStatus::DecodeFailed;
    }

    if (block.rawData.size != 0u) {
      data_.insert(
        data_.end(),
        block.rawData.data,
        block.rawData.data + block.rawData.size);
    }
    ++nextBlockNumber_;
    return XdlmsStatus::Ok;
  }

  const std::vector<std::uint8_t>& Data() const
  {
    return data_;
  }

private:
  std::size_t maxBytes_;
  std::uint32_t nextBlockNumber_;
  std::vector<std::uint8_t> data_;
};

dlms::apdu::XdlmsApdu MakeGetRequestNext(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::GetRequest;
  request.getRequestAny.choice = dlms::apdu::GetRequestChoice::Next;
  request.getRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.getRequestAny.blockNumber = blockNumber;
  return request;
}

XdlmsStatus MakeGetRequestNormal(
  std::uint8_t invokeIdAndPriority,
  const CosemAttributeDescriptor& descriptor,
  const SelectiveAccessDescriptor* selectiveAccess,
  dlms::apdu::XdlmsApdu& request)
{
  request = dlms::apdu::MakeGetRequestNormal(
    invokeIdAndPriority,
    descriptor.classId,
    ToApduLogicalName(descriptor.instanceId),
    descriptor.attributeId);

  if (selectiveAccess == 0) {
    return XdlmsStatus::Ok;
  }

  dlms::apdu::DlmsData parameters = {};
  const XdlmsStatus status =
    DecodeEncodedData(selectiveAccess->encodedParameters, parameters);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  request.getRequest.hasSelectiveAccess = true;
  request.getRequest.selectiveAccess.selector = selectiveAccess->selector;
  request.getRequest.selectiveAccess.parameters = parameters;
  request.getRequestAny.normal.hasSelection = true;
  request.getRequestAny.normal.selection =
    request.getRequest.selectiveAccess;
  return XdlmsStatus::Ok;
}

dlms::apdu::XdlmsApdu MakeActionRequestNextPblock(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  request.actionRequestAny.choice =
    dlms::apdu::ActionRequestChoice::NextPblock;
  request.actionRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.actionRequestAny.blockNumber = blockNumber;
  return request;
}

dlms::apdu::XdlmsApdu MakeActionRequestBlock(
  dlms::apdu::ActionRequestChoice choice,
  std::uint8_t invokeIdAndPriority,
  const CosemMethodDescriptor& descriptor,
  std::uint32_t blockNumber,
  bool lastBlock,
  const std::uint8_t* data,
  std::size_t size)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  request.actionRequestAny.choice = choice;
  request.actionRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.actionRequestAny.normal.descriptor = ToApduDescriptor(descriptor);
  request.actionRequestAny.dataBlock.lastBlock = lastBlock;
  request.actionRequestAny.dataBlock.blockNumber = blockNumber;
  request.actionRequestAny.dataBlock.rawData.data = data;
  request.actionRequestAny.dataBlock.rawData.size = size;
  return request;
}

XdlmsStatus ReceiveGetResponse(
  dlms::profile::IApduChannel& channel,
  IXdlmsSecurityProcessor* security,
  const dlms::apdu::XdlmsApdu& request,
  std::vector<std::uint8_t>& decodedResponseBytes,
  dlms::apdu::XdlmsApdu& response,
  const XdlmsTraceContext& trace)
{
  const XdlmsStatus status =
    SendAndReceive(
      channel,
      security,
      request,
      decodedResponseBytes,
      response,
      trace);
  if (status != XdlmsStatus::Ok) {
    return status;
  }
  return response.kind == dlms::apdu::XdlmsApduKind::GetResponse
    ? XdlmsStatus::Ok
    : XdlmsStatus::DecodeFailed;
}

XdlmsStatus ReceiveActionResponse(
  dlms::profile::IApduChannel& channel,
  IXdlmsSecurityProcessor* security,
  const dlms::apdu::XdlmsApdu& request,
  std::vector<std::uint8_t>& decodedResponseBytes,
  dlms::apdu::XdlmsApdu& response,
  const XdlmsTraceContext& trace)
{
  const XdlmsStatus status =
    SendAndReceive(
      channel,
      security,
      request,
      decodedResponseBytes,
      response,
      trace);
  if (status != XdlmsStatus::Ok) {
    return status;
  }
  return response.kind == dlms::apdu::XdlmsApduKind::ActionResponse
    ? XdlmsStatus::Ok
    : XdlmsStatus::DecodeFailed;
}

dlms::apdu::XdlmsApdu MakeSetRequestBlock(
  dlms::apdu::SetRequestChoice choice,
  std::uint8_t invokeIdAndPriority,
  const CosemAttributeDescriptor& descriptor,
  std::uint32_t blockNumber,
  bool lastBlock,
  const std::uint8_t* data,
  std::size_t size)
{
  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::SetRequest;
  request.setRequestAny.choice = choice;
  request.setRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.setRequestAny.normal.descriptor = ToApduDescriptor(descriptor);
  request.setRequestAny.normal.hasSelection = false;
  request.setRequestAny.dataBlock.lastBlock = lastBlock;
  request.setRequestAny.dataBlock.blockNumber = blockNumber;
  request.setRequestAny.dataBlock.rawData.data = data;
  request.setRequestAny.dataBlock.rawData.size = size;
  return request;
}

XdlmsStatus ValidateSetBlockResponse(
  const dlms::apdu::XdlmsApdu& response,
  std::uint8_t invokeId,
  std::uint32_t expectedBlockNumber,
  bool finalBlock,
  SetResult& result)
{
  if (response.kind != dlms::apdu::XdlmsApduKind::SetResponse) {
    return XdlmsStatus::DecodeFailed;
  }
  if ((response.setResponseAny.invokeIdAndPriority & 0x0Fu) != invokeId) {
    return XdlmsStatus::InvokeIdMismatch;
  }
  if (response.setResponseAny.blockNumber != expectedBlockNumber) {
    return XdlmsStatus::DecodeFailed;
  }

  if (!finalBlock) {
    return response.setResponseAny.choice ==
        dlms::apdu::SetResponseChoice::DataBlock
      ? XdlmsStatus::Ok
      : XdlmsStatus::DecodeFailed;
  }

  if (response.setResponseAny.choice !=
      dlms::apdu::SetResponseChoice::LastDataBlock) {
    return XdlmsStatus::DecodeFailed;
  }

  result.invokeId = invokeId;
  result.accessResult = response.setResponseAny.result;
  return result.accessResult == 0u
    ? XdlmsStatus::Ok
    : XdlmsStatus::ServiceRejected;
}

XdlmsStatus ValidateActionNextPblockResponse(
  const dlms::apdu::XdlmsApdu& response,
  std::uint8_t invokeId,
  std::uint32_t expectedBlockNumber)
{
  if (response.kind != dlms::apdu::XdlmsApduKind::ActionResponse) {
    return XdlmsStatus::DecodeFailed;
  }
  if ((response.actionResponseAny.invokeIdAndPriority & 0x0Fu) != invokeId) {
    return XdlmsStatus::InvokeIdMismatch;
  }
  if (response.actionResponseAny.choice !=
      dlms::apdu::ActionResponseChoice::NextPblock) {
    return XdlmsStatus::DecodeFailed;
  }
  return response.actionResponseAny.blockNumber == expectedBlockNumber
    ? XdlmsStatus::Ok
    : XdlmsStatus::DecodeFailed;
}

XdlmsStatus CopyActionResponse(
  const dlms::apdu::ActionResponse& response,
  std::uint8_t invokeId,
  ActionResult& result)
{
  if ((response.invokeIdAndPriority & 0x0Fu) != invokeId) {
    return XdlmsStatus::InvokeIdMismatch;
  }

  result.invokeId = invokeId;
  result.actionResult = response.normal.result;
  if (response.normal.hasReturnParameter) {
    const XdlmsStatus status = CopyEncodedData(
      response.normal.returnParameter,
      result);
    if (status != XdlmsStatus::Ok) {
      return status;
    }
  }

  return result.actionResult == 0u
    ? XdlmsStatus::Ok
    : XdlmsStatus::ServiceRejected;
}

XdlmsStatus DecodeActionBlockPayload(
  std::uint8_t invokeIdAndPriority,
  const std::vector<std::uint8_t>& payload,
  dlms::apdu::XdlmsApdu& response)
{
  std::vector<std::uint8_t> bytes;
  bytes.reserve(payload.size() + 3u);
  bytes.push_back(0xC7u);
  bytes.push_back(0x01u);
  bytes.push_back(invokeIdAndPriority);
  bytes.insert(bytes.end(), payload.begin(), payload.end());

  return dlms::apdu::DecodeXdlmsApdu(&bytes[0], bytes.size(), response) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::DecodeFailed;
}

XdlmsStatus CopyFinalActionResponse(
  dlms::profile::IApduChannel& channel,
  IXdlmsSecurityProcessor* security,
  std::uint8_t invokeId,
  std::uint8_t invokeIdAndPriority,
  const ServiceOptions& options,
  dlms::apdu::XdlmsApdu& response,
  ActionResult& result,
  const XdlmsTraceContext& trace)
{
  if (response.actionResponseAny.choice !=
      dlms::apdu::ActionResponseChoice::Normal) {
    if (response.actionResponseAny.choice !=
        dlms::apdu::ActionResponseChoice::WithPblock) {
      return XdlmsStatus::UnsupportedFeature;
    }
    if (!options.allowBlockTransfer) {
      return XdlmsStatus::BlockTransferRequired;
    }

    BlockTransferManager blocks(options.maxBlockTransferBytes);
    std::vector<std::uint8_t> decodedResponseBytes;
    for (;;) {
      if ((response.actionResponseAny.invokeIdAndPriority & 0x0Fu) != invokeId) {
        EmitTrace(
          trace,
          XdlmsTraceKind::InvokeIdRejected,
          XdlmsTraceDirection::Inbound,
          XdlmsStatus::InvokeIdMismatch,
          0u,
          0u);
        return XdlmsStatus::InvokeIdMismatch;
      }

      XdlmsStatus status = blocks.AppendBlock(response.actionResponseAny.dataBlock);
      if (status != XdlmsStatus::Ok) {
        return status;
      }

      XdlmsTraceContext blockTrace = trace;
      blockTrace.hasBlockNumber = true;
      blockTrace.blockNumber = response.actionResponseAny.dataBlock.blockNumber;
      EmitTrace(
        blockTrace,
        XdlmsTraceKind::BlockTransferStep,
        XdlmsTraceDirection::Inbound,
        XdlmsStatus::Ok,
        0u,
        response.actionResponseAny.dataBlock.rawData.size);

      if (response.actionResponseAny.dataBlock.lastBlock) {
        response = dlms::apdu::XdlmsApdu();
        status = DecodeActionBlockPayload(
          invokeIdAndPriority,
          blocks.Data(),
          response);
        if (status != XdlmsStatus::Ok) {
          return status;
        }
        return CopyActionResponse(response.actionResponseAny, invokeId, result);
      }

      const std::uint32_t acknowledgedBlock =
        response.actionResponseAny.dataBlock.blockNumber;
      response = dlms::apdu::XdlmsApdu();
      decodedResponseBytes.clear();
      status = ReceiveActionResponse(
        channel,
        security,
        MakeActionRequestNextPblock(invokeIdAndPriority, acknowledgedBlock),
        decodedResponseBytes,
        response,
        trace);
      if (status != XdlmsStatus::Ok) {
        return status;
      }
      if (response.actionResponseAny.choice !=
          dlms::apdu::ActionResponseChoice::WithPblock) {
        return XdlmsStatus::DecodeFailed;
      }
    }
  }

  const XdlmsStatus status =
    CopyActionResponse(response.actionResponseAny, invokeId, result);
  return status == XdlmsStatus::Ok ? XdlmsStatus::Ok : status;
}

} // namespace

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  IXdlmsAssociationState& association)
  : channel_(channel)
  , ownedAssociation_()
  , association_(&association)
  , ownedSecurity_()
  , security_(0)
  , invokeIds_()
  , traceSink_(0)
{
}

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  IXdlmsAssociationState& association,
  IXdlmsSecurityProcessor& security)
  : channel_(channel)
  , ownedAssociation_()
  , association_(&association)
  , ownedSecurity_()
  , security_(&security)
  , invokeIds_()
  , traceSink_(0)
{
}

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  dlms::association::AssociationClient& association)
  : XdlmsClient(
      channel,
      static_cast<dlms::association::IAssociationClient&>(association))
{
}

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  dlms::association::IAssociationClient& association)
  : channel_(channel)
  , ownedAssociation_(new AssociationClientXdlmsAssociationState(association))
  , association_(ownedAssociation_.get())
  , ownedSecurity_()
  , security_(0)
  , invokeIds_()
  , traceSink_(0)
{
}

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  dlms::association::AssociationClient& association,
  IXdlmsSecurityProcessor& security)
  : XdlmsClient(
      channel,
      static_cast<dlms::association::IAssociationClient&>(association),
      security)
{
}

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  dlms::association::IAssociationClient& association,
  IXdlmsSecurityProcessor& security)
  : channel_(channel)
  , ownedAssociation_(new AssociationClientXdlmsAssociationState(association))
  , association_(ownedAssociation_.get())
  , ownedSecurity_()
  , security_(&security)
  , invokeIds_()
  , traceSink_(0)
{
}

XdlmsClient::XdlmsClient(
  dlms::profile::IApduChannel& channel,
  dlms::association::AssociationClient& association,
  dlms::security::CipheredApduProcessor& security)
  : channel_(channel)
  , ownedAssociation_(
      new AssociationClientXdlmsAssociationState(
        static_cast<dlms::association::IAssociationClient&>(association)))
  , association_(ownedAssociation_.get())
  , ownedSecurity_(new CipheredXdlmsSecurityProcessor(security))
  , security_(ownedSecurity_.get())
  , invokeIds_()
  , traceSink_(0)
{
}

void XdlmsClient::SetTraceSink(IXdlmsTraceSink* sink)
{
  traceSink_ = sink;
}

IXdlmsTraceSink* XdlmsClient::TraceSink() const
{
  return traceSink_;
}

XdlmsStatus XdlmsClient::Get(
  const CosemAttributeDescriptor& descriptor,
  GetResult& result)
{
  return Get(descriptor, DefaultServiceOptions(), result);
}

XdlmsStatus XdlmsClient::Get(
  const CosemAttributeDescriptor& descriptor,
  const SelectiveAccessDescriptor& selectiveAccess,
  GetResult& result)
{
  return Get(descriptor, selectiveAccess, DefaultServiceOptions(), result);
}

XdlmsStatus XdlmsClient::Get(
  const CosemAttributeDescriptor& descriptor,
  const ServiceOptions& options,
  GetResult& result)
{
  return Get(descriptor, 0, options, result);
}

XdlmsStatus XdlmsClient::Get(
  const CosemAttributeDescriptor& descriptor,
  const SelectiveAccessDescriptor& selectiveAccess,
  const ServiceOptions& options,
  GetResult& result)
{
  return Get(descriptor, &selectiveAccess, options, result);
}

XdlmsStatus XdlmsClient::Get(
  const CosemAttributeDescriptor& descriptor,
  const SelectiveAccessDescriptor* selectiveAccess,
  const ServiceOptions& options,
  GetResult& result)
{
  result = EmptyGetResult();

  XdlmsStatus status = ValidateDescriptor(descriptor);
  if (status != XdlmsStatus::Ok) {
    return status;
  }
  if (selectiveAccess != 0) {
    status = ValidateSelectiveAccess(*selectiveAccess);
    if (status != XdlmsStatus::Ok) {
      return status;
    }
  }

  if (!association_->IsAssociated()) {
    return XdlmsStatus::NotAssociated;
  }

  const std::uint8_t invokeId = invokeIds_.Next();
  const std::uint8_t invokeIdAndPriority =
    MakeInvokeIdAndPriority(invokeId, options);

  XdlmsTraceContext trace = MakeTraceContext(
    traceSink_,
    MakeConversationId(association_->ConversationSeed(), invokeId),
    XdlmsTraceKind::GetRequest,
    XdlmsTraceKind::GetResponse,
    invokeId,
    options);
  FillDescriptor(trace, descriptor);
  channel_.SetCorrelation(trace.conversationId);

  dlms::apdu::XdlmsApdu request;
  status = MakeGetRequestNormal(
    invokeIdAndPriority,
    descriptor,
    selectiveAccess,
    request);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  std::vector<std::uint8_t> decodedResponseBytes;
  dlms::apdu::XdlmsApdu response;
  status = ReceiveGetResponse(
    channel_,
    security_,
    request,
    decodedResponseBytes,
    response,
    trace);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  if (response.getResponseAny.choice != dlms::apdu::GetResponseChoice::Normal) {
    if (response.getResponseAny.choice !=
        dlms::apdu::GetResponseChoice::WithDataBlock) {
      return XdlmsStatus::UnsupportedFeature;
    }
    if (!options.allowBlockTransfer) {
      return XdlmsStatus::BlockTransferRequired;
    }

    BlockTransferManager blocks(options.maxBlockTransferBytes);
    for (;;) {
      if ((response.getResponseAny.invokeIdAndPriority & 0x0Fu) != invokeId) {
        EmitTrace(
          trace,
          XdlmsTraceKind::InvokeIdRejected,
          XdlmsTraceDirection::Inbound,
          XdlmsStatus::InvokeIdMismatch,
          0u,
          0u);
        return XdlmsStatus::InvokeIdMismatch;
      }

      status = blocks.AppendBlock(response.getResponseAny.dataBlock);
      if (status != XdlmsStatus::Ok) {
        return status;
      }

      XdlmsTraceContext blockTrace = trace;
      blockTrace.hasBlockNumber = true;
      blockTrace.blockNumber = response.getResponseAny.dataBlock.blockNumber;
      EmitTrace(
        blockTrace,
        XdlmsTraceKind::BlockTransferStep,
        XdlmsTraceDirection::Inbound,
        XdlmsStatus::Ok,
        0u,
        response.getResponseAny.dataBlock.rawData.size);

      if (response.getResponseAny.dataBlock.lastBlock) {
        result.invokeId = invokeId;
        result.data = blocks.Data();
        result.hasData = true;
        return XdlmsStatus::Ok;
      }

      const std::uint32_t acknowledgedBlock =
        response.getResponseAny.dataBlock.blockNumber;
      response = dlms::apdu::XdlmsApdu();
      decodedResponseBytes.clear();
      status = ReceiveGetResponse(
        channel_,
        security_,
        MakeGetRequestNext(invokeIdAndPriority, acknowledgedBlock),
        decodedResponseBytes,
        response,
        trace);
      if (status != XdlmsStatus::Ok) {
        return status;
      }
      if (response.getResponseAny.choice !=
          dlms::apdu::GetResponseChoice::WithDataBlock) {
        return XdlmsStatus::DecodeFailed;
      }
    }
  }

  if ((response.getResponse.invokeIdAndPriority & 0x0Fu) != invokeId) {
    EmitTrace(
      trace,
      XdlmsTraceKind::InvokeIdRejected,
      XdlmsTraceDirection::Inbound,
      XdlmsStatus::InvokeIdMismatch,
      0u,
      0u);
    return XdlmsStatus::InvokeIdMismatch;
  }

  result.invokeId = invokeId;
  if (response.getResponse.resultChoice ==
      dlms::apdu::GetDataResultChoice::DataAccessError) {
    result.hasAccessResult = true;
    result.accessResult = response.getResponse.dataAccessError;
    return XdlmsStatus::ServiceRejected;
  }

  return CopyEncodedData(response.getResponse.data, result);
}

XdlmsStatus XdlmsClient::Set(
  const CosemAttributeDescriptor& descriptor,
  const std::vector<std::uint8_t>& encodedData,
  SetResult& result)
{
  return Set(descriptor, encodedData, DefaultServiceOptions(), result);
}

XdlmsStatus XdlmsClient::Set(
  const CosemAttributeDescriptor& descriptor,
  const std::vector<std::uint8_t>& encodedData,
  const ServiceOptions& options,
  SetResult& result)
{
  result = EmptySetResult();

  XdlmsStatus status = ValidateDescriptor(descriptor);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  const bool useBlocks =
    encodedData.size() > options.maxSetBlockPayloadBytes;
  if (!useBlocks) {
    dlms::apdu::DlmsData data;
    status = DecodeEncodedData(encodedData, data);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    if (!association_->IsAssociated()) {
      return XdlmsStatus::NotAssociated;
    }

    const std::uint8_t invokeId = invokeIds_.Next();
    const std::uint8_t invokeIdAndPriority =
      MakeInvokeIdAndPriority(invokeId, options);

    XdlmsTraceContext trace = MakeTraceContext(
      traceSink_,
      MakeConversationId(association_->ConversationSeed(), invokeId),
      XdlmsTraceKind::SetRequest,
      XdlmsTraceKind::SetResponse,
      invokeId,
      options);
    FillDescriptor(trace, descriptor);
    channel_.SetCorrelation(trace.conversationId);

    dlms::apdu::XdlmsApdu request;
    request.kind = dlms::apdu::XdlmsApduKind::SetRequest;
    request.setRequestAny.choice = dlms::apdu::SetRequestChoice::Normal;
    request.setRequestAny.invokeIdAndPriority = invokeIdAndPriority;
    request.setRequestAny.normal.descriptor = ToApduDescriptor(descriptor);
    request.setRequestAny.normal.hasSelection = false;
    request.setRequestAny.data = data;

    dlms::apdu::XdlmsApdu response;
    std::vector<std::uint8_t> decodedResponseBytes;
    status = SendAndReceive(
      channel_,
      security_,
      request,
      decodedResponseBytes,
      response,
      trace);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    if (response.kind != dlms::apdu::XdlmsApduKind::SetResponse) {
      return XdlmsStatus::DecodeFailed;
    }

    if (response.setResponseAny.choice !=
        dlms::apdu::SetResponseChoice::Normal) {
      return response.setResponseAny.choice ==
          dlms::apdu::SetResponseChoice::DataBlock ||
          response.setResponseAny.choice ==
          dlms::apdu::SetResponseChoice::LastDataBlock
        ? XdlmsStatus::BlockTransferRequired
        : XdlmsStatus::UnsupportedFeature;
    }

    if ((response.setResponseAny.invokeIdAndPriority & 0x0Fu) != invokeId) {
      EmitTrace(
        trace,
        XdlmsTraceKind::InvokeIdRejected,
        XdlmsTraceDirection::Inbound,
        XdlmsStatus::InvokeIdMismatch,
        0u,
        0u);
      return XdlmsStatus::InvokeIdMismatch;
    }

    result.invokeId = invokeId;
    result.accessResult = response.setResponseAny.result;
    return result.accessResult == 0u
      ? XdlmsStatus::Ok
      : XdlmsStatus::ServiceRejected;
  }

  if (encodedData.empty() || options.maxSetBlockPayloadBytes == 0u) {
    return XdlmsStatus::InvalidArgument;
  }
  if (!options.allowBlockTransfer) {
    return XdlmsStatus::BlockTransferRequired;
  }
  if (!association_->IsAssociated()) {
    return XdlmsStatus::NotAssociated;
  }

  const std::uint8_t invokeId = invokeIds_.Next();
  const std::uint8_t invokeIdAndPriority =
    MakeInvokeIdAndPriority(invokeId, options);
  XdlmsTraceContext trace = MakeTraceContext(
    traceSink_,
    MakeConversationId(association_->ConversationSeed(), invokeId),
    XdlmsTraceKind::SetRequest,
    XdlmsTraceKind::SetResponse,
    invokeId,
    options);
  FillDescriptor(trace, descriptor);
  channel_.SetCorrelation(trace.conversationId);
  std::size_t offset = 0u;
  std::uint32_t blockNumber = 1u;
  while (offset < encodedData.size()) {
    const std::size_t remaining = encodedData.size() - offset;
    const std::size_t blockSize =
      remaining < options.maxSetBlockPayloadBytes
        ? remaining
        : options.maxSetBlockPayloadBytes;
    const bool finalBlock = offset + blockSize == encodedData.size();
    const dlms::apdu::SetRequestChoice choice = blockNumber == 1u
      ? dlms::apdu::SetRequestChoice::WithFirstDataBlock
      : dlms::apdu::SetRequestChoice::WithDataBlock;
    const dlms::apdu::XdlmsApdu request = MakeSetRequestBlock(
      choice,
      invokeIdAndPriority,
      descriptor,
      blockNumber,
      finalBlock,
      &encodedData[offset],
      blockSize);

    dlms::apdu::XdlmsApdu response;
    std::vector<std::uint8_t> decodedResponseBytes;
    XdlmsTraceContext blockTrace = trace;
    blockTrace.hasBlockNumber = true;
    blockTrace.blockNumber = blockNumber;
    status = SendAndReceive(
      channel_,
      security_,
      request,
      decodedResponseBytes,
      response,
      blockTrace);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    EmitTrace(
      blockTrace,
      XdlmsTraceKind::BlockTransferStep,
      XdlmsTraceDirection::Outbound,
      XdlmsStatus::Ok,
      0u,
      blockSize);

    status = ValidateSetBlockResponse(
      response,
      invokeId,
      blockNumber,
      finalBlock,
      result);
    if (status == XdlmsStatus::InvokeIdMismatch) {
      EmitTrace(
        trace,
        XdlmsTraceKind::InvokeIdRejected,
        XdlmsTraceDirection::Inbound,
        XdlmsStatus::InvokeIdMismatch,
        0u,
        0u);
    }
    if (status != XdlmsStatus::Ok) {
      return status;
    }
    if (finalBlock) {
      return XdlmsStatus::Ok;
    }

    offset += blockSize;
    ++blockNumber;
  }

  return XdlmsStatus::InternalError;
}

XdlmsStatus XdlmsClient::Action(
  const CosemMethodDescriptor& descriptor,
  bool hasParameter,
  const std::vector<std::uint8_t>& encodedParameter,
  ActionResult& result)
{
  return Action(
    descriptor,
    hasParameter,
    encodedParameter,
    DefaultServiceOptions(),
    result);
}

XdlmsStatus XdlmsClient::Action(
  const CosemMethodDescriptor& descriptor,
  bool hasParameter,
  const std::vector<std::uint8_t>& encodedParameter,
  const ServiceOptions& options,
  ActionResult& result)
{
  result = EmptyActionResult();

  XdlmsStatus status = ValidateMethodDescriptor(descriptor);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  const bool useBlocks =
    hasParameter &&
    encodedParameter.size() > options.maxActionBlockPayloadBytes;
  dlms::apdu::DlmsData parameter;
  if (hasParameter && !useBlocks) {
    status = DecodeEncodedData(encodedParameter, parameter);
    if (status != XdlmsStatus::Ok) {
      return status;
    }
  }

  if (useBlocks && options.maxActionBlockPayloadBytes == 0u) {
    return XdlmsStatus::InvalidArgument;
  }
  if (useBlocks && !options.allowBlockTransfer) {
    return XdlmsStatus::BlockTransferRequired;
  }

  if (!association_->IsAssociated()) {
    return XdlmsStatus::NotAssociated;
  }

  const std::uint8_t invokeId = invokeIds_.Next();
  const std::uint8_t invokeIdAndPriority =
    MakeInvokeIdAndPriority(invokeId, options);

  XdlmsTraceContext trace = MakeTraceContext(
    traceSink_,
    MakeConversationId(association_->ConversationSeed(), invokeId),
    XdlmsTraceKind::ActionRequest,
    XdlmsTraceKind::ActionResponse,
    invokeId,
    options);
  FillDescriptor(trace, descriptor);
  channel_.SetCorrelation(trace.conversationId);

  if (useBlocks) {
    std::size_t offset = 0u;
    std::uint32_t blockNumber = 1u;
    for (;;) {
      const std::size_t remaining = encodedParameter.size() - offset;
      const std::size_t blockSize =
        remaining < options.maxActionBlockPayloadBytes
          ? remaining
          : options.maxActionBlockPayloadBytes;
      const bool finalBlock = offset + blockSize == encodedParameter.size();
      const dlms::apdu::ActionRequestChoice choice = blockNumber == 1u
        ? dlms::apdu::ActionRequestChoice::WithFirstPblock
        : dlms::apdu::ActionRequestChoice::WithPblock;
      const dlms::apdu::XdlmsApdu requestBlock = MakeActionRequestBlock(
        choice,
        invokeIdAndPriority,
        descriptor,
        blockNumber,
        finalBlock,
        &encodedParameter[offset],
        blockSize);

      dlms::apdu::XdlmsApdu response;
      std::vector<std::uint8_t> decodedResponseBytes;
      XdlmsTraceContext blockTrace = trace;
      blockTrace.hasBlockNumber = true;
      blockTrace.blockNumber = blockNumber;
      status = ReceiveActionResponse(
        channel_,
        security_,
        requestBlock,
        decodedResponseBytes,
        response,
        blockTrace);
      if (status != XdlmsStatus::Ok) {
        return status;
      }

      EmitTrace(
        blockTrace,
        XdlmsTraceKind::BlockTransferStep,
        XdlmsTraceDirection::Outbound,
        XdlmsStatus::Ok,
        0u,
        blockSize);

      if (finalBlock) {
        return CopyFinalActionResponse(
          channel_,
          security_,
          invokeId,
          invokeIdAndPriority,
          options,
          response,
          result,
          trace);
      }

      status = ValidateActionNextPblockResponse(
        response,
        invokeId,
        blockNumber);
      if (status == XdlmsStatus::InvokeIdMismatch) {
        EmitTrace(
          trace,
          XdlmsTraceKind::InvokeIdRejected,
          XdlmsTraceDirection::Inbound,
          XdlmsStatus::InvokeIdMismatch,
          0u,
          0u);
      }
      if (status != XdlmsStatus::Ok) {
        return status;
      }

      offset += blockSize;
      ++blockNumber;
    }
  }

  dlms::apdu::XdlmsApdu request;
  request.kind = dlms::apdu::XdlmsApduKind::ActionRequest;
  request.actionRequestAny.choice = dlms::apdu::ActionRequestChoice::Normal;
  request.actionRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  request.actionRequestAny.normal.descriptor = ToApduDescriptor(descriptor);
  request.actionRequestAny.normal.hasInvocationParameter = hasParameter;
  request.actionRequestAny.normal.invocationParameter = parameter;

  dlms::apdu::XdlmsApdu response;
  std::vector<std::uint8_t> decodedResponseBytes;
  status = ReceiveActionResponse(
    channel_,
    security_,
    request,
    decodedResponseBytes,
    response,
    trace);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  return CopyFinalActionResponse(
    channel_,
    security_,
    invokeId,
    invokeIdAndPriority,
    options,
    response,
    result,
    trace);
}

} // namespace xdlms
} // namespace dlms
