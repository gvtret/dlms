#include "dlms/cosem/object_registry.hpp"

namespace dlms {
namespace cosem {

ICosemObject::~ICosemObject()
{
}

CosemStatus ObjectRegistry::Register(
  const std::shared_ptr<ICosemObject>& object)
{
  if (!object) {
    return CosemStatus::InvalidArgument;
  }

  const CosemObjectDescriptor descriptor = object->Descriptor();
  const CosemStatus status = ValidateObjectDescriptor(descriptor);
  if (status != CosemStatus::Ok) {
    return status;
  }

  if (objects_.find(descriptor.key) != objects_.end()) {
    return CosemStatus::DuplicateObject;
  }

  objects_[descriptor.key] = object;
  return CosemStatus::Ok;
}

const ICosemObject* ObjectRegistry::Find(
  const CosemObjectKey& key) const
{
  if (ValidateObjectKey(key) != CosemStatus::Ok) {
    return 0;
  }

  std::map<CosemObjectKey, std::shared_ptr<ICosemObject> >::const_iterator it =
    objects_.find(key);
  return it == objects_.end() ? 0 : it->second.get();
}

bool ObjectRegistry::Contains(const CosemObjectKey& key) const
{
  return Find(key) != 0;
}

std::vector<CosemObjectDescriptor> ObjectRegistry::Descriptors() const
{
  std::vector<CosemObjectDescriptor> descriptors;
  for (std::map<CosemObjectKey, std::shared_ptr<ICosemObject> >::const_iterator
       it = objects_.begin();
       it != objects_.end();
       ++it) {
    descriptors.push_back(it->second->Descriptor());
  }
  return descriptors;
}

std::size_t ObjectRegistry::Size() const
{
  return objects_.size();
}

CosemStatus ObjectRegistry::ReadAttribute(
  const CosemAttributeDescriptor& descriptor,
  CosemByteBuffer& output) const
{
  return ReadAttribute(descriptor, PublicAccessContext(), output);
}

CosemStatus ObjectRegistry::ReadAttribute(
  const CosemAttributeDescriptor& descriptor,
  const CosemAccessContext& context,
  CosemByteBuffer& output) const
{
  const CosemStatus status = ValidateAttributeDescriptor(descriptor);
  if (status != CosemStatus::Ok) {
    return status;
  }

  const ICosemObject* object = Find(descriptor.object);
  if (!object) {
    return CosemStatus::ObjectNotFound;
  }

  if (!object->AccessRights().CanReadAttribute(
        descriptor.attributeId, context)) {
    return CosemStatus::AccessDenied;
  }

  CosemByteBuffer candidate;
  const CosemStatus readStatus =
    object->ReadAttribute(descriptor.attributeId, candidate);
  if (readStatus == CosemStatus::Ok) {
    output = candidate;
  }
  return readStatus;
}

CosemStatus ObjectRegistry::WriteAttribute(
  const CosemAttributeDescriptor& descriptor,
  const CosemByteBuffer& input)
{
  return WriteAttribute(descriptor, PublicAccessContext(), input);
}

CosemStatus ObjectRegistry::WriteAttribute(
  const CosemAttributeDescriptor& descriptor,
  const CosemAccessContext& context,
  const CosemByteBuffer& input)
{
  const CosemStatus status = ValidateAttributeDescriptor(descriptor);
  if (status != CosemStatus::Ok) {
    return status;
  }

  std::map<CosemObjectKey, std::shared_ptr<ICosemObject> >::iterator it =
    objects_.find(descriptor.object);
  if (it == objects_.end()) {
    return CosemStatus::ObjectNotFound;
  }

  if (!it->second->AccessRights().CanWriteAttribute(
        descriptor.attributeId, context)) {
    return CosemStatus::AccessDenied;
  }

  return it->second->WriteAttribute(descriptor.attributeId, input);
}

CosemStatus ObjectRegistry::InvokeMethod(
  const CosemMethodDescriptor& descriptor,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  return InvokeMethod(descriptor, PublicAccessContext(), input, output);
}

CosemStatus ObjectRegistry::InvokeMethod(
  const CosemMethodDescriptor& descriptor,
  const CosemAccessContext& context,
  const CosemByteBuffer& input,
  CosemByteBuffer& output)
{
  const CosemStatus status = ValidateMethodDescriptor(descriptor);
  if (status != CosemStatus::Ok) {
    return status;
  }

  std::map<CosemObjectKey, std::shared_ptr<ICosemObject> >::iterator it =
    objects_.find(descriptor.object);
  if (it == objects_.end()) {
    return CosemStatus::ObjectNotFound;
  }

  if (!it->second->AccessRights().CanInvokeMethod(
        descriptor.methodId, context)) {
    return CosemStatus::AccessDenied;
  }

  CosemByteBuffer candidate;
  const CosemStatus invokeStatus =
    it->second->InvokeMethod(descriptor.methodId, input, candidate);
  if (invokeStatus == CosemStatus::Ok) {
    output = candidate;
  }
  return invokeStatus;
}

} // namespace cosem
} // namespace dlms
