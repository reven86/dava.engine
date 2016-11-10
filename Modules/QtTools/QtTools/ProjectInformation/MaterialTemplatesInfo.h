#pragma once

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"

struct MaterialTemplateInfo
{
    DAVA::String name;
    DAVA::String path;
    DAVA::Vector<DAVA::String> qualities;
};

/**
Singleton that holds material templates info from assignable.yaml
Should be reloaded after reloading of project.
Class will probably be refactored/removed after implementation of resource system
*/
class MaterialTemplatesInfo : public DAVA::StaticSingleton<MaterialTemplatesInfo>
{
public:
    void Load(const DAVA::FilePath& pathToAssignable);
    const DAVA::Vector<MaterialTemplateInfo>& GetTemplatesInfo() const;

private:
    DAVA::Vector<MaterialTemplateInfo> templates;
};

inline const DAVA::Vector<MaterialTemplateInfo>& MaterialTemplatesInfo::GetTemplatesInfo() const
{
    return templates;
}
