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

class CosemPushSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 1u;

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
    const CosemByteBuffer& singleBuffer,
    const CosemByteBuffer& tableCellDefinition,
    const CosemByteBuffer& tableEntries,
    AttributeAccessMode mutableAccess);
  CosemRegisterTableObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& tableCellValues,
    const CosemByteBuffer& singleBuffer,
    const CosemByteBuffer& tableCellDefinition,
    const CosemByteBuffer& tableEntries,
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
  const CosemByteBuffer& SingleBuffer() const;
  const CosemByteBuffer& TableCellDefinition() const;
  const CosemByteBuffer& TableEntries() const;

  void SetTableCellValues(const CosemByteBuffer& value);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer tableCellValues_;
  CosemByteBuffer singleBuffer_;
  CosemByteBuffer tableCellDefinition_;
  CosemByteBuffer tableEntries_;
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
  static const std::uint8_t MaxSupportedVersion = 0u;

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
    AttributeAccessMode mutableAccess);
  CosemAutoAnswerObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& mode,
    const CosemByteBuffer& listeningWindow,
    const CosemByteBuffer& status,
    const CosemByteBuffer& numberOfCalls,
    const CosemByteBuffer& numberOfRings,
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

  void SetStatus(const CosemByteBuffer& status);

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer mode_;
  CosemByteBuffer listeningWindow_;
  CosemByteBuffer status_;
  CosemByteBuffer numberOfCalls_;
  CosemByteBuffer numberOfRings_;
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
    const CosemByteBuffer& smtpServer,
    const CosemByteBuffer& smtpServerPort,
    const CosemByteBuffer& userName,
    const CosemByteBuffer& loginPassword,
    const CosemByteBuffer& sender,
    const CosemByteBuffer& receivers,
    AttributeAccessMode mutableAccess);
  CosemSmtpSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& smtpServer,
    const CosemByteBuffer& smtpServerPort,
    const CosemByteBuffer& userName,
    const CosemByteBuffer& loginPassword,
    const CosemByteBuffer& sender,
    const CosemByteBuffer& receivers,
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

  const CosemByteBuffer& SmtpServer() const;
  const CosemByteBuffer& SmtpServerPort() const;
  const CosemByteBuffer& UserName() const;
  const CosemByteBuffer& LoginPassword() const;
  const CosemByteBuffer& Sender() const;
  const CosemByteBuffer& Receivers() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer smtpServer_;
  CosemByteBuffer smtpServerPort_;
  CosemByteBuffer userName_;
  CosemByteBuffer loginPassword_;
  CosemByteBuffer sender_;
  CosemByteBuffer receivers_;
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

  CosemIecTwistedPairSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& primaryAddress,
    const CosemByteBuffer& tabis,
    AttributeAccessMode mutableAccess);
  CosemIecTwistedPairSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& primaryAddress,
    const CosemByteBuffer& tabis,
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

  const CosemByteBuffer& PrimaryAddress() const;
  const CosemByteBuffer& Tabis() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer primaryAddress_;
  CosemByteBuffer tabis_;
  CosemAccessRights rights_;
};

class CosemMBusSlavePortSetupObject : public ICosemObject
{
public:
  static const std::uint8_t MaxSupportedVersion = 0u;

  CosemMBusSlavePortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& defaultBaud,
    const CosemByteBuffer& availableBaud,
    const CosemByteBuffer& status,
    const CosemByteBuffer& mbusPortReference,
    AttributeAccessMode mutableAccess);
  CosemMBusSlavePortSetupObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& defaultBaud,
    const CosemByteBuffer& availableBaud,
    const CosemByteBuffer& status,
    const CosemByteBuffer& mbusPortReference,
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
  const CosemByteBuffer& AvailableBaud() const;
  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& MBusPortReference() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer defaultBaud_;
  CosemByteBuffer availableBaud_;
  CosemByteBuffer status_;
  CosemByteBuffer mbusPortReference_;
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
    const CosemByteBuffer& status,
    const CosemByteBuffer& serialNumber,
    const CosemByteBuffer& deviceType,
    const CosemByteBuffer& manufacturerId,
    const CosemByteBuffer& firmwareVersion,
    const CosemByteBuffer& metrologyFirmwareVersion,
    const CosemByteBuffer& driver,
    const CosemByteBuffer& communicationDesc,
    const CosemByteBuffer& setupDesc,
    const CosemByteBuffer& measurementDesc,
    AttributeAccessMode mutableAccess);
  CosemSensorManagerObject(
    const CosemLogicalName& logicalName,
    const CosemByteBuffer& status,
    const CosemByteBuffer& serialNumber,
    const CosemByteBuffer& deviceType,
    const CosemByteBuffer& manufacturerId,
    const CosemByteBuffer& firmwareVersion,
    const CosemByteBuffer& metrologyFirmwareVersion,
    const CosemByteBuffer& driver,
    const CosemByteBuffer& communicationDesc,
    const CosemByteBuffer& setupDesc,
    const CosemByteBuffer& measurementDesc,
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

  const CosemByteBuffer& Status() const;
  const CosemByteBuffer& SerialNumber() const;
  const CosemByteBuffer& DeviceType() const;
  const CosemByteBuffer& ManufacturerId() const;
  const CosemByteBuffer& FirmwareVersion() const;
  const CosemByteBuffer& MetrologyFirmwareVersion() const;
  const CosemByteBuffer& Driver() const;
  const CosemByteBuffer& CommunicationDesc() const;
  const CosemByteBuffer& SetupDesc() const;
  const CosemByteBuffer& MeasurementDesc() const;

private:
  CosemObjectDescriptor descriptor_;
  CosemByteBuffer status_;
  CosemByteBuffer serialNumber_;
  CosemByteBuffer deviceType_;
  CosemByteBuffer manufacturerId_;
  CosemByteBuffer firmwareVersion_;
  CosemByteBuffer metrologyFirmwareVersion_;
  CosemByteBuffer driver_;
  CosemByteBuffer communicationDesc_;
  CosemByteBuffer setupDesc_;
  CosemByteBuffer measurementDesc_;
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

private:
  CosemObjectDescriptor descriptor_;
  std::vector<CosemByteBuffer> bufferRows_;
  std::vector<CosemCaptureObject> captureObjects_;
  std::uint32_t capturePeriod_;
  std::uint32_t profileEntries_;
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
