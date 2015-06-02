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


#include "QtPropertyDataMetaObject.h"

QtPropertyDataMetaObject::QtPropertyDataMetaObject(void *_object, const DAVA::MetaInfo *_meta)
	: QtPropertyDataDavaVariant(DAVA::VariantType::LoadData(_object, _meta))
	, object(_object)
	, meta(_meta)
	, lastCommand(NULL)
{ }

QtPropertyDataMetaObject::~QtPropertyDataMetaObject()
{
	DAVA::SafeDelete(lastCommand);
}

const DAVA::MetaInfo* QtPropertyDataMetaObject::MetaInfo() const
{
	return meta;
}

void QtPropertyDataMetaObject::SetValueInternal(const QVariant &value)
{
	QtPropertyDataDavaVariant::SetValueInternal(value);
	DAVA::VariantType newValue = QtPropertyDataDavaVariant::GetVariantValue();

	DAVA::SafeDelete(lastCommand);
	lastCommand = new MetaObjModifyCommand(meta, object, newValue);

	// also save value to meta-object
	DAVA::VariantType::SaveData(object, meta, newValue);
}

bool QtPropertyDataMetaObject::UpdateValueInternal()
{
	bool ret = false;

	// load current value from meta-object
	// we should do this because meta-object may change at any time 
	DAVA::VariantType v = DAVA::VariantType::LoadData(object, meta);

	// if current variant value not equel to the real meta-object value
	// we should update current variant value
	if(v != GetVariantValue())
	{
		QtPropertyDataDavaVariant::SetVariantValue(v);
		ret = true;
	}

	return ret;
}

bool QtPropertyDataMetaObject::EditorDoneInternal(QWidget *editor)
{
	bool ret = QtPropertyDataDavaVariant::EditorDoneInternal(editor);

	// if there was some changes in current value, done by editor
	// we should save them into meta-object
	if(ret)
	{
		DAVA::VariantType::SaveData(object, meta, QtPropertyDataDavaVariant::GetVariantValue());
	}

	return ret;
}

void* QtPropertyDataMetaObject::CreateLastCommand() const
{
	Command2 *command = NULL;

	if(NULL != lastCommand)
	{
		command = new MetaObjModifyCommand(*lastCommand);
	}

	return command;
}
