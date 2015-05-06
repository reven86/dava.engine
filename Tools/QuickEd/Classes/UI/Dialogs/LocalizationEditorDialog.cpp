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
#include "FileSystem/LocalizationSystem.h"
#include "Helpers/ResourcesManageHelper.h"

#include "regexpinputdialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QLocale>
#include "Utils/QtDavaConvertion.h"

#include "EditorCore.h"

using namespace DAVA;

//TODO: move to utils
// Helper methods to convert between QString and WideString.
// Don't use QString::fromStdWstring() here, it is not compatible with "treat wchar_t as embedded"
// option currently set for Framework.
const int LocalizationTableController::LOCALIZATION_KEY_INDEX = 0;
const int LocalizationTableController::LOCALIZATION_VALUE_INDEX = 1;

const QString LocalizationTableController::DEFAULT_LOCALIZATION_KEY = "LOCALIZATION_KEY_%1";
const QString LocalizationTableController::DEFAULT_LOCALIZATION_VALUE = "LOCALIZATION_VALUE_%1";


const int LocalizationFontsTableController::LOCALIZATION_FONT_SIZE_INDEX = 2;
const QString LocalizationFontsTableController::DEFAULT_LOCALIZATION_FONT_SIZE = "LOCALIZATION_FONT_SIZE_%1";

//---------------------------------------------------------------------

LocalizationTableController::LocalizationTableController(QTableView* view)
 : tableView(view)
 , tableModel(NULL)
 , sortOrder(Qt::AscendingOrder)
{
}

LocalizationTableController::~LocalizationTableController()
{
    SAFE_DELETE(tableModel);
} 

void LocalizationTableController::ConnectToSignals()
{
    // Change key and value when selected table item is changed.
    connect(tableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,
            SLOT(OnLocalizationStringSelected(const QItemSelection &, const QItemSelection &))
            );

    // Connect to column click to change items order.
    tableView->horizontalHeader()->setSortIndicatorShown(true);
    connect(tableView->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(OnTableHeaderClicked(int)));

    // Connect to the table view to show custom menu.
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
}

void LocalizationTableController::DisconnectFromSignals()
{
    // Change key and value when selected table item is changed.
    disconnect(tableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,
            SLOT(OnLocalizationStringSelected(const QItemSelection &, const QItemSelection &))
            );
    
    // Connect to column click to change items order.
    disconnect(tableView->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(OnTableHeaderClicked(int)));
    
    // Connect to the table view to show custom menu.
    disconnect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
}

void LocalizationTableController::SetupTable(QObject *parent)
{
    //Setup table appearence
    tableView->verticalHeader()->hide();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setSectionResizeMode/*setResizeMode*/(QHeaderView::Stretch);
    //Disable editing of table
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    CreateTableModel(parent);
//    //TODO: Create and set table view model
}

void LocalizationTableController::ReloadTable()
{
	//Remember the selected item (if any) to restore it when the selection will be changed.
	QModelIndex selectedItemIndex = QModelIndex();
	QItemSelectionModel* selectionModel = tableView->selectionModel();
	if (selectionModel && selectionModel->hasSelection() && selectionModel->selectedIndexes().size() > 0)
	{
		selectedItemIndex = selectionModel->selectedIndexes().takeAt(0);
	}
    
	// Do the cleanup.
	CleanupUIControls();
    if (tableModel->rowCount() > 0)
    {
        tableModel->removeRows(0, tableModel->rowCount());
    }
    
    if(LoadTable())
    {
        // Restore the selection if possible.
        if (selectedItemIndex.isValid())
        {
            tableView->selectRow(selectedItemIndex.row());
        }
    }
}

bool LocalizationTableController::IsValidSelection()
{
	QItemSelectionModel* selectionModel = tableView->selectionModel();
    if (selectionModel && selectionModel->hasSelection() && (selectionModel->selectedIndexes().size() > 0))
	{
		return true;
	}
    return false;
}

void LocalizationTableController::ProcessDeselectedItem(const QItemSelection & deselected)
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
    
    OnDeselectedItem(deselectedIndex);
}

void LocalizationTableController::OnDeselectedItem(const QModelIndex & deselectedIndex)
{
    //TODO: same deselect logic for strings and fonts?
}

void LocalizationTableController::ProcessSelectedItem(const QItemSelection & selected)
{
	CleanupUIControls();
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
	
    OnSelectedItem(selectedIndex);
}

