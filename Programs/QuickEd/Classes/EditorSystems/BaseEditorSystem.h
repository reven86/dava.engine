#pragma once
#include "EditorSystems/EditorSystemsManager.h"

namespace DAVA
{
class UIEvent;
}

class BaseEditorSystem
{
private:
    friend class EditorSystemsManager;

    enum eInternalState
    {
        NO_STATE = EditorSystemsManager::StatesCount
    };

    explicit BaseEditorSystem(EditorSystemsManager* parent);
    virtual ~BaseEditorSystem() = default;

    virtual bool CanProcessInput() const;

    virtual eInternalState RequireNewState() const;

    virtual void OnInput(DAVA::UIEvent* currentInput);

    EditorSystemsManager* systemsManager = nullptr;
};
