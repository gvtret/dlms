#include "dlms/hdlc/hdlc_error.hpp"

namespace dlms {
namespace hdlc {

const char* HdlcStatusName(HdlcStatus status)
{
  switch (status) {
    case HdlcStatus::Ok:
      return "Ok";
    case HdlcStatus::NeedMoreData:
      return "NeedMoreData";
    case HdlcStatus::OutputBufferTooSmall:
      return "OutputBufferTooSmall";
    case HdlcStatus::InvalidArgument:
      return "InvalidArgument";
    case HdlcStatus::InvalidFlag:
      return "InvalidFlag";
    case HdlcStatus::InvalidFrameFormat:
      return "InvalidFrameFormat";
    case HdlcStatus::InvalidFrameType:
      return "InvalidFrameType";
    case HdlcStatus::InvalidFrameLength:
      return "InvalidFrameLength";
    case HdlcStatus::InvalidAddress:
      return "InvalidAddress";
    case HdlcStatus::InvalidControlField:
      return "InvalidControlField";
    case HdlcStatus::InvalidHeaderChecksum:
      return "InvalidHeaderChecksum";
    case HdlcStatus::InvalidFrameChecksum:
      return "InvalidFrameChecksum";
    case HdlcStatus::FrameTooLarge:
      return "FrameTooLarge";
    case HdlcStatus::InformationFieldTooLarge:
      return "InformationFieldTooLarge";
    case HdlcStatus::SegmentationError:
      return "SegmentationError";
    case HdlcStatus::SegmentationIncomplete:
      return "SegmentationIncomplete";
    case HdlcStatus::SegmentationOverflow:
      return "SegmentationOverflow";
    case HdlcStatus::UnsupportedFrame:
      return "UnsupportedFrame";
    case HdlcStatus::UnsupportedAddress:
      return "UnsupportedAddress";
    case HdlcStatus::UnsupportedFeature:
      return "UnsupportedFeature";
    case HdlcStatus::InternalError:
      return "InternalError";
  }

  return "Unknown";
}

} // namespace hdlc
} // namespace dlms
