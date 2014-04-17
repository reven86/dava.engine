#include "previewsettingsdialog.h"
#include "ui_previewsettingsdialog.h"

#include "editpreviewsettingsdialog.h"

#include <QMessageBox>

using namespace DAVA;

static const QString PREVIEW_DEVICE_NAME_COLUMN = "Device Name";
static const QString PREVIEW_RESOLUTION_COLUMN = "Resolution";
static const QString PREVIEW_DPI_COLUMN = "DPI";
static const QString PREVIEW_SCREEN_SIZE_COLUMN = "Screen Size";

PreviewSettingsDialog::PreviewSettingsDialog(bool selectionMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewSettingsDialog),
    isSelectionMode(selectionMode)
{
    ui->setupUi(this);

    connect(ui->buttonAdd, SIGNAL(clicked()), this, SLOT(AddButtonClicked()));
    connect(ui->buttonRemove, SIGNAL(clicked()), this, SLOT(RemoveButtonClicked()));
	connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(CloseButtonClicked()));
    connect(ui->settingsTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(SettingsTableDoubleClicked(const QModelIndex&)));

    this->ui->buttonAdd->setVisible(!selectionMode);
    this->ui->buttonRemove->setVisible(!selectionMode);

    InitializeTableView();
    ReloadSettings();
}

PreviewSettingsDialog::~PreviewSettingsDialog()
{
    delete ui;
}

void PreviewSettingsDialog::InitializeTableView()
{
    //Setup table appearence
    ui->settingsTableView->verticalHeader()->hide();
    ui->settingsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->settingsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->settingsTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->settingsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Create and set table view model
    tableModel = new QStandardItemModel(this);

    tableModel->setHorizontalHeaderItem(0, new QStandardItem(QString(PREVIEW_DEVICE_NAME_COLUMN)));

    tableModel->setHorizontalHeaderItem(1, new QStandardItem(QString(PREVIEW_RESOLUTION_COLUMN)));
    tableModel->setHorizontalHeaderItem(2, new QStandardItem(QString(PREVIEW_DPI_COLUMN)));
    tableModel->setHorizontalHeaderItem(3, new QStandardItem(QString(PREVIEW_SCREEN_SIZE_COLUMN)));

    ui->settingsTableView->setModel(tableModel);

    ui->settingsTableView->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
}

void PreviewSettingsDialog::ReloadSettings()
{
    if (tableModel->rowCount() > 0)
    {
        tableModel->removeRows(0, tableModel->rowCount());
    }

    const List<PreviewSettingsData>& settings = PreviewController::Instance()->GetPreviewSettings();
    for (List<PreviewSettingsData>::const_iterator iter = settings.begin(); iter != settings.end(); iter ++)
    {
        const PreviewSettingsData& curItem = (*iter);
        QList<QStandardItem *> itemsList;

        // First item in the list should contain both item name and preview settings ID.
        QStandardItem* nameItem = new QStandardItem(QString::fromAscii(curItem.deviceName.c_str()));
        
        QVariant settingsValue;
        nameItem->setData(QVariant(curItem.id));
        itemsList.append(nameItem);

        QString deviceResolution = QString("%1x%2").arg(curItem.screenSize.x).arg(curItem.screenSize.y);
        itemsList.append(new QStandardItem(deviceResolution));
        itemsList.append(new QStandardItem(QString::number(curItem.dpi)));
        itemsList.append(new QStandardItem(QString::number(ScreenParamsConverter::ConverDPIToDiagonal(curItem.screenSize, curItem.dpi), 'f', 2)));

        tableModel->appendRow(itemsList);
    }
}

PreviewSettingsData PreviewSettingsDialog::GetSelectedData() const
{
    QItemSelectionModel *select = ui->settingsTableView->selectionModel();
    if (!select->hasSelection())
    {
        DVASSERT(false); // should not happen - validator should not allow this.
        return PreviewSettingsData();
    }
    
	QModelIndexList selectedIndexes = select->selectedIndexes();
    QStandardItem* selectedItem = tableModel->itemFromIndex(selectedIndexes.value(0));
    
    return PreviewController::Instance()->GetPreviewSettingsData(selectedItem->data().toInt());
}

void PreviewSettingsDialog::AddButtonClicked()
{
    EditPreviewSettingsDialog* dialog = new EditPreviewSettingsDialog(this);
    if (dialog->exec() != QDialog::Accepted)
    {
        delete dialog;
        return;
    }
    
    PreviewSettingsData data = dialog->GetData();
    PreviewController::Instance()->AddPreviewSettingsData(data);
    delete dialog;

    ReloadSettings();
}

void PreviewSettingsDialog::RemoveButtonClicked()
{
    const PreviewSettingsData& selectedData = GetSelectedData();
    if (selectedData.isPredefined)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("This device is predefined and cannot be deleted."));
        msgBox.exec();
        return;
    }
    
    PreviewController::Instance()->RemovePreviewSettingsData(selectedData.id);
    ReloadSettings();
}

void PreviewSettingsDialog::SettingsTableDoubleClicked(const QModelIndex& /*modelIndex*/)
{
    if (!isSelectionMode)
    {
        return; // probably open Edit dialog here.
    }

    // Allow user to select item by double-clicking on it.
    if (!ui->settingsTableView->selectionModel()->hasSelection())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Please select one of the devices to continue."));
        msgBox.exec();
        
        return;
    }

    accept();
}

void PreviewSettingsDialog::CloseButtonClicked()
{
    reject();
}
