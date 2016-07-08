#include "EditorSystems/BaseEditorSystem.h"

BaseEditorSystem::BaseEditorSystem(EditorSystemsManager* parent)
    : systemsManager(parent)
{
}

bool BaseEditorSystem::OnInput(DAVA::UIEvent* /*currentInput*/)
{
    return false;
}
