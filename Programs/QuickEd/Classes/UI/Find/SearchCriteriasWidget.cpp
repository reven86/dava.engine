#include "SearchCriteriasWidget.h"
#include "ui_SearchCriteriasWidget.h"

#include "SearchCriteriaWidget.h"
#include "Logger/Logger.h"

using namespace DAVA;

SearchCriteriasWidget::SearchCriteriasWidget(QWidget* parent)
    : QFrame(parent)
    , ui(new Ui::SearchCriteriasWidget())
{
    ui->setupUi(this);

    ui->criteriasList->setAlignment(Qt::AlignTop);
    Reset();
}

SearchCriteriasWidget::~SearchCriteriasWidget() = default;

void SearchCriteriasWidget::Reset()
{
    Set<SearchCriteriaWidget*>::iterator iter = filterWidgets.begin();
    while (iter != filterWidgets.end())
    {
        RemoveCriteriaWidget(*iter);

        iter = filterWidgets.begin();
    }

    OnAddCriteriaClicked();
}

void SearchCriteriasWidget::OnAddCriteriaClicked()
{
    SearchCriteriaWidget* criteria = new SearchCriteriaWidget();
    ui->criteriasList->addWidget(criteria);

    QObject::connect(criteria, SIGNAL(AddAnotherCriteria()), this, SLOT(OnAddCriteriaClicked()));
    QObject::connect(criteria, SIGNAL(RemoveCriteria()), this, SLOT(OnRemoveCriteriaClicked()));
    QObject::connect(criteria, SIGNAL(CriteriaChanged()), this, SIGNAL(CriteriasChanged()));

    setFocusProxy(criteria);

    filterWidgets.insert(criteria);

    emit CriteriasChanged();
}

void SearchCriteriasWidget::OnRemoveCriteriaClicked()
{
    if (filterWidgets.size() > 1)
    {
        SearchCriteriaWidget* widget = qobject_cast<SearchCriteriaWidget*>(QObject::sender());

        RemoveCriteriaWidget(widget);

        emit CriteriasChanged();
    }
}

void SearchCriteriasWidget::RemoveCriteriaWidget(SearchCriteriaWidget* widget)
{
    filterWidgets.erase(widget);
    ui->criteriasList->removeWidget(widget);
    widget->deleteLater();
}

std::unique_ptr<FindFilter> SearchCriteriasWidget::BuildFindFilter() const
{
    DAVA::Vector<std::shared_ptr<FindFilter>> filters;

    for (SearchCriteriaWidget* const filterWidget : filterWidgets)
    {
        filters.push_back(filterWidget->BuildFindFilter());
    }

    return std::make_unique<CompositeFilter>(filters);
}
