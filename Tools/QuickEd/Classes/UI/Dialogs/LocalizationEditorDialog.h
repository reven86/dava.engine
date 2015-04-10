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


#ifndef LOCALIZATIONEDITORDIALOG_H
#define LOCALIZATIONEDITORDIALOG_H

#include "Base/BaseTypes.h"
#include "ui_localizationeditordialog.h"

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelection>

#include <QTableView>

namespace Ui {
class LocalizationEditorDialog;
}

class LocalizationEditorDialog;


class LocalizationTableController : public QObject
{
    Q_OBJECT
    
public:
    // TODO: make private?
    static const int LOCALIZATION_KEY_INDEX;
    static const int LOCALIZATION_VALUE_INDEX;
    
    static const QString DEFAULT_LOCALIZATION_KEY;
    static const QString DEFAULT_LOCALIZATION_VALUE;
    //
    
    LocalizationTableController(QTableView* view);
    virtual ~LocalizationTableController();
    
    virtual void SetupTable(QObject *parent = 0);
    virtual void ReloadTable();
    
    virtual void ApplySortOrder(int headerIndex);
    
    virtual bool IsValidSelection();
    
    // connect to signals
    virtual void ConnectToSignals();
    virtual void DisconnectFromSignals();
    
protected:
    virtual void CreateTableModel(QObject *parent = 0) = 0;
    virtual bool LoadTable() = 0;
    virtual void CleanupUIControls() = 0;
    virtual void UpdateUIControls(const QModelIndex &selectedIndex) = 0;
    
    // Store the changes for the item being deselected.
    virtual void ProcessDeselectedItem(const QItemSelection & deselected);
    virtual void OnDeselectedItem(const QModelIndex & deselectedIndex);
    
    // Update the UI with the item currently selected.
    virtual void ProcessSelectedItem(const QItemSelection & selected);
    virtual void OnSelectedItem(const QModelIndex & selectedIndex);
    
    // Select the item in table by its key.
	virtual void SelectStringItemByKey(const QString& keyToBeSelected);
    
protected slots:
    // A string in the localization table is selected.
    virtual void OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected);
    // Table View header is clicked. Needed for sorting.
    virtual void OnTableHeaderClicked(int headerIndex);
    
    // Custom menu needs to be displayed.
    virtual void OnShowCustomMenu(const QPoint& /*pos*/);
    
    // Filter functionality.
    void OnFilterTextChanged(const QString& value);
    void OnFilterTextCleared();
    
protected:
    QTableView* tableView;
    QStandardItemModel* tableModel;
    
    // Current sort order.
    Qt::SortOrder sortOrder;
    
    // Current filter value.
    QString filterValue;
};

class LocalizationStringsTableController : public LocalizationTableController
{
public:
    LocalizationStringsTableController(QTableView* view, Ui::LocalizationEditorDialog *dialog);
    virtual ~LocalizationStringsTableController();
    
    // Update the Localization System with key and value currently set.
    virtual void UpdateLocalizationValueForCurrentKey();
    virtual void UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex);
    
    // Localization strings functions
    // Add/remove localization string.
    QString GetSelectedLocalizationKey();
	void AddNewLocalizationString(const QString &newLocalizationKey, const QString &newLocalizationValue);
	void RemoveSelectedLocalizationString(const QString &localizationKey);
    
    // Save the content of the Localization System.
	virtual void SaveLocalization();
    
protected:
    virtual void CreateTableModel(QObject *parent = 0);
    virtual bool LoadTable();
    virtual void CleanupUIControls();
    virtual void UpdateUIControls(const QModelIndex &selectedIndex);
    
    virtual void OnDeselectedItem(const QModelIndex & deselectedIndex);
    
protected:
    Ui::LocalizationEditorDialog *ui;
};

class LocalizationFontsTableController : public LocalizationTableController
{
public:
    LocalizationFontsTableController(QTableView* view, Ui::LocalizationEditorDialog *dialog);
    virtual ~LocalizationFontsTableController();
    
protected:
    virtual void CreateTableModel(QObject *parent = 0);
    virtual bool LoadTable();
    virtual void CleanupUIControls();
    virtual void UpdateUIControls(const QModelIndex &selectedIndex);
    
//TODO: invent something better than static int (ex. enum)
    static const int LOCALIZATION_FONT_SIZE_INDEX;
    static const QString DEFAULT_LOCALIZATION_FONT_SIZE;
    
protected:
    Ui::LocalizationEditorDialog *ui;
};

class LocalizationEditorDialog : public QDialog, public Ui::LocalizationEditorDialog
{
    Q_OBJECT
    
public:
    explicit LocalizationEditorDialog(QWidget *parent = 0);
    ~LocalizationEditorDialog();
    void SetDefaultLanguage();
signals:
    void LanguageChanged();
protected:
	// Setup the Localization Table Model.
//	void SetupLocalizationTable();
	
	// Reload the Localization Table each time Current Locale or Filter is changed.
//    void ReloadLocalizationTable();

	// Cleanup the UI controls related to selection.
//	void CleanupLocalizationUIControls();

	// Store the changes for the string being deselected.
//	void ProcessDeselectedString(const QItemSelection & deselected);
    
	// Update the UI with the string currently selected.
//	void ProcessSelectedString(const QItemSelection & selected);
	
	// Update the Localization System with key and value currently set.
//	void UpdateLocalizationValueForCurrentKey();
//	void UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex);

    //TODO: Update the Font Manager with key and value currently set.
    
	virtual void closeEvent(QCloseEvent *event);

	// Add/remove localization string.
	void AddNewLocalizationString();
	void RemoveSelectedLocalizationString();

//	// Select the item in table by its key.
//	void SelectStringItemByKey(const QString& keyToBeSelected);

private:
    void FillLocaleComboBox();
    
    void ConnectToSignals();
    void DisconnectFromSignals();
    
    void SetLocalizationDirectoryPath();

    void ReinitializeLocalizationSystem(const QString& localizationDirectory);

//	void ApplySortOrder(int headerIndex);

//	// Helper methods to convert between QString and WideString.
//	// Don't use QString::fromStdWstring() here, it is not compatible with "treat wchar_t as embedded"
//	// option currently set for Framework.
//	QString WideStringToQString(const DAVA::WideString& str);
//	DAVA::WideString QStringToWideString(const QString& str);

private slots:
    void OnOpenLocalizationFileButtonClicked();
    void OnCurrentLocaleChanged(int index);
  
	// A string in the localization table is selected.
//	void OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected);
	
	// Table View header is clicked. Needed for sorting.
//	void OnTableHeaderClicked(int headerIndex);

	// Custom menu needs to be displayed.
//	void OnShowCustomMenu(const QPoint& pos);

	// A localization string should be added/removed.
	void OnAddLocalizationStringAction();
	void OnRemoveLocalizationStringAction();

	// Save the content of the Localization System.
//	void SaveLocalization();

	// A dialog is closed.
	void CloseDialog();

	// Add/Remove String slots.
	void OnAddNewLocalizationString();
	void OnRemoveSelectedLocalizationString();
    
    // Filter functionality.
//    void OnFilterTextChanged(const QString& value);
//    void OnFilterTextCleared();

private:
    LocalizationStringsTableController* stringsTable;
    LocalizationFontsTableController* fontsTable;
	// Localization Table Models.
	//QStandardItemModel* stringsTableModel;
	
	// Current sort order.
	//Qt::SortOrder sortOrder;

	// Global index for the strings currently added.
	static int addedStringsCount;
    
    // Current filter value.
    //QString filterValue;
}; 

#endif // LOCALIZATIONEDITORDIALOG_H
