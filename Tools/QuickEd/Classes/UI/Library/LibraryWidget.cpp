#include "LibraryWidget.h"
#include "SharedData.h"
#include "Document.h"
#include "LibraryModel.h"

namespace
{
    struct LibraryContext : public WidgetContext
    {
        LibraryContext(Document *document)
        {
            DVASSERT(nullptr != document);
            libraryModel = new LibraryModel(document->GetPackage(), document);
        }
        LibraryModel *libraryModel;
    };
}

LibraryWidget::LibraryWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
{
    setupUi(this);
}

void LibraryWidget::OnDocumentChanged(SharedData *arg)
{
    sharedData = arg;
    LoadContext();
}

void LibraryWidget::LoadContext()
{
    if (nullptr == sharedData)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        LibraryContext *context = reinterpret_cast<LibraryContext*>(sharedData->GetContext(this));
        if (nullptr == context)
        {
            context = new LibraryContext(qobject_cast<Document*>(sharedData->parent())); //TODO this is arch. fail
            sharedData->SetContext(this, context);
        }
        treeView->setModel(context->libraryModel);
    }
}
