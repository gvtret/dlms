#include "dlms/security/ciphered_apdu_processor.hpp"
#include "dlms/security/in_memory_invocation_counter_store.hpp"
#include "dlms/security/in_memory_key_store.hpp"

#include <gtest/gtest.h>

namespace {

dlms::security::SecurityByteView ViewOf(
  const std::vector<std::uint8_t>& data)
{
  dlms::security::SecurityByteView view;
  view.data = data.empty() ? 0 : &data[0];
  view.size = data.size();
  return view;
}

} // namespace

TEST(SecurityIntegration, NoSecurityPolicyPassesOpaqueApduThroughRootBuild)
{
  dlms::security::SecurityContext context =
    dlms::security::EmptySecurityContext();
  context.clientSap = 16u;
  context.serverSap = 1u;

  dlms::security::InMemoryKeyStore keys;
  dlms::security::InMemoryInvocationCounterStore counters;
  dlms::security::CipheredApduProcessor processor(context, keys, counters);

  const std::vector<std::uint8_t> plainApdu =
    {0xc0u, 0x01u, 0xc1u, 0x00u, 0x01u};

  std::vector<std::uint8_t> protectedApdu;
  ASSERT_EQ(
    dlms::security::SecurityStatus::Ok,
    processor.Protect(ViewOf(plainApdu), protectedApdu));
  EXPECT_EQ(plainApdu, protectedApdu);

  std::vector<std::uint8_t> unprotectedApdu;
  ASSERT_EQ(
    dlms::security::SecurityStatus::Ok,
    processor.Unprotect(ViewOf(protectedApdu), unprotectedApdu));
  EXPECT_EQ(plainApdu, unprotectedApdu);
}
