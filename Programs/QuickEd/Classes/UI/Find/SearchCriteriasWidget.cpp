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
}

void SearchCriteriasWidget::OnRemoveCriteriaClicked()
{
    if (ui->criteriasList->count() > 1)
    {
        QWidget* s = qobject_cast<QWidget*>(QObject::sender());
        ui->criteriasList->removeWidget(s);
        s->deleteLater();
    }
}