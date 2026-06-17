#include "dlms/security/suite0_key_wrap.hpp"

#include <algorithm>
#include <cstring>

#include <openssl/evp.h>

namespace dlms {
namespace security {
namespace {

const std::uint8_t kDefaultIntegrityValue[8] = {
  0xA6u, 0xA6u, 0xA6u, 0xA6u, 0xA6u, 0xA6u, 0xA6u, 0xA6u};

class EvpCipherContext
{
public:
  EvpCipherContext()
    : context_(EVP_CIPHER_CTX_new())
  {
  }

  ~EvpCipherContext()
  {
    if (context_ != 0) {
      EVP_CIPHER_CTX_free(context_);
    }
  }

  bool IsValid() const
  {
    return context_ != 0;
  }

  EVP_CIPHER_CTX* Get() const
  {
    return context_;
  }

private:
  EvpCipherContext(const EvpCipherContext&);
  EvpCipherContext& operator=(const EvpCipherContext&);

  EVP_CIPHER_CTX* context_;
};

SecurityStatus ValidateKeyWrapInput(
  const SecurityKey& keyEncryptionKey,
  SecurityByteView input,
  std::size_t expectedSize)
{
  const SecurityStatus keyStatus =
    ValidateSecurityKey(SecuritySuite::Suite0, keyEncryptionKey);
  if (keyStatus != SecurityStatus::Ok) {
    return keyStatus;
  }
  if (!IsValidSecurityByteView(input) || input.size != expectedSize) {
    return SecurityStatus::InvalidArgument;
  }
  return SecurityStatus::Ok;
}

SecurityStatus AesEcbCryptBlock(
  bool encrypt,
  const SecurityKey& key,
  const std::uint8_t input[16],
  std::uint8_t output[16])
{
  EvpCipherContext context;
  if (!context.IsValid()) {
    return encrypt ? SecurityStatus::CipherFailed
                   : SecurityStatus::DecipherFailed;
  }

  const EVP_CIPHER* cipher = EVP_aes_128_ecb();
  const int initStatus = encrypt
    ? EVP_EncryptInit_ex(context.Get(), cipher, 0, key.bytes, 0)
    : EVP_DecryptInit_ex(context.Get(), cipher, 0, key.bytes, 0);
  if (initStatus != 1 ||
      EVP_CIPHER_CTX_set_padding(context.Get(), 0) != 1) {
    return encrypt ? SecurityStatus::CipherFailed
                   : SecurityStatus::DecipherFailed;
  }

  int outputSize = 0;
  const int updateStatus = encrypt
    ? EVP_EncryptUpdate(context.Get(), output, &outputSize, input, 16)
    : EVP_DecryptUpdate(context.Get(), output, &outputSize, input, 16);
  int finalSize = 0;
  const int finalStatus = encrypt
    ? EVP_EncryptFinal_ex(context.Get(), output + outputSize, &finalSize)
    : EVP_DecryptFinal_ex(context.Get(), output + outputSize, &finalSize);

  // Sum as size_t to avoid signed-int overflow UB on the int+int
  // intermediate. EVP_*Update/Final guarantee both are >= 0.
  if (updateStatus != 1 || finalStatus != 1 ||
      (static_cast<std::size_t>(outputSize) +
       static_cast<std::size_t>(finalSize)) != 16u) {
    std::fill(output, output + 16, 0u);
    return encrypt ? SecurityStatus::CipherFailed
                   : SecurityStatus::DecipherFailed;
  }
  return SecurityStatus::Ok;
}

void XorAWithT(std::uint8_t a[8], std::uint32_t t)
{
  for (int i = 7; i >= 0 && t != 0u; --i) {
    a[i] = static_cast<std::uint8_t>(a[i] ^ (t & 0xffu));
    t >>= 8u;
  }
}

} // namespace

SecurityStatus Suite0KeyWrap::Wrap(
  const SecurityKey& keyEncryptionKey,
  SecurityByteView plainKey,
  std::vector<std::uint8_t>& wrappedKey) const
{
  wrappedKey.clear();
  const SecurityStatus validation =
    ValidateKeyWrapInput(keyEncryptionKey, plainKey, kKeySize);
  if (validation != SecurityStatus::Ok) {
    return validation;
  }

  std::uint8_t a[8];
  std::memcpy(a, kDefaultIntegrityValue, sizeof(a));

  std::uint8_t r[2][8];
  std::memcpy(r[0], plainKey.data, 8u);
  std::memcpy(r[1], plainKey.data + 8u, 8u);

  for (std::uint32_t j = 0u; j < 6u; ++j) {
    for (std::uint32_t i = 0u; i < 2u; ++i) {
      std::uint8_t block[16];
      std::memcpy(block, a, 8u);
      std::memcpy(block + 8u, r[i], 8u);

      std::uint8_t encrypted[16];
      const SecurityStatus status =
        AesEcbCryptBlock(true, keyEncryptionKey, block, encrypted);
      if (status != SecurityStatus::Ok) {
        return status;
      }

      std::memcpy(a, encrypted, 8u);
      XorAWithT(a, 2u * j + i + 1u);
      std::memcpy(r[i], encrypted + 8u, 8u);
    }
  }

  wrappedKey.assign(kWrappedKeySize, 0u);
  std::memcpy(&wrappedKey[0], a, 8u);
  std::memcpy(&wrappedKey[8], r[0], 8u);
  std::memcpy(&wrappedKey[16], r[1], 8u);
  return SecurityStatus::Ok;
}

SecurityStatus Suite0KeyWrap::Unwrap(
  const SecurityKey& keyEncryptionKey,
  SecurityByteView wrappedKey,
  std::vector<std::uint8_t>& plainKey) const
{
  plainKey.clear();
  const SecurityStatus validation =
    ValidateKeyWrapInput(keyEncryptionKey, wrappedKey, kWrappedKeySize);
  if (validation != SecurityStatus::Ok) {
    return validation;
  }

  std::uint8_t a[8];
  std::memcpy(a, wrappedKey.data, 8u);

  std::uint8_t r[2][8];
  std::memcpy(r[0], wrappedKey.data + 8u, 8u);
  std::memcpy(r[1], wrappedKey.data + 16u, 8u);

  for (std::uint32_t j = 6u; j > 0u; --j) {
    for (std::uint32_t i = 2u; i > 0u; --i) {
      std::uint8_t block[16];
      std::memcpy(block, a, 8u);
      XorAWithT(block, 2u * (j - 1u) + i);
      std::memcpy(block + 8u, r[i - 1u], 8u);

      std::uint8_t decrypted[16];
      const SecurityStatus status =
        AesEcbCryptBlock(false, keyEncryptionKey, block, decrypted);
      if (status != SecurityStatus::Ok) {
        return status;
      }

      std::memcpy(a, decrypted, 8u);
      std::memcpy(r[i - 1u], decrypted + 8u, 8u);
    }
  }

  if (!std::equal(a, a + 8u, kDefaultIntegrityValue)) {
    return SecurityStatus::AuthenticationFailed;
  }

  plainKey.assign(kKeySize, 0u);
  std::memcpy(&plainKey[0], r[0], 8u);
  std::memcpy(&plainKey[8], r[1], 8u);
  return SecurityStatus::Ok;
}

} // namespace security
} // namespace dlms
