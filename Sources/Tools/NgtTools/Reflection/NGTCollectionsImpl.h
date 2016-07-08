#pragma once

#include <core_variant/collection.hpp>
#include "Base/BaseTypes.h"

namespace DAVA
{
class InspColl;
class KeyedArchive;
struct MetaInfo;
}

namespace NGTLayer
{
class NGTCollection : public wgt::CollectionImplBase
{
    class Iterator;

public:
    NGTCollection(void* object, const DAVA::InspColl* collectionImpl);

    bool empty() const;
    size_t size() const override;

    wgt::CollectionIteratorImplPtr begin() override;
    wgt::CollectionIteratorImplPtr end() override;

    std::pair<wgt::CollectionIteratorImplPtr, bool> get(const wgt::Variant& key, GetPolicy policy) override;

    wgt::CollectionIteratorImplPtr erase(const wgt::CollectionIteratorImplPtr& pos) override;
    wgt::CollectionIteratorImplPtr erase(const wgt::CollectionIteratorImplPtr& first, const wgt::CollectionIteratorImplPtr& last) override;
    size_t erase(const wgt::Variant& key) override;

    const wgt::TypeId& keyType() const override;
    const wgt::TypeId& valueType() const override;
    const wgt::TypeId& containerType() const override;
    const void* container() const override;
    int flags() const override;

private:
    void* object;
    const DAVA::InspColl* collectionImpl;
    wgt::TypeId keyId;
    wgt::TypeId valueId;
    wgt::TypeId containerId;
};

class NGTKeyedArchiveImpl : public wgt::CollectionImplBase
{
    class Iterator;

public:
    NGTKeyedArchiveImpl(DAVA::KeyedArchive* keyedArchive);

    bool empty() const;
    size_t size() const override;

    wgt::CollectionIteratorImplPtr begin() override;
    wgt::CollectionIteratorImplPtr end() override;
    std::pair<wgt::CollectionIteratorImplPtr, bool> get(const wgt::Variant& key, GetPolicy policy) override;
    wgt::CollectionIteratorImplPtr erase(const wgt::CollectionIteratorImplPtr& pos) override;
    size_t erase(const wgt::Variant& key) override;
    wgt::CollectionIteratorImplPtr erase(const wgt::CollectionIteratorImplPtr& first, const wgt::CollectionIteratorImplPtr& last) override;

    const wgt::TypeId& keyType() const override;
    const wgt::TypeId& valueType() const override;
    const wgt::TypeId& containerType() const override;
    const void* container() const override;
    int flags() const override;

private:
    DAVA::KeyedArchive* archive;
    wgt::TypeId keyTypeId;
    wgt::TypeId valueTypeId;
    wgt::TypeId containerTypeId;
};

} // namespace NGTLayer