#include "EditorSystems/BaseEditorSystem.h"

BaseEditorSystem::BaseEditorSystem(EditorSystemsManager* parent)
    : systemManager(parent)
{
}

bool BaseEditorSystem::OnInput(DAVA::UIEvent* /*currentInput*/)
{
    return false;
}
