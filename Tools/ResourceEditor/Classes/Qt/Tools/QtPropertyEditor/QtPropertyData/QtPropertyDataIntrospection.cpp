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
#include "QtPropertyDataDavaKeyedArchive.h"
#include "QtPropertyDataInspMember.h"
#include "QtPropertyDataInspColl.h"

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

QtPropertyData * QtPropertyDataIntrospection::CreateMemberData(void *_object, const DAVA::InspMember *member, int hasAllFlags)
{
	void *memberObject = member->Data(_object);
	const DAVA::MetaInfo *memberMetaInfo = member->Type();
	const DAVA::InspInfo *memberIntrospection = memberMetaInfo->GetIntrospection(memberObject);
	bool isKeyedArchive = false;

	QtPropertyData * retData = NULL;
	// keyed archive
	if(NULL != memberIntrospection && (memberIntrospection->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive>()))
	{
		retData = new QtPropertyDataDavaKeyedArcive((DAVA::KeyedArchive *) memberObject);
	}
	// introspection
	else if(NULL != memberObject && NULL != memberIntrospection)
    {
		retData = new QtPropertyDataIntrospection(memberObject, memberIntrospection, hasAllFlags);
    }
	// any other value
    else
    {
		// pointer
        if(memberMetaInfo->IsPointer())
        {
			QString s;
            retData = new QtPropertyData(s.sprintf("[%p] Pointer", memberObject));
            retData->SetFlags(retData->GetFlags() | FLAG_IS_DISABLED);
        }
		// other value
        else
        {
			// collection
            if(member->Collection() && !isKeyedArchive)
            {
                retData = new QtPropertyDataInspColl(memberObject, member->Collection(), hasAllFlags);
            }
			// variant
            else
            {
                QtPropertyDataInspMember *childData = new QtPropertyDataInspMember(_object, member);
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
                
                retData = childData;
            }
        }
    }
	return retData;
}

void QtPropertyDataIntrospection::AddMember(const DAVA::InspMember *member, int hasAllFlags)
{
	QtPropertyData* retData = CreateMemberData(object, member, hasAllFlags);
	void *memberObject = member->Data(object);
	const DAVA::MetaInfo *memberMetaInfo = member->Type();
	const DAVA::InspInfo *memberIntrospection = memberMetaInfo->GetIntrospection(memberObject);
	
	ChildAdd(member->Name(), retData);
	//condition for variant
	if((!memberMetaInfo->IsPointer()) && (!member->Collection()) && 
		(NULL == memberIntrospection || (memberIntrospection->Type() != DAVA::MetaInfo::Instance<DAVA::KeyedArchive>())))
	{
		QtPropertyDataInspMember *childData = dynamic_cast<QtPropertyDataInspMember *>(retData);
		if(NULL != childData)
		{
			childVariantMembers.insert(childData, member);
		}
	}
}

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	return QVariant(info->Name());
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

