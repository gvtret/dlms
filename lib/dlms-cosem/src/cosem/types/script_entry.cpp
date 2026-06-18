#include "dlms/cosem/types/script_entry.hpp"

namespace dlms {
namespace cosem {
namespace types {

const std::uint16_t ScriptEntry::NullScriptIdentifier;

ScriptEntry::ScriptEntry()
  : identifier_(NullScriptIdentifier),
    actions_()
{
}

ScriptEntry::ScriptEntry(
  std::uint16_t identifier,
  const std::vector<ActionSpecification>& actions)
  : identifier_(identifier),
    actions_()
{
  // Safe-fallback construction: drop the entire actions vector when
  // any action is malformed rather than holding a half-valid entry.
  bool ok = true;
  for (std::size_t i = 0; i < actions.size(); ++i) {
    if (!ActionSpecification::IsValid(actions[i])) {
      ok = false;
      break;
    }
  }
  if (ok) {
    actions_ = actions;
  }
}

std::uint16_t ScriptEntry::Identifier() const
{
  return identifier_;
}

const std::vector<ActionSpecification>& ScriptEntry::Actions() const
{
  return actions_;
}

void ScriptEntry::SetIdentifier(std::uint16_t value)
{
  identifier_ = value;
}

bool ScriptEntry::SetActions(const std::vector<ActionSpecification>& value)
{
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (!ActionSpecification::IsValid(value[i])) {
      return false;
    }
  }
  actions_ = value;
  return true;
}

bool ScriptEntry::IsValid(const ScriptEntry& entry)
{
  for (std::size_t i = 0; i < entry.actions_.size(); ++i) {
    if (!ActionSpecification::IsValid(entry.actions_[i])) {
      return false;
    }
  }
  return true;
}

bool ScriptEntry::operator==(const ScriptEntry& other) const
{
  return identifier_ == other.identifier_
      && actions_ == other.actions_;
}

bool ScriptEntry::operator!=(const ScriptEntry& other) const
{
  return !(*this == other);
}

} // namespace types
} // namespace cosem
} // namespace dlms
