#include "dlms/xdlms/xdlms_correlation.hpp"

#include <gtest/gtest.h>

#include <cstdint>

namespace {

using dlms::xdlms::MakeConversationId;
using dlms::xdlms::kNoConversationId;

TEST(MakeConversationId, LowNibbleEqualsInvokeId)
{
  for (std::uint8_t invokeId = 0; invokeId < 16; ++invokeId) {
    const std::uint64_t id = MakeConversationId(0xDEADBEEFCAFEBAB0ULL, invokeId);
    EXPECT_EQ(id & 0x0FULL, static_cast<std::uint64_t>(invokeId));
  }
}

TEST(MakeConversationId, HighBitsEqualSeedHighBits)
{
  const std::uint64_t seed = 0x0123456789ABCDEFULL;
  for (std::uint8_t invokeId = 0; invokeId < 16; ++invokeId) {
    const std::uint64_t id = MakeConversationId(seed, invokeId);
    EXPECT_EQ(id & ~0x0FULL, seed & ~0x0FULL);
  }
}

TEST(MakeConversationId, InvokeIdHighNibbleIgnored)
{
  // The xDLMS invoke-id is 4 bits; the upper nibble of the
  // invoke-id-and-priority byte is unrelated. Make sure we never
  // bleed it into the conversation id.
  const std::uint64_t seed = 0xFFFFFFFFFFFFFFF0ULL;
  EXPECT_EQ(MakeConversationId(seed, 0x05), MakeConversationId(seed, 0xF5));
  EXPECT_EQ(MakeConversationId(seed, 0x05) & 0x0FULL, 0x05ULL);
}

TEST(MakeConversationId, DistinctSeedsGiveDistinctIds)
{
  // Two associations with the same invoke-id must produce distinct
  // conversation ids — that is the whole point of mixing in the seed.
  EXPECT_NE(
    MakeConversationId(0x1111111111111110ULL, 7),
    MakeConversationId(0x2222222222222220ULL, 7));
}

TEST(MakeConversationId, DistinctInvokeIdsGiveDistinctIds)
{
  const std::uint64_t seed = 0xA5A5A5A5A5A5A5A0ULL;
  for (std::uint8_t a = 0; a < 16; ++a) {
    for (std::uint8_t b = static_cast<std::uint8_t>(a + 1); b < 16; ++b) {
      EXPECT_NE(MakeConversationId(seed, a), MakeConversationId(seed, b))
        << "invokeIds " << static_cast<int>(a) << " vs " << static_cast<int>(b);
    }
  }
}

TEST(MakeConversationId, SeedLowNibbleIsClearedNotMixed)
{
  // If the caller hands us a seed whose low nibble is non-zero, it must
  // be discarded — otherwise the invoke-id round-trip property breaks.
  const std::uint64_t dirty = 0xCAFEBABEDEADBEEFULL;
  const std::uint64_t id = MakeConversationId(dirty, 0x03);
  EXPECT_EQ(id & 0x0FULL, 0x03ULL);
  EXPECT_EQ(id & ~0x0FULL, dirty & ~0x0FULL);
}

TEST(MakeConversationId, ConstexprAndNoexcept)
{
  // Compile-time evaluable: pin the formula at constexpr.
  constexpr std::uint64_t kId = MakeConversationId(0x1234567890ABCDE0ULL, 0x07);
  static_assert(kId == ((0x1234567890ABCDE0ULL & ~0x0FULL) | 0x07ULL),
                "MakeConversationId formula drifted");
  static_assert(noexcept(MakeConversationId(0, 0)),
                "MakeConversationId must be noexcept");
  EXPECT_EQ(kId, (0x1234567890ABCDE0ULL & ~0x0FULL) | 0x07ULL);
}

TEST(MakeConversationId, NoConversationIdSentinelIsZero)
{
  static_assert(kNoConversationId == 0, "sentinel must be zero");
  EXPECT_EQ(kNoConversationId, 0ULL);
}

TEST(MakeConversationId, SeedZeroInvokeIdZeroEqualsSentinel)
{
  // Explicit: an event tagged with seed=0/invoke=0 is indistinguishable
  // from "no correlation context". Document that — consumers must use a
  // non-zero seed if they want every event to carry a real id.
  EXPECT_EQ(MakeConversationId(0, 0), kNoConversationId);
}

} // namespace
