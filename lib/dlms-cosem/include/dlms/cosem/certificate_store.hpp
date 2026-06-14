#ifndef DLMS_COSEM_CERTIFICATE_STORE_HPP
#define DLMS_COSEM_CERTIFICATE_STORE_HPP

#include <array>
#include <cstdint>
#include <vector>

#include "dlms/cosem/cosem_status.hpp"

namespace dlms {
namespace cosem {

/// Blue Book DLMS_CERTIFICATE_ENTITY enum values for COSEM Security Setup
/// certificate_info entries.
enum CertificateEntity
{
  CertificateEntity_Server = 0u,
  CertificateEntity_Client = 1u,
  CertificateEntity_CertificationAuthority = 2u,
  CertificateEntity_Other = 3u
};

/// Blue Book DLMS_CERTIFICATE_TYPE enum values for COSEM Security Setup
/// certificate_info entries.
enum CertificateType
{
  CertificateType_DigitalSignature = 0u,
  CertificateType_KeyAgreement = 1u,
  CertificateType_TlsAuthentication = 2u,
  CertificateType_Other = 3u
};

typedef std::array<std::uint8_t, 8u> CertificateSystemTitle;

/// Stored representation of one certificate_info entry plus its raw
/// X.509 bytes for export. The store layer does not parse X.509; callers
/// provide the parsed metadata together with the raw bytes when importing.
struct CertificateInfoEntry
{
  std::uint8_t entity;
  std::uint8_t type;
  CertificateSystemTitle systemTitle;
  std::vector<std::uint8_t> serialNumber;
  std::vector<std::uint8_t> issuer;
  std::vector<std::uint8_t> subject;
  std::vector<std::uint8_t> subjectAltName;
  std::vector<std::uint8_t> rawCertificate;

  CertificateInfoEntry();
};

/// Pluggable certificate store backing Security Setup v1 attribute 6 and
/// methods 6-8. Implementations may reject mutating operations by returning
/// CosemStatus::ObjectError; when no store is attached at all, the COSEM
/// object reports CosemStatus::UnsupportedFeature for those methods.
class ICosemCertificateStore
{
public:
  virtual ~ICosemCertificateStore();

  /// Snapshot of all known certificate entries.
  virtual CosemStatus List(std::vector<CertificateInfoEntry>& entries) const = 0;

  /// Import a certificate with caller-provided metadata + raw bytes.
  virtual CosemStatus Import(const CertificateInfoEntry& entry) = 0;

  /// Look up a certificate by (entity, type, system_title). Returns raw bytes
  /// on success.
  virtual CosemStatus ExportByEntity(
    std::uint8_t entity,
    std::uint8_t type,
    const CertificateSystemTitle& systemTitle,
    std::vector<std::uint8_t>& rawCertificate) const = 0;

  /// Look up a certificate by (serial, issuer). Returns raw bytes on success.
  virtual CosemStatus ExportBySerial(
    const std::vector<std::uint8_t>& serialNumber,
    const std::vector<std::uint8_t>& issuer,
    std::vector<std::uint8_t>& rawCertificate) const = 0;

  /// Remove a certificate by (entity, type, system_title).
  virtual CosemStatus RemoveByEntity(
    std::uint8_t entity,
    std::uint8_t type,
    const CertificateSystemTitle& systemTitle) = 0;

  /// Remove a certificate by (serial, issuer).
  virtual CosemStatus RemoveBySerial(
    const std::vector<std::uint8_t>& serialNumber,
    const std::vector<std::uint8_t>& issuer) = 0;
};

/// Default in-memory ICosemCertificateStore used by examples and tests.
/// Holds entries as plain records; does not parse or validate X.509.
class InMemoryCosemCertificateStore : public ICosemCertificateStore
{
public:
  InMemoryCosemCertificateStore();

  CosemStatus List(std::vector<CertificateInfoEntry>& entries) const;
  CosemStatus Import(const CertificateInfoEntry& entry);
  CosemStatus ExportByEntity(
    std::uint8_t entity,
    std::uint8_t type,
    const CertificateSystemTitle& systemTitle,
    std::vector<std::uint8_t>& rawCertificate) const;
  CosemStatus ExportBySerial(
    const std::vector<std::uint8_t>& serialNumber,
    const std::vector<std::uint8_t>& issuer,
    std::vector<std::uint8_t>& rawCertificate) const;
  CosemStatus RemoveByEntity(
    std::uint8_t entity,
    std::uint8_t type,
    const CertificateSystemTitle& systemTitle);
  CosemStatus RemoveBySerial(
    const std::vector<std::uint8_t>& serialNumber,
    const std::vector<std::uint8_t>& issuer);

  std::size_t Size() const;
  void Clear();

private:
  std::vector<CertificateInfoEntry> entries_;
};

} // namespace cosem
} // namespace dlms

#endif // DLMS_COSEM_CERTIFICATE_STORE_HPP
