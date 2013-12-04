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

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelection>

namespace Ui {
class LocalizationEditorDialog;
}
    
class LocalizationEditorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LocalizationEditorDialog(QWidget *parent = 0);
    ~LocalizationEditorDialog();
    
protected:
	// Setup the Localization Table Model.
	void SetupLocalizationTable();
	
	// Reload the Localization Table each time Current Locale or Filter is changed.
    void ReloadLocalizationTable();

	// Cleanup the UI controls related to selection.
	void CleanupLocalizationUIControls();

	// Store the changes for the string being deselected.
	void ProcessDeselectedString(const QItemSelection & deselected);
	
	// Update the UI with the string currently selected.
	void ProcessSelectedString(const QItemSelection & selected);
	
	// Update the Localization System with key and value currently set.
	void UpdateLocalizationValueForCurrentKey();
	void UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex);

	virtual void closeEvent(QCloseEvent *event);

	// Add/remove localization string.
	void AddNewLocalizationString();
	void RemoveSelectedLocalizationString();

	// Select the item in table by its key.
	void SelectItemByKey(const QString& keyToBeSelected);

private:
	static const int LOCALIZATION_KEY_INDEX;
	static const int LOCALIZATION_VALUE_INDEX;
	
	static const QString DEFAULT_LOCALIZATION_KEY;
	static const QString DEFAULT_LOCALIZATION_VALUE;

    Ui::LocalizationEditorDialog *ui;
    void FillLocaleComboBox();
    void ConnectToSignals();
    void SetLocalizationDirectoryPath();
    void SetDefaultLanguage();

    void ReinitializeLocalizationSystem(const QString& localizationDirectory);

	void ApplySortOrder(int headerIndex);

	// Helper methods to convert between QString and WideString.
	// Don't use QString::fromStdWstring() here, it is not compatible with "treat wchar_t as embedded"
	// option currently set for Framework.
	QString WideStringToQString(const DAVA::WideString& str);
	DAVA::WideString QStringToWideString(const QString& str);

private slots:
    void OnOpenLocalizationFileButtonClicked();
    void OnCurrentLocaleChanged(int index);
  
	// A string in the localization table is selected.
	void OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected);
	
	// Table View header is clicked. Needed for sorting.
	void OnTableHeaderClicked(int headerIndex);

	// Custom menu needs to be displayed.
	void OnShowCustomMenu(const QPoint& pos);

	// An localization string should be added/removed.
	void OnAddLocalizationStringAction();
	void OnRemoveLocalizationStringAction();

	// Save the content of the Localization System.
	void SaveLocalization();

	// A dialog is closed.
	void CloseDialog();

	// Add/Remove String slots.
	void OnAddNewLocalizationString();
	void OnRemoveSelectedLocalizationString();
    
    // Filter functionality.
    void OnFilterTextChanged(const QString& value);
    void OnFilterTextCleared();

private:
	// Localization Table Model.
	QStandardItemModel* tableModel;
	
	// Current sort order.
	Qt::SortOrder sortOrder;

	// Global index for the strings currently added.
	static int addedStringsCount;
    
    // Current filter value.
    QString filterValue;
};

#endif // LOCALIZATIONEDITORDIALOG_H
