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
