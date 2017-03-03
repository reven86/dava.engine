#pragma once

#include "DAVAEngine.h"

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui
{
class FMODSoundBrowser;
}

class FMODSoundBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit FMODSoundBrowser(QWidget* parent = 0);
    virtual ~FMODSoundBrowser();

    void SetCurrentEvent(const DAVA::String& eventPath);
    DAVA::String GetSelectSoundEvent();

private slots:
    void OnEventSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void OnEventDoubleClicked(QTreeWidgetItem* item, int column);

    void OnAccepted();
    void OnRejected();

private:
    void UpdateEventTree();

    void FillEventsTree(const DAVA::Vector<DAVA::String>& names);
    void SelectItemAndExpandTreeByEventName(const DAVA::String& eventName);

    void SetSelectedItem(QTreeWidgetItem* item);

    QTreeWidgetItem* selectedItem;
    Ui::FMODSoundBrowser* ui;
};
