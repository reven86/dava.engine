#include "Gui/ButtonsWidget.h"
#include "ui_buttonswidget.h"
#include "defines.h"
#include <QPushButton>

ButtonsWidget::ButtonsWidget(int rowNum, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ButtonsWidget)
    , rowNumber(rowNum)
{
    ui->setupUi(this);

    SetButtonsState(BUTTONS_STATE_DISABLED_ALL);

    connect(this, SIGNAL(OnInstall(int)), parent, SLOT(OnInstall(int)));
    connect(this, SIGNAL(OnRun(int)), parent, SLOT(OnRun(int)));
    connect(this, SIGNAL(OnRemove(int)), parent, SLOT(OnRemove(int)));

    connect(ui->installButton, SIGNAL(clicked()), this, SLOT(OnInstallClicked()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(OnRemoveClicked()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(OnRunClicked()));
}

ButtonsWidget::~ButtonsWidget()
{
    SafeDelete(ui);
}

void ButtonsWidget::SetButtonsState(int state)
{
    ui->installButton->setEnabled(false);
    ui->runButton->setEnabled(false);
    ui->removeButton->setEnabled(false);

    if (state & BUTTONS_STATE_AVALIBLE)
        ui->installButton->setEnabled(true);

    if (state & BUTTONS_STATE_INSTALLED)
    {
        ui->runButton->setEnabled(true);
        ui->removeButton->setEnabled(true);
    }
}

void ButtonsWidget::OnRunClicked()
{
    emit OnRun(rowNumber);
}

void ButtonsWidget::OnRemoveClicked()
{
    emit OnRemove(rowNumber);
}

void ButtonsWidget::OnInstallClicked()
{
    emit OnInstall(rowNumber);
}