void LocalizationTableController::OnSelectedItem(const QModelIndex & selectedIndex)
{
    UpdateUIControls(selectedIndex);
}

void LocalizationTableController::OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected)
{
	ProcessDeselectedItem(deselected);
	ProcessSelectedItem(selected);
}

void LocalizationTableController::OnTableHeaderClicked(int headerIndex)
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

void LocalizationTableController::ApplySortOrder(int headerIndex)
{
	tableView->horizontalHeader()->setSortIndicator(headerIndex, sortOrder);
	tableView->sortByColumn(headerIndex, sortOrder);
}

void LocalizationTableController::OnShowCustomMenu(const QPoint& /*pos*/)
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

void LocalizationTableController::OnFilterTextChanged(const QString& value)
{
    filterValue = value;
    ReloadTable();
}

void LocalizationTableController::OnFilterTextCleared()
{
    filterValue.clear();
    QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(QObject::sender());
    if(senderWidget)
    {
        senderWidget->clear();
    }
}


void LocalizationTableController::SelectStringItemByKey(const QString& keyToBeSelected)
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
			tableView->selectRow(i);
			break;
		}
	}
}

//---------------------------------------------------------------------

LocalizationStringsTableController::LocalizationStringsTableController(QTableView* view, Ui::LocalizationEditorDialog *dialog)
: LocalizationTableController(view)
, ui(dialog)
{
    
}

LocalizationStringsTableController::~LocalizationStringsTableController()
{

}

void LocalizationStringsTableController::CreateTableModel(QObject* parent)
{
    //Create and set table view model
    tableModel = new QStandardItemModel(parent); //0 Rows and 2 Columns
    
    //Setup column name
    tableModel->setHorizontalHeaderItem(LOCALIZATION_KEY_INDEX, new QStandardItem(QString("Key")));
    tableModel->setHorizontalHeaderItem(LOCALIZATION_VALUE_INDEX, new QStandardItem(QString("Localized Value")));
    
    tableView->setModel(tableModel);
}

bool LocalizationStringsTableController::LoadTable()
{
	Map<WideString, WideString> localizationTable;
	bool stringsFound = LocalizationSystem::Instance()->GetStringsForCurrentLocale(localizationTable);
	if (!stringsFound)
	{
		return false;
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
    return true;
}

void LocalizationStringsTableController::UpdateUIControls(const QModelIndex &selectedIndex)
{
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
	QString localizationValue = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_VALUE_INDEX), Qt::DisplayRole).toString();
    
	ui->keyTextEdit->insertPlainText(localizationKey);
    ui->valueTextEdit->insertPlainText(localizationValue);
}

void LocalizationStringsTableController::CleanupUIControls()
{
    ui->keyTextEdit->clear();
    ui->valueTextEdit->clear();
}

void LocalizationStringsTableController::OnDeselectedItem(const QModelIndex & deselectedIndex)
{
	UpdateLocalizationValueForCurrentKey(deselectedIndex);
}

void LocalizationStringsTableController::UpdateLocalizationValueForCurrentKey()
{
	// Update the value for the item currently selected by default.
	QModelIndex selectedItemIndex = QModelIndex();
	QItemSelectionModel* selectionModel = tableView->selectionModel();
	if (!selectionModel || !selectionModel->hasSelection() ||
		!(selectionModel->selectedIndexes().size() > 0))
	{
		return;
	}
	
	selectedItemIndex = selectionModel->selectedIndexes().takeAt(0);
	UpdateLocalizationValueForCurrentKey(selectedItemIndex);
}

void LocalizationStringsTableController::UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex)
{
	if (!selectedItemIndex.isValid())
	{
		return;
	}
    
	// Firstly verify if something was changed.
    QString localizationKey = ui->keyTextEdit->toPlainText();
    QString localizationValue = ui->valueTextEdit->toPlainText();
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
    //HierarchyTreeController::Instance()->UpdateLocalization(true);
}

void LocalizationStringsTableController::SaveLocalization()
{
	LocalizationSystem::Instance()->SaveLocalizedStrings();
}

QString LocalizationStringsTableController::GetSelectedLocalizationKey()
{
    QModelIndex selectedIndex = QModelIndex();
	QItemSelectionModel* selectionModel = tableView->selectionModel();
	if (selectionModel && selectionModel->hasSelection() && selectionModel->selectedIndexes().size() > 0)
	{
		selectedIndex = selectionModel->selectedIndexes().takeAt(0);
	}
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
    
    return localizationKey;
}

