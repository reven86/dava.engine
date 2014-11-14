/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "QtPropertyDataInspMember.h"

QtPropertyDataInspMember::QtPropertyDataInspMember(void *_object, const DAVA::InspMember *_member)
	: QtPropertyDataDavaVariant(DAVA::VariantType())
	, object(_object)
	, member(_member)
	, lastCommand(NULL)
{
	if(NULL != member)
	{
		SetVariantValue(member->Value(object));
	}
}

QtPropertyDataInspMember::~QtPropertyDataInspMember()
{
	DAVA::SafeDelete(lastCommand);
}

const DAVA::MetaInfo * QtPropertyDataInspMember::MetaInfo() const
{
	if(NULL != member)
	{
		return member->Type();
	}

	return NULL;
}

void QtPropertyDataInspMember::SetValueInternal(const QVariant &value)
{
	QtPropertyDataDavaVariant::SetValueInternal(value);
	DAVA::VariantType newValue = QtPropertyDataDavaVariant::GetVariantValue();

	// also save value to meta-object
	if(NULL != member)
	{
		DAVA::SafeDelete(lastCommand);
		lastCommand = new InspMemberModifyCommand(member, object, newValue);

		member->SetValue(object, newValue);
	}
}


void QtPropertyDataInspMember::SetTempValueInternal(const QVariant& value)
{
	QtPropertyDataDavaVariant::SetValueInternal(value);
	DAVA::VariantType newValue = GetVariantValue();

	// save value to meta-object
	if(NULL != member)
	{
		member->SetValue(object, newValue);
	}
}

bool QtPropertyDataInspMember::UpdateValueInternal()
{
	bool ret = false;

	// get current value from introspection member
	// we should do this because member may change at any time
	if(NULL != member)
	{
		DAVA::VariantType v = member->Value(object);

		// if current variant value not equal to the real member value
		// we should update current variant value
		if(v != GetVariantValue())
		{
			QtPropertyDataDavaVariant::SetVariantValue(v);
			ret = true;
		}
	}

	return ret;
}

bool QtPropertyDataInspMember::EditorDoneInternal(QWidget *editor)
{
	bool ret = QtPropertyDataDavaVariant::EditorDoneInternal(editor);

	// if there was some changes in current value, done by editor
	// we should save them into meta-object
	if(ret && NULL != member)
	{
		member->SetValue(object, QtPropertyDataDavaVariant::GetVariantValue());
	}

	return ret;
}

void* QtPropertyDataInspMember::CreateLastCommand() const
{
	Command2 *command = NULL;

	if(NULL != lastCommand)
	{
		command = new InspMemberModifyCommand(*lastCommand);
	}

	return command;
}
