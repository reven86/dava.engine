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



#ifndef __RESOURCEEDITORQT__GENERAL_SETTINGS_EDITOR__
#define __RESOURCEEDITORQT__GENERAL_SETTINGS_EDITOR__

#include "DAVAEngine.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataMetaObject.h"

class GeneralSettingsEditor: public QtPropertyEditor
{
	Q_OBJECT
	
public:
	explicit GeneralSettingsEditor(QWidget* parent = 0);
	
	~GeneralSettingsEditor();
	
	void InitializeProperties();
	
	void RestoreInitialSettings();

protected slots:
	
void	OnValueChanged(QtPropertyData::ValueChangeReason reason);
	
protected:
	
	struct PropertyData
	{
		DAVA::String					configName;
		DAVA::List<DAVA::VariantType>	argumentList;
		DAVA::VariantType				initialValue;
		PropertyData()
		{
		}
		PropertyData(const DAVA::String& _configName, const DAVA::List<DAVA::VariantType>& _argumentList, const DAVA::VariantType& _initialValue)
		{
			configName = _configName;
			argumentList = _argumentList;
			initialValue = _initialValue;
		}
	};

	DAVA::Map<QtPropertyDataDavaVariant* , PropertyData > propertiesMap;
		
};
#endif /* defined(__RESOURCEEDITORQT__GENERAL_SETTINGS_EDITOR__) */