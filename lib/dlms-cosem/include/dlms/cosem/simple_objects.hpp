#pragma once

#include "dlms/cosem/certificate_store.hpp"
#include "dlms/cosem/logical_device.hpp"
#include "dlms/security/invocation_counter_store.hpp"
#include "dlms/security/key_store.hpp"

#include <array>
#include <string>
#include <vector>

namespace dlms {
namespace cosem {

class CosemDataObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    AttributeAccessMode valueAccess);
  CosemDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    AttributeAccessMode valueAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Value() const;
  void SetValue(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer value_;
  CosemAccessRights rights_;
};

class CosemRegisterObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode valueAccess);
  CosemRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode valueAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Value() const;
  const CosemByteBuffer& ScalerUnit() const;
  void SetValue(const CosemByteBuffer& value);
  void SetScalerUnit(const CosemByteBuffer& scalerUnit);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer value_;
  CosemByteBuffer scalerUnit_;
  CosemAccessRights rights_;
};

class CosemExtendedRegisterObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemExtendedRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    const CosemByteBuffer& status,
    const CosemByteBuffer& captureTime,
    AttributeAccessMode valueAccess);
  CosemExtendedRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& value,
    const CosemByteBuffer& scalerUnit,
    const CosemByteBuffer& status,
    const CosemByteBuffer& captureTime,
    AttributeAccessMode valueAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Value() const;
  const CosemByteBuffer& ScalerUnit() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& CaptureTime() const;
  void SetValue(const CosemByteBuffer& value);
  void SetScalerUnit(const CosemByteBuffer& scalerUnit);
  void SetStatus(const CosemByteBuffer& status);
  void SetCaptureTime(const CosemByteBuffer& captureTime);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer value_;
  CosemByteBuffer scalerUnit_;
  CosemByteBuffer status_;
  CosemByteBuffer captureTime_;
  CosemAccessRights rights_;
};

class CosemDemandRegisterObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemDemandRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& currentAverageValue,
    const CosemByteBuffer& lastAverageValue,
    const CosemByteBuffer& scalerUnit,
    const CosemByteBuffer& status,
    const CosemByteBuffer& captureTime,
    const CosemByteBuffer& startTimeCurrent,
    std::uint32_t period,
    std::uint16_t numberOfPeriods);
  CosemDemandRegisterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& currentAverageValue,
    const CosemByteBuffer& lastAverageValue,
    const CosemByteBuffer& scalerUnit,
    const CosemByteBuffer& status,
    const CosemByteBuffer& captureTime,
    const CosemByteBuffer& startTimeCurrent,
    std::uint32_t period,
    std::uint16_t numberOfPeriods,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& CurrentAverageValue() const;
  const CosemByteBuffer& LastAverageValue() const;
  const CosemByteBuffer& ScalerUnit() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& CaptureTime() const;
  const CosemByteBuffer& StartTimeCurrent() const;
  std::uint32_t Period() const;
  std::uint16_t NumberOfPeriods() const;

  void SetCurrentAverageValue(const CosemByteBuffer& value);
  void SetLastAverageValue(const CosemByteBuffer& value);
  void SetScalerUnit(const CosemByteBuffer& scalerUnit);
  void SetStatus(const CosemByteBuffer& status);
  void SetCaptureTime(const CosemByteBuffer& captureTime);
  void SetStartTimeCurrent(const CosemByteBuffer& startTime);
  void SetPeriod(std::uint32_t period);
  void SetNumberOfPeriods(std::uint16_t numberOfPeriods);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer currentAverageValue_;
  CosemByteBuffer lastAverageValue_;
  CosemByteBuffer scalerUnit_;
  CosemByteBuffer status_;
  CosemByteBuffer captureTime_;
  CosemByteBuffer startTimeCurrent_;
  std::uint32_t period_;
  std::uint16_t numberOfPeriods_;
  CosemAccessRights rights_;
};

class CosemRegisterActivationObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemRegisterActivationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& registerAssignment,
    const CosemByteBuffer& maskList,
    const CosemByteBuffer& activeMask);
  CosemRegisterActivationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& registerAssignment,
    const CosemByteBuffer& maskList,
    const CosemByteBuffer& activeMask,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& RegisterAssignment() const;
  const CosemByteBuffer& MaskList() const;
  const CosemByteBuffer& ActiveMask() const;

  void SetRegisterAssignment(const CosemByteBuffer& assignment);
  void SetMaskList(const CosemByteBuffer& maskList);
  void SetActiveMask(const CosemByteBuffer& activeMask);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer registerAssignment_;
  CosemByteBuffer maskList_;
  CosemByteBuffer activeMask_;
  CosemAccessRights rights_;
};

class CosemRegisterMonitorObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemRegisterMonitorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& thresholds,
    const CosemByteBuffer& monitoredValue,
    const CosemByteBuffer& actions,
    AttributeAccessMode thresholdsAccess);
  CosemRegisterMonitorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& thresholds,
    const CosemByteBuffer& monitoredValue,
    const CosemByteBuffer& actions,
    AttributeAccessMode thresholdsAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Thresholds() const;
  const CosemByteBuffer& MonitoredValue() const;
  const CosemByteBuffer& Actions() const;

  void SetThresholds(const CosemByteBuffer& thresholds);
  void SetMonitoredValue(const CosemByteBuffer& monitoredValue);
  void SetActions(const CosemByteBuffer& actions);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer thresholds_;
  CosemByteBuffer monitoredValue_;
  CosemByteBuffer actions_;
  CosemAccessRights rights_;
};

class CosemScriptTableObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemScriptTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& scripts,
    AttributeAccessMode scriptsAccess);
  CosemScriptTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& scripts,
    AttributeAccessMode scriptsAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Scripts() const;
  void SetScripts(const CosemByteBuffer& scripts);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer scripts_;
  CosemAccessRights rights_;
};

class CosemActivityCalendarObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemActivityCalendarObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& calendarNameActive,
    const CosemByteBuffer& seasonProfileActive,
    const CosemByteBuffer& weekProfileTableActive,
    const CosemByteBuffer& dayProfileTableActive,
    const CosemByteBuffer& calendarNamePassive,
    const CosemByteBuffer& seasonProfilePassive,
    const CosemByteBuffer& weekProfileTablePassive,
    const CosemByteBuffer& dayProfileTablePassive,
    const CosemByteBuffer& activatePassiveCalendarTime,
    AttributeAccessMode passiveAccess);
  CosemActivityCalendarObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& calendarNameActive,
    const CosemByteBuffer& seasonProfileActive,
    const CosemByteBuffer& weekProfileTableActive,
    const CosemByteBuffer& dayProfileTableActive,
    const CosemByteBuffer& calendarNamePassive,
    const CosemByteBuffer& seasonProfilePassive,
    const CosemByteBuffer& weekProfileTablePassive,
    const CosemByteBuffer& dayProfileTablePassive,
    const CosemByteBuffer& activatePassiveCalendarTime,
    AttributeAccessMode passiveAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& CalendarNameActive() const;
  const CosemByteBuffer& SeasonProfileActive() const;
  const CosemByteBuffer& WeekProfileTableActive() const;
  const CosemByteBuffer& DayProfileTableActive() const;
  const CosemByteBuffer& CalendarNamePassive() const;
  const CosemByteBuffer& SeasonProfilePassive() const;
  const CosemByteBuffer& WeekProfileTablePassive() const;
  const CosemByteBuffer& DayProfileTablePassive() const;
  const CosemByteBuffer& ActivatePassiveCalendarTime() const;

  void SetCalendarNamePassive(const CosemByteBuffer& value);
  void SetSeasonProfilePassive(const CosemByteBuffer& value);
  void SetWeekProfileTablePassive(const CosemByteBuffer& value);
  void SetDayProfileTablePassive(const CosemByteBuffer& value);
  void SetActivatePassiveCalendarTime(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer calendarNameActive_;
  CosemByteBuffer seasonProfileActive_;
  CosemByteBuffer weekProfileTableActive_;
  CosemByteBuffer dayProfileTableActive_;
  CosemByteBuffer calendarNamePassive_;
  CosemByteBuffer seasonProfilePassive_;
  CosemByteBuffer weekProfileTablePassive_;
  CosemByteBuffer dayProfileTablePassive_;
  CosemByteBuffer activatePassiveCalendarTime_;
  CosemAccessRights rights_;
};

class CosemImageTransferObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemImageTransferObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& imageBlockSize,
    const CosemByteBuffer& imageTransferredBlocksStatus,
    const CosemByteBuffer& imageFirstNotTransferredBlockNumber,
    const CosemByteBuffer& imageTransferEnabled,
    const CosemByteBuffer& imageTransferStatus,
    const CosemByteBuffer& imageToActivateInfo,
    AttributeAccessMode transferEnabledAccess);
  CosemImageTransferObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& imageBlockSize,
    const CosemByteBuffer& imageTransferredBlocksStatus,
    const CosemByteBuffer& imageFirstNotTransferredBlockNumber,
    const CosemByteBuffer& imageTransferEnabled,
    const CosemByteBuffer& imageTransferStatus,
    const CosemByteBuffer& imageToActivateInfo,
    AttributeAccessMode transferEnabledAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ImageBlockSize() const;
  const CosemByteBuffer& ImageTransferredBlocksStatus() const;
  const CosemByteBuffer& ImageFirstNotTransferredBlockNumber() const;
  const CosemByteBuffer& ImageTransferEnabled() const;
  const CosemByteBuffer& ImageTransferStatus() const;
  const CosemByteBuffer& ImageToActivateInfo() const;

  void SetImageBlockSize(const CosemByteBuffer& value);
  void SetImageTransferredBlocksStatus(const CosemByteBuffer& value);
  void SetImageFirstNotTransferredBlockNumber(
    const CosemByteBuffer& value);
  void SetImageTransferEnabled(const CosemByteBuffer& value);
  void SetImageTransferStatus(const CosemByteBuffer& value);
  void SetImageToActivateInfo(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer imageBlockSize_;
  CosemByteBuffer imageTransferredBlocksStatus_;
  CosemByteBuffer imageFirstNotTransferredBlockNumber_;
  CosemByteBuffer imageTransferEnabled_;
  CosemByteBuffer imageTransferStatus_;
  CosemByteBuffer imageToActivateInfo_;
  CosemAccessRights rights_;
};

