#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "UI/Find/Finder.h"

class PreviewWidget;
class ControlNode;
class ControlsContainerNode;
class Document;

namespace DAVA
{
class Vector2;
}

struct FindContext
{
    std::shared_ptr<FindFilter> filter;
    DAVA::Vector<DAVA::String> results;
    DAVA::Vector<FindItem> resultsRaw;
    DAVA::int32 currentSelection = 0;
};

class FindController : public QObject, public DAVA::TrackedObject, PackageListener
{
    Q_OBJECT
public:
    FindController(PreviewWidget* previewWidget);
    ~FindController() override;

    void SelectNextFindResult();
    void SelectPrevFindResult();
    void FindAll();

    void SetFilter(std::shared_ptr<FindFilter> filter);

    void CancelFind();

    void SetFindScope(const DAVA::FilePath& packagePath, const DAVA::Vector<ControlNode*>& rootControls);

signals:
    void ShowFindResults(const DAVA::Vector<FindItem>& results);

private:
    void MoveSelection(DAVA::int32 step);

    DAVA::FilePath packagePath;
    DAVA::Vector<ControlNode*> rootControls;
    FindContext context;
    Document* document = nullptr;
    PreviewWidget* previewWidget = nullptr;
};
