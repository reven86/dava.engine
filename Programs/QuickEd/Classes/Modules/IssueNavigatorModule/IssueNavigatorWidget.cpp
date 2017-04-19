#include "IssueNavigatorWidget.h"

#include "ui_IssueNavigatorWidget.h"


#include <TArc/Core/FieldBinder.h>

IssueNavigatorWidget::IssueNavigatorWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IssueNavigatorWidget())
{
    ui->setupUi(this);
}

IssueNavigatorWidget::~IssueNavigatorWidget()
{
}
