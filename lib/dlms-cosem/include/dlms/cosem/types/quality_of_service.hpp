// SPDX-License-Identifier: BSD-2-Clause
//
// `types::QualityOfService` — typed representation of the
// `quality_of_service` structure carried by IC "GPRS modem setup"
// (class_id=45), per IEC 62056-6-2 ED4 (2021) §4.7.7.2.4 and DLMS UA
// Blue Book Ed. 12.1 §4.7.7.2.4.
//
//   quality_of_service ::= structure
//   {
//     default:   qos_element,
//     requested: qos_element
//   }
//   qos_element ::= structure
//   {
//     precedence:      unsigned,
//     delay:           unsigned,
//     reliability:     unsigned,
//     peak_throughput: unsigned,
//     mean_throughput: unsigned
//   }
//
// The AXDR codec is provided by the consuming IC; this header carries
// only the typed POD plus simple invariants.
#pragma once

#include <cstdint>

namespace dlms::cosem::types {

class QosElement
{
public:
  QosElement();
  QosElement(
    std::uint8_t precedence,
    std::uint8_t delay,
    std::uint8_t reliability,
    std::uint8_t peakThroughput,
    std::uint8_t meanThroughput);

  std::uint8_t Precedence() const { return precedence_; }
  std::uint8_t Delay() const { return delay_; }
  std::uint8_t Reliability() const { return reliability_; }
  std::uint8_t PeakThroughput() const { return peakThroughput_; }
  std::uint8_t MeanThroughput() const { return meanThroughput_; }

  void SetPrecedence(std::uint8_t v) { precedence_ = v; }
  void SetDelay(std::uint8_t v) { delay_ = v; }
  void SetReliability(std::uint8_t v) { reliability_ = v; }
  void SetPeakThroughput(std::uint8_t v) { peakThroughput_ = v; }
  void SetMeanThroughput(std::uint8_t v) { meanThroughput_ = v; }

  bool operator==(const QosElement& other) const
  {
    return precedence_ == other.precedence_
      && delay_ == other.delay_
      && reliability_ == other.reliability_
      && peakThroughput_ == other.peakThroughput_
      && meanThroughput_ == other.meanThroughput_;
  }
  bool operator!=(const QosElement& other) const { return !(*this == other); }

private:
  std::uint8_t precedence_;
  std::uint8_t delay_;
  std::uint8_t reliability_;
  std::uint8_t peakThroughput_;
  std::uint8_t meanThroughput_;
};

class QualityOfService
{
public:
  QualityOfService();
  QualityOfService(const QosElement& dflt, const QosElement& requested);

  const QosElement& Default() const { return default_; }
  const QosElement& Requested() const { return requested_; }

  void SetDefault(const QosElement& v) { default_ = v; }
  void SetRequested(const QosElement& v) { requested_ = v; }

  bool operator==(const QualityOfService& other) const
  {
    return default_ == other.default_ && requested_ == other.requested_;
  }
  bool operator!=(const QualityOfService& other) const { return !(*this == other); }

private:
  QosElement default_;
  QosElement requested_;
};

}  // namespace dlms::cosem::types
