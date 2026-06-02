#pragma once

#include "dlms/association/association_server_interface.hpp"
#include "dlms/association/association_types.hpp"
#include "dlms/profile/apdu_channel.hpp"

#include <vector>

namespace dlms {
namespace association {

class AssociationServer : public IAssociationServer
{
public:
  AssociationServer(
    dlms::profile::IApduChannel& channel,
    const AssociationServerOptions& options);

  AssociationStatus Open() override;
  AssociationStatus Close() override;
  AssociationStatus Accept() override;
  AssociationStatus Release() override;
  AssociationStatus Release(const std::vector<std::uint8_t>& rlrq) override;

  AssociationState State() const override;
  bool IsAssociated() const override;
  const AssociationResult& Result() const override;

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