void LocalizationStringsTableController::AddNewLocalizationString(const QString &newLocalizationKey, const QString &newLocalizationValue)
{
    LocalizationSystem::Instance()->SetLocalizedString(QStringToWideString(newLocalizationKey),
                                                   QStringToWideString(newLocalizationValue));
    SaveLocalization();
    ReloadTable();

    SelectStringItemByKey(newLocalizationKey);
}

void LocalizationStringsTableController::RemoveSelectedLocalizationString(const QString &localizationKey)
{
    
	LocalizationSystem::Instance()->RemoveLocalizedString(QStringToWideString(localizationKey));
	SaveLocalization();
    
	ReloadTable();
}

//---------------------------------------------------------------------
LocalizationFontsTableController::LocalizationFontsTableController(QTableView* view, Ui::LocalizationEditorDialog *dialog)
: LocalizationTableController(view)
, ui(dialog)
{
    
}

LocalizationFontsTableController::~LocalizationFontsTableController()
{
    
}

void LocalizationFontsTableController::CreateTableModel(QObject* parent)
{
    //Create and set table view model
    tableModel = new QStandardItemModel(0, 2, parent); //0 Rows and 3 Columns
    
    //Setup column name
    tableModel->setHorizontalHeaderItem(LOCALIZATION_KEY_INDEX, new QStandardItem(QString("Font Preset Name")));
    tableModel->setHorizontalHeaderItem(LOCALIZATION_VALUE_INDEX, new QStandardItem(QString("Localized Font")));
    //tableModel->setHorizontalHeaderItem(LOCALIZATION_FONT_SIZE_INDEX, new QStandardItem(QString("Font Size")));
    
    tableView->setModel(tableModel);
}

bool LocalizationFontsTableController::LoadTable()
{
    const String &locale = LocalizationSystem::Instance()->GetCurrentLocale();
    Logger::Debug("LocalizationFontsTableController::LoadTable locale=%s", locale.c_str());
    //const Map<String, Font*> &localizationFonts = EditorFontSystem::Instance()->GetLocalizedFonts(locale);
    return false;
	/*if (localizationFonts.empty())
	{
		return false;
	}
    
	// Fill the values.
    Map<String, Font*>::const_iterator it = localizationFonts.begin();
    Map<String, Font*>::const_iterator endIt = localizationFonts.end();
    
	for (; it != endIt; ++it)
	{
        // Add only strings which pass filter (or all strings if filter is not defined).
        QString keyValue(it->first.c_str());
        if (filterValue.isEmpty() || keyValue.contains(filterValue, Qt::CaseInsensitive))
        {
            //QString fontName = QString::fromStdString(EditorFontSystem::Instance()->GetFontDisplayName(font));
            //QString fontSize = QString("%1").arg(font->GetSize());
            
            QList<QStandardItem *> itemsList;
            itemsList.append(new QStandardItem(keyValue));
            //TODO : restore it or remove it
            itemsList.append(new QStandardItem("not implemented"));
            //itemsList.append(new QStandardItem(fontSize));
            tableModel->appendRow(itemsList);
        }
    }
    return true;*/
}

void LocalizationFontsTableController::UpdateUIControls(const QModelIndex &selectedIndex)
{
    //TODO: update ui controls on fonts tab
    
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
	QString localizationValue = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_VALUE_INDEX), Qt::DisplayRole).toString();
//    
//	ui->keyTextEdit->insertPlainText(localizationKey);
//	ui->valueTextEdit->insertPlainText(localizationValue);
}

void LocalizationFontsTableController::CleanupUIControls()
{
    //TODO: cleanup ui controls on fonts tab
//	ui->keyTextEdit->clear();
//	ui->valueTextEdit->clear();
}

//---------------------------------------------------------------------

int LocalizationEditorDialog::addedStringsCount = 0;


LocalizationEditorDialog::LocalizationEditorDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    stringsTable = new LocalizationStringsTableController(stringsTableView, this);
    fontsTable = new LocalizationFontsTableController(fontsTableView, this);
    
    stringsTable->SetupTable();
    fontsTable->SetupTable();
    
    ConnectToSignals();
    SetLocalizationDirectoryPath();
    
