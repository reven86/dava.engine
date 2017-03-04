#include "FindInProjectDialog.h"
#include "SearchCriteriasWidget.h"
#include "UI/Find/Finder.h"
#include "Logger/Logger.h"

#include "ui_FindInProjectDialog.h"

using namespace DAVA;

FindInProjectDialog::FindInProjectDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::FindInProjectDialog())
{
    ui->setupUi(this);

    QObject::connect(ui->findButton, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

FindInProjectDialog::~FindInProjectDialog()
{
}

std::unique_ptr<FindFilter> FindInProjectDialog::BuildFindFilter() const
{
    return ui->searchCriterias->BuildFindFilter();
}
