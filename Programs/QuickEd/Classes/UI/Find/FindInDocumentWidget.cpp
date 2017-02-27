#include "FindInDocumentWidget.h"
#include "SearchCriteriasWidget.h"
#include "EditorSystems/EditorSystemsManager.h"
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

void FindInDocumentWidget::OnFindClicked()
{
    std::shared_ptr<FindFilter> filter = findFiltersWidget->BuildFindFilter();

    emit OnFindFilterReady(filter);
}
