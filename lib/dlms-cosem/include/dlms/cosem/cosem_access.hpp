#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <map>
#include <cstdint>
#include <vector>

namespace dlms {
namespace cosem {

enum class AttributeAccessMode
{
  NoAccess,
  ReadOnly,
  WriteOnly,
  ReadAndWrite,
  AuthenticatedReadOnly,
  AuthenticatedWriteOnly,
  AuthenticatedReadAndWrite
};

enum class MethodAccessMode
{
  NoAccess,
  Access,
  AuthenticatedAccess
};

struct AttributeAccessEntry
{
  std::uint8_t attributeId;
  AttributeAccessMode mode;
};

struct MethodAccessEntry
{
  std::uint8_t methodId;
  MethodAccessMode mode;
};

struct CosemAccessContext
{
  bool authenticated;
};

class CosemAccessRights
{
public:
  void SetAttributeAccess(
    std::uint8_t attributeId,
    AttributeAccessMode mode);
  void SetMethodAccess(
    std::uint8_t methodId,
    MethodAccessMode mode);

  AttributeAccessMode AttributeAccess(std::uint8_t attributeId) const;
  MethodAccessMode MethodAccess(std::uint8_t methodId) const;
  std::vector<AttributeAccessEntry> AttributeAccessEntries() const;
  std::vector<MethodAccessEntry> MethodAccessEntries() const;

  bool CanReadAttribute(
    std::uint8_t attributeId,
    const CosemAccessContext& context) const;
  bool CanWriteAttribute(
    std::uint8_t attributeId,
    const CosemAccessContext& context) const;
  bool CanInvokeMethod(
    std::uint8_t methodId,
    const CosemAccessContext& context) const;

private:
  std::map<std::uint8_t, AttributeAccessMode> attributeAccess_;
  std::map<std::uint8_t, MethodAccessMode> methodAccess_;
};

CosemAccessContext PublicAccessContext();
CosemAccessContext AuthenticatedAccessContext();

} // namespace cosem
} // namespace dlms
