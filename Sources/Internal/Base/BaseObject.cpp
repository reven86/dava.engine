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


#include "Base/BaseObject.h"
#include "Base/ObjectFactory.h"
#include "FileSystem/KeyedArchive.h"
#include "Base/Introspection.h"
#include "Utils/StringFormat.h"

namespace DAVA
{

const String & BaseObject::GetClassName() const
{
    return ObjectFactory::Instance()->GetName(this);
}
    
/**
    \brief virtual function to save node to KeyedArchive
 */
void BaseObject::SaveObject(KeyedArchive * archive)
{
    archive->SetString("##name", GetClassName());
    
//    SaveIntrospection(GetClassName(), archive, GetIntrospection(this), this);
}

/*
void BaseObject::SaveIntrospection(const String &key, KeyedArchive * archive, const IntrospectionInfo *info, void *object)
{
    String keyPrefix = key;
    while(NULL != info)
    {
        keyPrefix += String(info->Name());
        
        for(int32 i = 0; i < info->MembersCount(); ++i)
        {
			const IntrospectionMember *member = info->Member(i);
            if(!member || 0 == (member->Flags() & INTROSPECTION_SERIALIZABLE))
            {
                continue;
            }
                
            String memberKey = keyPrefix + String(member->Name());
            
            const MetaInfo *memberMetaInfo = member->Type();
            if(memberMetaInfo->GetIntrospection())
            {
                SaveIntrospection(memberKey, archive, memberMetaInfo->GetIntrospection(), GetMemberObject(member, object));
            }
            else if(memberMetaInfo->IsPointer())
            {
                //TODO: need to do anything?
            }
            else if(member->Collection())
            {
                SaveCollection(memberKey, archive, member, object);
            }
            else
            {
                VariantType value = member->Value(object);
                archive->SetVariant(memberKey, &value);
            }
        }
        
        info = info->BaseInfo();
    }
}

void BaseObject::SaveCollection(const String &key, KeyedArchive * archive, const IntrospectionMember *member, void *object)
{
    const IntrospectionCollectionBase *collection = member->Collection();
    void *collectedObject = member->Pointer(object);
    
    int32 collectionSize = collection->Size(collectedObject);
    if(collectionSize > 0)
    {
        archive->SetInt32(key + "sizeOfCollection", collectionSize);
        
        MetaInfo *valueType = collection->ValueType();
        IntrospectionCollectionBase::Iterator it = collection->Begin(collectedObject);
        
        if(valueType->GetIntrospection())
        {
            for(int32 index = 0; index < collectionSize; ++index, it = collection->Next(it))
            {
                String itemKey = key + String(Format("item_%d", index));
                if(valueType->IsPointer())
                {
                    SaveIntrospection(itemKey, archive, valueType->GetIntrospection(), *((void **)collection->ItemPointer(it)));
                }
                else
                {
                    SaveIntrospection(itemKey, archive, valueType->GetIntrospection(), collection->ItemPointer(it));
                }
            }
        }
        else
        {
            for(int32 index = 0; index < collectionSize; ++index, it = collection->Next(it))
            {
                String itemKey = key + String(Format("item_%d", index));
                
                if(valueType->IsPointer())
                {
                    //TODO: need to do anything?
                }
                else
                {
                    VariantType value = VariantType::LoadData(collection->ItemPointer(it), valueType);
                    archive->SetVariant(itemKey, &value);
                }
            }
        }
    }
}
    
void * BaseObject::GetMemberObject(const IntrospectionMember *member, void * object) const
{
    const MetaInfo *memberMetaInfo = member->Type();
    if(memberMetaInfo->IsPointer())
    {
        return (*((void **)member->Pointer(object)));
    }

    return member->Pointer(object);
}
*/

BaseObject * BaseObject::LoadFromArchive(KeyedArchive * archive)
{
    String name = archive->GetString("##name");
    BaseObject * object = ObjectFactory::Instance()->New<BaseObject>(name);
    if (object)
    {
        object->LoadObject(archive);
    }
    return object;
}

/**
    \brief virtual function to load node to KeyedArchive
 */
void BaseObject::LoadObject(KeyedArchive * archive)
{
//    LoadIntrospection(GetClassName(), archive, GetIntrospection(this), this);
}

/*
void BaseObject::LoadIntrospection(const String &key, KeyedArchive * archive, const IntrospectionInfo *info, void *object)
{
    String keyPrefix = key;
    while(NULL != info)
    {
        keyPrefix += String(info->Name());
        
        for(int32 i = 0; i < info->MembersCount(); ++i)
        {
			const IntrospectionMember *member = info->Member(i);
            if(!member || 0 == (member->Flags() & INTROSPECTION_SERIALIZABLE))
            {
                continue;
            }
            
            String memberKey = keyPrefix + String(member->Name());
            
            const MetaInfo *memberMetaInfo = member->Type();
            if(memberMetaInfo->GetIntrospection())
            {
                LoadIntrospection(memberKey, archive, memberMetaInfo->GetIntrospection(), GetMemberObject(member, object));
            }
            else if(memberMetaInfo->IsPointer())
            {
                //TODO: need to do anything?
            }
            else if(member->Collection())
            {
                LoadCollection(memberKey, archive, member, object);
            }
            else
            {
                const VariantType * value = archive->GetVariant(memberKey);
                if(value)
                {
                    member->SetValue(object, *value);
                }
            }
        }
        
        info = info->BaseInfo();
    }
}
 
void BaseObject::LoadCollection(const String &key, KeyedArchive * archive, const IntrospectionMember *member, void *object)
{
    const IntrospectionCollectionBase *collection = member->Collection();
    void *collectedObject = member->Pointer(object);
    
    int32 collectionSize = collection->Size(collectedObject);
    int32 requestedCollectionSize = archive->GetInt32(key + "sizeOfCollection", 0);
    if(collectionSize > 0 && requestedCollectionSize == collectionSize) //Do we need to realocate collection?
    {
        MetaInfo *valueType = collection->ValueType();
        IntrospectionCollectionBase::Iterator it = collection->Begin(collectedObject);
        
        if(valueType->GetIntrospection())
        {
            for(int32 index = 0; index < collectionSize; ++index, it = collection->Next(it))
            {
                String itemKey = key + String(Format("item_%d", index));
                if(valueType->IsPointer())
                {
                    LoadIntrospection(itemKey, archive, valueType->GetIntrospection(), *((void **)collection->ItemPointer(it)));
                }
                else
                {
                    LoadIntrospection(itemKey, archive, valueType->GetIntrospection(), collection->ItemPointer(it));
                }
            }
        }
        else
        {
            for(int32 index = 0; index < collectionSize; ++index, it = collection->Next(it))
            {
                String itemKey = key + String(Format("item_%d", index));
                
                if(valueType->IsPointer())
                {
                    //TODO: need to do anything?
                }
                else
                {
                    const VariantType * value = archive->GetVariant(itemKey);
                    if(value)
                    {
                        VariantType::SaveData(collection->ItemPointer(it), valueType, *value);
                    }
                }
            }
        }
    }
    else
    {
        Logger::Error("[BaseObject::LoadCollection] Need resize collection");
    }
}
*/

};


