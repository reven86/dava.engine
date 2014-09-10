#include "previewsettingswidget.h"
#include "ui_previewsettingswidget.h"

#include "PreviewController.h"
using namespace DAVA;

#define ID_ROLE Qt::UserRole + 1

PreviewSettingsWidget::PreviewSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreviewSettingsWidget)
{
    ui->setupUi(this);
    connect(ui->previewSettings, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectedModeChanged()));
}

PreviewSettingsWidget::~PreviewSettingsWidget()
{
    delete ui;
}

void PreviewSettingsWidget::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
    FillSettingsList();
}

void PreviewSettingsWidget::FillSettingsList()
{
    ui->previewSettings->clear();
    
    int32 activeID = PreviewController::Instance()->GetActivePreviewSettingsID();
    const List<PreviewSettingsData>& settings = PreviewController::Instance()->GetPreviewSettings();

    for (List<PreviewSettingsData>::const_iterator iter = settings.begin(); iter != settings.end(); iter ++)
    {
        const PreviewSettingsData& curData = (*iter);
        QListWidgetItem* curItem = new QListWidgetItem();
        
        curItem->setData(ID_ROLE, QVariant(curData.id));
        curItem->setText(QString::fromStdString(curData.deviceName));
        ui->previewSettings->addItem(curItem);
        
        curItem->setSelected(curData.id == activeID);
    }
}

void PreviewSettingsWidget::OnSelectedModeChanged()
{
    QList<QListWidgetItem*> selectedItems = ui->previewSettings->selectedItems();
    if (selectedItems.size() == 0)
    {
        return;
    }
    
    int previewIndex = selectedItems.first()->data(ID_ROLE).toInt();
    emit PreviewModeChanged(previewIndex);
}
