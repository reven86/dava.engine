#include "EditorSystems/FindSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

using namespace DAVA;

FindSystem::FindSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
}

FindSystem::~FindSystem()
{
}

void FindSystem::SelectNextFindResult()
{
    //edit
}

void FindSystem::SelectPrevFindResult()
{
}

void FindSystem::FindInDocument(std::shared_ptr<FindFilter> filter)
{
    context.filter = filter;
}
