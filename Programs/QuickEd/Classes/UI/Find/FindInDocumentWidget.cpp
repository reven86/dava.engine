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
    findButton->setText(tr("Find"));

    findAllButton = new QToolButton();
    findAllButton->setText(tr("Find All"));

    QObject::connect(findButton, SIGNAL(clicked()), this, SLOT(OnFindClicked()));
    QObject::connect(findAllButton, SIGNAL(clicked()), this, SLOT(OnFindAllClicked()));
    QObject::connect(findFiltersWidget, SIGNAL(CriteriasChanged()), this, SLOT(OnCriteriasChanged()));

    layout->addWidget(findFiltersWidget);
    layout->addWidget(findButton);
    layout->addWidget(findAllButton);

    setFocusProxy(findFiltersWidget);
}

FindInDocumentWidget::~FindInDocumentWidget()
{
}

std::shared_ptr<FindFilter> FindInDocumentWidget::BuildFindFilter() const
{
    return findFiltersWidget->BuildFindFilter();
}

void FindInDocumentWidget::Reset()
{
    findFiltersWidget->Reset();
}

void FindInDocumentWidget::OnFindClicked()
{
    EmitFilterChanges();

    emit OnFindNext();
}

void FindInDocumentWidget::OnFindAllClicked()
{
    EmitFilterChanges();

    emit OnFindAll();
}

void FindInDocumentWidget::OnCriteriasChanged()
{
    hasChanges = true;
}

bool FindInDocumentWidget::event(QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
        {
            OnFindClicked();
            return true;
        }
        else if (ke->key() == Qt::Key_Escape)
        {
            emit OnCancelFind();
        }
    }

    return QWidget::event(event);
}

void FindInDocumentWidget::EmitFilterChanges()
{
    if (hasChanges)
    {
        std::shared_ptr<FindFilter> filter = findFiltersWidget->BuildFindFilter();

        emit OnFindFilterReady(filter);

        hasChanges = false;
    }
}
