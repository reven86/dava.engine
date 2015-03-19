
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
    LegacyEditorUIPackageLoader(DAVA::AbstractUIPackageBuilder *builder, LegacyControlData *data = NULL);
    virtual ~LegacyEditorUIPackageLoader();
    
public:
    virtual DAVA::UIPackage *LoadPackage(const DAVA::FilePath &packagePath) override;
    virtual bool LoadControlByName(const DAVA::String &name) override;

private:
    void LoadControl(const DAVA::String &controlName, const DAVA::YamlNode *node);
    
private:
    void LoadControlPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const DAVA::YamlNode *node);
    void LoadBgPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node);
    void LoadInternalControlPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node);
    
protected:
    virtual DAVA::VariantType ReadVariantTypeFromYamlNode(const DAVA::InspMember *member, const DAVA::YamlNode *node, DAVA::int32 subNodeIndex, const DAVA::String &propertyName);
    
private:
    DAVA::String GetOldPropertyName(const DAVA::String &controlClassName, const DAVA::String &name) const;
    DAVA::String GetOldBgPrefix(const DAVA::String &controlClassName, const DAVA::String &name) const;
    DAVA::String GetOldBgPostfix(const DAVA::String &controlClassName, const DAVA::String &name) const;
    
private:
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::String> > propertyNamesMap;
    DAVA::Map<DAVA::String, DAVA::String> baseClasses;
    
private:
    DAVA::AbstractUIPackageBuilder *builder;
    LegacyControlData *legacyData;
};


#endif // __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__
