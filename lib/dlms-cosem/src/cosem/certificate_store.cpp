#include "dlms/cosem/certificate_store.hpp"

namespace dlms {
namespace cosem {

CertificateInfoEntry::CertificateInfoEntry()
  : entity(CertificateEntity_Other)
  , type(CertificateType_Other)
{
  systemTitle.fill(0u);
}

ICosemCertificateStore::~ICosemCertificateStore()
{
}

InMemoryCosemCertificateStore::InMemoryCosemCertificateStore()
{
}

CosemStatus InMemoryCosemCertificateStore::List(
  std::vector<CertificateInfoEntry>& entries) const
{
  entries = entries_;
  return CosemStatus::Ok;
}

CosemStatus InMemoryCosemCertificateStore::Import(
  const CertificateInfoEntry& entry)
{
  for (std::size_t i = 0u; i < entries_.size(); ++i) {
    if (entries_[i].entity == entry.entity
        && entries_[i].type == entry.type
        && entries_[i].systemTitle == entry.systemTitle) {
      entries_[i] = entry;
      return CosemStatus::Ok;
    }
  }
  entries_.push_back(entry);
  return CosemStatus::Ok;
}

CosemStatus InMemoryCosemCertificateStore::ExportByEntity(
  std::uint8_t entity,
  std::uint8_t type,
  const CertificateSystemTitle& systemTitle,
  std::vector<std::uint8_t>& rawCertificate) const
{
  for (std::size_t i = 0u; i < entries_.size(); ++i) {
    if (entries_[i].entity == entity
        && entries_[i].type == type
        && entries_[i].systemTitle == systemTitle) {
      rawCertificate = entries_[i].rawCertificate;
      return CosemStatus::Ok;
    }
  }
  rawCertificate.clear();
  return CosemStatus::ObjectError;
}

CosemStatus InMemoryCosemCertificateStore::ExportBySerial(
  const std::vector<std::uint8_t>& serialNumber,
  const std::vector<std::uint8_t>& issuer,
  std::vector<std::uint8_t>& rawCertificate) const
{
  for (std::size_t i = 0u; i < entries_.size(); ++i) {
    if (entries_[i].serialNumber == serialNumber
        && entries_[i].issuer == issuer) {
      rawCertificate = entries_[i].rawCertificate;
      return CosemStatus::Ok;
    }
  }
  rawCertificate.clear();
  return CosemStatus::ObjectError;
}

CosemStatus InMemoryCosemCertificateStore::RemoveByEntity(
  std::uint8_t entity,
  std::uint8_t type,
  const CertificateSystemTitle& systemTitle)
{
  for (std::vector<CertificateInfoEntry>::iterator it = entries_.begin();
       it != entries_.end();
       ++it) {
    if (it->entity == entity
        && it->type == type
        && it->systemTitle == systemTitle) {
      entries_.erase(it);
      return CosemStatus::Ok;
    }
  }
  return CosemStatus::ObjectError;
}

CosemStatus InMemoryCosemCertificateStore::RemoveBySerial(
  const std::vector<std::uint8_t>& serialNumber,
  const std::vector<std::uint8_t>& issuer)
{
  for (std::vector<CertificateInfoEntry>::iterator it = entries_.begin();
       it != entries_.end();
       ++it) {
    if (it->serialNumber == serialNumber
        && it->issuer == issuer) {
      entries_.erase(it);
      return CosemStatus::Ok;
    }
  }
  return CosemStatus::ObjectError;
}

std::size_t InMemoryCosemCertificateStore::Size() const
{
  return entries_.size();
}

void InMemoryCosemCertificateStore::Clear()
{
  entries_.clear();
}

} // namespace cosem
} // namespace dlms
