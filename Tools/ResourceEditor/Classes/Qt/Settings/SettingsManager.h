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



#ifndef __RESOURCEEDITORQT__SETTINGS_MANAGER__
#define __RESOURCEEDITORQT__SETTINGS_MANAGER__

#include <QObject>
#include "DAVAEngine.h"
#include "FileSystem/VariantType.h"

class SettingsManager: public QObject, public DAVA::Singleton<SettingsManager>
{
	Q_OBJECT
	
public:
	
	enum eDefaultSettings
	{
		LOD_LEVELS_COUNT = 8,
		RESENT_FILES_COUNT = 5,
	};
	
	SettingsManager();
	
	~SettingsManager();
	
	void SetValue(const DAVA::String& _name, const DAVA::VariantType& _value, DAVA::List<DAVA::VariantType>& additionalArguments);
	
	void SetValue(const DAVA::String& _name, const DAVA::VariantType& _value);

	DAVA::VariantType GetValue(const DAVA::String& _name, const DAVA::List<DAVA::VariantType>& additionalArguments);
	
	DAVA::VariantType GetValue(const DAVA::String& _name);

	DAVA::KeyedArchive* GetSettings();
	
	DAVA::FilePath GetParticlesConfigsPath();
	
	float GetCameraSpeed();
	
	void SetMaterialsColor(const DAVA::Color &ambient, const DAVA::Color &diffuse, const DAVA::Color &specular);

	void Save();
		
signals:
	
	void ConfigurationChanged(const DAVA::String& key);

protected:

	void ApplyOptions();
	
	/* For Getters  */

	void ResolveCameraSpeedValueKeyDefault(const DAVA::List<DAVA::VariantType>& additionalArguments, DAVA::VariantType& newDefValue, DAVA::String& newKey);
	
	void ResolveLastOpenedFileKeyDefault(const DAVA::List<DAVA::VariantType>& additionalArguments, DAVA::VariantType& newDefValue, DAVA::String& newKey);
	
	void ResolveLodLevelKeyDefault(const DAVA::List<DAVA::VariantType>& additionalArguments, DAVA::VariantType& newDefValue, DAVA::String& newKey);
	
	/* For Setters  */
	void ResolveCameraSpeedValueKey(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey);
	
	void ResolveLastOpenedFileKey(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey);
	
	void AddLastOpenedFile(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey);
	
	void CameraSpeedIndexCheck(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey);
	
	void ResolveLodLevelKey(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey);
	
	void SetImposterOption(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey);
	
	
	DAVA::KeyedArchive *settings;
};

/*
class SettingsManager2: public QObject, public DAVA::Singleton<SettingsManager2>
{
	Q_OBJECT

public:
	SettingsManager2();
	virtual ~SettingsManager2();

	const DAVA::VariantType& GetValue(DAVA::FastName key);
	void SetValue(DAVA::FastName key, const DAVA::VariantType& value);

	void TrackSettings(DAVA::InspInfo *insp, void *object);

protected:
	struct SettingRow
	{
		void *object;
		DAVA::InspMember *member;
	};

	DAVA::FastNameMap<SettingRow> settings;
};
*/

#endif /* defined(__RESOURCEEDITORQT__SETTINGS_MANAGER__) */