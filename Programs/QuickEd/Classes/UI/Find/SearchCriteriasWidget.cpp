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

    OnAddCriteriaClicked();
}

SearchCriteriasWidget::~SearchCriteriasWidget() = default;

void SearchCriteriasWidget::OnAddCriteriaClicked()
{
    SearchCriteriaWidget* criteria = new SearchCriteriaWidget();
    ui->criteriasList->addWidget(criteria);

    QObject::connect(criteria, SIGNAL(AddAnotherCriteria()), this, SLOT(OnAddCriteriaClicked()));
    QObject::connect(criteria, SIGNAL(RemoveCriteria()), this, SLOT(OnRemoveCriteriaClicked()));

    setFocusProxy(criteria);

    filterWidgets.insert(criteria);
}

void SearchCriteriasWidget::OnRemoveCriteriaClicked()
{
    if (filterWidgets.size() > 1)
    {
        SearchCriteriaWidget* s = qobject_cast<SearchCriteriaWidget*>(QObject::sender());

        filterWidgets.erase(s);

        ui->criteriasList->removeWidget(s);
        s->deleteLater();
    }
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
