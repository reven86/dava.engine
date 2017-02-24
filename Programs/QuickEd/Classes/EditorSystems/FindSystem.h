#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "UI/Find/Finder.h"

class EditorSystemsManager;
class ControlNode;
class ControlsContainerNode;

namespace DAVA
{
class Vector2;
}

struct FindContext
{
    std::shared_ptr<FindFilter> filter;
    DAVA::Vector<DAVA::String> results;
    int32 currentSelection = 0;
};

class FindSystem : public BaseEditorSystem, PackageListener, public DAVA::InspBase
{
public:
    FindSystem(EditorSystemsManager* doc);
    ~FindSystem() override;

    void SelectNextFindResult();
    void SelectPrevFindResult();

    void FindInDocument(std::shared_ptr<FindFilter> filter);

private:
    FindContext context;
};
