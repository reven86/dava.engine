#include "FindInDocumentWidget.h"
#include "SearchCriteriasWidget.h"
#include "UI/Find/Finder.h"
#include "Logger/Logger.h"

#include <QCheckBox>
#include <QTextEdit>
#include <QMenu>

using namespace DAVA;

FindInDocumentWidget::FindInDocumentWidget(QWidget* parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    findFiltersWidget = new SearchCriteriasWidget();

    findButton = new QToolButton();
    findButton->setText("Find");

    QObject::connect(findButton, SIGNAL(clicked()), this, SLOT(OnFindClicked()));

    layout->addWidget(findFiltersWidget);
    layout->addWidget(findButton);

    setFocusProxy(findFiltersWidget);
}

FindInDocumentWidget::~FindInDocumentWidget()
{
}

std::shared_ptr<FindFilter> FindInDocumentWidget::BuildFindFilter() const
{
    return findFiltersWidget->BuildFindFilter();
}

void FindInDocumentWidget::OnDocumentChanged(Document* document_)
{
    document = document_;
}

void FindInDocumentWidget::OnFindClicked()
{
    if (document)
    {
        std::unique_ptr<FindFilter> filter = findFiltersWidget->BuildFindFilter();

        Finder finder(std::move(filter), nullptr);

        connect(&finder, &Finder::ProgressChanged, this, &FindInDocumentWidget::OnProgressChanged);
        connect(&finder, &Finder::ItemFound, this, &FindInDocumentWidget::OnItemFound);
        connect(&finder, &Finder::Finished, this, &FindInDocumentWidget::OnFindFinished);

        finder.Process(document->GetPackage());
    }
}

void FindInDocumentWidget::OnItemFound(FindItem item)
{
    for (auto c : item.GetControlPaths())
    {
        Logger::Debug("%s %s %s",
                      __FUNCTION__,
                      item.GetFile().GetStringValue().c_str(),
                      c.c_str());
    }
}

void FindInDocumentWidget::OnProgressChanged(int filesProcessed, int totalFiles)
{
}

void FindInDocumentWidget::OnFindFinished()
{
    Logger::Debug("%s", __FUNCTION__);
}
