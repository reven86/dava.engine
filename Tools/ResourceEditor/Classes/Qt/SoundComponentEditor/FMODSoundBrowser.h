#pragma once

#include "DAVAEngine.h"

#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/DataProcessing/DataListener.h"

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui
{
class FMODSoundBrowser;
}

class SceneEditor2;

class FMODSoundBrowser : public QDialog, public DAVA::Singleton<FMODSoundBrowser>, private DAVA::TArc::DataListener
{
    Q_OBJECT

public:
    explicit FMODSoundBrowser(QWidget* parent = 0);
    virtual ~FMODSoundBrowser();

    void SetCurrentEvent(const DAVA::String& eventPath);
    DAVA::String GetSelectSoundEvent();

private slots:
    void OnEventSelected(QTreeWidgetItem* item, int column);
    void OnEventDoubleClicked(QTreeWidgetItem* item, int column);

    void OnAccepted();
    void OnRejected();

private:
    void UpdateEventTree();

    void FillEventsTree(const DAVA::Vector<DAVA::String>& names);
    void SelectItemAndExpandTreeByEventName(const DAVA::String& eventName);

    void SetSelectedItem(QTreeWidgetItem* item);

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

    QTreeWidgetItem* selectedItem;
    Ui::FMODSoundBrowser* ui;

    DAVA::TArc::DataWrapper projectDataWrapper;
};
