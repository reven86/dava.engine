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


#include "QtPropertyDataInspColl.h"
#include "QtPropertyDataIntrospection.h"
#include "QtPropertyDataMetaObject.h"

QtPropertyDataInspColl::QtPropertyDataInspColl(const DAVA::FastName& name, void* _object, const DAVA::InspColl* _collection, bool autoAddChilds)
    : QtPropertyData(name)
    , object(_object)
    , collection(_collection)
{
	if(NULL != collection && collection->Size(object) > 0 && autoAddChilds)
	{
		int index = 0;
		DAVA::MetaInfo *valueType = collection->ItemType();
		DAVA::InspColl::Iterator i = collection->Begin(object);
		while(NULL != i)
		{
            DAVA::FastName childName(std::to_string(index));
            if(NULL != valueType->GetIntrospection())
			{
				void * itemObject = collection->ItemData(i);
				const DAVA::InspInfo *itemInfo = valueType->GetIntrospection(itemObject);

				if(NULL != itemInfo && NULL != itemObject)
				{
                    std::unique_ptr<QtPropertyData> childData(new QtPropertyDataIntrospection(childName, itemObject, itemInfo));
                    ChildAdd(std::move(childData));
                }
				else
				{
					QString s;
                    std::unique_ptr<QtPropertyData> childData(new QtPropertyData(childName, s.sprintf("[%p] Pointer", itemObject)));
                    childData->SetEnabled(false);
                    ChildAdd(std::move(childData));
                }
			}
			else
			{
				if(!valueType->IsPointer())
				{
                    std::unique_ptr<QtPropertyData> childData(new QtPropertyDataMetaObject(childName, collection->ItemPointer(i), valueType));
                    ChildAdd(std::move(childData));
                }
				else
				{
                    DAVA::FastName localChildName = childName;
                    if (collection->ItemKeyType() == DAVA::MetaInfo::Instance<DAVA::FastName>())
                    {
                        localChildName = *reinterpret_cast<const DAVA::FastName*>(collection->ItemKeyData(i));
                    }

                    QString s;
                    std::unique_ptr<QtPropertyData> childData(new QtPropertyData(localChildName, s.sprintf("[%p] Pointer", collection->ItemData(i))));
                    childData->SetEnabled(false);
                    ChildAdd(std::move(childData));
                }
			}

			index++;
			i = collection->Next(i);
		}
	}

	SetEnabled(false);
}

QtPropertyDataInspColl::~QtPropertyDataInspColl()
{ }

const DAVA::MetaInfo * QtPropertyDataInspColl::MetaInfo() const
{
	if(NULL != collection)
	{
		return collection->Type();
	}

	return NULL;
}

QVariant QtPropertyDataInspColl::GetValueInternal() const
{
	return QString().sprintf("Collection, size %d", collection->Size(object));
}
