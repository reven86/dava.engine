#include "EditorSystems/BaseEditorSystem.h"

BaseEditorSystem::BaseEditorSystem(EditorSystemsManager* parent)
    : systemsManager(parent)
{
}

void BaseEditorSystem::OnInput(DAVA::UIEvent* /*currentInput*/)
{

}

bool BaseEditorSystem::CanProcessInput() const
{
    return false;
}

BaseEditorSystem::eInternalState BaseEditorSystem::RequireNewState() const
{
    return NO_STATE;
}
