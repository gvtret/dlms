#pragma once

#include "dlms/association/association_types.hpp"
#include "dlms/profile/apdu_channel.hpp"

#include <vector>

namespace dlms {
namespace association {

class AssociationServer
{
public:
  AssociationServer(
    dlms::profile::IApduChannel& channel,
    const AssociationServerOptions& options);

  AssociationStatus Open();
  AssociationStatus Close();
  AssociationStatus Accept();
  AssociationStatus Release();
  AssociationStatus Release(const std::vector<std::uint8_t>& rlrq);

  AssociationState State() const;
  bool IsAssociated() const;
  const AssociationResult& Result() const;

private:
  AssociationServer(const AssociationServer&);
  AssociationServer& operator=(const AssociationServer&);

  AssociationStatus ValidateOptions() const;
  AssociationStatus DecodeAarq(const std::vector<std::uint8_t>& input);
  AssociationStatus BuildAare(std::vector<std::uint8_t>& output) const;
  AssociationStatus DecodeRlrq(const std::vector<std::uint8_t>& input) const;
  AssociationStatus BuildRlre(std::vector<std::uint8_t>& output) const;
  AssociationStatus SendRlreAndClose();

  dlms::profile::IApduChannel& channel_;
  AssociationServerOptions options_;
  AssociationState state_;
  AssociationResult result_;
  HighLevelSecurityMechanism highLevelSecurityMechanism_;
};

} // namespace association
} // namespace dlms
