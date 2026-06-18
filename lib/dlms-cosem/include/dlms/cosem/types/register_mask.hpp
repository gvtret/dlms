// SPDX-License-Identifier: BSD-2-Clause
//
// `dlms::cosem::types::RegisterMask` — typed view of one
// COSEM `mask` entry used by IC 6 Register Activation `mask_list`.
//
//   register_mask_element ::= structure {
//     mask_name  octet-string,
//     index_list array of long-unsigned
//   }
//
// `index_list` items are 1-based indices into `register_assignment`.
// Per IEC 62056-6-2 ED4 (2021) §4.3.5 and DLMS UA Blue Book Ed. 12.1
// §4.3.5.
#pragma once

#include "dlms/cosem/cosem_types.hpp"

#include <cstdint>
#include <vector>

namespace dlms::cosem::types {

class RegisterMask
{
public:
  RegisterMask() = default;
  RegisterMask(CosemByteBuffer maskName, std::vector<std::uint16_t> indexList)
    : maskName_(std::move(maskName)), indexList_(std::move(indexList))
  {
  }

  const CosemByteBuffer& MaskName() const { return maskName_; }
  const std::vector<std::uint16_t>& IndexList() const { return indexList_; }

  void SetMaskName(CosemByteBuffer maskName) { maskName_ = std::move(maskName); }
  void SetIndexList(std::vector<std::uint16_t> indexList)
  {
    indexList_ = std::move(indexList);
  }

  bool operator==(const RegisterMask& other) const
  {
    return maskName_ == other.maskName_ && indexList_ == other.indexList_;
  }
  bool operator!=(const RegisterMask& other) const { return !(*this == other); }

private:
  CosemByteBuffer maskName_;
  std::vector<std::uint16_t> indexList_;
};

}  // namespace dlms::cosem::types
