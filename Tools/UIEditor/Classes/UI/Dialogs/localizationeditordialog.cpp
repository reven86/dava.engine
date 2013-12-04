/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "localizationeditordialog.h"
#include "ui_localizationeditordialog.h"
#include "LocalizationSystemHelper.h"
#include "PropertiesGridController.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/FileSystem.h"
#include "ResourcesManageHelper.h"

#include "regexpinputdialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

using namespace DAVA;

const int LocalizationEditorDialog::LOCALIZATION_KEY_INDEX = 0;
const int LocalizationEditorDialog::LOCALIZATION_VALUE_INDEX = 1;

const QString LocalizationEditorDialog::DEFAULT_LOCALIZATION_KEY = "LOCALIZATION_KEY_%1";
const QString LocalizationEditorDialog::DEFAULT_LOCALIZATION_VALUE = "LOCALIZATION_VALUE_%1";

int LocalizationEditorDialog::addedStringsCount = 0;

LocalizationEditorDialog::LocalizationEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocalizationEditorDialog),
	tableModel(NULL),
	sortOrder(Qt::AscendingOrder)
{
    ui->setupUi(this);
	
	SetupLocalizationTable();
    FillLocaleComboBox();
    ConnectToSignals();
    SetLocalizationDirectoryPath();
    
    HierarchyTreePlatformNode* platformNode = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platformNode)
	{
    	QString platformName = platformNode ? platformNode->GetName() : "none";
    	QString windowTitle = QString("Localization settings for platform \"%1\"").arg(platformName);
    	setWindowTitle(windowTitle);
		// Enable open localization dialog button only if platfrom is available
		ui->openLocalizationFileButton->setEnabled(true);

		// Apply default sort order when the loading is complete.
		ApplySortOrder(LOCALIZATION_KEY_INDEX);
	}
}

LocalizationEditorDialog::~LocalizationEditorDialog()
{
    delete ui;
}

void LocalizationEditorDialog::FillLocaleComboBox()
{
    // Get count of supported languages
    int languagesCount = LocalizationSystemHelper::GetSupportedLanguagesCount();
    QString languageDescription;
    // Fill combobox with language values
    for (int i = 0; i < languagesCount; ++i) {
        languageDescription =  QString::fromStdString(LocalizationSystemHelper::GetSupportedLanguageDesc(i));
        ui->currentLocaleComboBox->addItem(languageDescription);
    }
    // Setup default locale
    SetDefaultLanguage();
}

void LocalizationEditorDialog::ConnectToSignals()
{
    // Open locale directory button clicked event
    connect(ui->openLocalizationFileButton, SIGNAL(clicked()), this, SLOT(OnOpenLocalizationFileButtonClicked()));
    // Locale combobox value changed event
    connect(ui->currentLocaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentLocaleChanged(int)));
    // Close dialog if ok button clicked
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(CloseDialog()));
	
	// Change key and value when selected table item is changed.
	connect(ui->tableView->selectionModel(),
			SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this,
			SLOT(OnLocalizationStringSelected(const QItemSelection &, const QItemSelection &))
			);
	
	// Connect to column click to change items order.
	ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
	connect(ui->tableView->horizontalHeader(), SIGNAL(sectionClicked(int)),
			this, SLOT(OnTableHeaderClicked(int)));

	// Connect to the table view to show custom menu.
	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
	
	connect(ui->addStringButton, SIGNAL(clicked()), this, SLOT(OnAddNewLocalizationString()));
	connect(ui->removeStringButton, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedLocalizationString()));
    
    // Filter behaviour.
    connect(ui->filterTextBox, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTextChanged(const QString&)));
    connect(ui->clearFilterButton, SIGNAL(clicked()), this, SLOT(OnFilterTextCleared()));
}

void LocalizationEditorDialog::SetLocalizationDirectoryPath()
{
    QString defaultPath = QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath().GetAbsolutePathname());
    if (!defaultPath.isEmpty())
    {
        ui->localizationFilePathLineEdit->setText(defaultPath);
		ReloadLocalizationTable();
    }
}

void LocalizationEditorDialog::SetDefaultLanguage()
{
    // Get description for current language ID
    String currentLanguageID = LocalizationSystem::Instance()->GetCurrentLocale();
    String languageDescription = LocalizationSystemHelper::GetLanguageDescByLanguageID(currentLanguageID);

    // Setup combo box value
    int index = ui->currentLocaleComboBox->findText(QString::fromStdString(languageDescription));
    ui->currentLocaleComboBox->setCurrentIndex(index);
}

