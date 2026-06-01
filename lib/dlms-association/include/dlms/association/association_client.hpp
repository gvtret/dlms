#pragma once

#include "dlms/association/association_types.hpp"
#include "dlms/profile/apdu_channel.hpp"

#include <vector>

namespace dlms {
namespace association {

class IAssociationClient
{
public:
  virtual ~IAssociationClient()
  {
  }

  virtual AssociationStatus Open() = 0;
  virtual AssociationStatus Close() = 0;
  virtual AssociationStatus Establish() = 0;
  virtual AssociationStatus Release() = 0;

  virtual AssociationState State() const = 0;
  virtual bool IsAssociated() const = 0;
  virtual const AssociationResult& Result() const = 0;
};

class AssociationClient : public IAssociationClient
{
public:
  AssociationClient(
    dlms::profile::IApduChannel& channel,
    const AssociationOptions& options);

  AssociationStatus Open() override;
  AssociationStatus Close() override;
  AssociationStatus Establish() override;
  AssociationStatus Release() override;

  AssociationState State() const override;
  bool IsAssociated() const override;
  const AssociationResult& Result() const override;

private:
  AssociationClient(const AssociationClient&);
  AssociationClient& operator=(const AssociationClient&);

  AssociationStatus BuildAarq(std::vector<std::uint8_t>& output) const;
  AssociationStatus DecodeAare(const std::vector<std::uint8_t>& input);
  AssociationStatus BuildRlrq(std::vector<std::uint8_t>& output) const;
  AssociationStatus DecodeRlre(const std::vector<std::uint8_t>& input) const;
  AssociationStatus ValidateOptions() const;

  dlms::profile::IApduChannel& channel_;
  AssociationOptions options_;
  AssociationState state_;
  AssociationResult result_;
};

} // namespace association
} // namespace dlms