//     HierarchyTreePlatformNode* platformNode = NULL;//HierarchyTreeController::Instance()->GetActivePlatform();
// 	if (platformNode)
// 	{
//     	QString platformName = platformNode ? platformNode->GetName() : "none";
//     	QString windowTitle = QString("Localization settings for platform \"%1\"").arg(platformName);
//     	setWindowTitle(windowTitle);
// 		// Enable open localization dialog button only if platfrom is available
// 		ui->openLocalizationFileButton->setEnabled(true);
// 
// 		// Apply default sort order when the loading is complete.
// 		stringsTable->ApplySortOrder(LocalizationTableController::LOCALIZATION_KEY_INDEX);
//         fontsTable->ApplySortOrder(LocalizationTableController::LOCALIZATION_KEY_INDEX);
// 	}
}

LocalizationEditorDialog::~LocalizationEditorDialog()
{
    
    delete stringsTable;
    delete fontsTable;
}

void LocalizationEditorDialog::FillLocaleComboBox()
{
    currentLocaleComboBox->blockSignals(true);
    currentLocaleComboBox->clear();
    // Get count of supported languages
    const auto &locales = GetEditorLocalizationSystem()->GetAvailableLocales();
    for (const auto &localeName : locales)    // Fill combobox with language values
    {        
        QLocale locale(localeName);
        QString lang;
        switch (locale.script())
        {
        default: 
            lang = QLocale::languageToString(locale.language());
        break;
        case QLocale::SimplifiedChineseScript: 
            lang = "Chinese simpl.";
        break;
        case QLocale::TraditionalChineseScript:
            lang = "Chinese trad.";
        break;
        }
        currentLocaleComboBox->addItem(lang);
    }
    currentLocaleComboBox->blockSignals(false);
    // Setup default locale
    UpdateDefaultLanguage();
}

void LocalizationEditorDialog::ConnectToSignals()
{
    stringsTable->ConnectToSignals();
    fontsTable->ConnectToSignals();
    
    // Open locale directory button clicked event
    connect(openLocalizationFileButton, SIGNAL(clicked()), this, SLOT(OnOpenLocalizationFileButtonClicked()));
    // Locale combobox value changed event
    connect(currentLocaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentLocaleChanged(int)));
    // Close dialog if ok button clicked
    connect(closeButton, SIGNAL(clicked()), this, SLOT(CloseDialog()));
	
	connect(addStringButton, SIGNAL(clicked()), this, SLOT(OnAddNewLocalizationString()));
	connect(removeStringButton, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedLocalizationString()));
    
    // Filter behaviour.
    connect(filterLineEdit, SIGNAL(textChanged(const QString&)), this->stringsTable, SLOT(OnFilterTextChanged(const QString&)));
    connect(clearFilterButton, SIGNAL(clicked()), this->stringsTable, SLOT(OnFilterTextCleared()));
    
    connect(fontsFilterLineEdit, SIGNAL(textChanged(const QString&)), this->fontsTable, SLOT(OnFilterTextChanged(const QString&)));
    connect(clearFontsFilterButton, SIGNAL(clicked()), this->fontsTable, SLOT(OnFilterTextCleared()));
}

void LocalizationEditorDialog::DisconnectFromSignals()
{
    stringsTable->DisconnectFromSignals();
    fontsTable->DisconnectFromSignals();
    
    // Open locale directory button clicked event
    disconnect(openLocalizationFileButton, SIGNAL(clicked()), this, SLOT(OnOpenLocalizationFileButtonClicked()));
    // Locale combobox value changed event
    disconnect(currentLocaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentLocaleChanged(int)));
    // Close dialog if ok button clicked
    disconnect(closeButton, SIGNAL(clicked()), this, SLOT(CloseDialog()));
	
	disconnect(addStringButton, SIGNAL(clicked()), this, SLOT(OnAddNewLocalizationString()));
	disconnect(removeStringButton, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedLocalizationString()));
    
    // Filter behaviour.
    disconnect(filterLineEdit, SIGNAL(textChanged(const QString&)), this->stringsTable, SLOT(OnFilterTextChanged(const QString&)));
    disconnect(clearFilterButton, SIGNAL(clicked()), this->stringsTable, SLOT(OnFilterTextCleared()));
    
    disconnect(fontsFilterLineEdit, SIGNAL(textChanged(const QString&)), this->fontsTable, SLOT(OnFilterTextChanged(const QString&)));
    disconnect(clearFontsFilterButton, SIGNAL(clicked()), this->fontsTable, SLOT(OnFilterTextCleared()));
}

