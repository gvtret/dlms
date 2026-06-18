#include "dlms/cosem/types/monitored_value.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::int8_t MonitoredValue::AttributeIndexMin;
const std::int8_t MonitoredValue::AttributeIndexMax;

MonitoredValue::MonitoredValue()
  : classId_(0u)
  , logicalName_()
  , attributeIndex_(AttributeIndexMin)
{
}

MonitoredValue::MonitoredValue(
  std::uint16_t classId,
  const CosemLogicalName& logicalName,
  std::int8_t attributeIndex)
  : classId_(classId)
  , logicalName_(logicalName)
  , attributeIndex_(
      attributeIndex >= AttributeIndexMin ? attributeIndex
                                          : AttributeIndexMin)
{
}

std::uint16_t MonitoredValue::ClassId() const { return classId_; }
const CosemLogicalName& MonitoredValue::LogicalName() const { return logicalName_; }
std::int8_t MonitoredValue::AttributeIndex() const { return attributeIndex_; }

void MonitoredValue::SetClassId(std::uint16_t value) { classId_ = value; }
void MonitoredValue::SetLogicalName(const CosemLogicalName& value) { logicalName_ = value; }

bool MonitoredValue::SetAttributeIndex(std::int8_t value)
{
  if (value < AttributeIndexMin) {
    return false;
  }
  attributeIndex_ = value;
  return true;
}

bool MonitoredValue::IsValid(const MonitoredValue& value)
{
  return value.attributeIndex_ >= AttributeIndexMin;
}

bool MonitoredValue::operator==(const MonitoredValue& other) const
{
  return classId_ == other.classId_
      && logicalName_ == other.logicalName_
      && attributeIndex_ == other.attributeIndex_;
}

bool MonitoredValue::operator!=(const MonitoredValue& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
