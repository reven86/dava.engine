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
    virtual ~BaseEditorSystem() = delete;

    virtual bool OnInput(DAVA::UIEvent* currentInput);

protected:
    EditorSystemsManager* systemsManager = nullptr;
};

#endif // __QUICKED_BASE_SYSTEM_H__