// Push setup (class_id = 40), IEC 62056-6-2 ED4 (2021).
//
// Per-version attribute layout:
//   v0: 1..7   (1..7 always present)
//   v1: 1..10  (adds 8 port_reference, 9 push_client_SAP,
//              10 push_protection_parameters)
//   v2: 1..13  (adds 11 push_operation_method, 12 confirmation_parameters,
//              13 last_confirmation_date_time)
//
// Methods:
//   v0/v1: 1 push (m)
//   v2:    1 push (m), 2 reset (o)
//
// repetition_delay (attr 7) data type:
//   v0/v1: long-unsigned (seconds)
//   v2:    structure { repetition_delay_min: long-unsigned,
//                      repetition_delay_exponent: long-unsigned,
//                      repetition_delay_max: long-unsigned }
// The buffer is opaque; the caller is responsible for encoding the value
// that matches the negotiated version.
class CosemPushSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 2u;

  CosemPushSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& pushObjectList,
    const CosemByteBuffer& sendDestinationAndMethod,
    const CosemByteBuffer& communicationWindow,
    const CosemByteBuffer& randomisationStartInterval,
    const CosemByteBuffer& numberOfRetries,
    const CosemByteBuffer& repetitionDelay,
    const CosemByteBuffer& portReference,
    const CosemByteBuffer& pushClientSap,
    const CosemByteBuffer& pushProtectionParameters,
    const CosemByteBuffer& pushOperationMethod,
    const CosemByteBuffer& confirmationParameters,
    const CosemByteBuffer& lastConfirmationDateTime,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& PushObjectList() const;
  const CosemByteBuffer& SendDestinationAndMethod() const;
  const CosemByteBuffer& CommunicationWindow() const;
  const CosemByteBuffer& RandomisationStartInterval() const;
  const CosemByteBuffer& NumberOfRetries() const;
  const CosemByteBuffer& RepetitionDelay() const;
  const CosemByteBuffer& PortReference() const;
  const CosemByteBuffer& PushClientSap() const;
  const CosemByteBuffer& PushProtectionParameters() const;
  const CosemByteBuffer& PushOperationMethod() const;
  const CosemByteBuffer& ConfirmationParameters() const;
  const CosemByteBuffer& LastConfirmationDateTime() const;

  void SetLastConfirmationDateTime(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer pushObjectList_;
  CosemByteBuffer sendDestinationAndMethod_;
  CosemByteBuffer communicationWindow_;
  CosemByteBuffer randomisationStartInterval_;
  CosemByteBuffer numberOfRetries_;
  CosemByteBuffer repetitionDelay_;
  CosemByteBuffer portReference_;
  CosemByteBuffer pushClientSap_;
  CosemByteBuffer pushProtectionParameters_;
  CosemByteBuffer pushOperationMethod_;
  CosemByteBuffer confirmationParameters_;
  CosemByteBuffer lastConfirmationDateTime_;
  CosemAccessRights rights_;
};

class CosemDisconnectControlObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemDisconnectControlObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& outputState,
    const CosemByteBuffer& controlState,
    const CosemByteBuffer& controlMode,
    AttributeAccessMode controlModeAccess);
  CosemDisconnectControlObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& outputState,
    const CosemByteBuffer& controlState,
    const CosemByteBuffer& controlMode,
    AttributeAccessMode controlModeAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& OutputState() const;
  const CosemByteBuffer& ControlState() const;
  const CosemByteBuffer& ControlMode() const;

  void SetOutputState(const CosemByteBuffer& value);
  void SetControlState(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer outputState_;
  CosemByteBuffer controlState_;
  CosemByteBuffer controlMode_;
  CosemAccessRights rights_;
};

class CosemLimiterObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemLimiterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& monitoredValue,
    const CosemByteBuffer& thresholdActive,
    const CosemByteBuffer& thresholdNormal,
    const CosemByteBuffer& thresholdEmergency,
    const CosemByteBuffer& minOverThresholdDuration,
    const CosemByteBuffer& minUnderThresholdDuration,
    const CosemByteBuffer& emergencyProfile,
    const CosemByteBuffer& emergencyProfileGroupIdList,
    const CosemByteBuffer& emergencyProfileActive,
    const CosemByteBuffer& actions,
    AttributeAccessMode mutableAccess);
  CosemLimiterObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& monitoredValue,
    const CosemByteBuffer& thresholdActive,
    const CosemByteBuffer& thresholdNormal,
    const CosemByteBuffer& thresholdEmergency,
    const CosemByteBuffer& minOverThresholdDuration,
    const CosemByteBuffer& minUnderThresholdDuration,
    const CosemByteBuffer& emergencyProfile,
    const CosemByteBuffer& emergencyProfileGroupIdList,
    const CosemByteBuffer& emergencyProfileActive,
    const CosemByteBuffer& actions,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& MonitoredValue() const;
  const CosemByteBuffer& ThresholdActive() const;
  const CosemByteBuffer& ThresholdNormal() const;
  const CosemByteBuffer& ThresholdEmergency() const;
  const CosemByteBuffer& MinOverThresholdDuration() const;
  const CosemByteBuffer& MinUnderThresholdDuration() const;
  const CosemByteBuffer& EmergencyProfile() const;
  const CosemByteBuffer& EmergencyProfileGroupIdList() const;
  const CosemByteBuffer& EmergencyProfileActive() const;
  const CosemByteBuffer& Actions() const;

  void SetThresholdActive(const CosemByteBuffer& value);
  void SetEmergencyProfileActive(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer monitoredValue_;
  CosemByteBuffer thresholdActive_;
  CosemByteBuffer thresholdNormal_;
  CosemByteBuffer thresholdEmergency_;
  CosemByteBuffer minOverThresholdDuration_;
  CosemByteBuffer minUnderThresholdDuration_;
  CosemByteBuffer emergencyProfile_;
  CosemByteBuffer emergencyProfileGroupIdList_;
  CosemByteBuffer emergencyProfileActive_;
  CosemByteBuffer actions_;
  CosemAccessRights rights_;
};

class CosemIecHdlcSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemIecHdlcSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& commSpeed,
    const CosemByteBuffer& windowSizeTransmit,
    const CosemByteBuffer& windowSizeReceive,
    const CosemByteBuffer& maxInfoFieldLengthTransmit,
    const CosemByteBuffer& maxInfoFieldLengthReceive,
    const CosemByteBuffer& interOctetTimeOut,
    const CosemByteBuffer& inactivityTimeOut,
    const CosemByteBuffer& deviceAddress,
    AttributeAccessMode mutableAccess);
  CosemIecHdlcSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& commSpeed,
    const CosemByteBuffer& windowSizeTransmit,
    const CosemByteBuffer& windowSizeReceive,
    const CosemByteBuffer& maxInfoFieldLengthTransmit,
    const CosemByteBuffer& maxInfoFieldLengthReceive,
    const CosemByteBuffer& interOctetTimeOut,
    const CosemByteBuffer& inactivityTimeOut,
    const CosemByteBuffer& deviceAddress,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& CommSpeed() const;
  const CosemByteBuffer& WindowSizeTransmit() const;
  const CosemByteBuffer& WindowSizeReceive() const;
  const CosemByteBuffer& MaxInfoFieldLengthTransmit() const;
  const CosemByteBuffer& MaxInfoFieldLengthReceive() const;
  const CosemByteBuffer& InterOctetTimeOut() const;
  const CosemByteBuffer& InactivityTimeOut() const;
  const CosemByteBuffer& DeviceAddress() const;

  void SetDeviceAddress(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer commSpeed_;
  CosemByteBuffer windowSizeTransmit_;
  CosemByteBuffer windowSizeReceive_;
  CosemByteBuffer maxInfoFieldLengthTransmit_;
  CosemByteBuffer maxInfoFieldLengthReceive_;
  CosemByteBuffer interOctetTimeOut_;
  CosemByteBuffer inactivityTimeOut_;
  CosemByteBuffer deviceAddress_;
  CosemAccessRights rights_;
};

class CosemRegisterTableObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemRegisterTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tableCellValues,
    const CosemByteBuffer& tableCellDefinition,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode mutableAccess);
  CosemRegisterTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tableCellValues,
    const CosemByteBuffer& tableCellDefinition,
    const CosemByteBuffer& scalerUnit,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& TableCellValues() const;
  const CosemByteBuffer& TableCellDefinition() const;
  const CosemByteBuffer& ScalerUnit() const;

  void SetTableCellValues(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer tableCellValues_;
  CosemByteBuffer tableCellDefinition_;
  CosemByteBuffer scalerUnit_;
  CosemAccessRights rights_;
};

class CosemTcpUdpSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemTcpUdpSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tcpUdpPort,
    const CosemByteBuffer& ipReference,
    const CosemByteBuffer& mss,
    const CosemByteBuffer& nbOfSimConn,
    const CosemByteBuffer& inactivityTimeOut,
    AttributeAccessMode mutableAccess);
  CosemTcpUdpSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tcpUdpPort,
    const CosemByteBuffer& ipReference,
    const CosemByteBuffer& mss,
    const CosemByteBuffer& nbOfSimConn,
    const CosemByteBuffer& inactivityTimeOut,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& TcpUdpPort() const;
  const CosemByteBuffer& IpReference() const;
  const CosemByteBuffer& Mss() const;
  const CosemByteBuffer& NbOfSimConn() const;
  const CosemByteBuffer& InactivityTimeOut() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer tcpUdpPort_;
  CosemByteBuffer ipReference_;
  CosemByteBuffer mss_;
  CosemByteBuffer nbOfSimConn_;
  CosemByteBuffer inactivityTimeOut_;
  CosemAccessRights rights_;
};

class CosemScheduleObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemScheduleObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& entries,
    AttributeAccessMode entriesAccess);
  CosemScheduleObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& entries,
    AttributeAccessMode entriesAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Entries() const;
  void SetEntries(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer entries_;
  CosemAccessRights rights_;
};

class CosemSpecialDaysTableObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSpecialDaysTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& entries,
    AttributeAccessMode entriesAccess);
  CosemSpecialDaysTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& entries,
    AttributeAccessMode entriesAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Entries() const;
  void SetEntries(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer entries_;
  CosemAccessRights rights_;
};

class CosemSingleActionScheduleObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSingleActionScheduleObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& executedScript,
    const CosemByteBuffer& type,
    const CosemByteBuffer& executionTime,
    AttributeAccessMode mutableAccess);
  CosemSingleActionScheduleObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& executedScript,
    const CosemByteBuffer& type,
    const CosemByteBuffer& executionTime,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ExecutedScript() const;
  const CosemByteBuffer& Type() const;
  const CosemByteBuffer& ExecutionTime() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer executedScript_;
  CosemByteBuffer type_;
  CosemByteBuffer executionTime_;
  CosemAccessRights rights_;
};

class CosemModemConfigurationObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemModemConfigurationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& communicationSpeed,
    const CosemByteBuffer& initialisationStrings,
    const CosemByteBuffer& modemProfile,
    AttributeAccessMode mutableAccess);
  CosemModemConfigurationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& communicationSpeed,
    const CosemByteBuffer& initialisationStrings,
    const CosemByteBuffer& modemProfile,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& CommunicationSpeed() const;
  const CosemByteBuffer& InitialisationStrings() const;
  const CosemByteBuffer& ModemProfile() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer communicationSpeed_;
  CosemByteBuffer initialisationStrings_;
  CosemByteBuffer modemProfile_;
  CosemAccessRights rights_;
};

class CosemAutoConnectObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 2u;

  CosemAutoConnectObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mode,
    const CosemByteBuffer& repetitions,
    const CosemByteBuffer& repetitionDelay,
    const CosemByteBuffer& callingWindow,
    const CosemByteBuffer& destinationList,
    AttributeAccessMode mutableAccess);
  CosemAutoConnectObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mode,
    const CosemByteBuffer& repetitions,
    const CosemByteBuffer& repetitionDelay,
    const CosemByteBuffer& callingWindow,
    const CosemByteBuffer& destinationList,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Mode() const;
  const CosemByteBuffer& Repetitions() const;
  const CosemByteBuffer& RepetitionDelay() const;
  const CosemByteBuffer& CallingWindow() const;
  const CosemByteBuffer& DestinationList() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer mode_;
  CosemByteBuffer repetitions_;
  CosemByteBuffer repetitionDelay_;
  CosemByteBuffer callingWindow_;
  CosemByteBuffer destinationList_;
  CosemAccessRights rights_;
};

class CosemGprsModemSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemGprsModemSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& apn,
    const CosemByteBuffer& pinCode,
    const CosemByteBuffer& qualityOfService,
    AttributeAccessMode mutableAccess);
  CosemGprsModemSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& apn,
    const CosemByteBuffer& pinCode,
    const CosemByteBuffer& qualityOfService,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Apn() const;
  const CosemByteBuffer& PinCode() const;
  const CosemByteBuffer& QualityOfService() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer apn_;
  CosemByteBuffer pinCode_;
  CosemByteBuffer qualityOfService_;
  CosemAccessRights rights_;
};

class CosemAutoAnswerObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemAutoAnswerObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mode,
    const CosemByteBuffer& listeningWindow,
    const CosemByteBuffer& status,
    const CosemByteBuffer& numberOfCalls,
    const CosemByteBuffer& numberOfRings,
    const CosemByteBuffer& listOfAllowedCallers,
    AttributeAccessMode mutableAccess);
  CosemAutoAnswerObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mode,
    const CosemByteBuffer& listeningWindow,
    const CosemByteBuffer& status,
    const CosemByteBuffer& numberOfCalls,
    const CosemByteBuffer& numberOfRings,
    const CosemByteBuffer& listOfAllowedCallers,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Mode() const;
  const CosemByteBuffer& ListeningWindow() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& NumberOfCalls() const;
  const CosemByteBuffer& NumberOfRings() const;
  const CosemByteBuffer& ListOfAllowedCallers() const;

  void SetStatus(const CosemByteBuffer& status);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer mode_;
  CosemByteBuffer listeningWindow_;
  CosemByteBuffer status_;
  CosemByteBuffer numberOfCalls_;
  CosemByteBuffer numberOfRings_;
  CosemByteBuffer listOfAllowedCallers_;
  CosemAccessRights rights_;
};

class CosemIpv4SetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemIpv4SetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& dlReference,
    const CosemByteBuffer& ipAddress,
    const CosemByteBuffer& multicastIpAddress,
    const CosemByteBuffer& ipOptions,
    const CosemByteBuffer& subnetMask,
    const CosemByteBuffer& gatewayIpAddress,
    const CosemByteBuffer& useDhcpFlag,
    const CosemByteBuffer& primaryDnsAddress,
    const CosemByteBuffer& secondaryDnsAddress,
    AttributeAccessMode mutableAccess);
  CosemIpv4SetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& dlReference,
    const CosemByteBuffer& ipAddress,
    const CosemByteBuffer& multicastIpAddress,
    const CosemByteBuffer& ipOptions,
    const CosemByteBuffer& subnetMask,
    const CosemByteBuffer& gatewayIpAddress,
    const CosemByteBuffer& useDhcpFlag,
    const CosemByteBuffer& primaryDnsAddress,
    const CosemByteBuffer& secondaryDnsAddress,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& DlReference() const;
  const CosemByteBuffer& IpAddress() const;
  const CosemByteBuffer& MulticastIpAddress() const;
  const CosemByteBuffer& IpOptions() const;
  const CosemByteBuffer& SubnetMask() const;
  const CosemByteBuffer& GatewayIpAddress() const;
  const CosemByteBuffer& UseDhcpFlag() const;
  const CosemByteBuffer& PrimaryDnsAddress() const;
  const CosemByteBuffer& SecondaryDnsAddress() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer dlReference_;
  CosemByteBuffer ipAddress_;
  CosemByteBuffer multicastIpAddress_;
  CosemByteBuffer ipOptions_;
  CosemByteBuffer subnetMask_;
  CosemByteBuffer gatewayIpAddress_;
  CosemByteBuffer useDhcpFlag_;
  CosemByteBuffer primaryDnsAddress_;
  CosemByteBuffer secondaryDnsAddress_;
  CosemAccessRights rights_;
};

class CosemMacAddressSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemMacAddressSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& macAddress,
    AttributeAccessMode mutableAccess);
  CosemMacAddressSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& macAddress,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& MacAddress() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer macAddress_;
  CosemAccessRights rights_;
};

class CosemPppSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemPppSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& phyReference,
    const CosemByteBuffer& lcpOptions,
    const CosemByteBuffer& ipcpOptions,
    const CosemByteBuffer& pppAuthentication,
    AttributeAccessMode mutableAccess);
  CosemPppSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& phyReference,
    const CosemByteBuffer& lcpOptions,
    const CosemByteBuffer& ipcpOptions,
    const CosemByteBuffer& pppAuthentication,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& PhyReference() const;
  const CosemByteBuffer& LcpOptions() const;
  const CosemByteBuffer& IpcpOptions() const;
  const CosemByteBuffer& PppAuthentication() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer phyReference_;
  CosemByteBuffer lcpOptions_;
  CosemByteBuffer ipcpOptions_;
  CosemByteBuffer pppAuthentication_;
  CosemAccessRights rights_;
};

class CosemSmtpSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSmtpSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& serverPort,
    const CosemByteBuffer& userName,
    const CosemByteBuffer& loginPassword,
    const CosemByteBuffer& serverAddress,
    const CosemByteBuffer& senderAddress,
    AttributeAccessMode mutableAccess);
  CosemSmtpSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& serverPort,
    const CosemByteBuffer& userName,
    const CosemByteBuffer& loginPassword,
    const CosemByteBuffer& serverAddress,
    const CosemByteBuffer& senderAddress,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ServerPort() const;
  const CosemByteBuffer& UserName() const;
  const CosemByteBuffer& LoginPassword() const;
  const CosemByteBuffer& ServerAddress() const;
  const CosemByteBuffer& SenderAddress() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer serverPort_;
  CosemByteBuffer userName_;
  CosemByteBuffer loginPassword_;
  CosemByteBuffer serverAddress_;
  CosemByteBuffer senderAddress_;
  CosemAccessRights rights_;
};

class CosemGsmDiagnosticObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemGsmDiagnosticObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& operatorName,
    const CosemByteBuffer& status,
    const CosemByteBuffer& circuitSwitchedStatus,
    const CosemByteBuffer& packetSwitchedStatus,
    const CosemByteBuffer& cellInfo,
    const CosemByteBuffer& adjacentCells,
    const CosemByteBuffer& captureTime,
    AttributeAccessMode mutableAccess);
  CosemGsmDiagnosticObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& operatorName,
    const CosemByteBuffer& status,
    const CosemByteBuffer& circuitSwitchedStatus,
    const CosemByteBuffer& packetSwitchedStatus,
    const CosemByteBuffer& cellInfo,
    const CosemByteBuffer& adjacentCells,
    const CosemByteBuffer& captureTime,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& OperatorName() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& CircuitSwitchedStatus() const;
  const CosemByteBuffer& PacketSwitchedStatus() const;
  const CosemByteBuffer& CellInfo() const;
  const CosemByteBuffer& AdjacentCells() const;
  const CosemByteBuffer& CaptureTime() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer operatorName_;
  CosemByteBuffer status_;
  CosemByteBuffer circuitSwitchedStatus_;
  CosemByteBuffer packetSwitchedStatus_;
  CosemByteBuffer cellInfo_;
  CosemByteBuffer adjacentCells_;
  CosemByteBuffer captureTime_;
  CosemAccessRights rights_;
};

class CosemIecTwistedPairSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  // IEC 62056-6-2 ED4 (2021) §4.7.3 and DLMS UA Blue Book Ed. 12.1
  // §4.7.3 define class_id 24, version 0 with five attributes:
  // 1 logical_name, 2 secondary_address, 3 primary_address_list,
  // 4 tabi_list, 5 fatal_error. Attributes 2..4 are static, 5 is
  // dynamic. The fatal_error attribute always returns the latest
  // observed protocol fatal error (server-managed); the built-in
  // object treats it as ReadOnly even when mutableAccess is
  // ReadAndWrite.
  CosemIecTwistedPairSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& secondaryAddress,
    const CosemByteBuffer& primaryAddressList,
    const CosemByteBuffer& tabiList,
    const CosemByteBuffer& fatalError,
    AttributeAccessMode mutableAccess);
  CosemIecTwistedPairSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& secondaryAddress,
    const CosemByteBuffer& primaryAddressList,
    const CosemByteBuffer& tabiList,
    const CosemByteBuffer& fatalError,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& SecondaryAddress() const;
  const CosemByteBuffer& PrimaryAddressList() const;
  const CosemByteBuffer& TabiList() const;
  const CosemByteBuffer& FatalError() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer secondaryAddress_;
  CosemByteBuffer primaryAddressList_;
  CosemByteBuffer tabiList_;
  CosemByteBuffer fatalError_;
  CosemAccessRights rights_;
};

class CosemMBusSlavePortSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemMBusSlavePortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& defaultBaud,
    const CosemByteBuffer& availBaud,
    const CosemByteBuffer& addrState,
    const CosemByteBuffer& busAddress,
    AttributeAccessMode mutableAccess);
  CosemMBusSlavePortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& defaultBaud,
    const CosemByteBuffer& availBaud,
    const CosemByteBuffer& addrState,
    const CosemByteBuffer& busAddress,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& DefaultBaud() const;
  const CosemByteBuffer& AvailBaud() const;
  const CosemByteBuffer& AddrState() const;
  const CosemByteBuffer& BusAddress() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer defaultBaud_;
  CosemByteBuffer availBaud_;
  CosemByteBuffer addrState_;
  CosemByteBuffer busAddress_;
  CosemAccessRights rights_;
};

class CosemIpv6SetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemIpv6SetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& dataLinkLayerReference,
    const CosemByteBuffer& addressConfigMode,
    const CosemByteBuffer& unicastIpAddress,
    const CosemByteBuffer& multicastIpAddress,
    const CosemByteBuffer& gatewayIpAddress,
    const CosemByteBuffer& primaryDnsAddress,
    const CosemByteBuffer& secondaryDnsAddress,
    const CosemByteBuffer& trafficClass,
    const CosemByteBuffer& neighborDiscoverySetup,
    AttributeAccessMode mutableAccess);
  CosemIpv6SetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& dataLinkLayerReference,
    const CosemByteBuffer& addressConfigMode,
    const CosemByteBuffer& unicastIpAddress,
    const CosemByteBuffer& multicastIpAddress,
    const CosemByteBuffer& gatewayIpAddress,
    const CosemByteBuffer& primaryDnsAddress,
    const CosemByteBuffer& secondaryDnsAddress,
    const CosemByteBuffer& trafficClass,
    const CosemByteBuffer& neighborDiscoverySetup,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& DataLinkLayerReference() const;
  const CosemByteBuffer& AddressConfigMode() const;
  const CosemByteBuffer& UnicastIpAddress() const;
  const CosemByteBuffer& MulticastIpAddress() const;
  const CosemByteBuffer& GatewayIpAddress() const;
  const CosemByteBuffer& PrimaryDnsAddress() const;
  const CosemByteBuffer& SecondaryDnsAddress() const;
  const CosemByteBuffer& TrafficClass() const;
  const CosemByteBuffer& NeighborDiscoverySetup() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer dataLinkLayerReference_;
  CosemByteBuffer addressConfigMode_;
  CosemByteBuffer unicastIpAddress_;
  CosemByteBuffer multicastIpAddress_;
  CosemByteBuffer gatewayIpAddress_;
  CosemByteBuffer primaryDnsAddress_;
  CosemByteBuffer secondaryDnsAddress_;
  CosemByteBuffer trafficClass_;
  CosemByteBuffer neighborDiscoverySetup_;
  CosemAccessRights rights_;
};

class CosemUtilityTablesObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemUtilityTablesObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tableId,
    const CosemByteBuffer& length,
    const CosemByteBuffer& buffer,
    AttributeAccessMode mutableAccess);
  CosemUtilityTablesObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tableId,
    const CosemByteBuffer& length,
    const CosemByteBuffer& buffer,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& TableId() const;
  const CosemByteBuffer& Length() const;
  const CosemByteBuffer& Buffer() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer tableId_;
  CosemByteBuffer length_;
  CosemByteBuffer buffer_;
  CosemAccessRights rights_;
};

class CosemSensorManagerObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSensorManagerObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& serialNumber,
    const CosemByteBuffer& metrologicalIdentification,
    const CosemByteBuffer& outputType,
    const CosemByteBuffer& adjustmentMethod,
    const CosemByteBuffer& sealingMethod,
    const CosemByteBuffer& rawValue,
    const CosemByteBuffer& scalerUnit,
    const CosemByteBuffer& status,
    const CosemByteBuffer& captureTime,
    const CosemByteBuffer& rawValueThresholds,
    const CosemByteBuffer& rawValueActions,
    const CosemByteBuffer& processedValue,
    const CosemByteBuffer& processedValueThresholds,
    const CosemByteBuffer& processedValueActions,
    AttributeAccessMode mutableAccess);
  CosemSensorManagerObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& serialNumber,
    const CosemByteBuffer& metrologicalIdentification,
    const CosemByteBuffer& outputType,
    const CosemByteBuffer& adjustmentMethod,
    const CosemByteBuffer& sealingMethod,
    const CosemByteBuffer& rawValue,
    const CosemByteBuffer& scalerUnit,
    const CosemByteBuffer& status,
    const CosemByteBuffer& captureTime,
    const CosemByteBuffer& rawValueThresholds,
    const CosemByteBuffer& rawValueActions,
    const CosemByteBuffer& processedValue,
    const CosemByteBuffer& processedValueThresholds,
    const CosemByteBuffer& processedValueActions,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& SerialNumber() const;
  const CosemByteBuffer& MetrologicalIdentification() const;
  const CosemByteBuffer& OutputType() const;
  const CosemByteBuffer& AdjustmentMethod() const;
  const CosemByteBuffer& SealingMethod() const;
  const CosemByteBuffer& RawValue() const;
  const CosemByteBuffer& ScalerUnit() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& CaptureTime() const;
  const CosemByteBuffer& RawValueThresholds() const;
  const CosemByteBuffer& RawValueActions() const;
  const CosemByteBuffer& ProcessedValue() const;
  const CosemByteBuffer& ProcessedValueThresholds() const;
  const CosemByteBuffer& ProcessedValueActions() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer serialNumber_;
  CosemByteBuffer metrologicalIdentification_;
  CosemByteBuffer outputType_;
  CosemByteBuffer adjustmentMethod_;
  CosemByteBuffer sealingMethod_;
  CosemByteBuffer rawValue_;
  CosemByteBuffer scalerUnit_;
  CosemByteBuffer status_;
  CosemByteBuffer captureTime_;
  CosemByteBuffer rawValueThresholds_;
  CosemByteBuffer rawValueActions_;
  CosemByteBuffer processedValue_;
  CosemByteBuffer processedValueThresholds_;
  CosemByteBuffer processedValueActions_;
  CosemAccessRights rights_;
};

class CosemArbitratorObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemArbitratorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& actions,
    const CosemByteBuffer& permissionsTable,
    const CosemByteBuffer& weightingsTable,
    const CosemByteBuffer& mostRecentRequestsTable,
    const CosemByteBuffer& lastOutcome,
    AttributeAccessMode mutableAccess);
  CosemArbitratorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& actions,
    const CosemByteBuffer& permissionsTable,
    const CosemByteBuffer& weightingsTable,
    const CosemByteBuffer& mostRecentRequestsTable,
    const CosemByteBuffer& lastOutcome,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Actions() const;
  const CosemByteBuffer& PermissionsTable() const;
  const CosemByteBuffer& WeightingsTable() const;
  const CosemByteBuffer& MostRecentRequestsTable() const;
  const CosemByteBuffer& LastOutcome() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer actions_;
  CosemByteBuffer permissionsTable_;
  CosemByteBuffer weightingsTable_;
  CosemByteBuffer mostRecentRequestsTable_;
  CosemByteBuffer lastOutcome_;
  CosemAccessRights rights_;
};

class CosemStatusMappingObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemStatusMappingObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& statusWord,
    const CosemByteBuffer& mappings,
    AttributeAccessMode mutableAccess);
  CosemStatusMappingObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& statusWord,
    const CosemByteBuffer& mappings,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& StatusWord() const;
  const CosemByteBuffer& Mappings() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer statusWord_;
  CosemByteBuffer mappings_;
  CosemAccessRights rights_;
};

class CosemParameterMonitorObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemParameterMonitorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& changedParameter,
    const CosemByteBuffer& captureTime,
    const CosemByteBuffer& parameters,
    const CosemByteBuffer& parameterListName,
    const CosemByteBuffer& hashAlgorithmId,
    const CosemByteBuffer& parameterValueDigest,
    const CosemByteBuffer& parameterValues,
    AttributeAccessMode mutableAccess);
  CosemParameterMonitorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& changedParameter,
    const CosemByteBuffer& captureTime,
    const CosemByteBuffer& parameters,
    const CosemByteBuffer& parameterListName,
    const CosemByteBuffer& hashAlgorithmId,
    const CosemByteBuffer& parameterValueDigest,
    const CosemByteBuffer& parameterValues,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ChangedParameter() const;
  const CosemByteBuffer& CaptureTime() const;
  const CosemByteBuffer& Parameters() const;
  const CosemByteBuffer& ParameterListName() const;
  const CosemByteBuffer& HashAlgorithmId() const;
  const CosemByteBuffer& ParameterValueDigest() const;
  const CosemByteBuffer& ParameterValues() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer changedParameter_;
  CosemByteBuffer captureTime_;
  CosemByteBuffer parameters_;
  CosemByteBuffer parameterListName_;
  CosemByteBuffer hashAlgorithmId_;
  CosemByteBuffer parameterValueDigest_;
  CosemByteBuffer parameterValues_;
  CosemAccessRights rights_;
};

class CosemCompactDataObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemCompactDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& buffer,
    const CosemByteBuffer& captureObjects,
    const CosemByteBuffer& templateId,
    const CosemByteBuffer& templateDescription,
    const CosemByteBuffer& captureMethod,
    AttributeAccessMode mutableAccess);
  CosemCompactDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& buffer,
    const CosemByteBuffer& captureObjects,
    const CosemByteBuffer& templateId,
    const CosemByteBuffer& templateDescription,
    const CosemByteBuffer& captureMethod,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Buffer() const;
  const CosemByteBuffer& CaptureObjects() const;
  const CosemByteBuffer& TemplateId() const;
  const CosemByteBuffer& TemplateDescription() const;
  const CosemByteBuffer& CaptureMethod() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer buffer_;
  CosemByteBuffer captureObjects_;
  CosemByteBuffer templateId_;
  CosemByteBuffer templateDescription_;
  CosemByteBuffer captureMethod_;
  CosemAccessRights rights_;
};

class CosemDataProtectionObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemDataProtectionObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& protectionBuffer,
    const CosemByteBuffer& protectionObjectList,
    const CosemByteBuffer& protectionParametersGet,
    const CosemByteBuffer& protectionParametersSet,
    const CosemByteBuffer& requiredProtection,
    AttributeAccessMode mutableAccess);
  CosemDataProtectionObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& protectionBuffer,
    const CosemByteBuffer& protectionObjectList,
    const CosemByteBuffer& protectionParametersGet,
    const CosemByteBuffer& protectionParametersSet,
    const CosemByteBuffer& requiredProtection,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ProtectionBuffer() const;
  const CosemByteBuffer& ProtectionObjectList() const;
  const CosemByteBuffer& ProtectionParametersGet() const;
  const CosemByteBuffer& ProtectionParametersSet() const;
  const CosemByteBuffer& RequiredProtection() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer protectionBuffer_;
  CosemByteBuffer protectionObjectList_;
  CosemByteBuffer protectionParametersGet_;
  CosemByteBuffer protectionParametersSet_;
  CosemByteBuffer requiredProtection_;
  CosemAccessRights rights_;
};

class CosemIecLocalPortSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemIecLocalPortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& defaultMode,
    const CosemByteBuffer& defaultBaud,
    const CosemByteBuffer& proposedBaud,
    const CosemByteBuffer& responseTime,
    const CosemByteBuffer& deviceAddress,
    const CosemByteBuffer& password1,
    const CosemByteBuffer& password2,
    const CosemByteBuffer& password5,
    AttributeAccessMode mutableAccess);
  CosemIecLocalPortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& defaultMode,
    const CosemByteBuffer& defaultBaud,
    const CosemByteBuffer& proposedBaud,
    const CosemByteBuffer& responseTime,
    const CosemByteBuffer& deviceAddress,
    const CosemByteBuffer& password1,
    const CosemByteBuffer& password2,
    const CosemByteBuffer& password5,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& DefaultMode() const;
  const CosemByteBuffer& DefaultBaud() const;
  const CosemByteBuffer& ProposedBaud() const;
  const CosemByteBuffer& ResponseTime() const;
  const CosemByteBuffer& DeviceAddress() const;
  const CosemByteBuffer& Password1() const;
  const CosemByteBuffer& Password2() const;
  const CosemByteBuffer& Password5() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer defaultMode_;
  CosemByteBuffer defaultBaud_;
  CosemByteBuffer proposedBaud_;
  CosemByteBuffer responseTime_;
  CosemByteBuffer deviceAddress_;
  CosemByteBuffer password1_;
  CosemByteBuffer password2_;
  CosemByteBuffer password5_;
  CosemAccessRights rights_;
};

class CosemAssociationSnObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 4u;

  CosemAssociationSnObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& objectList,
    const CosemByteBuffer& accessRightsList,
    const CosemByteBuffer& securitySetupReference,
    const CosemByteBuffer& userList,
    const CosemByteBuffer& currentUser,
    AttributeAccessMode mutableAccess);
  CosemAssociationSnObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& objectList,
    const CosemByteBuffer& accessRightsList,
    const CosemByteBuffer& securitySetupReference,
    const CosemByteBuffer& userList,
    const CosemByteBuffer& currentUser,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ObjectList() const;
  const CosemByteBuffer& AccessRightsList() const;
  const CosemByteBuffer& SecuritySetupReference() const;
  const CosemByteBuffer& UserList() const;
  const CosemByteBuffer& CurrentUser() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer objectList_;
  CosemByteBuffer accessRightsList_;
  CosemByteBuffer securitySetupReference_;
  CosemByteBuffer userList_;
  CosemByteBuffer currentUser_;
  CosemAccessRights rights_;
};

class CosemMBusClientObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemMBusClientObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mbusPortReference,
    const CosemByteBuffer& captureDefinition,
    const CosemByteBuffer& capturePeriod,
    const CosemByteBuffer& primaryAddress,
    const CosemByteBuffer& identificationNumber,
    const CosemByteBuffer& manufacturerId,
    const CosemByteBuffer& version,
    const CosemByteBuffer& deviceType,
    const CosemByteBuffer& accessNumber,
    const CosemByteBuffer& status,
    const CosemByteBuffer& alarm,
    const CosemByteBuffer& configuration,
    const CosemByteBuffer& encryptionKeyStatus,
    AttributeAccessMode mutableAccess);
  CosemMBusClientObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mbusPortReference,
    const CosemByteBuffer& captureDefinition,
    const CosemByteBuffer& capturePeriod,
    const CosemByteBuffer& primaryAddress,
    const CosemByteBuffer& identificationNumber,
    const CosemByteBuffer& manufacturerId,
    const CosemByteBuffer& version,
    const CosemByteBuffer& deviceType,
    const CosemByteBuffer& accessNumber,
    const CosemByteBuffer& status,
    const CosemByteBuffer& alarm,
    const CosemByteBuffer& configuration,
    const CosemByteBuffer& encryptionKeyStatus,
    AttributeAccessMode mutableAccess,
    std::uint8_t version_);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& MBusPortReference() const;
  const CosemByteBuffer& CaptureDefinition() const;
  const CosemByteBuffer& CapturePeriod() const;
  const CosemByteBuffer& PrimaryAddress() const;
  const CosemByteBuffer& IdentificationNumber() const;
  const CosemByteBuffer& ManufacturerId() const;
  const CosemByteBuffer& Version() const;
  const CosemByteBuffer& DeviceType() const;
  const CosemByteBuffer& AccessNumber() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& Alarm() const;
  const CosemByteBuffer& Configuration() const;
  const CosemByteBuffer& EncryptionKeyStatus() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer mbusPortReference_;
  CosemByteBuffer captureDefinition_;
  CosemByteBuffer capturePeriod_;
  CosemByteBuffer primaryAddress_;
  CosemByteBuffer identificationNumber_;
  CosemByteBuffer manufacturerId_;
  CosemByteBuffer version_;
  CosemByteBuffer deviceType_;
  CosemByteBuffer accessNumber_;
  CosemByteBuffer status_;
  CosemByteBuffer alarm_;
  CosemByteBuffer configuration_;
  CosemByteBuffer encryptionKeyStatus_;
  CosemAccessRights rights_;
};

class CosemMBusMasterPortSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemMBusMasterPortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& commSpeed,
    AttributeAccessMode mutableAccess);
  CosemMBusMasterPortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& commSpeed,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& CommSpeed() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer commSpeed_;
  CosemAccessRights rights_;
};

class CosemMBusDiagnosticObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemMBusDiagnosticObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& receivedSignalStrength,
    const CosemByteBuffer& channelId,
    const CosemByteBuffer& linkStatus,
    const CosemByteBuffer& broadcastFramesCounter,
    const CosemByteBuffer& transmissionsCounter,
    const CosemByteBuffer& fcsOkFramesCounter,
    const CosemByteBuffer& fcsNokFramesCounter,
    const CosemByteBuffer& captureTime,
    AttributeAccessMode mutableAccess);
  CosemMBusDiagnosticObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& receivedSignalStrength,
    const CosemByteBuffer& channelId,
    const CosemByteBuffer& linkStatus,
    const CosemByteBuffer& broadcastFramesCounter,
    const CosemByteBuffer& transmissionsCounter,
    const CosemByteBuffer& fcsOkFramesCounter,
    const CosemByteBuffer& fcsNokFramesCounter,
    const CosemByteBuffer& captureTime,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ReceivedSignalStrength() const;
  const CosemByteBuffer& ChannelId() const;
  const CosemByteBuffer& LinkStatus() const;
  const CosemByteBuffer& BroadcastFramesCounter() const;
  const CosemByteBuffer& TransmissionsCounter() const;
  const CosemByteBuffer& FcsOkFramesCounter() const;
  const CosemByteBuffer& FcsNokFramesCounter() const;
  const CosemByteBuffer& CaptureTime() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer receivedSignalStrength_;
  CosemByteBuffer channelId_;
  CosemByteBuffer linkStatus_;
  CosemByteBuffer broadcastFramesCounter_;
  CosemByteBuffer transmissionsCounter_;
  CosemByteBuffer fcsOkFramesCounter_;
  CosemByteBuffer fcsNokFramesCounter_;
  CosemByteBuffer captureTime_;
  CosemAccessRights rights_;
};

class CosemPrimePlcMacSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemPrimePlcMacSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& macMinConWindow,
    const CosemByteBuffer& macMaxConWindow,
    const CosemByteBuffer& macChannelAccessFairnessLimit,
    const CosemByteBuffer& macEma,
    const CosemByteBuffer& macSarSize,
    const CosemByteBuffer& macMaxPduSize,
    const CosemByteBuffer& macMinSwitchSearchTime,
    const CosemByteBuffer& macMaxPromotionPdu,
    const CosemByteBuffer& macPromotionPduTxPeriod,
    const CosemByteBuffer& macBeaconsPerFrame,
    const CosemByteBuffer& macScpMaxTxAttempts,
    const CosemByteBuffer& macCtlReTxTimer,
    const CosemByteBuffer& macMaxLnid,
    AttributeAccessMode mutableAccess);
  CosemPrimePlcMacSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& macMinConWindow,
    const CosemByteBuffer& macMaxConWindow,
    const CosemByteBuffer& macChannelAccessFairnessLimit,
    const CosemByteBuffer& macEma,
    const CosemByteBuffer& macSarSize,
    const CosemByteBuffer& macMaxPduSize,
    const CosemByteBuffer& macMinSwitchSearchTime,
    const CosemByteBuffer& macMaxPromotionPdu,
    const CosemByteBuffer& macPromotionPduTxPeriod,
    const CosemByteBuffer& macBeaconsPerFrame,
    const CosemByteBuffer& macScpMaxTxAttempts,
    const CosemByteBuffer& macCtlReTxTimer,
    const CosemByteBuffer& macMaxLnid,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& MacMinConWindow() const;
  const CosemByteBuffer& MacMaxConWindow() const;
  const CosemByteBuffer& MacChannelAccessFairnessLimit() const;
  const CosemByteBuffer& MacEma() const;
  const CosemByteBuffer& MacSarSize() const;
  const CosemByteBuffer& MacMaxPduSize() const;
  const CosemByteBuffer& MacMinSwitchSearchTime() const;
  const CosemByteBuffer& MacMaxPromotionPdu() const;
  const CosemByteBuffer& MacPromotionPduTxPeriod() const;
  const CosemByteBuffer& MacBeaconsPerFrame() const;
  const CosemByteBuffer& MacScpMaxTxAttempts() const;
  const CosemByteBuffer& MacCtlReTxTimer() const;
  const CosemByteBuffer& MacMaxLnid() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer macMinConWindow_;
  CosemByteBuffer macMaxConWindow_;
  CosemByteBuffer macChannelAccessFairnessLimit_;
  CosemByteBuffer macEma_;
  CosemByteBuffer macSarSize_;
  CosemByteBuffer macMaxPduSize_;
  CosemByteBuffer macMinSwitchSearchTime_;
  CosemByteBuffer macMaxPromotionPdu_;
  CosemByteBuffer macPromotionPduTxPeriod_;
  CosemByteBuffer macBeaconsPerFrame_;
  CosemByteBuffer macScpMaxTxAttempts_;
  CosemByteBuffer macCtlReTxTimer_;
  CosemByteBuffer macMaxLnid_;
  CosemAccessRights rights_;
};

class CosemPrimePlcMacFunctionalParametersObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemPrimePlcMacFunctionalParametersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& lnid,
    const CosemByteBuffer& lsid,
    const CosemByteBuffer& sid,
    const CosemByteBuffer& sna,
    const CosemByteBuffer& state,
    const CosemByteBuffer& scpLength,
    const CosemByteBuffer& nodeHierarchyLevel,
    const CosemByteBuffer& beaconSlotCount,
    const CosemByteBuffer& beaconRxSlot,
    const CosemByteBuffer& beaconTxSlot,
    const CosemByteBuffer& beaconRxFrequency,
    const CosemByteBuffer& beaconTxFrequency,
    const CosemByteBuffer& capabilities,
    AttributeAccessMode mutableAccess);
  CosemPrimePlcMacFunctionalParametersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& lnid,
    const CosemByteBuffer& lsid,
    const CosemByteBuffer& sid,
    const CosemByteBuffer& sna,
    const CosemByteBuffer& state,
    const CosemByteBuffer& scpLength,
    const CosemByteBuffer& nodeHierarchyLevel,
    const CosemByteBuffer& beaconSlotCount,
    const CosemByteBuffer& beaconRxSlot,
    const CosemByteBuffer& beaconTxSlot,
    const CosemByteBuffer& beaconRxFrequency,
    const CosemByteBuffer& beaconTxFrequency,
    const CosemByteBuffer& capabilities,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Lnid() const;
  const CosemByteBuffer& Lsid() const;
  const CosemByteBuffer& Sid() const;
  const CosemByteBuffer& Sna() const;
  const CosemByteBuffer& State() const;
  const CosemByteBuffer& ScpLength() const;
  const CosemByteBuffer& NodeHierarchyLevel() const;
  const CosemByteBuffer& BeaconSlotCount() const;
  const CosemByteBuffer& BeaconRxSlot() const;
  const CosemByteBuffer& BeaconTxSlot() const;
  const CosemByteBuffer& BeaconRxFrequency() const;
  const CosemByteBuffer& BeaconTxFrequency() const;
  const CosemByteBuffer& Capabilities() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer lnid_;
  CosemByteBuffer lsid_;
  CosemByteBuffer sid_;
  CosemByteBuffer sna_;
  CosemByteBuffer state_;
  CosemByteBuffer scpLength_;
  CosemByteBuffer nodeHierarchyLevel_;
  CosemByteBuffer beaconSlotCount_;
  CosemByteBuffer beaconRxSlot_;
  CosemByteBuffer beaconTxSlot_;
  CosemByteBuffer beaconRxFrequency_;
  CosemByteBuffer beaconTxFrequency_;
  CosemByteBuffer capabilities_;
  CosemAccessRights rights_;
};

class CosemPrimePlcMacCountersObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemPrimePlcMacCountersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& txDataPktCount,
    const CosemByteBuffer& rxDataPktCount,
    const CosemByteBuffer& txCtrlPktCount,
    const CosemByteBuffer& rxCtrlPktCount,
    const CosemByteBuffer& csmaFailCount,
    const CosemByteBuffer& csmaChBusyCount,
    AttributeAccessMode mutableAccess);
  CosemPrimePlcMacCountersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& txDataPktCount,
    const CosemByteBuffer& rxDataPktCount,
    const CosemByteBuffer& txCtrlPktCount,
    const CosemByteBuffer& rxCtrlPktCount,
    const CosemByteBuffer& csmaFailCount,
    const CosemByteBuffer& csmaChBusyCount,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& TxDataPktCount() const;
  const CosemByteBuffer& RxDataPktCount() const;
  const CosemByteBuffer& TxCtrlPktCount() const;
  const CosemByteBuffer& RxCtrlPktCount() const;
  const CosemByteBuffer& CsmaFailCount() const;
  const CosemByteBuffer& CsmaChBusyCount() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer txDataPktCount_;
  CosemByteBuffer rxDataPktCount_;
  CosemByteBuffer txCtrlPktCount_;
  CosemByteBuffer rxCtrlPktCount_;
  CosemByteBuffer csmaFailCount_;
  CosemByteBuffer csmaChBusyCount_;
  CosemAccessRights rights_;
};

class CosemPrimePlcMacNetworkAdminDataObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemPrimePlcMacNetworkAdminDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& nodeRegistrations,
    const CosemByteBuffer& nodeUnregistrations,
    const CosemByteBuffer& processedAliveMsgs,
    const CosemByteBuffer& handledPromotions,
    AttributeAccessMode mutableAccess);
  CosemPrimePlcMacNetworkAdminDataObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& nodeRegistrations,
    const CosemByteBuffer& nodeUnregistrations,
    const CosemByteBuffer& processedAliveMsgs,
    const CosemByteBuffer& handledPromotions,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& NodeRegistrations() const;
  const CosemByteBuffer& NodeUnregistrations() const;
  const CosemByteBuffer& ProcessedAliveMsgs() const;
  const CosemByteBuffer& HandledPromotions() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer nodeRegistrations_;
  CosemByteBuffer nodeUnregistrations_;
  CosemByteBuffer processedAliveMsgs_;
  CosemByteBuffer handledPromotions_;
  CosemAccessRights rights_;
};

class CosemPrimePlcApplicationIdentificationObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemPrimePlcApplicationIdentificationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& applicationIdentifier,
    AttributeAccessMode mutableAccess);
  CosemPrimePlcApplicationIdentificationObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& applicationIdentifier,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ApplicationIdentifier() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer applicationIdentifier_;
  CosemAccessRights rights_;
};

class CosemSFskPlcPhyMacSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  // Attribute layout per IEC 62056-6-2 ED4 4.10.3 / DLMS UA Blue Book
  // IC 50 class_id=50, version=1:
  //   2  initiator_electrical_phase  enum
  //   3  delta_electrical_phase      enum
  //   4  max_receiving_gain          unsigned
  //   5  max_transmitting_gain       unsigned
  //   6  search_initiator_threshold  unsigned (default 98)
  //   7  frequencies                 structure { mark_frequency:
  //                                              double-long-unsigned,
  //                                              space_frequency:
  //                                              double-long-unsigned }
  //   8  mac_address                 long-unsigned (default FFE)
  //   9  mac_group_addresses         array of long-unsigned
  //  10  repeater                    enum
  //  11  repeater_status             boolean
  //  12  min_delta_credit            unsigned
  //  13  initiator_mac_address       long-unsigned
  //  14  synchronization_locked      boolean
  //  15  transmission_speed          enum (v1 only, default 3)
  struct Attributes
  {
    CosemByteBuffer initiatorElectricalPhase;
    CosemByteBuffer deltaElectricalPhase;
    CosemByteBuffer maxReceivingGain;
    CosemByteBuffer maxTransmittingGain;
    CosemByteBuffer searchInitiatorThreshold;
    CosemByteBuffer frequencies;
    CosemByteBuffer macAddress;
    CosemByteBuffer macGroupAddresses;
    CosemByteBuffer repeater;
    CosemByteBuffer repeaterStatus;
    CosemByteBuffer minDeltaCredit;
    CosemByteBuffer initiatorMacAddress;
    CosemByteBuffer synchronizationLocked;
    CosemByteBuffer transmissionSpeed;
  };

  CosemSFskPlcPhyMacSetupObject(
    const CosemLogicalName& logicalName,
    const Attributes& attributes,
    AttributeAccessMode mutableAccess);
  CosemSFskPlcPhyMacSetupObject(
    const CosemLogicalName& logicalName,
    const Attributes& attributes,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const Attributes& AttributeData() const;

private:
  CosemObjectDescriptor descriptor_;
  Attributes attributes_;
  CosemAccessRights rights_;
};

class CosemSFskActiveInitiatorObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSFskActiveInitiatorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& activeInitiator,
    AttributeAccessMode mutableAccess);
  CosemSFskActiveInitiatorObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& activeInitiator,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& ActiveInitiator() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer activeInitiator_;
  CosemAccessRights rights_;
};

class CosemSFskMacSyncTimeoutsObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  // IEC 62056-6-2 ED4 (2021) §4.10.5 and DLMS UA Blue Book Ed. 12.1
  // §4.10.5 define class_id 52, version 0 with five attributes:
  // 1 logical_name, 2 search_initiator_timeout,
  // 3 synchronization_confirmation_timeout, 4 time_out_not_addressed,
  // 5 time_out_frame_not_OK. Attributes 2..5 are static long-unsigned
  // timers (per IEC 61334-4-512 / IEC 61334-5-1 MIB variables). The
  // class defines no specific methods.
  CosemSFskMacSyncTimeoutsObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& searchInitiatorTimeout,
    const CosemByteBuffer& synchronizationConfirmationTimeout,
    const CosemByteBuffer& timeOutNotAddressed,
    const CosemByteBuffer& timeOutFrameNotOk,
    AttributeAccessMode mutableAccess);
  CosemSFskMacSyncTimeoutsObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& searchInitiatorTimeout,
    const CosemByteBuffer& synchronizationConfirmationTimeout,
    const CosemByteBuffer& timeOutNotAddressed,
    const CosemByteBuffer& timeOutFrameNotOk,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& SearchInitiatorTimeout() const;
  const CosemByteBuffer& SynchronizationConfirmationTimeout() const;
  const CosemByteBuffer& TimeOutNotAddressed() const;
  const CosemByteBuffer& TimeOutFrameNotOk() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer searchInitiatorTimeout_;
  CosemByteBuffer synchronizationConfirmationTimeout_;
  CosemByteBuffer timeOutNotAddressed_;
  CosemByteBuffer timeOutFrameNotOk_;
  CosemAccessRights rights_;
};

class CosemSFskMacCountersObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  // IEC 62056-6-2 ED4 (2021) §4.10.6 and DLMS UA Blue Book Ed. 12.1
  // §4.10.6 define class_id 53, version 0 with eight attributes:
  // 1 logical_name, 2 synchronization_register (array of
  // synchronization_couples), 3 desynchronization_listing
  // (structure), 4 broadcast_frames_counter (array of
  // broadcast_frame_counter structures), 5 repetitions_counter
  // (double-long-unsigned), 6 transmissions_counter, 7
  // CRC_OK_frames_counter, 8 CRC_NOK_frames_counter. Attributes
  // 2..8 are dynamic counters/MIB variables from IEC 61334-4-512
  // and IEC 61334-5-1. The class defines one specific method:
  // 1 reset(data) which clears the dynamic counters; the built-in
  // object surfaces it as UnsupportedFeature because counter
  // bookkeeping is backend-owned.
  CosemSFskMacCountersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& synchronizationRegister,
    const CosemByteBuffer& desynchronizationListing,
    const CosemByteBuffer& broadcastFramesCounter,
    const CosemByteBuffer& repetitionsCounter,
    const CosemByteBuffer& transmissionsCounter,
    const CosemByteBuffer& crcOkFramesCounter,
    const CosemByteBuffer& crcNokFramesCounter,
    AttributeAccessMode mutableAccess);
  CosemSFskMacCountersObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& synchronizationRegister,
    const CosemByteBuffer& desynchronizationListing,
    const CosemByteBuffer& broadcastFramesCounter,
    const CosemByteBuffer& repetitionsCounter,
    const CosemByteBuffer& transmissionsCounter,
    const CosemByteBuffer& crcOkFramesCounter,
    const CosemByteBuffer& crcNokFramesCounter,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& SynchronizationRegister() const;
  const CosemByteBuffer& DesynchronizationListing() const;
  const CosemByteBuffer& BroadcastFramesCounter() const;
  const CosemByteBuffer& RepetitionsCounter() const;
  const CosemByteBuffer& TransmissionsCounter() const;
  const CosemByteBuffer& CrcOkFramesCounter() const;
  const CosemByteBuffer& CrcNokFramesCounter() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer synchronizationRegister_;
  CosemByteBuffer desynchronizationListing_;
  CosemByteBuffer broadcastFramesCounter_;
  CosemByteBuffer repetitionsCounter_;
  CosemByteBuffer transmissionsCounter_;
  CosemByteBuffer crcOkFramesCounter_;
  CosemByteBuffer crcNokFramesCounter_;
  CosemAccessRights rights_;
};

class CosemIec61334432LlcSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  // IEC 62056-6-2 ED4 (2021) §4.10.7 and DLMS UA Blue Book Ed. 12.1
  // §4.10.7 define class_id 55, version 1 with three attributes:
  // 1 logical_name, 2 max_frame_length (long-unsigned, length of
  // the LLC frame in bytes per IEC 61334-4-32:1996 §5.1.4; S-FSK
  // profile min/def/max 26/134/242 per IEC 61334-5-1:2001 §4.2.2),
  // 3 reply_status_list (array of reply_status structures
  // {L-SAP-selector: unsigned, length-of-waiting-L-SDU: unsigned};
  // MIB variable reply-status-list (variable 11) per
  // IEC 61334-4-512:2001 §5.4). The class defines no specific
  // methods. Note: ED4 deprecates version 0.
  CosemIec61334432LlcSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& maxFrameLength,
    const CosemByteBuffer& replyStatusList,
    AttributeAccessMode mutableAccess);
  CosemIec61334432LlcSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& maxFrameLength,
    const CosemByteBuffer& replyStatusList,
    AttributeAccessMode mutableAccess,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& MaxFrameLength() const;
  const CosemByteBuffer& ReplyStatusList() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer maxFrameLength_;
  CosemByteBuffer replyStatusList_;
  CosemAccessRights rights_;
};

enum class CosemClockBase
{
  NotDefined = 0,
  InternalCrystal = 1,
  MainsFrequency50Hz = 2,
  MainsFrequency60Hz = 3,
  Gps = 4,
  RadioControlled = 5
};

class CosemClockObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemClockObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& time,
    std::int16_t timeZone,
    std::uint8_t status,
    const CosemByteBuffer& daylightSavingsBegin,
    const CosemByteBuffer& daylightSavingsEnd,
    std::int8_t daylightSavingsDeviation,
    bool daylightSavingsEnabled,
    CosemClockBase clockBase);
  CosemClockObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& time,
    std::int16_t timeZone,
    std::uint8_t status,
    const CosemByteBuffer& daylightSavingsBegin,
    const CosemByteBuffer& daylightSavingsEnd,
    std::int8_t daylightSavingsDeviation,
    bool daylightSavingsEnabled,
    CosemClockBase clockBase,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const CosemByteBuffer& Time() const;
  std::int16_t TimeZone() const;
  std::uint8_t Status() const;
  const CosemByteBuffer& DaylightSavingsBegin() const;
  const CosemByteBuffer& DaylightSavingsEnd() const;
  std::int8_t DaylightSavingsDeviation() const;
  bool DaylightSavingsEnabled() const;
  CosemClockBase ClockBase() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer time_;
  std::int16_t timeZone_;
  std::uint8_t status_;
  CosemByteBuffer daylightSavingsBegin_;
  CosemByteBuffer daylightSavingsEnd_;
  std::int8_t daylightSavingsDeviation_;
  bool daylightSavingsEnabled_;
  CosemClockBase clockBase_;
  CosemAccessRights rights_;
};

