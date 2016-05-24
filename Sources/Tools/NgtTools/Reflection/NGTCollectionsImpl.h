#ifndef __QTTOOLS_NGTCOLLECTIONSIMPL_H__
#define __QTTOOLS_NGTCOLLECTIONSIMPL_H__

#include "Base/BaseTypes.h"

#include "core_variant/collection.hpp"

namespace DAVA
{
class InspColl;
class KeyedArchive;
struct MetaInfo;
}

namespace NGTLayer
{
class NGTCollection : public CollectionImplBase
{
    class Iterator;

public:
    NGTCollection(void* object, const DAVA::InspColl* collectionImpl);

    bool empty() const override;
    size_t size() const override;

    CollectionIteratorImplPtr begin() override;
    CollectionIteratorImplPtr end() override;

    std::pair<CollectionIteratorImplPtr, bool> get(const Variant& key, GetPolicy policy) override;

    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& pos) override;
    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& first, const CollectionIteratorImplPtr& last) override;
    size_t erase(const Variant& key) override;

    const TypeId& keyType() const override;
    const TypeId& valueType() const override;

    const TypeId& containerType() const override;
    void* containerData() const override;

    bool isMapping() const override
    {
        return false;
    }
    bool canResize() const override
    {
        return false;
    }

private:
    void* object;
    const DAVA::InspColl* collectionImpl;
    TypeId keyId;
    TypeId valueId;
    TypeId containerId;
};

class NGTKeyedArchiveImpl : public CollectionImplBase
{
    class Iterator;

public:
    NGTKeyedArchiveImpl(DAVA::KeyedArchive* keyedArchive);

    bool empty() const override;
    size_t size() const override;

    CollectionIteratorImplPtr begin() override;
    CollectionIteratorImplPtr end() override;
    std::pair<CollectionIteratorImplPtr, bool> get(const Variant& key, GetPolicy policy) override;
    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& pos) override;
    size_t erase(const Variant& key) override;
    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& first, const CollectionIteratorImplPtr& last) override;

    const TypeId& keyType() const override;
    const TypeId& valueType() const override;
    const TypeId& containerType() const override;
    void* containerData() const override;
    bool isMapping() const override;
    bool canResize() const override;

private:
    DAVA::KeyedArchive* archive;
    TypeId keyTypeId;
    TypeId valueTypeId;
    TypeId containerTypeId;
};

} // namespace NGTLayer

#endif // __QTTOOLS_NGTCOLLECTIONSIMPL_H__