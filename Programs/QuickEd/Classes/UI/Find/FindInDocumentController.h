#pragma once

#include <Base/BaseTypes.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <QObject>

class DocumentsModule;
class FindInDocumentWidget;
class FindFilter;
class MainWindow;

class FindInDocumentController : public QObject, public DAVA::TArc::DataListener
{
    Q_OBJECT
public:
    FindInDocumentController(DocumentsModule* documentsModule, MainWindow* mainWindow, FindInDocumentWidget* findInDocumentWidget);
    ~FindInDocumentController() override;

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
        DAVA::int32 currentSelection = 0;
    };

    void MoveSelection(DAVA::int32 step);

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    DocumentsModule* documentsModule = nullptr;
    FindContext context;
    FindInDocumentWidget* findInDocumentWidget = nullptr;

    DAVA::TArc::DataWrapper documentDataWrapper;
};