void LocalizationEditorDialog::SetLocalizationDirectoryPath()
{
    QString defaultPath = QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath().GetAbsolutePathname());
    if (!defaultPath.isEmpty())
    {
        localizationFilePathLineEdit->setText(defaultPath);
		stringsTable->ReloadTable();
        fontsTable->ReloadTable();
    }
}

void LocalizationEditorDialog::UpdateDefaultLanguage()
{
    // Get description for current language ID
    auto id = LocalizationSystem::Instance()->GetCurrentLocale();
    const auto &locales = GetEditorLocalizationSystem()->GetAvailableLocales();
    auto it = std::find(locales.begin(), locales.end(), QString::fromStdString(id));
    auto index = std::distance(locales.begin(), it); // this is works only for arrays and vectors
    currentLocaleComboBox->setCurrentIndex(index);
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
		fileDirectory = QDir::toNativeSeparators(fileDirectory);

		if (ResourcesManageHelper::ValidateResourcePath(fileDirectory))
        {
			// Get localization relative path
			QString localizationRelativePath = ResourcesManageHelper::GetResourceRelativePath(fileDirectory);
      		// Update ui
      		localizationFilePathLineEdit->setText(localizationRelativePath);
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
	stringsTable->UpdateLocalizationValueForCurrentKey();

    int languageItemID = currentLocaleComboBox->currentIndex();
    if (languageItemID == -1)
    {
        return;
    }

    const auto &locales = GetEditorLocalizationSystem()->GetAvailableLocales();
    QString locale = locales.at(languageItemID);
    // Re-initialize the Localization System with the new Locale.
    GetEditorLocalizationSystem()->Cleanup();
    
    if (!localizationDirectory.isEmpty())
    {
        FilePath localizationFilePath(localizationDirectory.toStdString());
        localizationFilePath.MakeDirectoryPathname();

        GetEditorLocalizationSystem()->InitLanguageWithDirectory(localizationFilePath, locale.toStdString());
    }
    
	stringsTable->ReloadTable();
    fontsTable->ReloadTable();    
//    HierarchyTreeController::Instance()->UpdateLocalization(true);
}

void LocalizationEditorDialog::closeEvent(QCloseEvent* /*event*/)
{
	// Save the last-minute changes, if any.
//	UpdateLocalizationValueForCurrentKey();
    stringsTable->UpdateLocalizationValueForCurrentKey();
}

void LocalizationEditorDialog::CloseDialog()
{
//	UpdateLocalizationValueForCurrentKey();
    stringsTable->UpdateLocalizationValueForCurrentKey();
    
	accept();
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
	QString newLocalizationKey = QString(LocalizationFontsTableController::DEFAULT_LOCALIZATION_KEY).arg(addedStringsCount);
	QString newLocalizationValue = QString(LocalizationFontsTableController::DEFAULT_LOCALIZATION_VALUE).arg(addedStringsCount);

	bool isOK = false;
	QRegExp asciiRegExp("[ -~]+");
	QString text = RegExpInputDialog::getText(this, "Localization Key",
										 "New Localization Key (ASCII characters only)",
										 newLocalizationKey, asciiRegExp, &isOK);
	if (isOK && !text.isEmpty())
	{
		newLocalizationKey = text;
        
        stringsTable->AddNewLocalizationString(newLocalizationKey, newLocalizationValue);
	}
	else
	{
		// String ID wasn't used, return it.
		addedStringsCount --;
	}
}

void LocalizationEditorDialog::RemoveSelectedLocalizationString()
{
    if(!stringsTable->IsValidSelection())
    {
        return;
    }
    
    QString localizationKey = stringsTable->GetSelectedLocalizationKey();

	int ret = QMessageBox::warning(this, qApp->applicationName(),
								   QString("Are you sure you want to delete localization string with key '%1'?").arg(localizationKey),
								   QMessageBox::Ok | QMessageBox::Cancel,
								   QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
	{
		return;
	}

    stringsTable->RemoveSelectedLocalizationString(localizationKey);
}

void LocalizationEditorDialog::OnAddNewLocalizationString()
{
	AddNewLocalizationString();
}

void LocalizationEditorDialog::OnRemoveSelectedLocalizationString()
{
	RemoveSelectedLocalizationString();
}