void LocalizationEditorDialog::OnOpenLocalizationFileButtonClicked()
{
	FilePath relativeLocalizationPath = LocalizationSystem::Instance()->GetDirectoryPath();
	QString absoluteLocalizationPath = QString::fromStdString(relativeLocalizationPath.GetAbsolutePathname());

	if (absoluteLocalizationPath.isEmpty())
	{
		absoluteLocalizationPath = ResourcesManageHelper::GetResourceRootDirectory();
	}

    QString fileDirectory = QFileDialog::getExistingDirectory(this, tr( "Select localization files directory" ), absoluteLocalizationPath);

	if(!fileDirectory.isNull() && !fileDirectory.isEmpty())
    {
		// Convert directory path into Unix-style path
		fileDirectory = ResourcesManageHelper::ConvertPathToUnixStyle(fileDirectory);

		if (ResourcesManageHelper::ValidateResourcePath(fileDirectory))
        {
			// Get localization relative path
			QString localizationRelativePath = ResourcesManageHelper::GetResourceRelativePath(fileDirectory);
      		// Update ui
      		ui->localizationFilePathLineEdit->setText(localizationRelativePath);
     		ReinitializeLocalizationSystem(localizationRelativePath);
        }
		else
		{
			ResourcesManageHelper::ShowErrorMessage(fileDirectory);
		}
     }
}

void LocalizationEditorDialog::OnCurrentLocaleChanged(int /*index*/)
{
    ReinitializeLocalizationSystem(QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath().GetAbsolutePathname()));
}

void LocalizationEditorDialog::ReinitializeLocalizationSystem(const QString& localizationDirectory)
{
	// Store the latest changes before the reinitialization.
	UpdateLocalizationValueForCurrentKey();

    int languageItemID = this->ui->currentLocaleComboBox->currentIndex();
    if (languageItemID == -1)
    {
        return;
    }

    String languageId = LocalizationSystemHelper::GetSupportedLanguageID(languageItemID);
    
    // Re-initialize the Localization System with the new Locale.
    LocalizationSystem::Instance()->Cleanup();
    
    if (!localizationDirectory.isEmpty())
    {
        FilePath localizationFilePath(localizationDirectory.toStdString());
        localizationFilePath.MakeDirectoryPathname();

        LocalizationSystem::Instance()->SetCurrentLocale(languageId);
        LocalizationSystem::Instance()->InitWithDirectory(localizationFilePath);
    }
    
	ReloadLocalizationTable();
    HierarchyTreeController::Instance()->UpdateLocalization(true);
}

void LocalizationEditorDialog::SetupLocalizationTable()
{
    //Setup table appearence
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    //Disable editing of table
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Create and set table view model
    this->tableModel = new QStandardItemModel(this); //0 Rows and 2 Columns
	
    //Setup column name
    tableModel->setHorizontalHeaderItem(LOCALIZATION_KEY_INDEX, new QStandardItem(QString("Key")));
    tableModel->setHorizontalHeaderItem(LOCALIZATION_VALUE_INDEX, new QStandardItem(QString("Localized Value")));
	
    ui->tableView->setModel(tableModel);
}

void LocalizationEditorDialog::ReloadLocalizationTable()
{
	//Remember the selected item (if any) to restore it when the selection will be changed.
	QModelIndex selectedItemIndex = QModelIndex();
	QItemSelectionModel* selectionModel = ui->tableView->selectionModel();
	if (selectionModel && selectionModel->hasSelection() && selectionModel->selectedIndexes().size() > 0)
	{
		selectedItemIndex = selectionModel->selectedIndexes().takeAt(0);
	}

	// Do the cleanup.
	CleanupLocalizationUIControls();
    if (tableModel->rowCount() > 0)
    {
        tableModel->removeRows(0, tableModel->rowCount());
    }
    
	Map<WideString, WideString> localizationTable;
	bool stringsFound = LocalizationSystem::Instance()->GetStringsForCurrentLocale(localizationTable);
	if (!stringsFound)
	{
		return;
	}

	// Fill the values.
	for (Map<WideString, WideString>::iterator iter = localizationTable.begin(); iter != localizationTable.end(); iter ++)
	{
        // Add only strings which pass filter (or all strings if filter is not defined).
        QString keyValue = WideStringToQString(iter->first);
        if (filterValue.isEmpty() || keyValue.contains(filterValue, Qt::CaseInsensitive))
        {
            QList<QStandardItem *> itemsList;
            itemsList.append(new QStandardItem(keyValue));
            itemsList.append(new QStandardItem(WideStringToQString(iter->second)));
            tableModel->appendRow(itemsList);
        }
    }
	
	// Restore the selection if possible.
	if (selectedItemIndex.isValid())
	{
		ui->tableView->selectRow(selectedItemIndex.row());
	}
}

