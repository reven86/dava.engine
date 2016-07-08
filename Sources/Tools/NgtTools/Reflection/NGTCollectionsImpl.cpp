#include "NGTCollectionsImpl.h"

#include "ReflectionBridge.h"
#include "VariantConverter.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Base/Introspection.h"
#include "Base/IntrospectionCollection.h"
#include "Base/IntrospectionDynamic.h"
#include "Base/Meta.h"
#include "FileSystem/KeyedArchive.h"

#include "Debug/DVAssert.h"

#include <core_reflection/i_definition_manager.hpp>

namespace NGTLayer
{
wgt::TypeId GetItemKeyTypeId(const DAVA::InspColl* collection)
{
    const DAVA::MetaInfo* itemType = collection->ItemKeyType();
    if (itemType == nullptr)
        return wgt::getClassIdentifier<int>();
    else
        return itemType->GetTypeName();
}

class NGTCollection::Iterator : public wgt::CollectionIteratorImplBase
{
public:
    static const DAVA::uint32 END_ITERATOR_POSITION = static_cast<DAVA::uint32>(-1);

    Iterator(void* object_, const DAVA::InspColl* collection_, DAVA::uint32 linearKey_)
        : object(object_)
        , collection(collection_)
        , linearKey(linearKey_)
    {
        keyTypeId = GetItemKeyTypeId(collection);
        valueTypeId = collection->ItemType()->GetTypeName();

        if (linearKey != END_ITERATOR_POSITION)
        {
            iterator = collection->Begin(object);
            DAVA::uint32 counter = linearKey;
            while (counter > 0 && iterator != nullptr)
            {
                iterator = collection->Next(iterator);
                --counter;
            }
        }

#ifdef __DAVAENGINE_DEBUG__
        dbg_name = collection->Name().c_str();
#endif
    }

    ~Iterator()
    {
        collection->Finish(iterator);
    }

    wgt::Variant key() const override
    {
        const void* key = collection->ItemKeyData(iterator);
        if (key == nullptr)
        {
            return wgt::Variant(linearKey);
        }

        DVASSERT(collection->ItemKeyType() != nullptr);
        return VariantConverter::Convert(DAVA::VariantType::LoadData(key, collection->ItemKeyType()));
    }

    wgt::Variant value() const override
    {
        const DAVA::MetaInfo* valueTypeInfo = collection->ItemType();
        DVASSERT(valueTypeInfo != nullptr);
        if (collection->ItemType()->GetIntrospection() != nullptr)
        {
            void* itemData = collection->ItemData(iterator);
            const DAVA::InspInfo* itemInsp = valueTypeInfo->GetIntrospection(itemData);

            if (itemData != nullptr && itemInsp != nullptr)
            {
                wgt::IDefinitionManager* defMng = queryInterface<wgt::IDefinitionManager>();
                DVASSERT(defMng != nullptr);
                return CreateObjectHandle(*defMng, itemInsp, itemData);
            }
        }

        void* valuePointer = collection->ItemPointer(iterator);
        DVASSERT(valuePointer != nullptr);

        return VariantConverter::Convert(DAVA::VariantType::LoadData(valuePointer, valueTypeInfo));
    }

    bool setValue(const wgt::Variant& v) const override
    {
        void* valuePointer = collection->ItemPointer(iterator);
        DVASSERT(valuePointer != nullptr);
        DVASSERT(collection->ItemType() != nullptr);

        DVASSERT((collection->Flags() & DAVA::I_EDIT) != 0)
        DAVA::VariantType value = VariantConverter::Convert(v, collection->ItemType());

        if (value.GetType() == DAVA::VariantType::TYPE_NONE)
            return false;

        DAVA::VariantType::SaveData(valuePointer, collection->ItemType(), value);

        return true;
    }

    void inc() override
    {
        iterator = collection->Next(iterator);
        ++linearKey;
    }

    bool equals(const CollectionIteratorImplBase& that) const override
    {
        DVASSERT(dynamic_cast<const Iterator*>(&that) != nullptr);
        const Iterator& thatIter = static_cast<const Iterator&>(that);
        return object == thatIter.object &&
        collection == thatIter.collection &&
        iterator == thatIter.iterator &&
        linearKey == thatIter.linearKey;
    }

    wgt::CollectionIteratorImplPtr clone() const override
    {
        return wgt::CollectionIteratorImplPtr(new Iterator(object, collection, linearKey));
    }

    bool isValid() const
    {
        return iterator != nullptr;
    }

    const wgt::TypeId& keyType() const override
    {
        return keyTypeId;
    }

    const wgt::TypeId& valueType() const override
    {
        return valueTypeId;
    }

private:
    void* object;
    const DAVA::InspColl* collection;
    DAVA::InspColl::Iterator iterator = nullptr;
    DAVA::uint32 linearKey = 0;
#ifdef __DAVAENGINE_DEBUG__
    DAVA::String dbg_name;
#endif

