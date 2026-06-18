#include "dlms/cosem/types/script.hpp"

namespace dlms {
namespace cosem {
namespace types {

Script::Script()
  : logicalName_()
  , selector_(0u)
{
}

Script::Script(const CosemLogicalName& logicalName, std::uint16_t selector)
  : logicalName_(logicalName)
  , selector_(selector)
{
}

const CosemLogicalName& Script::LogicalName() const { return logicalName_; }
std::uint16_t Script::Selector() const { return selector_; }

void Script::SetLogicalName(const CosemLogicalName& value)
{
  logicalName_ = value;
}

void Script::SetSelector(std::uint16_t value)
{
  selector_ = value;
}

bool Script::operator==(const Script& other) const
{
  return logicalName_ == other.logicalName_ && selector_ == other.selector_;
}

bool Script::operator!=(const Script& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
