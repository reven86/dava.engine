#pragma once

#include "EditorSystems/EditorSystemsManager.h"

namespace DAVA
{
class UIEvent;
namespace TArc
{
class ContextAccessor;
}
}

class BaseEditorSystem
{
public:
    explicit BaseEditorSystem(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);
    virtual ~BaseEditorSystem() = default;

protected:
    EditorSystemsManager* systemsManager = nullptr;
    DAVA::TArc::ContextAccessor* accessor = nullptr;

private:
    //this class is designed to be used only by EditorSystemsManager
    friend class EditorSystemsManager;

    //Ask system for a new state
    //this state must be unique
    //if two different systems require different states at the same time - this is logical error
    //this method is not const because it can update cached states
    virtual EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput);
    //if system can not process input - OnInput method will not be called
    virtual bool CanProcessInput(DAVA::UIEvent* currentInput) const;
    //process input to realize state logic
    virtual void ProcessInput(DAVA::UIEvent* currentInput);
    //invalidate caches or prepare to work depending on states
    virtual void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState);
    virtual void OnDisplayStateChanged(EditorSystemsManager::eDisplayState currentState, EditorSystemsManager::eDisplayState previousState);
};
