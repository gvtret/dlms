#include "dlms/cosem/cosem_access.hpp"

namespace dlms {
namespace cosem {

namespace {

bool IsReadAllowed(
  AttributeAccessMode mode,
  const CosemAccessContext& context)
{
  switch (mode) {
  case AttributeAccessMode::ReadOnly:
  case AttributeAccessMode::ReadAndWrite:
    return true;
  case AttributeAccessMode::AuthenticatedReadOnly:
  case AttributeAccessMode::AuthenticatedReadAndWrite:
    return context.authenticated;
  default:
    return false;
  }
}

bool IsWriteAllowed(
  AttributeAccessMode mode,
  const CosemAccessContext& context)
{
  switch (mode) {
  case AttributeAccessMode::WriteOnly:
  case AttributeAccessMode::ReadAndWrite:
    return true;
  case AttributeAccessMode::AuthenticatedWriteOnly:
  case AttributeAccessMode::AuthenticatedReadAndWrite:
    return context.authenticated;
  default:
    return false;
  }
}

bool IsMethodAllowed(
  MethodAccessMode mode,
  const CosemAccessContext& context)
{
  switch (mode) {
  case MethodAccessMode::Access:
    return true;
  case MethodAccessMode::AuthenticatedAccess:
    return context.authenticated;
  default:
    return false;
  }
}

} // namespace

void CosemAccessRights::SetAttributeAccess(
  std::uint8_t attributeId,
  AttributeAccessMode mode)
{
  attributeAccess_[attributeId] = mode;
}

void CosemAccessRights::SetMethodAccess(
  std::uint8_t methodId,
  MethodAccessMode mode)
{
  methodAccess_[methodId] = mode;
}

AttributeAccessMode CosemAccessRights::AttributeAccess(
  std::uint8_t attributeId) const
{
  std::map<std::uint8_t, AttributeAccessMode>::const_iterator it =
    attributeAccess_.find(attributeId);
  return it == attributeAccess_.end()
    ? AttributeAccessMode::NoAccess
    : it->second;
}

MethodAccessMode CosemAccessRights::MethodAccess(
  std::uint8_t methodId) const
{
  std::map<std::uint8_t, MethodAccessMode>::const_iterator it =
    methodAccess_.find(methodId);
  return it == methodAccess_.end()
    ? MethodAccessMode::NoAccess
    : it->second;
}

std::vector<AttributeAccessEntry>
CosemAccessRights::AttributeAccessEntries() const
{
  std::vector<AttributeAccessEntry> entries;
  for (std::map<std::uint8_t, AttributeAccessMode>::const_iterator it =
         attributeAccess_.begin();
       it != attributeAccess_.end();
       ++it) {
    AttributeAccessEntry entry;
    entry.attributeId = it->first;
    entry.mode = it->second;
    entries.push_back(entry);
  }
  return entries;
}

std::vector<MethodAccessEntry>
CosemAccessRights::MethodAccessEntries() const
{
  std::vector<MethodAccessEntry> entries;
  for (std::map<std::uint8_t, MethodAccessMode>::const_iterator it =
         methodAccess_.begin();
       it != methodAccess_.end();
       ++it) {
    MethodAccessEntry entry;
    entry.methodId = it->first;
    entry.mode = it->second;
    entries.push_back(entry);
  }
  return entries;
}

bool CosemAccessRights::CanReadAttribute(
  std::uint8_t attributeId,
  const CosemAccessContext& context) const
{
  return IsReadAllowed(AttributeAccess(attributeId), context);
}

bool CosemAccessRights::CanWriteAttribute(
  std::uint8_t attributeId,
  const CosemAccessContext& context) const
{
  return IsWriteAllowed(AttributeAccess(attributeId), context);
}

bool CosemAccessRights::CanInvokeMethod(
  std::uint8_t methodId,
  const CosemAccessContext& context) const
{
  return IsMethodAllowed(MethodAccess(methodId), context);
}

CosemAccessContext PublicAccessContext()
{
  CosemAccessContext context;
  context.authenticated = false;
  return context;
}

CosemAccessContext AuthenticatedAccessContext()
{
  CosemAccessContext context;
  context.authenticated = true;
  return context;
}

} // namespace cosem
} // namespace dlms
