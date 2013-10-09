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



#include "DAVAEngine.h"
#include "QtPropertyDataIntrospection.h"
#include "QtPropertyDataDavaVariant.h"
#include "QtPropertyDataDavaKeyedArchive.h"
#include "QtPropertyDataIntoCollection.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::InspInfo *_info, int hasAllFlags)
	: object(_object)
	, info(_info)
{
	CreateCustomButtonsForRenderObject();

	while(NULL != _info && NULL != object)
	{
		for(DAVA::int32 i = 0; i < _info->MembersCount(); ++i)
		{
			const DAVA::InspMember *member = _info->Member(i);
			if(NULL != member && (member->Flags() & hasAllFlags) == hasAllFlags)
			{
                AddMember(member, hasAllFlags);
			}
		}

		_info = _info->BaseInfo();
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

void QtPropertyDataIntrospection::AddMember(const DAVA::InspMember *member, int hasAllFlags)
{
	void *memberObject = member->Data(object);
	const DAVA::MetaInfo *memberMetaInfo = member->Type();
	const DAVA::InspInfo *memberIntrospection = memberMetaInfo->GetIntrospection(memberObject);
	bool isKeyedArchive = false;

	if(memberMetaInfo->IsPointer())
	{
		const DAVA::InspInfo *ii = memberMetaInfo->GetIntrospection(memberObject);
	}

	// keyed archive
	if(NULL != memberIntrospection && (memberIntrospection->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive>()))
	{
		QtPropertyDataDavaKeyedArcive *childData = new QtPropertyDataDavaKeyedArcive((DAVA::KeyedArchive *) memberObject);
		ChildAdd(member->Name(), childData);
	}
	// introspection
	else if(NULL != memberObject && NULL != memberIntrospection)
    {
		QtPropertyDataIntrospection *childData = new QtPropertyDataIntrospection(memberObject, memberIntrospection, hasAllFlags);
		ChildAdd(member->Name(), childData);
    }
	// any other value
    else
    {
		// pointer
        if(memberMetaInfo->IsPointer())
        {
			QString s;
            QtPropertyData* childData = new QtPropertyData(s.sprintf("[%p] Pointer", memberObject));
            childData->SetFlags(childData->GetFlags() | FLAG_IS_DISABLED);
            ChildAdd(member->Name(), childData);
        }
		// other value
        else
        {
			// collection
            if(member->Collection() && !isKeyedArchive)
            {
                QtPropertyDataIntroCollection *childCollection = new QtPropertyDataIntroCollection(memberObject, member->Collection(), hasAllFlags);
                ChildAdd(member->Name(), childCollection);
            }
			// variant
            else
            {
                QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(member->Value(object));
                if(!(member->Flags() & DAVA::I_EDIT))
                {
                    childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
                }
				else
				{
					// check if description has some predefines enum values
					const DAVA::InspDesc &desc = member->Desc();

					if(NULL != desc.enumMap)
					{
						for(size_t i = 0; i < desc.enumMap->GetCount(); ++i)
						{
							int v;
							if(desc.enumMap->GetValue(i, v))
							{
								childData->AddAllowedValue(DAVA::VariantType(v), desc.enumMap->ToString(v));
							}
						}
					}
				}
                
                ChildAdd(member->Name(), childData);
                childVariantMembers.insert(childData, member);
            }
        }
    }
}

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	ChildNeedUpdate();
	return QVariant(info->Name());
}

void QtPropertyDataIntrospection::ChildChanged(const QString &key, QtPropertyData *data)
{
	QtPropertyDataDavaVariant *dataVariant = (QtPropertyDataDavaVariant *) data;

	if(childVariantMembers.contains(dataVariant))
	{
		const DAVA::InspMember *member = childVariantMembers[dataVariant];
		member->SetValue(object, dataVariant->GetVariantValue());
	}
}

void QtPropertyDataIntrospection::ChildNeedUpdate()
{
	QMapIterator<QtPropertyDataDavaVariant*, const DAVA::InspMember *> i = QMapIterator<QtPropertyDataDavaVariant*, const DAVA::InspMember *>(childVariantMembers);

	while(i.hasNext())
	{
		i.next();

		QtPropertyDataDavaVariant *childData = i.key();
		DAVA::VariantType childCurValue = i.value()->Value(object);

		if(childCurValue != childData->GetVariantValue())
		{
			childData->SetVariantValue(childCurValue);
		}

	}
}

void QtPropertyDataIntrospection::CreateCustomButtonsForRenderObject()
{
	if(NULL != info && (info->Type() == DAVA::MetaInfo::Instance<DAVA::RenderObject>()))
	{
		QPushButton *bakeButton = new QPushButton(QIcon(":/QtIcons/transform_bake.png"), "");
		bakeButton->setToolTip("Bake Transform");
		bakeButton->setIconSize(QSize(12, 12));
		AddOW(QtPropertyOW(bakeButton));
		QObject::connect(bakeButton, SIGNAL(pressed()), this, SLOT(BakeTransform()));
	}
}

void QtPropertyDataIntrospection::BakeTransform()
{
	QtPropertyDataIntrospection * renderComponentProperty = static_cast<QtPropertyDataIntrospection*>(parent);
	DAVA::Entity * entity = (static_cast<DAVA::RenderComponent*>(renderComponentProperty->object))->GetEntity();
	DAVA::RenderObject * ro = static_cast<DAVA::RenderObject*>(object);
	ro->BakeTransform(entity->GetLocalTransform());
	entity->SetLocalTransform(DAVA::Matrix4::IDENTITY);
}

