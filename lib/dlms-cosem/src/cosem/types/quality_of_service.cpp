// SPDX-License-Identifier: BSD-2-Clause
#include "dlms/cosem/types/quality_of_service.hpp"

namespace dlms::cosem::types {

QosElement::QosElement()
  : precedence_(0u)
  , delay_(0u)
  , reliability_(0u)
  , peakThroughput_(0u)
  , meanThroughput_(0u)
{
}

QosElement::QosElement(
  std::uint8_t precedence,
  std::uint8_t delay,
  std::uint8_t reliability,
  std::uint8_t peakThroughput,
  std::uint8_t meanThroughput)
  : precedence_(precedence)
  , delay_(delay)
  , reliability_(reliability)
  , peakThroughput_(peakThroughput)
  , meanThroughput_(meanThroughput)
{
}

QualityOfService::QualityOfService()
  : default_()
  , requested_()
{
}

QualityOfService::QualityOfService(
  const QosElement& dflt, const QosElement& requested)
  : default_(dflt)
  , requested_(requested)
{
}

}  // namespace dlms::cosem::types
