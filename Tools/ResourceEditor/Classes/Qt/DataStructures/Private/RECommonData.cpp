#include "Classes/Qt/DataStructures/RECommonData.h"

namespace RECommonDataDetails
{
template <typename T>
struct SingletonDeleter
{
    void operator()(T* p)
    {
        p->Release();
    }
};
}

RECommonData::RECommonData()
{
}

RECommonData::~RECommonData()
{
}

void RECommonData::Init()
{
    DVASSERT(EditorConfig::Instance() == nullptr);
    DVASSERT(SceneValidator::Instance() == nullptr);
    editorConfig.reset(new EditorConfig(), RECommonDataDetails::SingletonDeleter<EditorConfig>());
    sceneValidator.reset(new SceneValidator(), RECommonDataDetails::SingletonDeleter<SceneValidator>());
}

bool RECommonData::WasDataChanged() const
{
    return wasDataChanged;
}

EditorConfig* RECommonData::GetEditorConfig()
{
    return editorConfig.get();
}

SceneValidator* RECommonData::GetSceneValidator()
{
    return sceneValidator.get();
}
