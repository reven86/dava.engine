#include "EditorSystems/BaseEditorSystem.h"
#include "UI/UIEvent.h"

BaseEditorSystem::BaseEditorSystem(EditorSystemsManager* parent)
    : systemsManager(parent)
{
}

void BaseEditorSystem::ProcessInput(DAVA::UIEvent* /*currentInput*/)
{
}

bool BaseEditorSystem::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    return false;
}

EditorSystemsManager::eDragState BaseEditorSystem::RequireNewState(DAVA::UIEvent* currentInput)
{
    return EditorSystemsManager::NoDrag;
}

void BaseEditorSystem::OnDragStateChanged(EditorSystemsManager::eDragState /*currentState*/, EditorSystemsManager::eDragState /*previousState*/)
{
}

void BaseEditorSystem::OnDisplayStateChanged(EditorSystemsManager::eDisplayState /*currentState*/, EditorSystemsManager::eDisplayState /*previousState*/)
{
}
