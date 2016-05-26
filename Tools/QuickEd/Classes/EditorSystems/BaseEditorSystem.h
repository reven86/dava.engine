#ifndef __QUICKED_BASE_SYSTEM_H__
#define __QUICKED_BASE_SYSTEM_H__

namespace DAVA
{
class UIEvent;
}

class EditorSystemsManager;

class BaseEditorSystem
{
public:
    explicit BaseEditorSystem(EditorSystemsManager* parent);
    virtual ~BaseEditorSystem() = 0;

    virtual bool OnInput(DAVA::UIEvent* currentInput);

protected:
    EditorSystemsManager* systemManager = nullptr;
};

inline BaseEditorSystem::~BaseEditorSystem()
{
}

#endif // __QUICKED_BASE_SYSTEM_H__