void LocalizationEditorDialog::OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected)
{
	ProcessDeselectedString(deselected);
	ProcessSelectedString(selected);
}

void LocalizationEditorDialog::CleanupLocalizationUIControls()
{
	ui->txtLocalizationKey->clear();
	ui->txtLocalizationValue->clear();
}

void LocalizationEditorDialog::ProcessDeselectedString(const QItemSelection & deselected)
{
	// Just do update.
	if (deselected.indexes().size() == 0)
	{
		// Nothing is selected - nothing to do more.
		return;
	}
	
	QModelIndex deselectedIndex = deselected.indexes().takeAt(0);
	if (!deselectedIndex.isValid())
	{
		return;
	}

	UpdateLocalizationValueForCurrentKey(deselectedIndex);
}

void LocalizationEditorDialog::ProcessSelectedString(const QItemSelection & selected)
{
	CleanupLocalizationUIControls();
	if (selected.indexes().size() == 0)
	{
		// Nothing is selected - nothing to do more.
		return;
	}
	
	QModelIndex selectedIndex = selected.indexes().takeAt(0);
	if (!selectedIndex.isValid())
	{
		return;
	}
	
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
	QString localizationValue = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_VALUE_INDEX), Qt::DisplayRole).toString();

	ui->txtLocalizationKey->insertPlainText(localizationKey);
	ui->txtLocalizationValue->insertPlainText(localizationValue);
}

void LocalizationEditorDialog::UpdateLocalizationValueForCurrentKey()
{
	// Update the value for the item currently selected by default.
	QModelIndex selectedItemIndex = QModelIndex();
	QItemSelectionModel* selectionModel = ui->tableView->selectionModel();
	if (!selectionModel || !selectionModel->hasSelection() ||
		!(selectionModel->selectedIndexes().size() > 0))
	{
		return;
	}
	
	selectedItemIndex = selectionModel->selectedIndexes().takeAt(0);
	UpdateLocalizationValueForCurrentKey(selectedItemIndex);
}

void LocalizationEditorDialog::UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex)
{
	if (!selectedItemIndex.isValid())
	{
		return;
	}

	// Firstly verify if something was changed.
	QString localizationKey = ui->txtLocalizationKey->toPlainText();
	QString localizationValue = ui->txtLocalizationValue->toPlainText();
	if (localizationKey.isEmpty())
	{
		return;
	}
	
	QString existingLocalizationValue = tableModel->data(tableModel->index(selectedItemIndex.row(), 1), Qt::DisplayRole).toString();
	if (existingLocalizationValue == localizationValue)
	{
		return;
	}

	// Change indeed happened - update the localized string.
	LocalizationSystem::Instance()->SetLocalizedString(QStringToWideString(localizationKey),
													   QStringToWideString(localizationValue));
	SaveLocalization();

	// Update the current localized string in the table.
	tableModel->setData(tableModel->index(selectedItemIndex.row(), 1), localizationValue);

	// Update the UI.
    HierarchyTreeController::Instance()->UpdateLocalization(true);
}

void LocalizationEditorDialog::closeEvent(QCloseEvent* /*event*/)
{
	// Save the last-minute changes, if any.
	UpdateLocalizationValueForCurrentKey();
}

void LocalizationEditorDialog::CloseDialog()
{
	UpdateLocalizationValueForCurrentKey();
	accept();
}

void LocalizationEditorDialog::OnTableHeaderClicked(int headerIndex)
{
	// Revert the current sort order and re-apply it.
	if (sortOrder == Qt::DescendingOrder)
	{
		sortOrder = Qt::AscendingOrder;
	}
	else
	{
		sortOrder = Qt::DescendingOrder;
	}
	
	ApplySortOrder(headerIndex);
}

void LocalizationEditorDialog::ApplySortOrder(int headerIndex)
{
	ui->tableView->horizontalHeader()->setSortIndicator(headerIndex, sortOrder);
	ui->tableView->sortByColumn(headerIndex, sortOrder);
}

