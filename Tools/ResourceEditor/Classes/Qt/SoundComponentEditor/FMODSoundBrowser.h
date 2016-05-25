#ifndef __SOUND_BROWSER_H__
#define __SOUND_BROWSER_H__

#include <QDialog>
#include <QTreeWidgetItem>
#include "DAVAEngine.h"

namespace Ui
{
class FMODSoundBrowser;
}

class SceneEditor2;

class FMODSoundBrowser : public QDialog, public DAVA::Singleton<FMODSoundBrowser>
{
    Q_OBJECT

public:
    explicit FMODSoundBrowser(QWidget* parent = 0);
    virtual ~FMODSoundBrowser();

    DAVA::String GetSelectSoundEvent();

private slots:
    void OnEventSelected(QTreeWidgetItem* item, int column);
    void OnEventDoubleClicked(QTreeWidgetItem* item, int column);

    void OnProjectOpened(const QString&);

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

#endif // SOUNDBROWSER_H
