#include "dlms/xdlms/xdlms_server.hpp"
#include "dlms/profile/apdu_channel.hpp"
#include "dlms/xdlms/xdlms_association_state_interface.hpp"
#include "dlms/xdlms/xdlms_correlation.hpp"

#include "dlms/apdu/action.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/set.hpp"
#include "dlms/apdu/xdlms.hpp"
#include "dlms/security/ciphered_apdu_processor.hpp"
#include "dlms/xdlms/xdlms_security_processor.hpp"
#include "dlms/xdlms/xdlms_trace.hpp"

namespace dlms {
namespace xdlms {
namespace {

void EmitServerTrace(
  IXdlmsTraceSink* sink,
  XdlmsTraceKind kind,
  XdlmsStatus status,
  std::uint8_t invokeId,
  const ServiceOptions& options,
  std::uint16_t classId,
  std::uint8_t attributeOrMethodId,
  const std::uint8_t (&logicalName)[6],
  bool hasBlockNumber,
  std::uint32_t blockNumber,
  std::size_t apduSize,
  std::size_t payloadSize,
  std::uint64_t conversationId)
{
  if (sink == 0) {
    return;
  }
  XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
  event.kind = kind;
  event.direction = XdlmsTraceDirection::Inbound;
  event.status = status;
  event.invokeId = invokeId;
  event.options = options;
  event.classId = classId;
  event.attributeOrMethodId = attributeOrMethodId;
  for (std::size_t i = 0; i < sizeof(event.logicalName); ++i) {
    event.logicalName[i] = logicalName[i];
  }
  event.hasBlockNumber = hasBlockNumber;
  event.blockNumber = blockNumber;
  event.apduSize = apduSize;
  event.payloadSize = payloadSize;
  event.conversationId = conversationId;
  sink->OnXdlmsTrace(event);
}

void EmitServerSimpleTrace(
  IXdlmsTraceSink* sink,
  XdlmsTraceKind kind,
  XdlmsStatus status,
  std::uint8_t invokeId,
  const ServiceOptions& options,
  std::size_t apduSize,
  std::uint64_t conversationId)
{
  if (sink == 0) {
    return;
  }
  XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
  event.kind = kind;
  event.direction = XdlmsTraceDirection::Inbound;
  event.status = status;
  event.invokeId = invokeId;
  event.options = options;
  event.apduSize = apduSize;
  event.conversationId = conversationId;
  sink->OnXdlmsTrace(event);
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

ServiceOptions ParseServiceOptions(
  std::uint8_t invokeIdAndPriority,
  const ServiceOptions& defaults)
{
  ServiceOptions options = defaults;
  options.confirmed = (invokeIdAndPriority & 0x80u) != 0u;
  options.highPriority = (invokeIdAndPriority & 0x40u) != 0u;
  return options;
}

CosemAttributeDescriptor ToXdlmsDescriptor(
  const dlms::apdu::CosemAttributeDescriptor& descriptor)
{
  CosemAttributeDescriptor xdlmsDescriptor;
  xdlmsDescriptor.classId = descriptor.classId;
  xdlmsDescriptor.instanceId = CosemLogicalName(
    descriptor.logicalName[0],
    descriptor.logicalName[1],
    descriptor.logicalName[2],
    descriptor.logicalName[3],
    descriptor.logicalName[4],
    descriptor.logicalName[5]);
  xdlmsDescriptor.attributeId = descriptor.attributeId;
  return xdlmsDescriptor;
}

CosemMethodDescriptor ToXdlmsDescriptor(
  const dlms::apdu::CosemMethodDescriptor& descriptor)
{
  CosemMethodDescriptor xdlmsDescriptor;
  xdlmsDescriptor.classId = descriptor.classId;
  xdlmsDescriptor.instanceId = CosemLogicalName(
    descriptor.logicalName[0],
    descriptor.logicalName[1],
    descriptor.logicalName[2],
    descriptor.logicalName[3],
    descriptor.logicalName[4],
    descriptor.logicalName[5]);
  xdlmsDescriptor.methodId = descriptor.methodId;
  return xdlmsDescriptor;
}

XdlmsStatus DecodeEncodedData(
  const std::vector<std::uint8_t>& encodedData,
  dlms::apdu::DlmsData& output)
{
  if (encodedData.empty()) {
    return XdlmsStatus::EncodeFailed;
  }

  return dlms::apdu::DecodeDlmsData(
      &encodedData[0],
      encodedData.size(),
      8,
      output) == dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus EncodeDataBytes(
  const dlms::apdu::DlmsData& data,
  std::vector<std::uint8_t>& output)
{
  std::vector<std::uint8_t> buffer(65535u);
  dlms::apdu::ApduWriter writer(&buffer[0], buffer.size());
  if (dlms::apdu::EncodeDlmsData(data, writer) !=
      dlms::apdu::ApduStatus::Ok) {
    return XdlmsStatus::EncodeFailed;
  }

  output.assign(buffer.begin(), buffer.begin() + writer.WrittenSize());
  return XdlmsStatus::Ok;
}

XdlmsStatus ValidateEncodedActionParameter(
  const std::vector<std::uint8_t>& encodedData)
{
  if (encodedData.empty()) {
    return XdlmsStatus::DecodeFailed;
  }

  dlms::apdu::DlmsData data;
  return dlms::apdu::DecodeDlmsData(
      &encodedData[0],
      encodedData.size(),
      8,
      data) == dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::DecodeFailed;
}

XdlmsStatus EncodeGetResponse(
  std::uint8_t invokeIdAndPriority,
  const GetResult& result,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::GetResponse;
  response.getResponseAny.choice = dlms::apdu::GetResponseChoice::Normal;
  response.getResponseAny.invokeIdAndPriority = invokeIdAndPriority;

  if (result.hasAccessResult) {
    response.getResponseAny.result.choice =
      dlms::apdu::GetDataResultChoice::DataAccessError;
    response.getResponseAny.result.dataAccessError = result.accessResult;
  } else if (result.hasData) {
    response.getResponseAny.result.choice =
      dlms::apdu::GetDataResultChoice::Data;
    const XdlmsStatus status =
      DecodeEncodedData(result.data, response.getResponseAny.result.data);
    if (status != XdlmsStatus::Ok) {
      return status;
    }
  } else {
    return XdlmsStatus::InternalError;
  }

  response.getResponse.invokeIdAndPriority = invokeIdAndPriority;
  response.getResponse.resultChoice = response.getResponseAny.result.choice;
  response.getResponse.data = response.getResponseAny.result.data;
  response.getResponse.dataAccessError =
    response.getResponseAny.result.dataAccessError;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus EncodeGetDataBlockResponse(
  std::uint8_t invokeIdAndPriority,
  bool lastBlock,
  std::uint32_t blockNumber,
  const std::uint8_t* data,
  std::size_t dataSize,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::GetResponse;
  response.getResponseAny.choice =
    dlms::apdu::GetResponseChoice::WithDataBlock;
  response.getResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.getResponseAny.dataBlock.lastBlock = lastBlock;
  response.getResponseAny.dataBlock.blockNumber = blockNumber;
  response.getResponseAny.dataBlock.rawData.data = data;
  response.getResponseAny.dataBlock.rawData.size = dataSize;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus SendNextGetResponseBlock(
  GetResponseBlockState& blocks,
  std::vector<std::uint8_t>& responseApdu)
{
  if (!blocks.active ||
      blocks.options.maxGetBlockPayloadBytes == 0u ||
      blocks.offset >= blocks.data.size()) {
    blocks = EmptyGetResponseBlockState();
    return XdlmsStatus::InvalidArgument;
  }

  const std::size_t remaining = blocks.data.size() - blocks.offset;
  const std::size_t blockSize =
    remaining < blocks.options.maxGetBlockPayloadBytes
      ? remaining
      : blocks.options.maxGetBlockPayloadBytes;
  const bool lastBlock = blocks.offset + blockSize == blocks.data.size();
  const std::uint32_t blockNumber = blocks.nextBlockNumber;
  const std::uint8_t responseInvokeIdAndPriority =
    MakeInvokeIdAndPriority(blocks.invokeId, blocks.options);

  const XdlmsStatus status =
    EncodeGetDataBlockResponse(
      responseInvokeIdAndPriority,
      lastBlock,
      blockNumber,
      blockSize == 0u ? 0 : &blocks.data[blocks.offset],
      blockSize,
      responseApdu);
  if (status != XdlmsStatus::Ok) {
    blocks = EmptyGetResponseBlockState();
    return status;
  }

  blocks.offset += blockSize;
  ++blocks.nextBlockNumber;
  if (lastBlock) {
    blocks = EmptyGetResponseBlockState();
  }
  return XdlmsStatus::Ok;
}

XdlmsStatus EncodeGetResponseOrStartBlocks(
  std::uint8_t invokeIdAndPriority,
  const GetIndication& indication,
  const GetResult& result,
  GetResponseBlockState& blocks,
  std::vector<std::uint8_t>& responseApdu)
{
  if (!result.hasData || result.data.size() <=
        indication.options.maxGetBlockPayloadBytes) {
    return EncodeGetResponse(invokeIdAndPriority, result, responseApdu);
  }

  if (result.data.size() > indication.options.maxBlockTransferBytes) {
    return XdlmsStatus::DecodeFailed;
  }

  if (!indication.options.allowBlockTransfer) {
    return XdlmsStatus::BlockTransferRequired;
  }

  if (indication.options.maxGetBlockPayloadBytes == 0u) {
    return XdlmsStatus::InvalidArgument;
  }

  blocks.active = true;
  blocks.invokeId = indication.invokeId;
  blocks.options = indication.options;
  blocks.nextBlockNumber = 1u;
  blocks.offset = 0u;
  blocks.data = result.data;
  return SendNextGetResponseBlock(blocks, responseApdu);
}

XdlmsStatus EncodeSetResponse(
  std::uint8_t invokeIdAndPriority,
  const SetResult& result,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::SetResponse;
  response.setResponseAny.choice = dlms::apdu::SetResponseChoice::Normal;
  response.setResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.setResponseAny.result = result.accessResult;
  response.setResponse.invokeIdAndPriority = invokeIdAndPriority;
  response.setResponse.result = result.accessResult;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus EncodeSetBlockAckResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::SetResponse;
  response.setResponseAny.choice = dlms::apdu::SetResponseChoice::DataBlock;
  response.setResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.setResponseAny.blockNumber = blockNumber;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus EncodeSetLastBlockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber,
  const SetResult& result,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::SetResponse;
  response.setResponseAny.choice =
    dlms::apdu::SetResponseChoice::LastDataBlock;
  response.setResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.setResponseAny.blockNumber = blockNumber;
  response.setResponseAny.result = result.accessResult;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus EncodeActionResponse(
  std::uint8_t invokeIdAndPriority,
  const ActionResult& result,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::ActionResponse;
  response.actionResponseAny.choice =
    dlms::apdu::ActionResponseChoice::Normal;
  response.actionResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponseAny.normal.result = result.actionResult;
  response.actionResponseAny.normal.hasReturnParameter = result.hasData;

  if (result.hasData) {
    const XdlmsStatus status =
      DecodeEncodedData(result.data,
                        response.actionResponseAny.normal.returnParameter);
    if (status != XdlmsStatus::Ok) {
      return status;
    }
  }

  response.actionResponse.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponse.result = result.actionResult;
  response.actionResponse.hasReturnParameter = result.hasData;
  response.actionResponse.returnParameter =
    response.actionResponseAny.normal.returnParameter;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus EncodeActionNextPblockResponse(
  std::uint8_t invokeIdAndPriority,
  std::uint32_t blockNumber,
  std::vector<std::uint8_t>& responseApdu)
{
  dlms::apdu::XdlmsApdu response;
  response.kind = dlms::apdu::XdlmsApduKind::ActionResponse;
  response.actionResponseAny.choice =
    dlms::apdu::ActionResponseChoice::NextPblock;
  response.actionResponseAny.invokeIdAndPriority = invokeIdAndPriority;
  response.actionResponseAny.blockNumber = blockNumber;

  return dlms::apdu::EncodeXdlmsApdu(response, responseApdu) ==
      dlms::apdu::ApduStatus::Ok
    ? XdlmsStatus::Ok
    : XdlmsStatus::EncodeFailed;
}

XdlmsStatus ProcessGetRequest(
  const dlms::apdu::XdlmsApdu& request,
  IXdlmsServerDispatcher& dispatcher,
  const ServiceOptions& processorOptions,
  GetResponseBlockState& getBlocks,
  std::vector<std::uint8_t>& responseApdu,
  IXdlmsTraceSink* traceSink,
  std::uint64_t conversationId)
{
  if (request.getRequestAny.choice == dlms::apdu::GetRequestChoice::Next) {
    if (!getBlocks.active) {
      return XdlmsStatus::DecodeFailed;
    }

    const std::uint8_t invokeId = static_cast<std::uint8_t>(
      request.getRequestAny.invokeIdAndPriority & 0x0Fu);
    if (invokeId != getBlocks.invokeId) {
      EmitServerSimpleTrace(
        traceSink,
        XdlmsTraceKind::InvokeIdRejected,
        XdlmsStatus::InvokeIdMismatch,
        invokeId,
        getBlocks.options,
        0u,
        conversationId);
      getBlocks = EmptyGetResponseBlockState();
      return XdlmsStatus::InvokeIdMismatch;
    }

    if (request.getRequestAny.blockNumber !=
        getBlocks.nextBlockNumber - 1u) {
      getBlocks = EmptyGetResponseBlockState();
      return XdlmsStatus::DecodeFailed;
    }

    const std::uint32_t blockNumber = getBlocks.nextBlockNumber;
    const std::uint8_t blockInvokeId = getBlocks.invokeId;
    const ServiceOptions blockOptions = getBlocks.options;
    const XdlmsStatus blockStatus =
      SendNextGetResponseBlock(getBlocks, responseApdu);
    if (blockStatus == XdlmsStatus::Ok && traceSink != 0) {
      XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
      event.kind = XdlmsTraceKind::BlockTransferStep;
      event.direction = XdlmsTraceDirection::Inbound;
      event.status = XdlmsStatus::Ok;
      event.invokeId = blockInvokeId;
      event.options = blockOptions;
      event.hasBlockNumber = true;
      event.blockNumber = blockNumber;
      event.apduSize = responseApdu.size();
      event.conversationId = conversationId;
      traceSink->OnXdlmsTrace(event);
    }
    return blockStatus;
  }

  if (request.getRequestAny.choice != dlms::apdu::GetRequestChoice::Normal) {
    return XdlmsStatus::UnsupportedFeature;
  }

  if (getBlocks.active) {
    getBlocks = EmptyGetResponseBlockState();
    return XdlmsStatus::DecodeFailed;
  }

  if (request.getRequest.hasSelectiveAccess) {
    return XdlmsStatus::UnsupportedFeature;
  }

  GetIndication indication = EmptyGetIndication();
  indication.invokeId =
    static_cast<std::uint8_t>(request.getRequest.invokeIdAndPriority & 0x0Fu);
  indication.options =
    ParseServiceOptions(
      request.getRequest.invokeIdAndPriority,
      processorOptions);
  indication.descriptor = ToXdlmsDescriptor(request.getRequest.descriptor);

  if (!indication.options.confirmed) {
    return XdlmsStatus::UnsupportedFeature;
  }

  GetResult result = EmptyGetResult();
  const XdlmsStatus status = dispatcher.DispatchGet(indication, result);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  const std::uint8_t responseInvokeIdAndPriority =
    MakeInvokeIdAndPriority(indication.invokeId, indication.options);
  return EncodeGetResponseOrStartBlocks(
    responseInvokeIdAndPriority,
    indication,
    result,
    getBlocks,
    responseApdu);
}

XdlmsStatus ProcessSetRequest(
  const dlms::apdu::XdlmsApdu& request,
  IXdlmsServerDispatcher& dispatcher,
  const ServiceOptions& processorOptions,
  SetRequestBlockState& setBlocks,
  std::vector<std::uint8_t>& responseApdu,
  IXdlmsTraceSink* traceSink,
  std::uint64_t conversationId)
{
  if (request.setRequestAny.choice ==
      dlms::apdu::SetRequestChoice::WithFirstDataBlock) {
    if (setBlocks.active) {
      setBlocks = EmptySetRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }
    if (request.setRequestAny.normal.hasSelection) {
      return XdlmsStatus::UnsupportedFeature;
    }
    if (request.setRequestAny.dataBlock.blockNumber != 1u) {
      return XdlmsStatus::DecodeFailed;
    }

    const std::uint8_t invokeId = static_cast<std::uint8_t>(
      request.setRequestAny.invokeIdAndPriority & 0x0Fu);
    const ServiceOptions options =
      ParseServiceOptions(
        request.setRequestAny.invokeIdAndPriority,
        processorOptions);
    if (!options.confirmed) {
      return XdlmsStatus::UnsupportedFeature;
    }
    if (request.setRequestAny.dataBlock.rawData.size >
        options.maxBlockTransferBytes) {
      return XdlmsStatus::DecodeFailed;
    }
    if (request.setRequestAny.dataBlock.rawData.size != 0u &&
        request.setRequestAny.dataBlock.rawData.data == 0) {
      return XdlmsStatus::DecodeFailed;
    }

    std::vector<std::uint8_t> data;
    if (request.setRequestAny.dataBlock.rawData.size != 0u) {
      data.assign(
        request.setRequestAny.dataBlock.rawData.data,
        request.setRequestAny.dataBlock.rawData.data +
          request.setRequestAny.dataBlock.rawData.size);
    }

    if (!request.setRequestAny.dataBlock.lastBlock) {
      setBlocks.active = true;
      setBlocks.invokeId = invokeId;
      setBlocks.options = options;
      setBlocks.descriptor =
        ToXdlmsDescriptor(request.setRequestAny.normal.descriptor);
      setBlocks.nextBlockNumber = 2u;
      setBlocks.data = data;
      const XdlmsStatus ackStatus = EncodeSetBlockAckResponse(
        request.setRequestAny.invokeIdAndPriority,
        1u,
        responseApdu);
      if (ackStatus == XdlmsStatus::Ok && traceSink != 0) {
        XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
        event.kind = XdlmsTraceKind::BlockTransferStep;
        event.direction = XdlmsTraceDirection::Inbound;
        event.status = XdlmsStatus::Ok;
        event.invokeId = invokeId;
        event.options = options;
        event.hasBlockNumber = true;
        event.blockNumber = 1u;
        event.apduSize = responseApdu.size();
        event.conversationId = conversationId;
      traceSink->OnXdlmsTrace(event);
      }
      return ackStatus;
    }

    SetIndication indication = EmptySetIndication();
    indication.invokeId = invokeId;
    indication.options = options;
    indication.descriptor =
      ToXdlmsDescriptor(request.setRequestAny.normal.descriptor);
    indication.data = data;

    SetResult result = EmptySetResult();
    XdlmsStatus status = dispatcher.DispatchSet(indication, result);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    const std::uint8_t responseInvokeIdAndPriority =
      MakeInvokeIdAndPriority(indication.invokeId, indication.options);
    return EncodeSetLastBlockResponse(
      responseInvokeIdAndPriority,
      1u,
      result,
      responseApdu);
  }

  if (request.setRequestAny.choice ==
      dlms::apdu::SetRequestChoice::WithDataBlock) {
    if (!setBlocks.active) {
      return XdlmsStatus::DecodeFailed;
    }
    if ((request.setRequestAny.invokeIdAndPriority & 0x0Fu) !=
        setBlocks.invokeId) {
      EmitServerSimpleTrace(
        traceSink,
        XdlmsTraceKind::InvokeIdRejected,
        XdlmsStatus::InvokeIdMismatch,
        static_cast<std::uint8_t>(
          request.setRequestAny.invokeIdAndPriority & 0x0Fu),
        setBlocks.options,
        0u,
        conversationId);
      setBlocks = EmptySetRequestBlockState();
      return XdlmsStatus::InvokeIdMismatch;
    }
    if (request.setRequestAny.dataBlock.blockNumber !=
        setBlocks.nextBlockNumber) {
      setBlocks = EmptySetRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }
    if (request.setRequestAny.dataBlock.rawData.size >
          setBlocks.options.maxBlockTransferBytes ||
        setBlocks.data.size() >
          setBlocks.options.maxBlockTransferBytes -
          request.setRequestAny.dataBlock.rawData.size) {
      setBlocks = EmptySetRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }
    if (request.setRequestAny.dataBlock.rawData.size != 0u &&
        request.setRequestAny.dataBlock.rawData.data == 0) {
      setBlocks = EmptySetRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }

    if (request.setRequestAny.dataBlock.rawData.size != 0u) {
      setBlocks.data.insert(
        setBlocks.data.end(),
        request.setRequestAny.dataBlock.rawData.data,
        request.setRequestAny.dataBlock.rawData.data +
          request.setRequestAny.dataBlock.rawData.size);
    }

    const std::uint32_t acceptedBlock =
      request.setRequestAny.dataBlock.blockNumber;
    if (!request.setRequestAny.dataBlock.lastBlock) {
      ++setBlocks.nextBlockNumber;
      const XdlmsStatus ackStatus = EncodeSetBlockAckResponse(
        request.setRequestAny.invokeIdAndPriority,
        acceptedBlock,
        responseApdu);
      if (ackStatus == XdlmsStatus::Ok && traceSink != 0) {
        XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
        event.kind = XdlmsTraceKind::BlockTransferStep;
        event.direction = XdlmsTraceDirection::Inbound;
        event.status = XdlmsStatus::Ok;
        event.invokeId = setBlocks.invokeId;
        event.options = setBlocks.options;
        event.hasBlockNumber = true;
        event.blockNumber = acceptedBlock;
        event.apduSize = responseApdu.size();
        event.conversationId = conversationId;
      traceSink->OnXdlmsTrace(event);
      }
      return ackStatus;
    }

    SetIndication indication = EmptySetIndication();
    indication.invokeId = setBlocks.invokeId;
    indication.options = setBlocks.options;
    indication.descriptor = setBlocks.descriptor;
    indication.data = setBlocks.data;
    setBlocks = EmptySetRequestBlockState();

    SetResult result = EmptySetResult();
    XdlmsStatus status = dispatcher.DispatchSet(indication, result);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    const std::uint8_t responseInvokeIdAndPriority =
      MakeInvokeIdAndPriority(indication.invokeId, indication.options);
    return EncodeSetLastBlockResponse(
      responseInvokeIdAndPriority,
      acceptedBlock,
      result,
      responseApdu);
  }

  if (request.setRequestAny.choice != dlms::apdu::SetRequestChoice::Normal) {
    return XdlmsStatus::UnsupportedFeature;
  }

  if (request.setRequest.hasSelectiveAccess) {
    return XdlmsStatus::UnsupportedFeature;
  }

  SetIndication indication = EmptySetIndication();
  indication.invokeId =
    static_cast<std::uint8_t>(request.setRequest.invokeIdAndPriority & 0x0Fu);
  indication.options =
    ParseServiceOptions(
      request.setRequest.invokeIdAndPriority,
      processorOptions);
  indication.descriptor = ToXdlmsDescriptor(request.setRequest.descriptor);

  if (!indication.options.confirmed) {
    return XdlmsStatus::UnsupportedFeature;
  }

  XdlmsStatus status = EncodeDataBytes(request.setRequest.data, indication.data);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  SetResult result = EmptySetResult();
  status = dispatcher.DispatchSet(indication, result);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  const std::uint8_t responseInvokeIdAndPriority =
    MakeInvokeIdAndPriority(indication.invokeId, indication.options);
  return EncodeSetResponse(responseInvokeIdAndPriority, result, responseApdu);
}

XdlmsStatus ProcessActionRequest(
  const dlms::apdu::XdlmsApdu& request,
  IXdlmsServerDispatcher& dispatcher,
  const ServiceOptions& processorOptions,
  ActionRequestBlockState& actionBlocks,
  std::vector<std::uint8_t>& responseApdu,
  IXdlmsTraceSink* traceSink,
  std::uint64_t conversationId)
{
  if (request.actionRequestAny.choice ==
      dlms::apdu::ActionRequestChoice::WithFirstPblock) {
    if (actionBlocks.active) {
      actionBlocks = EmptyActionRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }
    if (request.actionRequestAny.dataBlock.blockNumber != 1u) {
      return XdlmsStatus::DecodeFailed;
    }

    const std::uint8_t invokeId = static_cast<std::uint8_t>(
      request.actionRequestAny.invokeIdAndPriority & 0x0Fu);
    const ServiceOptions options =
      ParseServiceOptions(
        request.actionRequestAny.invokeIdAndPriority,
        processorOptions);
    if (!options.confirmed) {
      return XdlmsStatus::UnsupportedFeature;
    }

    if (request.actionRequestAny.dataBlock.rawData.size >
        options.maxBlockTransferBytes) {
      return XdlmsStatus::DecodeFailed;
    }
    if (request.actionRequestAny.dataBlock.rawData.size != 0u &&
        request.actionRequestAny.dataBlock.rawData.data == 0) {
      return XdlmsStatus::DecodeFailed;
    }

    std::vector<std::uint8_t> data;
    if (request.actionRequestAny.dataBlock.rawData.size != 0u) {
      data.assign(
        request.actionRequestAny.dataBlock.rawData.data,
        request.actionRequestAny.dataBlock.rawData.data +
          request.actionRequestAny.dataBlock.rawData.size);
    }

    if (!request.actionRequestAny.dataBlock.lastBlock) {
      actionBlocks.active = true;
      actionBlocks.invokeId = invokeId;
      actionBlocks.options = options;
      actionBlocks.descriptor =
        ToXdlmsDescriptor(request.actionRequestAny.normal.descriptor);
      actionBlocks.nextBlockNumber = 2u;
      actionBlocks.data = data;
      const XdlmsStatus ackStatus = EncodeActionNextPblockResponse(
        request.actionRequestAny.invokeIdAndPriority,
        1u,
        responseApdu);
      if (ackStatus == XdlmsStatus::Ok && traceSink != 0) {
        XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
        event.kind = XdlmsTraceKind::BlockTransferStep;
        event.direction = XdlmsTraceDirection::Inbound;
        event.status = XdlmsStatus::Ok;
        event.invokeId = invokeId;
        event.options = options;
        event.hasBlockNumber = true;
        event.blockNumber = 1u;
        event.apduSize = responseApdu.size();
        event.conversationId = conversationId;
      traceSink->OnXdlmsTrace(event);
      }
      return ackStatus;
    }

    XdlmsStatus status = ValidateEncodedActionParameter(data);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    ActionIndication indication = EmptyActionIndication();
    indication.invokeId = invokeId;
    indication.options = options;
    indication.descriptor =
      ToXdlmsDescriptor(request.actionRequestAny.normal.descriptor);
    indication.hasParameter = true;
    indication.parameter = data;

    ActionResult result = EmptyActionResult();
    status = dispatcher.DispatchAction(indication, result);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    const std::uint8_t responseInvokeIdAndPriority =
      MakeInvokeIdAndPriority(indication.invokeId, indication.options);
    return EncodeActionResponse(responseInvokeIdAndPriority, result, responseApdu);
  }

  if (request.actionRequestAny.choice ==
      dlms::apdu::ActionRequestChoice::WithPblock) {
    if (!actionBlocks.active) {
      return XdlmsStatus::DecodeFailed;
    }
    if ((request.actionRequestAny.invokeIdAndPriority & 0x0Fu) !=
        actionBlocks.invokeId) {
      EmitServerSimpleTrace(
        traceSink,
        XdlmsTraceKind::InvokeIdRejected,
        XdlmsStatus::InvokeIdMismatch,
        static_cast<std::uint8_t>(
          request.actionRequestAny.invokeIdAndPriority & 0x0Fu),
        actionBlocks.options,
        0u,
        conversationId);
      actionBlocks = EmptyActionRequestBlockState();
      return XdlmsStatus::InvokeIdMismatch;
    }
    if (request.actionRequestAny.dataBlock.blockNumber !=
        actionBlocks.nextBlockNumber) {
      actionBlocks = EmptyActionRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }
    if (request.actionRequestAny.dataBlock.rawData.size >
          actionBlocks.options.maxBlockTransferBytes ||
        actionBlocks.data.size() >
          actionBlocks.options.maxBlockTransferBytes -
          request.actionRequestAny.dataBlock.rawData.size) {
      actionBlocks = EmptyActionRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }
    if (request.actionRequestAny.dataBlock.rawData.size != 0u &&
        request.actionRequestAny.dataBlock.rawData.data == 0) {
      actionBlocks = EmptyActionRequestBlockState();
      return XdlmsStatus::DecodeFailed;
    }

    if (request.actionRequestAny.dataBlock.rawData.size != 0u) {
      actionBlocks.data.insert(
        actionBlocks.data.end(),
        request.actionRequestAny.dataBlock.rawData.data,
        request.actionRequestAny.dataBlock.rawData.data +
          request.actionRequestAny.dataBlock.rawData.size);
    }

    const std::uint32_t acceptedBlock =
      request.actionRequestAny.dataBlock.blockNumber;
    if (!request.actionRequestAny.dataBlock.lastBlock) {
      ++actionBlocks.nextBlockNumber;
      const XdlmsStatus ackStatus = EncodeActionNextPblockResponse(
        request.actionRequestAny.invokeIdAndPriority,
        acceptedBlock,
        responseApdu);
      if (ackStatus == XdlmsStatus::Ok && traceSink != 0) {
        XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
        event.kind = XdlmsTraceKind::BlockTransferStep;
        event.direction = XdlmsTraceDirection::Inbound;
        event.status = XdlmsStatus::Ok;
        event.invokeId = actionBlocks.invokeId;
        event.options = actionBlocks.options;
        event.hasBlockNumber = true;
        event.blockNumber = acceptedBlock;
        event.apduSize = responseApdu.size();
        event.conversationId = conversationId;
      traceSink->OnXdlmsTrace(event);
      }
      return ackStatus;
    }

    ActionIndication indication = EmptyActionIndication();
    indication.invokeId = actionBlocks.invokeId;
    indication.options = actionBlocks.options;
    indication.descriptor = actionBlocks.descriptor;
    indication.hasParameter = true;
    indication.parameter = actionBlocks.data;
    actionBlocks = EmptyActionRequestBlockState();

    XdlmsStatus status =
      ValidateEncodedActionParameter(indication.parameter);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    ActionResult result = EmptyActionResult();
    status = dispatcher.DispatchAction(indication, result);
    if (status != XdlmsStatus::Ok) {
      return status;
    }

    const std::uint8_t responseInvokeIdAndPriority =
      MakeInvokeIdAndPriority(indication.invokeId, indication.options);
    return EncodeActionResponse(responseInvokeIdAndPriority, result, responseApdu);
  }

  if (request.actionRequestAny.choice !=
      dlms::apdu::ActionRequestChoice::Normal) {
    return XdlmsStatus::UnsupportedFeature;
  }

  ActionIndication indication = EmptyActionIndication();
  indication.invokeId = static_cast<std::uint8_t>(
    request.actionRequest.invokeIdAndPriority & 0x0Fu);
  indication.options =
    ParseServiceOptions(
      request.actionRequest.invokeIdAndPriority,
      processorOptions);
  indication.descriptor = ToXdlmsDescriptor(request.actionRequest.descriptor);
  indication.hasParameter = request.actionRequest.hasInvocationParameter;

  if (!indication.options.confirmed) {
    return XdlmsStatus::UnsupportedFeature;
  }

  if (indication.hasParameter) {
    const XdlmsStatus status =
      EncodeDataBytes(request.actionRequest.invocationParameter,
                      indication.parameter);
    if (status != XdlmsStatus::Ok) {
      return status;
    }
  }

  ActionResult result = EmptyActionResult();
  const XdlmsStatus status = dispatcher.DispatchAction(indication, result);
  if (status != XdlmsStatus::Ok) {
    return status;
  }

  const std::uint8_t responseInvokeIdAndPriority =
    MakeInvokeIdAndPriority(indication.invokeId, indication.options);
  return EncodeActionResponse(
    responseInvokeIdAndPriority,
    result,
    responseApdu);
}

} // namespace

IXdlmsServerHandler::~IXdlmsServerHandler()
{
}

XdlmsStatus IXdlmsServerHandler::HandleSet(
  const SetIndication& indication,
  SetResult& result)
{
  (void)indication;
  (void)result;
  return XdlmsStatus::UnsupportedFeature;
}

XdlmsStatus IXdlmsServerHandler::HandleAction(
  const ActionIndication& indication,
  ActionResult& result)
{
  (void)indication;
  (void)result;
  return XdlmsStatus::UnsupportedFeature;
}

IXdlmsServerDispatcher::~IXdlmsServerDispatcher()
{
}

XdlmsServerDispatcher::XdlmsServerDispatcher(IXdlmsServerHandler& handler)
  : handler_(handler)
{
}

XdlmsStatus XdlmsServerDispatcher::DispatchGet(
  const GetIndication& indication,
  GetResult& result)
{
  XdlmsStatus status = ValidateInvokeId(indication.invokeId);
  if (status != XdlmsStatus::Ok) {
    result = EmptyGetResult();
    return status;
  }

  status = ValidateDescriptor(indication.descriptor);
  if (status != XdlmsStatus::Ok) {
    result = EmptyGetResult();
    return status;
  }

  GetResult handlerResult = EmptyGetResult();
  status = handler_.HandleGet(indication, handlerResult);
  if (status != XdlmsStatus::Ok) {
    result = EmptyGetResult();
    return status;
  }

  handlerResult.invokeId = indication.invokeId;
  result = handlerResult;
  return XdlmsStatus::Ok;
}

XdlmsStatus XdlmsServerDispatcher::DispatchSet(
  const SetIndication& indication,
  SetResult& result)
{
  XdlmsStatus status = ValidateInvokeId(indication.invokeId);
  if (status != XdlmsStatus::Ok) {
    result = EmptySetResult();
    return status;
  }

  status = ValidateDescriptor(indication.descriptor);
  if (status != XdlmsStatus::Ok) {
    result = EmptySetResult();
    return status;
  }

  if (indication.data.empty()) {
    result = EmptySetResult();
    return XdlmsStatus::InvalidArgument;
  }

  SetResult handlerResult = EmptySetResult();
  status = handler_.HandleSet(indication, handlerResult);
  if (status != XdlmsStatus::Ok) {
    result = EmptySetResult();
    return status;
  }

  handlerResult.invokeId = indication.invokeId;
  result = handlerResult;
  return XdlmsStatus::Ok;
}

XdlmsStatus XdlmsServerDispatcher::DispatchAction(
  const ActionIndication& indication,
  ActionResult& result)
{
  XdlmsStatus status = ValidateInvokeId(indication.invokeId);
  if (status != XdlmsStatus::Ok) {
    result = EmptyActionResult();
    return status;
  }

  status = ValidateMethodDescriptor(indication.descriptor);
  if (status != XdlmsStatus::Ok) {
    result = EmptyActionResult();
    return status;
  }

  if (indication.hasParameter && indication.parameter.empty()) {
    result = EmptyActionResult();
    return XdlmsStatus::InvalidArgument;
  }

  ActionResult handlerResult = EmptyActionResult();
  status = handler_.HandleAction(indication, handlerResult);
  if (status != XdlmsStatus::Ok) {
    result = EmptyActionResult();
    return status;
  }

  handlerResult.invokeId = indication.invokeId;
  result = handlerResult;
  return XdlmsStatus::Ok;
}

XdlmsServerApduProcessor::XdlmsServerApduProcessor(
  IXdlmsServerDispatcher& dispatcher)
  : dispatcher_(dispatcher)
  , ownedSecurity_()
  , security_(0)
  , options_(DefaultServiceOptions())
  , getBlocks_(EmptyGetResponseBlockState())
  , setBlocks_(EmptySetRequestBlockState())
  , actionBlocks_(EmptyActionRequestBlockState())
  , traceSink_(0)
  , channel_(0)
  , seedSource_(0)
{
}

XdlmsServerApduProcessor::XdlmsServerApduProcessor(
  IXdlmsServerDispatcher& dispatcher,
  const ServiceOptions& options)
  : dispatcher_(dispatcher)
  , ownedSecurity_()
  , security_(0)
  , options_(options)
  , getBlocks_(EmptyGetResponseBlockState())
  , setBlocks_(EmptySetRequestBlockState())
  , actionBlocks_(EmptyActionRequestBlockState())
  , traceSink_(0)
  , channel_(0)
  , seedSource_(0)
{
}

XdlmsServerApduProcessor::XdlmsServerApduProcessor(
  IXdlmsServerDispatcher& dispatcher,
  IXdlmsSecurityProcessor& security)
  : dispatcher_(dispatcher)
  , ownedSecurity_()
  , security_(&security)
  , options_(DefaultServiceOptions())
  , getBlocks_(EmptyGetResponseBlockState())
  , setBlocks_(EmptySetRequestBlockState())
  , actionBlocks_(EmptyActionRequestBlockState())
  , traceSink_(0)
  , channel_(0)
  , seedSource_(0)
{
}

XdlmsServerApduProcessor::XdlmsServerApduProcessor(
  IXdlmsServerDispatcher& dispatcher,
  IXdlmsSecurityProcessor& security,
  const ServiceOptions& options)
  : dispatcher_(dispatcher)
  , ownedSecurity_()
  , security_(&security)
  , options_(options)
  , getBlocks_(EmptyGetResponseBlockState())
  , setBlocks_(EmptySetRequestBlockState())
  , actionBlocks_(EmptyActionRequestBlockState())
  , traceSink_(0)
  , channel_(0)
  , seedSource_(0)
{
}

XdlmsServerApduProcessor::XdlmsServerApduProcessor(
  IXdlmsServerDispatcher& dispatcher,
  dlms::security::CipheredApduProcessor& security)
  : dispatcher_(dispatcher)
  , ownedSecurity_(new CipheredXdlmsSecurityProcessor(security))
  , security_(ownedSecurity_.get())
  , options_(DefaultServiceOptions())
  , getBlocks_(EmptyGetResponseBlockState())
  , setBlocks_(EmptySetRequestBlockState())
  , actionBlocks_(EmptyActionRequestBlockState())
  , traceSink_(0)
  , channel_(0)
  , seedSource_(0)
{
}

XdlmsServerApduProcessor::XdlmsServerApduProcessor(
  IXdlmsServerDispatcher& dispatcher,
  dlms::security::CipheredApduProcessor& security,
  const ServiceOptions& options)
  : dispatcher_(dispatcher)
  , ownedSecurity_(new CipheredXdlmsSecurityProcessor(security))
  , security_(ownedSecurity_.get())
  , options_(options)
  , getBlocks_(EmptyGetResponseBlockState())
  , setBlocks_(EmptySetRequestBlockState())
  , actionBlocks_(EmptyActionRequestBlockState())
  , traceSink_(0)
  , channel_(0)
  , seedSource_(0)
{
}

XdlmsStatus XdlmsServerApduProcessor::ProcessRequest(
  const std::vector<std::uint8_t>& requestApdu,
  std::vector<std::uint8_t>& responseApdu)
{
  responseApdu.clear();

  std::vector<std::uint8_t> plainRequest = requestApdu;
  if (security_ != 0) {
    dlms::security::SecurityByteView protectedApdu;
    protectedApdu.data = requestApdu.empty() ? 0 : &requestApdu[0];
    protectedApdu.size = requestApdu.size();
    const dlms::security::SecurityStatus status =
      security_->Unprotect(protectedApdu, plainRequest);
    if (status != dlms::security::SecurityStatus::Ok) {
      EmitServerSimpleTrace(
        traceSink_,
        XdlmsTraceKind::SecurityFailed,
        XdlmsStatus::SecurityFailed,
        0u,
        options_,
        requestApdu.size(),
        0u);
      return XdlmsStatus::SecurityFailed;
    }
  }

  dlms::apdu::XdlmsApdu request;
  if (dlms::apdu::DecodeXdlmsApdu(
        plainRequest.empty() ? 0 : &plainRequest[0],
        plainRequest.size(),
        request) != dlms::apdu::ApduStatus::Ok) {
    EmitServerSimpleTrace(
      traceSink_,
      XdlmsTraceKind::DecodeFailed,
      XdlmsStatus::DecodeFailed,
      0u,
      options_,
      plainRequest.size(),
        0u);
    return XdlmsStatus::DecodeFailed;
  }

  std::uint8_t requestInvokeId = 0u;
  switch (request.kind) {
    case dlms::apdu::XdlmsApduKind::GetRequest:
      requestInvokeId = static_cast<std::uint8_t>(
        request.getRequest.invokeIdAndPriority & 0x0Fu);
      break;
    case dlms::apdu::XdlmsApduKind::SetRequest:
      requestInvokeId = static_cast<std::uint8_t>(
        request.setRequest.invokeIdAndPriority & 0x0Fu);
      break;
    case dlms::apdu::XdlmsApduKind::ActionRequest:
      requestInvokeId = static_cast<std::uint8_t>(
        request.actionRequest.invokeIdAndPriority & 0x0Fu);
      break;
    default:
      break;
  }
  const std::uint64_t conversationSeed =
    seedSource_ != 0 ? seedSource_->ConversationSeed() : 0u;
  const std::uint64_t conversationId =
    MakeConversationId(conversationSeed, requestInvokeId);
  if (channel_ != 0) {
    channel_->SetCorrelation(conversationId);
  }

  if (traceSink_ != 0) {
    XdlmsTraceEvent event = EmptyXdlmsTraceEvent();
    event.kind = XdlmsTraceKind::RequestReceived;
    event.direction = XdlmsTraceDirection::Inbound;
    event.status = XdlmsStatus::Ok;
    event.apduSize = plainRequest.size();
    switch (request.kind) {
      case dlms::apdu::XdlmsApduKind::GetRequest:
        event.invokeId = static_cast<std::uint8_t>(
          request.getRequest.invokeIdAndPriority & 0x0Fu);
        event.options = ParseServiceOptions(
          request.getRequest.invokeIdAndPriority, options_);
        event.classId = request.getRequest.descriptor.classId;
        event.attributeOrMethodId = static_cast<std::uint8_t>(
          request.getRequest.descriptor.attributeId);
        for (std::size_t i = 0; i < 6u; ++i) {
          event.logicalName[i] =
            request.getRequest.descriptor.logicalName[i];
        }
        break;
      case dlms::apdu::XdlmsApduKind::SetRequest:
        event.invokeId = static_cast<std::uint8_t>(
          request.setRequest.invokeIdAndPriority & 0x0Fu);
        event.options = ParseServiceOptions(
          request.setRequest.invokeIdAndPriority, options_);
        event.classId = request.setRequest.descriptor.classId;
        event.attributeOrMethodId = static_cast<std::uint8_t>(
          request.setRequest.descriptor.attributeId);
        for (std::size_t i = 0; i < 6u; ++i) {
          event.logicalName[i] =
            request.setRequest.descriptor.logicalName[i];
        }
        break;
      case dlms::apdu::XdlmsApduKind::ActionRequest:
        event.invokeId = static_cast<std::uint8_t>(
          request.actionRequest.invokeIdAndPriority & 0x0Fu);
        event.options = ParseServiceOptions(
          request.actionRequest.invokeIdAndPriority, options_);
        event.classId = request.actionRequest.descriptor.classId;
        event.attributeOrMethodId = static_cast<std::uint8_t>(
          request.actionRequest.descriptor.methodId);
        for (std::size_t i = 0; i < 6u; ++i) {
          event.logicalName[i] =
            request.actionRequest.descriptor.logicalName[i];
        }
        break;
      default:
        break;
    }
    event.conversationId = conversationId;
    traceSink_->OnXdlmsTrace(event);
  }

  XdlmsStatus status = XdlmsStatus::UnsupportedFeature;
  switch (request.kind) {
    case dlms::apdu::XdlmsApduKind::GetRequest:
      status = ProcessGetRequest(
        request,
        dispatcher_,
        options_,
        getBlocks_,
        responseApdu,
        traceSink_,
        conversationId);
      break;

    case dlms::apdu::XdlmsApduKind::SetRequest:
      status = ProcessSetRequest(
        request,
        dispatcher_,
        options_,
        setBlocks_,
        responseApdu,
        traceSink_,
        conversationId);
      break;

    case dlms::apdu::XdlmsApduKind::ActionRequest:
      status = ProcessActionRequest(
        request,
        dispatcher_,
        options_,
        actionBlocks_,
        responseApdu,
        traceSink_,
        conversationId);
      break;

    default:
      return XdlmsStatus::UnsupportedFeature;
  }

  if (status != XdlmsStatus::Ok) {
    return status;
  }

  if (security_ != 0) {
    std::vector<std::uint8_t> plainResponse = responseApdu;
    dlms::security::SecurityByteView plain;
    plain.data = plainResponse.empty() ? 0 : &plainResponse[0];
    plain.size = plainResponse.size();
    const dlms::security::SecurityStatus securityStatus =
      security_->Protect(plain, responseApdu);
    if (securityStatus != dlms::security::SecurityStatus::Ok) {
      responseApdu.clear();
      EmitServerSimpleTrace(
        traceSink_,
        XdlmsTraceKind::SecurityFailed,
        XdlmsStatus::SecurityFailed,
        0u,
        options_,
        plainResponse.size(),
        conversationId);
      return XdlmsStatus::SecurityFailed;
    }
  }

  EmitServerSimpleTrace(
    traceSink_,
    XdlmsTraceKind::ResponseSent,
    XdlmsStatus::Ok,
    0u,
    options_,
    responseApdu.size(),
        conversationId);

  return XdlmsStatus::Ok;
}

GetIndication EmptyGetIndication()
{
  GetIndication indication;
  indication.invokeId = 0u;
  indication.options = DefaultServiceOptions();
  indication.descriptor = EmptyCosemAttributeDescriptor();
  return indication;
}

SetIndication EmptySetIndication()
{
  SetIndication indication;
  indication.invokeId = 0u;
  indication.options = DefaultServiceOptions();
  indication.descriptor = EmptyCosemAttributeDescriptor();
  indication.data.clear();
  return indication;
}

ActionIndication EmptyActionIndication()
{
  ActionIndication indication;
  indication.invokeId = 0u;
  indication.options = DefaultServiceOptions();
  indication.descriptor = EmptyCosemMethodDescriptor();
  indication.hasParameter = false;
  indication.parameter.clear();
  return indication;
}

GetResponseBlockState EmptyGetResponseBlockState()
{
  GetResponseBlockState state;
  state.active = false;
  state.invokeId = 0u;
  state.options = DefaultServiceOptions();
  state.nextBlockNumber = 1u;
  state.offset = 0u;
  state.data.clear();
  return state;
}

ActionRequestBlockState EmptyActionRequestBlockState()
{
  ActionRequestBlockState state;
  state.active = false;
  state.invokeId = 0u;
  state.options = DefaultServiceOptions();
  state.descriptor = EmptyCosemMethodDescriptor();
  state.nextBlockNumber = 1u;
  state.data.clear();
  return state;
}

SetRequestBlockState EmptySetRequestBlockState()
{
  SetRequestBlockState state;
  state.active = false;
  state.invokeId = 0u;
  state.options = DefaultServiceOptions();
  state.descriptor = EmptyCosemAttributeDescriptor();
  state.nextBlockNumber = 1u;
  state.data.clear();
  return state;
}

XdlmsStatus ValidateInvokeId(std::uint8_t invokeId)
{
  return invokeId >= 1u && invokeId <= 15u
    ? XdlmsStatus::Ok
    : XdlmsStatus::InvalidArgument;
}

} // namespace xdlms
} // namespace dlms
