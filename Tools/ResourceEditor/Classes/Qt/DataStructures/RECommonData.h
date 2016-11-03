#pragma once

#include "Classes/Deprecated/EditorConfig.h"
#include "Classes/Deprecated/SceneValidator.h"

#include "TArc/DataProcessing/DataNode.h"

#include "Reflection/Reflection.h"

class RECommonData : public DAVA::TArc::DataNode
{
public:
    RECommonData();
    ~RECommonData();

    void Init();

    bool WasDataChanged() const;
    EditorConfig* GetEditorConfig();

private:
    friend class InitModule;
    bool wasDataChanged = false;
    std::shared_ptr<EditorConfig> editorConfig;

    DAVA_VIRTUAL_REFLECTION(RECommonData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<RECommonData>::Begin()
        .Field("WasDataChanged", &RECommonData::WasDataChanged, nullptr)
        .End();
    }
};