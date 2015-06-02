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


#include "QtPropertyDataKeyedArchiveMember.h"
#include "FileSystem/KeyedArchive.h"
#include "Deprecated/EditorConfig.h"
#include "Main/QtUtils.h"

QtPropertyKeyedArchiveMember::QtPropertyKeyedArchiveMember(DAVA::KeyedArchive* _archive, const DAVA::String& _key)
	: QtPropertyDataDavaVariant(DAVA::VariantType())
	, archive(_archive)
	, key(_key)
	, lastCommand(NULL)
{
    CheckAndFillPresetValues();

	if(NULL != archive)
	{
		DAVA::VariantType *val = archive->GetVariant(key);
		if(NULL != val)
		{
			SetVariantValue(*val);
		}
	}
}

QtPropertyKeyedArchiveMember::~QtPropertyKeyedArchiveMember()
{
	DAVA::SafeDelete(lastCommand);
}

void QtPropertyKeyedArchiveMember::CheckAndFillPresetValues()
{
    const int valueType = archive->GetVariant(key)->GetType();

    const int presetValueType = EditorConfig::Instance()->GetPropertyValueType(key);
    if (presetValueType != DAVA::VariantType::TYPE_NONE)
    {
        if (valueType == presetValueType)
        {
            const DAVA::Vector<DAVA::String>& allowedValues = EditorConfig::Instance()->GetComboPropertyValues(key);
            if (allowedValues.size() > 0)
            {
                for (size_t i = 0; i < allowedValues.size(); ++i)
                {
                    AddAllowedValue(DAVA::VariantType((int)i), allowedValues[i].c_str());
                }
            }
            else
            {
                const DAVA::Vector<Color> & allowedColors = EditorConfig::Instance()->GetColorPropertyValues(key);
                for (size_t i = 0; i < allowedColors.size(); ++i)
                {
                    AddAllowedValue(DAVA::VariantType((int)i), ColorToQColor(allowedColors[i]));
                }
            }
        }
    }
}

void QtPropertyKeyedArchiveMember::SetValueInternal(const QVariant &value)
{
	QtPropertyDataDavaVariant::SetValueInternal(value);
	DAVA::VariantType newValue = QtPropertyDataDavaVariant::GetVariantValue();

	// also save value to meta-object
	if(NULL != archive && archive->IsKeyExists(key))
	{
		DAVA::SafeDelete(lastCommand);
		lastCommand = new KeyeadArchiveSetValueCommand(archive, key, newValue);

		archive->SetVariant(key, newValue);
	}
}

bool QtPropertyKeyedArchiveMember::UpdateValueInternal()
{
	bool ret = false;

	// get current value from introspection member
	// we should do this because member may change at any time
	if(NULL != archive)
	{
		DAVA::VariantType *val = archive->GetVariant(key);
		if(NULL != val)
		{
			DAVA::VariantType v = *val;

			// if current variant value not equal to the real member value
			// we should update current variant value
			if(v != GetVariantValue())
			{
				QtPropertyDataDavaVariant::SetVariantValue(v);
				ret = true;
			}
		}
	}

	return ret;
}

bool QtPropertyKeyedArchiveMember::EditorDoneInternal(QWidget *editor)
{
	bool ret = QtPropertyDataDavaVariant::EditorDoneInternal(editor);

	// if there was some changes in current value, done by editor
	// we should save them into meta-object
	if(ret && NULL != archive && archive->IsKeyExists(key))
	{
		archive->SetVariant(key, QtPropertyDataDavaVariant::GetVariantValue());
	}

	return ret;
}

void* QtPropertyKeyedArchiveMember::CreateLastCommand() const
{
	Command2 *command = NULL;

	if(NULL != lastCommand)
	{
		command = new KeyeadArchiveSetValueCommand(*lastCommand);
	}

	return command;
}