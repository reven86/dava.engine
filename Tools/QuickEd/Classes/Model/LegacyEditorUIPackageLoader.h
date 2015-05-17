
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
        return it == map.end() ? NULL : &(it->second);
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
    LegacyEditorUIPackageLoader(LegacyControlData *data = NULL);
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
    
protected:
    virtual DAVA::VariantType ReadVariantTypeFromYamlNode(const DAVA::InspMember *member, const DAVA::YamlNode *node, DAVA::int32 subNodeIndex, const DAVA::String &propertyName);
    
private:
    DAVA::String GetOldPropertyName(const DAVA::String &controlClassName, const DAVA::String &name) const;
    DAVA::String GetOldBgPrefix(const DAVA::String &controlClassName, const DAVA::String &name) const;
    DAVA::String GetOldBgPostfix(const DAVA::String &controlClassName, const DAVA::String &name) const;
    
private:
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::String> > propertyNamesMap;
    DAVA::Map<DAVA::String, DAVA::String> baseClasses;
    bool storeControlName;
    DAVA::String firstControlName;
    
private:
    LegacyControlData *legacyData;
};


#endif // __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__
