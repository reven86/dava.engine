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
    int32 currentSelection = 0;
};

class FindController : public QObject, public TrackedObject, PackageListener
{
public:
    FindController(PreviewWidget* previewWidget);
    ~FindController() override;

    void SelectNextFindResult();
    void SelectPrevFindResult();

    void FindInDocument(std::shared_ptr<FindFilter> filter);

    void OnDocumentChanged(Document* document);

    void CancelFind();

private:
    void UpdateHighlight();
    void MoveSelection(DAVA::int32 step);

    FindContext context;
    Document* document = nullptr;
    PreviewWidget* previewWidget = nullptr;
};
