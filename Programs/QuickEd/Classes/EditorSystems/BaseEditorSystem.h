#pragma once
#include "EditorSystems/EditorSystemsManager.h"

namespace DAVA
{
class UIEvent;
}

class BaseEditorSystem
{
public:
    explicit BaseEditorSystem(EditorSystemsManager* parent);
    virtual ~BaseEditorSystem() = default;

protected:
    enum eInternalState
    {
        NO_STATE = EditorSystemsManager::StatesCount
    };


    EditorSystemsManager* systemsManager = nullptr;
    
private:
    friend class EditorSystemsManager;

    virtual eInternalState RequireNewState(DAVA::UIEvent* currentInput) const;

    virtual void OnInput(DAVA::UIEvent* currentInput);
    virtual bool CanProcessInput() const;
    virtual void OnStateChanged(EditorSystemsManager::eState state);

};
