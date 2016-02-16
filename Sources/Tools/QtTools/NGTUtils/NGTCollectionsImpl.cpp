/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "NGTCollectionsImpl.h"

#include "ReflectionBridge.h"
#include "VariantConverter.h"

#include "Base/Introspection.h"
#include "Base/IntrospectionCollection.h"
#include "Base/IntrospectionDynamic.h"
#include "Base/Meta.h"

#include "Debug/DVAssert.h"

#include "core_reflection/i_definition_manager.hpp"

namespace DAVA
{
class NGTCollection::Iterator : public CollectionIteratorImplBase
{
public:
    static const uint32 END_ITERATOR_POSITION = static_cast<uint32>(-1);

    Iterator(const NGTCollection* collection, uint32 linearKey_)
        : collection(collection)
        , linearKey(linearKey_)
    {
        iterator = collection->collectionImpl->Begin(collection->object);
        uint32 counter = linearKey;
        while (counter > 0 && iterator != nullptr)
        {
            iterator = collection->collectionImpl->Next(iterator);
            --counter;
        }
    }

    ~Iterator()
    {
        collection->collectionImpl->Finish(iterator);
    }

    Variant key() const override
    {
        const void* key = collection->collectionImpl->ItemKeyData(iterator);
        if (key == nullptr)
        {
            return Variant(linearKey);
        }

        DVASSERT(collection->collectionImpl->ItemKeyType() != nullptr);
        return VariantConverter::Instance()->Convert(VariantType::LoadData(key, collection->collectionImpl->ItemKeyType()));
    }

    Variant value() const override
    {
        const MetaInfo* valueTypeInfo = collection->collectionImpl->ItemType();
        DVASSERT(valueTypeInfo != nullptr);
        if (collection->collectionImpl->ItemType()->GetIntrospection() != nullptr)
        {
            void* itemData = collection->collectionImpl->ItemData(iterator);
            const InspInfo* itemInsp = valueTypeInfo->GetIntrospection(itemData);

            if (itemData != nullptr && itemInsp != nullptr)
            {
                IDefinitionManager* defMng = Context::queryInterface<IDefinitionManager>();
                DVASSERT(defMng != nullptr);
                return CreateObjectHandle(*defMng, itemInsp, itemData);
            }
        }

        void* valuePointer = collection->collectionImpl->ItemPointer(iterator);
        DVASSERT(valuePointer != nullptr);

        return VariantConverter::Instance()->Convert(VariantType::LoadData(valuePointer, valueTypeInfo));
    }

    bool setValue(const Variant& v) const override
    {
        void* valuePointer = collection->collectionImpl->ItemPointer(iterator);
        DVASSERT(valuePointer != nullptr);
        DVASSERT(collection->collectionImpl->ItemType() != nullptr);

        if ((collection->collectionImpl->Flags() & I_EDIT) == 0)
            return false;

        VariantType value = VariantConverter::Instance()->Convert(v, collection->collectionImpl->ItemType());

        if (value.GetType() == VariantType::TYPE_NONE)
            return false;

        VariantType::SaveData(valuePointer, collection->collectionImpl->ItemType(), value);

        return true;
    }

    void inc() override
    {
        iterator = collection->collectionImpl->Next(iterator);
        ++linearKey;
    }

    bool equals(const CollectionIteratorImplBase& that) const override
    {
        DVASSERT(dynamic_cast<const Iterator*>(&that) != nullptr);
        const Iterator& thatIter = static_cast<const Iterator&>(that);
        return collection->object == thatIter.collection->object &&
        collection->collectionImpl == thatIter.collection->collectionImpl &&
        iterator == thatIter.iterator &&
        linearKey == thatIter.linearKey;
    }

    CollectionIteratorImplPtr clone() const override
    {
        return CollectionIteratorImplPtr(new Iterator(collection, linearKey));
    }

    bool isValid() const
    {
        return iterator != nullptr;
    }

    const TypeId& keyType() const override
    {
        return collection->keyType();
    }

    const TypeId& valueType() const override
    {
        return collection->valueType();
    }

private:
    const NGTCollection* collection;
    InspColl::Iterator iterator = nullptr;
    uint32 linearKey = 0;
};

NGTCollection::NGTCollection(void* object_, const InspColl* collectionImpl_)
    : object(object_)
    , collectionImpl(collectionImpl_)
    , keyId(collectionImpl->ItemKeyType()->GetTypeName())
    , valueId("", 0)
    , containerId(collectionImpl->Type()->GetTypeName())
{
    const MetaInfo* itemType = collectionImpl->ItemType();
    if (itemType == nullptr)
        valueId = getClassIdentifier<int>();
    else
        valueId = itemType->GetTypeName();
}

bool NGTCollection::empty() const
{
    return size() == 0;
}

size_t NGTCollection::size() const
{
    return collectionImpl->Size(object);
}

CollectionIteratorImplPtr NGTCollection::begin()
{
    return CollectionIteratorImplPtr(new Iterator(this, 0));
}

CollectionIteratorImplPtr NGTCollection::end()
{
    return CollectionIteratorImplPtr(new Iterator(this, Iterator::END_ITERATOR_POSITION));
}

std::pair<CollectionIteratorImplPtr, bool> NGTCollection::get(const Variant& key, GetPolicy policy)
{
    DVASSERT_MSG(policy != CollectionImplBase::GET_NEW &&
                 policy != CollectionImplBase::GET_AUTO,
                 "GET_NEW and GET_AUTO policy does't implemented");

    using TRet = std::pair<CollectionIteratorImplPtr, bool>;

    Iterator iter(this, 0);
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

CollectionIteratorImplPtr NGTCollection::erase(const CollectionIteratorImplPtr& pos)
{
    DVASSERT_MSG(false, "Not implemented operation erase");
    return nullptr;
}

const TypeId& NGTCollection::keyType() const
{
    return keyId;
}

const TypeId& NGTCollection::valueType() const
{
    return valueId;
}

CollectionIteratorImplPtr NGTCollection::erase(const CollectionIteratorImplPtr& first, const CollectionIteratorImplPtr& last)
{
    DVASSERT_MSG(false, "Not implemented operation erase");
    return nullptr;
}

size_t NGTCollection::erase(const Variant& key)
{
    DVASSERT_MSG(false, "Not implemented operation erase");
    return 0;
}

const TypeId& NGTCollection::containerType() const
{
    return containerId;
}

void* NGTCollection::containerData() const
{
    return (void*)(this);
}
}