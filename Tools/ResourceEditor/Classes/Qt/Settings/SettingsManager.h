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

#include "Base/IntrospectionBase.h"
#include "Base/FastNameMap.h"
#include "FileSystem/VariantType.h"

#include "Settings/Settings.h"

#include <QObject>

struct SettingsNode
{
    DAVA::VariantType value;
    DAVA::InspDesc desc;
};

class SettingsManager: public DAVA::Singleton<SettingsManager>
{
public:
	SettingsManager();
	~SettingsManager();

    static DAVA::VariantType GetValue(const DAVA::FastName& path);
	static void SetValue(const DAVA::FastName& path, const DAVA::VariantType &value);

    static DAVA::VariantType GetValue(const DAVA::String& path) { return GetValue(DAVA::FastName(path)); }
    static void SetValue(const DAVA::String& path, const DAVA::VariantType &value) { SetValue(DAVA::FastName(path), value); } 

    static size_t GetSettingsCount();
    static DAVA::FastName GetSettingsName(size_t index);
    static SettingsNode* GetSettingsNode(const DAVA::FastName &name);

    static void ResetPerProjectSettings();

    static void ResetToDefault();

    DAVA_DEPRECATED(static void UpdateGPUSettings());
    
protected:
    DAVA::Vector<DAVA::FastName> settingsOrder;
    DAVA::FastNameMap<SettingsNode> settingsMap;

    void Init();
    void Save();
	void Load();
    void CreateValue(const DAVA::FastName& path, const DAVA::VariantType &defaultValue, const DAVA::InspDesc &description = DAVA::InspDesc(""));
    DAVA_DEPRECATED(bool CustomTextureViewGPULoad(const DAVA::String& paramName, const DAVA::VariantType& src_value, DAVA::VariantType& dstValue));
};

#endif /* defined(__RESOURCEEDITORQT__SETTINGS_MANAGER__) */
