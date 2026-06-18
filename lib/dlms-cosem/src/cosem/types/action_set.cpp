#include "dlms/cosem/types/action_set.hpp"

namespace dlms {
namespace cosem {
namespace types {

ActionSet::ActionSet() = default;

ActionSet::ActionSet(const Script& actionUp, const Script& actionDown)
  : actionUp_(actionUp)
  , actionDown_(actionDown)
{
}

const Script& ActionSet::ActionUp() const { return actionUp_; }
const Script& ActionSet::ActionDown() const { return actionDown_; }

void ActionSet::SetActionUp(const Script& value) { actionUp_ = value; }
void ActionSet::SetActionDown(const Script& value) { actionDown_ = value; }

bool ActionSet::operator==(const ActionSet& other) const
{
  return actionUp_ == other.actionUp_ && actionDown_ == other.actionDown_;
}

bool ActionSet::operator!=(const ActionSet& other) const { return !(*this == other); }

} // namespace types
} // namespace cosem
} // namespace dlms
