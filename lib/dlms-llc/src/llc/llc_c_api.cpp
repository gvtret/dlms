#include "dlms/llc/llc_c_api.h"

#include "dlms/llc/llc_codec.hpp"
#include "dlms/llc/llc_header.hpp"

namespace {

dlms_llc_status_t MapStatus(dlms::llc::LlcStatus status)
{
  switch (status) {
    case dlms::llc::LlcStatus::Ok:
      return DLMS_LLC_STATUS_OK;
    case dlms::llc::LlcStatus::NeedMoreData:
      return DLMS_LLC_STATUS_NEED_MORE_DATA;
    case dlms::llc::LlcStatus::OutputBufferTooSmall:
      return DLMS_LLC_STATUS_OUTPUT_BUFFER_TOO_SMALL;
    case dlms::llc::LlcStatus::InvalidArgument:
      return DLMS_LLC_STATUS_INVALID_ARGUMENT;
    case dlms::llc::LlcStatus::InvalidHeader:
      return DLMS_LLC_STATUS_INVALID_HEADER;
    case dlms::llc::LlcStatus::InvalidDsap:
      return DLMS_LLC_STATUS_INVALID_DSAP;
    case dlms::llc::LlcStatus::InvalidSsap:
      return DLMS_LLC_STATUS_INVALID_SSAP;
    case dlms::llc::LlcStatus::InvalidControl:
      return DLMS_LLC_STATUS_INVALID_CONTROL;
    case dlms::llc::LlcStatus::InvalidLpduLength:
      return DLMS_LLC_STATUS_INVALID_LPDU_LENGTH;
    case dlms::llc::LlcStatus::LsduTooLarge:
      return DLMS_LLC_STATUS_LSDU_TOO_LARGE;
    case dlms::llc::LlcStatus::BroadcastEncodeForbidden:
      return DLMS_LLC_STATUS_BROADCAST_ENCODE_FORBIDDEN;
    case dlms::llc::LlcStatus::UnsupportedAddress:
      return DLMS_LLC_STATUS_UNSUPPORTED_ADDRESS;
    case dlms::llc::LlcStatus::UnsupportedControl:
      return DLMS_LLC_STATUS_UNSUPPORTED_CONTROL;
    case dlms::llc::LlcStatus::UnsupportedFeature:
      return DLMS_LLC_STATUS_UNSUPPORTED_FEATURE;
    case dlms::llc::LlcStatus::InternalError:
      return DLMS_LLC_STATUS_INTERNAL_ERROR;
  }

  return DLMS_LLC_STATUS_INTERNAL_ERROR;
}

dlms::llc::LlcHeader ToCppHeader(const dlms_llc_header_t& header)
{
  dlms::llc::LlcHeader result;
  result.dsap = header.dsap;
  result.ssap = header.ssap;
  result.control = header.control;
  return result;
}

void WriteHeader(const dlms::llc::LlcHeader& source, dlms_llc_header_t& target)
{
  target.dsap = source.dsap;
  target.ssap = source.ssap;
  target.control = source.control;
}

} // namespace

extern "C" {

dlms_llc_status_t dlms_llc_encode_lpdu(
  const dlms_llc_header_t* header,
  const uint8_t* lsdu,
  size_t lsdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size)
{
  if (written_size != 0) {
    *written_size = 0;
  }

  if (header == 0 || output == 0 || written_size == 0 ||
      (lsdu == 0 && lsdu_size != 0)) {
    return DLMS_LLC_STATUS_INVALID_ARGUMENT;
  }

  try {
    std::size_t written = 0;
    const dlms::llc::LlcStatus status = dlms::llc::EncodeLpduToBuffer(
      ToCppHeader(*header), lsdu, lsdu_size, output, output_size, written);
    *written_size = written;
    return MapStatus(status);
  } catch (...) {
    *written_size = 0;
    return DLMS_LLC_STATUS_INTERNAL_ERROR;
  }
}

dlms_llc_status_t dlms_llc_encode_request(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size)
{
  const dlms_llc_header_t header = {0xE6u, 0xE6u, 0x00u};
  return dlms_llc_encode_lpdu(
    &header, apdu, apdu_size, output, output_size, written_size);
}

dlms_llc_status_t dlms_llc_encode_response(
  const uint8_t* apdu,
  size_t apdu_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size)
{
  const dlms_llc_header_t header = {0xE6u, 0xE7u, 0x00u};
  return dlms_llc_encode_lpdu(
    &header, apdu, apdu_size, output, output_size, written_size);
}

dlms_llc_status_t dlms_llc_decode_lpdu(
  const uint8_t* input,
  size_t input_size,
  int allow_broadcast_destination,
  dlms_llc_lpdu_view_t* lpdu)
{
  if (lpdu != 0) {
    lpdu->header.dsap = 0;
    lpdu->header.ssap = 0;
    lpdu->header.control = 0;
    lpdu->lsdu = 0;
    lpdu->lsdu_size = 0;
  }

  if (input == 0 || lpdu == 0) {
    return DLMS_LLC_STATUS_INVALID_ARGUMENT;
  }

  try {
    dlms::llc::LlcLpdu view;
    const dlms::llc::LlcStatus status = dlms::llc::DecodeLpduFromBuffer(
      input, input_size, allow_broadcast_destination != 0, view);
    if (status != dlms::llc::LlcStatus::Ok) {
      return MapStatus(status);
    }

    WriteHeader(view.header, lpdu->header);
    lpdu->lsdu = view.lsduData;
    lpdu->lsdu_size = view.lsduSize;
    return DLMS_LLC_STATUS_OK;
  } catch (...) {
    lpdu->header.dsap = 0;
    lpdu->header.ssap = 0;
    lpdu->header.control = 0;
    lpdu->lsdu = 0;
    lpdu->lsdu_size = 0;
    return DLMS_LLC_STATUS_INTERNAL_ERROR;
  }
}

dlms_llc_status_t dlms_llc_validate_header(
  const dlms_llc_header_t* header,
  int allow_broadcast_destination)
{
  if (header == 0) {
    return DLMS_LLC_STATUS_INVALID_ARGUMENT;
  }

  try {
    return MapStatus(dlms::llc::ValidateLlcHeader(
      ToCppHeader(*header), allow_broadcast_destination != 0));
  } catch (...) {
    return DLMS_LLC_STATUS_INTERNAL_ERROR;
  }
}

} // extern "C"
