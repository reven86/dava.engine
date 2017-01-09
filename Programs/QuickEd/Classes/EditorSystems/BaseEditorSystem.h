#pragma once
#include "EditorSystems/EditorSystemsManager.h"

namespace DAVA
{
class UIEvent;
}

class BaseEditorSystem
{
protected:
    enum eInternalState
    {
        NO_STATE = EditorSystemsManager::StatesCount
    };
    explicit BaseEditorSystem(EditorSystemsManager* parent);
    virtual ~BaseEditorSystem() = default;

    EditorSystemsManager* systemsManager = nullptr;
    
private:
    friend class EditorSystemsManager;
    virtual bool CanProcessInput() const;

    virtual eInternalState RequireNewState() const;

    virtual void OnInput(DAVA::UIEvent* currentInput);

};