    wgt::TypeId keyTypeId;
    wgt::TypeId valueTypeId;
};

NGTCollection::NGTCollection(void* object_, const DAVA::InspColl* collectionImpl_)
    : object(object_)
    , collectionImpl(collectionImpl_)
    , keyId("", 0)
    , valueId(collectionImpl->ItemType()->GetTypeName())
    , containerId(collectionImpl->Type()->GetTypeName())
{
    const DAVA::MetaInfo* itemType = collectionImpl->ItemKeyType();
    if (itemType == nullptr)
        keyId = wgt::getClassIdentifier<int>();
    else
        keyId = itemType->GetTypeName();
}

bool NGTCollection::empty() const
{
    return size() == 0;
}

size_t NGTCollection::size() const
{
    return collectionImpl->Size(object);
}

wgt::CollectionIteratorImplPtr NGTCollection::begin()
{
    return wgt::CollectionIteratorImplPtr(new Iterator(object, collectionImpl, 0));
}

wgt::CollectionIteratorImplPtr NGTCollection::end()
{
    return wgt::CollectionIteratorImplPtr(new Iterator(object, collectionImpl, Iterator::END_ITERATOR_POSITION));
}

std::pair<wgt::CollectionIteratorImplPtr, bool> NGTCollection::get(const wgt::Variant& key, GetPolicy policy)
{
    DVASSERT_MSG(policy != CollectionImplBase::GET_NEW &&
                 policy != CollectionImplBase::GET_AUTO,
                 "GET_NEW and GET_AUTO policy does't implemented");

    using TRet = std::pair<wgt::CollectionIteratorImplPtr, bool>;

    Iterator iter(object, collectionImpl, 0);
    while (iter.isValid())
    {
        if (iter.key() == key)
            break;

        iter.inc();
    }

    if (iter.isValid())
        return TRet(iter.clone(), true);

    return TRet(nullptr, false);
}

wgt::CollectionIteratorImplPtr NGTCollection::erase(const wgt::CollectionIteratorImplPtr& pos)
{
    DVASSERT_MSG(false, "Not implemented operation erase");
    return nullptr;
}

const wgt::TypeId& NGTCollection::keyType() const
{
    return keyId;
}

const wgt::TypeId& NGTCollection::valueType() const
{
    return valueId;
}

wgt::CollectionIteratorImplPtr NGTCollection::erase(const wgt::CollectionIteratorImplPtr& first, const wgt::CollectionIteratorImplPtr& last)
{
    DVASSERT_MSG(false, "Not implemented operation erase");
    return nullptr;
}

size_t NGTCollection::erase(const wgt::Variant& key)
{
    DVASSERT_MSG(false, "Not implemented operation erase");
    return 0;
}

const wgt::TypeId& NGTCollection::containerType() const
{
    return containerId;
}

const void* NGTCollection::container() const
{
    return nullptr;
}

int NGTCollection::flags() const
{
    return NON_UNIQUE_KEYS | WRITABLE;
}

//////////////////////////////////////////////////////////////////////////////

class NGTKeyedArchiveImpl::Iterator : public wgt::CollectionIteratorImplBase
{
public:
    static const DAVA::String END_KEY_VALUE;
    Iterator(DAVA::KeyedArchive* archive_, const DAVA::String& key_)
        : archive(archive_)
        , itemKey(key_)
    {
        keyTypeId = wgt::TypeId(DAVA::MetaInfo::Instance<typename DAVA::KeyedArchive::UnderlyingMap::key_type>()->GetTypeName());
        if (itemKey == END_KEY_VALUE)
        {
            valueTypeId = wgt::getClassIdentifier<void>();
        }
        else
        {
            valueTypeId = archive->GetVariant(itemKey)->Meta()->GetTypeName();
        }
    }

    const wgt::TypeId& keyType() const override
    {
        return keyTypeId;
    }

    const wgt::TypeId& valueType() const override
    {
        return valueTypeId;
    }

    wgt::Variant key() const override
    {
        return wgt::Variant(itemKey);
    }