void LocalizationEditorDialog::OnShowCustomMenu(const QPoint& /*pos*/)
{
	QMenu menu;
	QAction* addStringAction = new QAction("Add string", &menu);
	connect(addStringAction, SIGNAL(triggered()), this, SLOT(OnAddLocalizationStringAction()));
	menu.addAction(addStringAction);

	QAction* removeStringAction = new QAction("Remove selected string", &menu);
	connect(removeStringAction, SIGNAL(triggered()), this, SLOT(OnRemoveLocalizationStringAction()));
	menu.addAction(removeStringAction);

	menu.exec(QCursor::pos());
}

void LocalizationEditorDialog::OnAddLocalizationStringAction()
{
	AddNewLocalizationString();
}

void LocalizationEditorDialog::OnRemoveLocalizationStringAction()
{
	RemoveSelectedLocalizationString();
}

void LocalizationEditorDialog::AddNewLocalizationString()
{
	addedStringsCount ++;
	QString newLocalizationKey = QString(DEFAULT_LOCALIZATION_KEY).arg(addedStringsCount);
	QString newLocalizationValue = QString(DEFAULT_LOCALIZATION_VALUE).arg(addedStringsCount);

	bool isOK = false;
	QRegExp asciiRegExp("[ -~]+");
	QString text = RegExpInputDialog::getText(this, "Localization Key",
										 "New Localization Key (ASCII characters only)",
										 newLocalizationKey, asciiRegExp, &isOK);
	if (isOK && !text.isEmpty())
	{
		newLocalizationKey = text;
		LocalizationSystem::Instance()->SetLocalizedString(QStringToWideString(newLocalizationKey),
													   QStringToWideString(newLocalizationValue));
		SaveLocalization();
		ReloadLocalizationTable();

		SelectItemByKey(newLocalizationKey);
	}
	else
	{
		// String ID wasn't used, return it.
		addedStringsCount --;
	}
}

void LocalizationEditorDialog::RemoveSelectedLocalizationString()
{
	QItemSelectionModel* selectionModel = ui->tableView->selectionModel();
	if (!selectionModel || !selectionModel->hasSelection() ||
		!(selectionModel->selectedIndexes().size() > 0))
	{
		return;
	}

	QModelIndex selectedIndex = selectionModel->selectedIndexes().takeAt(0);
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();

	int ret = QMessageBox::warning(this, qApp->applicationName(),
								   QString("Are you sure you want to delete localization string with key '%1'?").arg(localizationKey),
								   QMessageBox::Ok | QMessageBox::Cancel,
								   QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
	{
		return;
	}

	LocalizationSystem::Instance()->RemoveLocalizedString(QStringToWideString(localizationKey));
	SaveLocalization();

	ReloadLocalizationTable();
}

void LocalizationEditorDialog::SelectItemByKey(const QString& keyToBeSelected)
{
	int rowsCount = tableModel->rowCount();
	for (int i = 0; i < rowsCount; i ++)
	{
		QModelIndex rowIndex = tableModel->index(i, LOCALIZATION_KEY_INDEX);
		if (!rowIndex.isValid())
		{
			continue;
		}

		QString localizationKey = tableModel->data(rowIndex, Qt::DisplayRole).toString();
		if (localizationKey == keyToBeSelected)
		{
			ui->tableView->selectRow(i);
			break;
		}
	}
}

void LocalizationEditorDialog::SaveLocalization()
{
	LocalizationSystem::Instance()->SaveLocalizedStrings();
}

QString LocalizationEditorDialog::WideStringToQString(const DAVA::WideString& str)
{
	// Convert Wide String to UTF8 and create from it.
	String utf8String = UTF8Utils::EncodeToUTF8(str);
	return QString::fromUtf8(utf8String.c_str());
}

DAVA::WideString LocalizationEditorDialog::QStringToWideString(const QString& str)
{
	QByteArray utf8Array = str.toUtf8();
	WideString resultString;
	UTF8Utils::EncodeToWideString((uint8*)utf8Array.data(), utf8Array.size(), resultString);

	return resultString;
}

void LocalizationEditorDialog::OnAddNewLocalizationString()
{
	AddNewLocalizationString();
}

void LocalizationEditorDialog::OnRemoveSelectedLocalizationString()
{
	RemoveSelectedLocalizationString();
}

void LocalizationEditorDialog::OnFilterTextChanged(const QString& value)
{
    filterValue = value;
    ReloadLocalizationTable();
}

void LocalizationEditorDialog::OnFilterTextCleared()
{
    filterValue.clear();
    ui->filterTextBox->clear();
}