enum class CosemProfileGenericSortMethod
{
  Fifo = 1,
  Lifo = 2,
  Largest = 3,
  Smallest = 4,
  NearestToZero = 5,
  FarthestFromZero = 6
};

struct CosemCaptureObject
{
  CosemObjectKey object;
  std::uint8_t attributeId;
  std::uint16_t dataIndex;
};

struct CosemProfileGenericRangeDescriptor
{
  CosemCaptureObject restrictingObject;
  CosemByteBuffer fromValue;
  CosemByteBuffer toValue;
  std::vector<CosemCaptureObject> selectedValues;
};

struct CosemProfileGenericEntryDescriptor
{
  std::uint32_t fromEntry;
  std::uint32_t toEntry;
  std::uint16_t fromSelectedValue;
  std::uint16_t toSelectedValue;
};

std::uint8_t ProfileGenericRangeAccessSelector();
std::uint8_t ProfileGenericEntryAccessSelector();
CosemByteBuffer EncodeProfileGenericCaptureObject(
  const CosemCaptureObject& object);
CosemStatus DecodeProfileGenericCaptureObject(
  const CosemByteBuffer& input,
  CosemCaptureObject& object);
CosemByteBuffer EncodeProfileGenericCaptureObjects(
  const std::vector<CosemCaptureObject>& objects);
CosemStatus DecodeProfileGenericCaptureObjects(
  const CosemByteBuffer& input,
  std::vector<CosemCaptureObject>& objects);
CosemByteBuffer EncodeProfileGenericBuffer(
  const std::vector<CosemByteBuffer>& rows);
CosemStatus DecodeProfileGenericBuffer(
  const CosemByteBuffer& input,
  std::vector<CosemByteBuffer>& rows);
CosemByteBuffer EncodeProfileGenericRangeDescriptor(
  const CosemProfileGenericRangeDescriptor& descriptor);
CosemStatus DecodeProfileGenericRangeDescriptor(
  const CosemByteBuffer& input,
  CosemProfileGenericRangeDescriptor& descriptor);
CosemByteBuffer EncodeProfileGenericEntryDescriptor(
  const CosemProfileGenericEntryDescriptor& descriptor);
CosemStatus DecodeProfileGenericEntryDescriptor(
  const CosemByteBuffer& input,
  CosemProfileGenericEntryDescriptor& descriptor);

class CosemProfileGenericObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  CosemProfileGenericObject(
    const CosemLogicalName& logicalName,
    const std::vector<CosemByteBuffer>& bufferRows,
    const std::vector<CosemCaptureObject>& captureObjects,
    std::uint32_t capturePeriod,
    std::uint32_t profileEntries);
  CosemProfileGenericObject(
    const CosemLogicalName& logicalName,
    const std::vector<CosemByteBuffer>& bufferRows,
    const std::vector<CosemCaptureObject>& captureObjects,
    std::uint32_t capturePeriod,
    std::uint32_t profileEntries,
    CosemProfileGenericSortMethod sortMethod,
    const CosemCaptureObject& sortObject);
  CosemProfileGenericObject(
    const CosemLogicalName& logicalName,
    const std::vector<CosemByteBuffer>& bufferRows,
    const std::vector<CosemCaptureObject>& captureObjects,
    std::uint32_t capturePeriod,
    std::uint32_t profileEntries,
    std::uint8_t version);
  CosemProfileGenericObject(
    const CosemLogicalName& logicalName,
    const std::vector<CosemByteBuffer>& bufferRows,
    const std::vector<CosemCaptureObject>& captureObjects,
    std::uint32_t capturePeriod,
    std::uint32_t profileEntries,
    CosemProfileGenericSortMethod sortMethod,
    const CosemCaptureObject& sortObject,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  const std::vector<CosemByteBuffer>& BufferRows() const;
  const std::vector<CosemCaptureObject>& CaptureObjects() const;
  CosemProfileGenericSortMethod SortMethod() const;
  const CosemCaptureObject& SortObject() const;

private:
  CosemObjectDescriptor descriptor_;
  std::vector<CosemByteBuffer> bufferRows_;
  std::vector<CosemCaptureObject> captureObjects_;
  std::uint32_t capturePeriod_;
  std::uint32_t profileEntries_;
  CosemProfileGenericSortMethod sortMethod_;
  CosemCaptureObject sortObject_;
  CosemAccessRights rights_;
};

CosemLogicalName CurrentAssociationLnName();
CosemLogicalName SapAssignmentName();
CosemLogicalName LogicalDeviceNameObjectName();
CosemLogicalName SecuritySetupName();
CosemLogicalName InvocationCounterObjectName();
CosemDataObject MakeLogicalDeviceNameObject(
  const std::string& logicalDeviceName);
CosemDataObject MakeInvocationCounterObject(
  std::uint32_t invocationCounter);
CosemByteBuffer EncodeAssociationAccessRights(
  const CosemAccessRights& rights);
CosemStatus DecodeAssociationAccessRights(
  const CosemByteBuffer& input,
  CosemAccessRights& rights);
CosemByteBuffer EncodeAssociationObjectList(
  const AssociationView& objectList);
CosemStatus DecodeAssociationObjectList(
  const CosemByteBuffer& input,
  AssociationView& objectList);

enum class CosemAssociationStatus
{
  NonAssociated = 0,
  AssociationPending = 1,
  Associated = 2
};

struct CosemAssociationUser
{
  std::uint8_t userId = 0u;
  std::string userName;
};

struct CosemAssociationLnConfig
{
  std::uint8_t version = 0u;
  CosemAssociationStatus associationStatus = CosemAssociationStatus::Associated;
  bool hasSecuritySetupReference = false;
  CosemLogicalName securitySetupReference;
  std::vector<CosemAssociationUser> users;
  CosemAssociationUser currentUser;
};

class CosemAssociationLnObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 3u;

  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList);
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList,
    std::uint8_t version);
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList,
    CosemAssociationStatus associationStatus);
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList,
    std::uint8_t version,
    CosemAssociationStatus associationStatus);
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList,
    CosemAssociationStatus associationStatus,
    const CosemLogicalName& securitySetupReference);
  CosemAssociationLnObject(
    const CosemLogicalName& logicalName,
    const AssociationView& objectList,
    const CosemAssociationLnConfig& config);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  AssociationView ObjectList() const;
  CosemAssociationStatus AssociationStatus() const;
  bool HasSecuritySetupReference() const;
  CosemLogicalName SecuritySetupReference() const;
  std::vector<CosemAssociationUser> Users() const;
  CosemAssociationUser CurrentUser() const;

private:
  CosemObjectDescriptor descriptor_;
  AssociationView objectList_;
  CosemAssociationStatus associationStatus_;
  bool hasSecuritySetupReference_;
  CosemLogicalName securitySetupReference_;
  std::vector<CosemAssociationUser> users_;
  CosemAssociationUser currentUser_;
  CosemAccessRights rights_;
};

class CosemSapAssignmentObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemSapAssignmentObject(
    const CosemLogicalName& logicalName,
    const std::vector<SapAssignment>& assignments);
  CosemSapAssignmentObject(
    const CosemLogicalName& logicalName,
    const std::vector<SapAssignment>& assignments,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  std::vector<SapAssignment> Assignments() const;

private:
  CosemObjectDescriptor descriptor_;
  std::vector<SapAssignment> assignments_;
  CosemAccessRights rights_;
};

class CosemSecuritySetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

  typedef std::array<std::uint8_t, 8u> SystemTitle;

  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle);
  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    std::uint8_t version);

  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    dlms::security::IMutableKeyStore* keyStore);
  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    dlms::security::IMutableKeyStore* keyStore,
    std::uint8_t version);

  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    dlms::security::IMutableKeyStore* keyStore,
    dlms::security::IInvocationCounterResetPolicy* counterResetPolicy);
  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    dlms::security::IMutableKeyStore* keyStore,
    dlms::security::IInvocationCounterResetPolicy* counterResetPolicy,
    std::uint8_t version);

  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    dlms::security::IMutableKeyStore* keyStore,
    dlms::security::IInvocationCounterResetPolicy* counterResetPolicy,
    ICosemCertificateStore* certificateStore);
  CosemSecuritySetupObject(
    const CosemLogicalName& logicalName,
    std::uint8_t securityPolicy,
    std::uint8_t securitySuite,
    const SystemTitle& clientSystemTitle,
    const SystemTitle& serverSystemTitle,
    dlms::security::IMutableKeyStore* keyStore,
    dlms::security::IInvocationCounterResetPolicy* counterResetPolicy,
    ICosemCertificateStore* certificateStore,
    std::uint8_t version);

  CosemObjectDescriptor Descriptor() const;
  CosemAccessRights AccessRights() const;
  CosemStatus ReadAttribute(
    std::uint8_t attributeId,
    CosemByteBuffer& output) const;
  CosemStatus WriteAttribute(
    std::uint8_t attributeId,
    const CosemByteBuffer& input);
  CosemStatus InvokeMethod(
    std::uint8_t methodId,
    const CosemByteBuffer& input,
    CosemByteBuffer& output);

  std::uint8_t SecurityPolicy() const;
  std::uint8_t SecuritySuite() const;
  const SystemTitle& ClientSystemTitle() const;
  const SystemTitle& ServerSystemTitle() const;

private:
  CosemObjectDescriptor descriptor_;
  std::uint8_t securityPolicy_;
  std::uint8_t securitySuite_;
  SystemTitle clientSystemTitle_;
  SystemTitle serverSystemTitle_;
  dlms::security::IMutableKeyStore* keyStore_;
  dlms::security::IInvocationCounterResetPolicy* counterResetPolicy_;
  ICosemCertificateStore* certificateStore_;
  CosemAccessRights rights_;
};

} // namespace cosem
} // namespace dlms
