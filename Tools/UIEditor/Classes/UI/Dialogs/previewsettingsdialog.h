#ifndef PREVIEWSETTINGSDIALOG_H
#define PREVIEWSETTINGSDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

#include "PreviewController.h"
using namespace DAVA;

namespace Ui {
class PreviewSettingsDialog;
}

class PreviewSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewSettingsDialog(bool selectionMode, QWidget *parent = 0);
    ~PreviewSettingsDialog();

    // Get the selected Preview Settings Data.
    PreviewSettingsData GetSelectedData() const;

protected:
    void InitializeTableView();
    void ReloadSettings();
    
protected slots:
    void AddButtonClicked();
    void RemoveButtonClicked();
    void CloseButtonClicked();
    void SettingsTableDoubleClicked(const QModelIndex& modelIndex);

private:
    Ui::PreviewSettingsDialog *ui;
    QStandardItemModel* tableModel;
    
    // true if this dialog is opened just for selection preview mode.
    bool isSelectionMode;
};

#endif // PREVIEWSETTINGSDIALOG_H