    wgt::Variant value() const override
    {
        DVASSERT(itemKey != END_KEY_VALUE);
        DVASSERT(archive->IsKeyExists(itemKey));
        DAVA::VariantType* value = archive->GetVariant(itemKey);
        if (value->GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
        {
            wgt::IDefinitionManager* defMng = queryInterface<wgt::IDefinitionManager>();
            DVASSERT(defMng != nullptr);
            return CreateObjectHandle(*defMng, DAVA::GetIntrospection<DAVA::KeyedArchive>(), value->AsKeyedArchive());
        }
        return VariantConverter::Convert(*archive->GetVariant(itemKey));
    }

    bool setValue(const wgt::Variant& v) const override
    {
        DVASSERT(itemKey != END_KEY_VALUE);
        DAVA::VariantType* oldValue = archive->GetVariant(itemKey);
        archive->SetVariant(itemKey, VariantConverter::Convert(v, oldValue->Meta()));
        return true;
    }

    void inc() override
    {
        DVASSERT(itemKey != END_KEY_VALUE);
        const DAVA::KeyedArchive::UnderlyingMap& data = archive->GetArchieveData();
        DAVA::KeyedArchive::UnderlyingMap::const_iterator iter = data.find(itemKey);
        DVASSERT(iter != data.end());
        iter++;
        if (iter != data.end())
        {
            itemKey = iter->first;
        }
        else
        {
            itemKey = END_KEY_VALUE;
        }
    }

    bool equals(const CollectionIteratorImplBase& that) const override
    {
        const Iterator& other = dynamic_cast<const Iterator&>(that);
        return archive == other.archive &&
        itemKey == other.itemKey;
    }

    wgt::CollectionIteratorImplPtr clone() const override
    {
        return std::make_shared<Iterator>(archive, itemKey);
    }

    bool isValid() const
    {
        return archive->IsKeyExists(itemKey) && itemKey != END_KEY_VALUE;
    }

private:
    friend class NGTKeyedArchiveImpl;

    DAVA::KeyedArchive* archive;
    DAVA::String itemKey;
    wgt::TypeId keyTypeId;
    wgt::TypeId valueTypeId;
};

const DAVA::String NGTKeyedArchiveImpl::Iterator::END_KEY_VALUE = "END_KEY_VALUE";

NGTKeyedArchiveImpl::NGTKeyedArchiveImpl(DAVA::KeyedArchive* keyedArchive)
    : archive(keyedArchive)
{
    DVASSERT(archive != nullptr);
    containerTypeId = wgt::TypeId(DAVA::MetaInfo::Instance<DAVA::KeyedArchive>()->GetTypeName());
    keyTypeId = wgt::TypeId(DAVA::MetaInfo::Instance<typename DAVA::KeyedArchive::UnderlyingMap::key_type>()->GetTypeName());
    valueTypeId = wgt::getClassIdentifier<DAVA::VariantType>();
}

bool NGTKeyedArchiveImpl::empty() const
{
    return archive->GetArchieveData().empty();
}

size_t NGTKeyedArchiveImpl::size() const
{
    return archive->GetArchieveData().size();
}

wgt::CollectionIteratorImplPtr NGTKeyedArchiveImpl::begin()
{
    if (empty())
        return end();

    return std::make_shared<Iterator>(archive, archive->GetArchieveData().begin()->first);
}

wgt::CollectionIteratorImplPtr NGTKeyedArchiveImpl::end()
{
    return std::make_shared<Iterator>(archive, Iterator::END_KEY_VALUE);
}

std::pair<wgt::CollectionIteratorImplPtr, bool> NGTKeyedArchiveImpl::get(const wgt::Variant& key, GetPolicy policy)
{
    wgt::CollectionIteratorImplPtr resultIter;
    bool isSuccess = false;

    DAVA::String itemKey;
    if (key.tryCast(itemKey))
    {
        switch (policy)
        {
        case CollectionImplBase::GET_EXISTING:
            if (archive->IsKeyExists(itemKey))
            {
                resultIter.reset(new Iterator(archive, itemKey));
                isSuccess = true;
            }
            break;
        case CollectionImplBase::GET_NEW:
            break;
        case CollectionImplBase::GET_AUTO:
            break;
        default:
            break;
        }
    }

    return std::make_pair(resultIter, isSuccess);
}

wgt::CollectionIteratorImplPtr NGTKeyedArchiveImpl::erase(const wgt::CollectionIteratorImplPtr& pos)
{
    Iterator* iter = dynamic_cast<Iterator*>(pos.get());
    DVASSERT(iter != nullptr);
    DVASSERT(iter->isValid());

    wgt::CollectionIteratorImplPtr result = iter->clone();
    result->inc();
    archive->DeleteKey(iter->itemKey);
    return result;
}

size_t NGTKeyedArchiveImpl::erase(const wgt::Variant& key)
{
    DAVA::String keyValue;
    DVVERIFY(key.tryCast(keyValue));

    if (!archive->IsKeyExists(keyValue))
        return 0;

    archive->DeleteKey(keyValue);
    return 1;
}

wgt::CollectionIteratorImplPtr NGTKeyedArchiveImpl::erase(const wgt::CollectionIteratorImplPtr& first, const wgt::CollectionIteratorImplPtr& last)
{
    Iterator* begIter = dynamic_cast<Iterator*>(first.get());
    DVASSERT(begIter != nullptr);

    Iterator* endIter = dynamic_cast<Iterator*>(last.get());
    DVASSERT(endIter != nullptr);

    Iterator eraseIter = *begIter;

    wgt::CollectionIteratorImplPtr result = endIter->clone();
    if (endIter->isValid())
        result->inc();

    while (!eraseIter.equals(*last))
    {
        DAVA::String itemKey = eraseIter.itemKey;
        eraseIter.inc();
        archive->DeleteKey(itemKey);
    }

    return result;
}

const wgt::TypeId& NGTKeyedArchiveImpl::keyType() const
{
    return keyTypeId;
}

const wgt::TypeId& NGTKeyedArchiveImpl::valueType() const
{
    return valueTypeId;
}

const wgt::TypeId& NGTKeyedArchiveImpl::containerType() const
{
    return containerTypeId;
}

const void* NGTKeyedArchiveImpl::container() const
{
    return nullptr;
}

int NGTKeyedArchiveImpl::flags() const
{
    return WRITABLE | MAPPING | RESIZABLE | ORDERED;
}

} // namesapce NGTLayer