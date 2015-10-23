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


#include "QtPropertyDataInspDynamic.h"

#include <QDebug>


QtPropertyDataInspDynamic::QtPropertyDataInspDynamic(DAVA::InspInfoDynamic *_dynamicInfo, DAVA::InspInfoDynamic::DynamicData _ddata, DAVA::FastName _name)
	: QtPropertyDataDavaVariant(DAVA::VariantType())
	, dynamicInfo(_dynamicInfo)
    , ddata(_ddata)
    , name(_name)
    , inspFlags(0)
	, lastCommand(NULL)
{
	if(NULL != dynamicInfo)
	{
		SetVariantValue(dynamicInfo->MemberValueGet(ddata, name));
        inspFlags = dynamicInfo->MemberFlags(ddata, name);
	}
}

QtPropertyDataInspDynamic::~QtPropertyDataInspDynamic()
{
	DAVA::SafeDelete(lastCommand);
}

const DAVA::MetaInfo * QtPropertyDataInspDynamic::MetaInfo() const
{
	if(NULL != dynamicInfo && NULL != dynamicInfo->GetMember())
	{
		return dynamicInfo->GetMember()->Type();
	}

	return NULL;
}

int QtPropertyDataInspDynamic::InspFlags() const
{
	return inspFlags;
}

QVariant QtPropertyDataInspDynamic::GetValueAlias() const
{
	QVariant ret;

	if(NULL != dynamicInfo)
	{
        ret = FromDavaVariant(dynamicInfo->MemberAliasGet(ddata, name));
	}

	return ret;
}

void QtPropertyDataInspDynamic::SetValueInternal(const QVariant &value)
{
	QtPropertyDataDavaVariant::SetValueInternal(value);
	DAVA::VariantType newValue;
	
	if(!value.isNull())
	{
		newValue = QtPropertyDataDavaVariant::GetVariantValue();
	}

	// also save value to meta-object
	if(NULL != dynamicInfo)
	{
		DAVA::SafeDelete(lastCommand);
        lastCommand = new InspDynamicModifyCommand(dynamicInfo, ddata, name, newValue);

        dynamicInfo->MemberValueSet(ddata, name, newValue);
	}
}


void QtPropertyDataInspDynamic::SetTempValueInternal(const QVariant& value)
{
	QtPropertyDataDavaVariant::SetValueInternal(value);
	DAVA::VariantType newValue;
	
	if(!value.isNull())
	{
		newValue = QtPropertyDataDavaVariant::GetVariantValue();
	}

	// save value to meta-object
	if(NULL != dynamicInfo)
	{
        dynamicInfo->MemberValueSet(ddata, name, newValue);
	}
}

bool QtPropertyDataInspDynamic::UpdateValueInternal()
{
	bool ret = false;

	// get current value from introspection member
	// we should do this because member may change at any time
	if(NULL != dynamicInfo)
	{
        DAVA::VariantType v = dynamicInfo->MemberValueGet(ddata, name);

		// if current variant value not equal to the real member value
		// we should update current variant value
		if(v.GetType() != DAVA::VariantType::TYPE_NONE && v != GetVariantValue())
		{
			QtPropertyDataDavaVariant::SetVariantValue(v);
			ret = true;
		}
	}

	return ret;
}

bool QtPropertyDataInspDynamic::EditorDoneInternal(QWidget *editor)
{
	bool ret = QtPropertyDataDavaVariant::EditorDoneInternal(editor);

	// if there was some changes in current value, done by editor
	// we should save them into meta-object
	if(ret && NULL != dynamicInfo)
	{
        dynamicInfo->MemberValueSet(ddata, name, QtPropertyDataDavaVariant::GetVariantValue());
	}

	return ret;
}

void* QtPropertyDataInspDynamic::CreateLastCommand() const
 {
 	Command2 *command = NULL;
 
 	if(NULL != lastCommand)
 	{
 		command = new InspDynamicModifyCommand(*lastCommand);
 	}
 
 	return command;
}
