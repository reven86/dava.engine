#include "LibraryWidget.h"
#include "UI/WidgetContext.h"
#include "Document.h"
#include "LibraryModel.h"

struct LibraryDelta : public WidgetDelta
{
    LibraryDelta(Document *document)
    {
        libraryModel = new LibraryModel(document->GetPackage(), document);
    }
    LibraryModel *libraryModel;
};

LibraryWidget::LibraryWidget(QWidget *parent)
    : QDockWidget(parent)
    , widgetContext(nullptr)
{
    setupUi(this);
}

void LibraryWidget::OnContextChanged(WidgetContext *arg)
{
    widgetContext = arg;
    LoadDelta();
}

void LibraryWidget::LoadDelta()
{
    if (nullptr == widgetContext)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        LibraryDelta *delta = reinterpret_cast<LibraryDelta*>(widgetContext->GetDelta(this));
        if (nullptr == delta)
        {
            delta = new LibraryDelta(qobject_cast<Document*>(widgetContext->parent())); //TODO this is arch. fail
            widgetContext->SetDelta(this, delta);
        }
        treeView->setModel(delta->libraryModel);
    }
}
