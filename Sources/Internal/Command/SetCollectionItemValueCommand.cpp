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

#include "SetCollectionItemValueCommand.h"

#include "Base/Introspection.h"
#include "Base/IntrospectionCollection.h"

namespace DAVA
{
class SetCollectionItemValueCommand::CollectionIteratorHelper
{
public:
    CollectionIteratorHelper(const ObjectHandle& object, const InspColl* collection_, VariantType key)
        : collection(collection_)
    {
        DVASSERT(collection != nullptr);

        const MetaInfo* itemKeyType = collection->ItemKeyType();
        if (itemKeyType == nullptr)
        {
            key = VariantType::Convert(key, VariantType::TYPE_INT32);
            DVASSERT(key.GetType() == VariantType::TYPE_INT32);
            DAVA::int32 index = key.AsInt32();

            iterator = collection->Begin(collection->Pointer(object.GetObjectPointer()));
            while (iterator != nullptr && index > 0)
            {
                iterator = collection->Next(iterator);
                index--;
            }
        }
        else
        {
            DVASSERT(key.Meta() == itemKeyType);
            iterator = collection->Begin(collection->Pointer(object.GetObjectPointer()));
            while (iterator != nullptr)
            {
                if (VariantType::LoadData(collection->ItemKeyData(iterator), key.Meta()) == key)
                {
                    break;
                }
                iterator = collection->Next(iterator);
            }
        }
    }

    ~CollectionIteratorHelper()
    {
        collection->Finish(iterator);
    }

    InspColl::Iterator GetIterator() const
    {
        return iterator;
    }

private:
    const InspColl* collection;
    InspColl::Iterator iterator;
};

SetCollectionItemValueCommand::SetCollectionItemValueCommand(const ObjectHandle& object_, const InspColl* collection_,
                                                             const VariantType& key_, const VariantType& newValue_)
    : object(object_)
    , collection(collection_)
    , key(key_)
    , newValue(newValue_)
{
    DVASSERT(object.IsValid());
    DVASSERT(collection != nullptr);
    DVASSERT(object.GetIntrospection()->Member(collection->Name()) != nullptr);
    DVASSERT(CollectionIteratorHelper(object, collection, key).GetIterator() != nullptr);

    const DAVA::MetaInfo* itemType = collection_->ItemType();
    if (newValue.Meta() != itemType)
    {
        newValue = VariantType::Convert(newValue, itemType);
        DVASSERT(newValue.Meta() == itemType);
    }
}

void SetCollectionItemValueCommand::Execute()
{
    CollectionIteratorHelper iterHelper(object, collection, key);
    oldValue = VariantType::LoadData(collection->ItemData(iterHelper.GetIterator()), collection->ItemType());
    SetValue(iterHelper, newValue);
}

void SetCollectionItemValueCommand::Redo()
{
    SetValue(CollectionIteratorHelper(object, collection, key), newValue);
}

void SetCollectionItemValueCommand::Undo()
{
    SetValue(CollectionIteratorHelper(object, collection, key), oldValue);
}

void SetCollectionItemValueCommand::SetValue(const CollectionIteratorHelper& iterHelper, VariantType value)
{
    collection->ItemValueSet(iterHelper.GetIterator(), value.MetaObject());
}
}