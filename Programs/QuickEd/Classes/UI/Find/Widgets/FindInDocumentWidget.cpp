#include "UI/Find/Widgets/FindInDocumentWidget.h"
#include "UI/Find/Filters/FindFilter.h"
#include "UI/Find/Widgets/CompositeFindFilterWidget.h"

#include <QKeyEvent>

using namespace DAVA;

FindInDocumentWidget::FindInDocumentWidget(QWidget* parent)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    findFiltersWidget = new CompositeFindFilterWidget(this);

    menu = new QMenu(this);
    findAction = new QAction(tr("Find"), this);
    findAllAction = new QAction(tr("Find All"), this);
    menu->addAction(findAction);
    menu->addAction(findAllAction);

    findButton = new QToolButton(this);
    findButton->setMenu(menu);
    findButton->setDefaultAction(findAction);

    QObject::connect(findAction, SIGNAL(triggered()), this, SLOT(OnFindNextClicked()));
    QObject::connect(findAllAction, SIGNAL(triggered()), this, SLOT(OnFindAllClicked()));
    QObject::connect(findFiltersWidget, SIGNAL(FiltersChanged()), this, SLOT(OnFiltersChanged()));

    layout->addWidget(findFiltersWidget);
    layout->addWidget(findButton);

    setFocusProxy(findFiltersWidget);
}

std::shared_ptr<FindFilter> FindInDocumentWidget::BuildFindFilter() const
{
    return findFiltersWidget->BuildFindFilter();
}

void FindInDocumentWidget::Reset()
{
    findFiltersWidget->Reset();
}

void FindInDocumentWidget::OnFindNextClicked()
{
    EmitFilterChanges();

    emit OnFindNext();
}

void FindInDocumentWidget::OnFindPreviousClicked()
{
    EmitFilterChanges();

    emit OnFindPrevious();
}

void FindInDocumentWidget::OnFindAllClicked()
{
    EmitFilterChanges();

    emit OnFindAll();
}

void FindInDocumentWidget::OnFiltersChanged()
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
            if (ke->modifiers() & Qt::ControlModifier)
            {
                OnFindAllClicked();
            }
            else
            {
                if (ke->modifiers() & Qt::ShiftModifier)
                {
                    OnFindPreviousClicked();
                }
                else
                {
                    OnFindNextClicked();
                }
            }
            return true;
        }
        else if (ke->key() == Qt::Key_Escape)
        {
            emit OnStopFind();
            return true;
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
