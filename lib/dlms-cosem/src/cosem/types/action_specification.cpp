#include "dlms/cosem/types/action_specification.hpp"

namespace dlms {
namespace cosem {
namespace types {

namespace {

const CosemLogicalName kZeroLogicalName(0u, 0u, 0u, 0u, 0u, 0u);

bool IsZeroLogicalName(const CosemLogicalName& name)
{
  return name == kZeroLogicalName;
}

} // namespace

const std::uint8_t ActionSpecification::ServiceIdDummyValue;
const std::uint8_t ActionSpecification::ServiceIdWriteAttributeValue;
const std::uint8_t ActionSpecification::ServiceIdExecuteMethodValue;
const std::int8_t ActionSpecification::IndexMin;
const std::int8_t ActionSpecification::IndexMax;

ActionSpecification::ActionSpecification()
  : serviceId_(ScriptServiceId::Dummy),
    classId_(0u),
    logicalName_(kZeroLogicalName),
    index_(0),
    parameter_()
{
}

ActionSpecification::ActionSpecification(
  ScriptServiceId serviceId,
  std::uint16_t classId,
  const CosemLogicalName& logicalName,
  std::int8_t index,
  const CosemByteBuffer& parameter)
  : serviceId_(serviceId),
    classId_(classId),
    logicalName_(logicalName),
    index_(index),
    parameter_(parameter)
{
}

ScriptServiceId ActionSpecification::ServiceId() const
{
  return serviceId_;
}

std::uint16_t ActionSpecification::ClassId() const
{
  return classId_;
}

const CosemLogicalName& ActionSpecification::LogicalName() const
{
  return logicalName_;
}

std::int8_t ActionSpecification::Index() const
{
  return index_;
}

const CosemByteBuffer& ActionSpecification::Parameter() const
{
  return parameter_;
}

void ActionSpecification::SetServiceId(ScriptServiceId value)
{
  serviceId_ = value;
}

void ActionSpecification::SetClassId(std::uint16_t value)
{
  classId_ = value;
}

void ActionSpecification::SetLogicalName(const CosemLogicalName& value)
{
  logicalName_ = value;
}

void ActionSpecification::SetIndex(std::int8_t value)
{
  index_ = value;
}

void ActionSpecification::SetParameter(const CosemByteBuffer& value)
{
  parameter_ = value;
}

bool ActionSpecification::IsDummy() const
{
  return serviceId_ == ScriptServiceId::Dummy
      && classId_ == 0u
      && IsZeroLogicalName(logicalName_)
      && index_ == 0
      && parameter_.empty();
}

bool ActionSpecification::IsValid(const ActionSpecification& entry)
{
  switch (entry.serviceId_) {
    case ScriptServiceId::Dummy:
      return entry.classId_ == 0u
          && IsZeroLogicalName(entry.logicalName_)
          && entry.index_ == 0
          && entry.parameter_.empty();
    case ScriptServiceId::WriteAttribute:
    case ScriptServiceId::ExecuteMethod:
      return entry.index_ >= 1;
    default:
      return false;
  }
}

bool ActionSpecification::operator==(const ActionSpecification& other) const
{
  return serviceId_ == other.serviceId_
      && classId_ == other.classId_
      && logicalName_ == other.logicalName_
      && index_ == other.index_
      && parameter_ == other.parameter_;
}

bool ActionSpecification::operator!=(const ActionSpecification& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
