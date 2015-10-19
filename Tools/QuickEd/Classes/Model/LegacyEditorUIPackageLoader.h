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


#ifndef __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__
#define __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__

#include "DAVAEngine.h"

#include "UI/AbstractUIPackageBuilder.h"

class LegacyControlData : public DAVA::BaseObject
{
public:
    struct Data {
        DAVA::String name;
        bool isAggregator;
        DAVA::Vector2 size;
    };
public:
    LegacyControlData(){}
    
    const Data *Get(const DAVA::String &fwPath)
    {
        auto it = map.find(fwPath);
        return it == map.end() ? nullptr : &(it->second);
    }
    
    void Put(const DAVA::String &fwPath, const Data &data)
    {
        map[fwPath] = data;
    }
    
public:
    
    DAVA::Map<DAVA::String, Data> map;
};

class LegacyEditorUIPackageLoader : public DAVA::AbstractUIPackageLoader
{
public:
    LegacyEditorUIPackageLoader(LegacyControlData *data = nullptr);
    virtual ~LegacyEditorUIPackageLoader();
    
public:
    bool LoadPackage(const DAVA::FilePath &packagePath, DAVA::AbstractUIPackageBuilder *builder) override;
    bool LoadControlByName(const DAVA::String &name, DAVA::AbstractUIPackageBuilder *builder) override;

private:
    void LoadControl(const DAVA::String &controlName, const DAVA::YamlNode *node, DAVA::AbstractUIPackageBuilder *builder);
    
private:
    void LoadControlPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const DAVA::YamlNode *node, DAVA::AbstractUIPackageBuilder *builder);
    void LoadBgPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node, DAVA::AbstractUIPackageBuilder *builder);
    void LoadInternalControlPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node, DAVA::AbstractUIPackageBuilder *builder);
    void ProcessLegacyAligns(DAVA::UIControl *control, const DAVA::YamlNode *node, DAVA::AbstractUIPackageBuilder *builder);
    
protected:
    virtual DAVA::VariantType ReadVariantTypeFromYamlNode(const DAVA::InspMember *member, const DAVA::YamlNode *node, DAVA::int32 subNodeIndex, const DAVA::String &propertyName);
    
private:
    DAVA::String GetOldPropertyName(const DAVA::String &controlClassName, const DAVA::String &name) const;
    DAVA::String GetOldBgPrefix(const DAVA::String &controlClassName, const DAVA::String &name) const;
    DAVA::String GetOldBgPostfix(const DAVA::String &controlClassName, const DAVA::String &name) const;
    
private:
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::String> > propertyNamesMap;
    DAVA::Map<DAVA::String, DAVA::String> baseClasses;
    DAVA::Map<DAVA::String, DAVA::String> legacyAlignsMap;

private:
    LegacyControlData *legacyData = nullptr;
};


#endif // __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__
