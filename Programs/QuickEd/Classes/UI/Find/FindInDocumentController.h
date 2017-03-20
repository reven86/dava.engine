#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/FieldBinder.h>
#include <QObject>

class DocumentsModule;
class FindInDocumentWidget;
class FindFilter;
class MainWindow;

class FindInDocumentController : public QObject
{
    Q_OBJECT
public:
    FindInDocumentController(DocumentsModule* documentsModule, MainWindow* mainWindow, FindInDocumentWidget* findInDocumentWidget);

private slots:
    void ShowFindInDocumentWidget();
    void HideFindInDocumentWidget();

    void SelectNextFindResult();
    void SelectPreviousFindResult();

    void FindAll();

    void SetFilter(std::shared_ptr<FindFilter> filter);

private:
    struct FindContext
    {
        std::shared_ptr<FindFilter> filter;
        DAVA::Vector<DAVA::String> results;
        DAVA::int32 currentSelection = -1;
    };

    void MoveSelection(DAVA::int32 step);

    void OnEditedRootControlsChanged(const DAVA::Any& value);

    DocumentsModule* documentsModule = nullptr;
    FindContext context;
    FindInDocumentWidget* findInDocumentWidget = nullptr;

    std::unique_ptr<DAVA::TArc::FieldBinder> editedRootControlsFieldBinder;
};
