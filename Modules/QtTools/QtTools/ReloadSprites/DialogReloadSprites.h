#ifndef __DIALOG_RELOAD_SPRITES_H__
#define __DIALOG_RELOAD_SPRITES_H__


#include "Base/Introspection.h"
#include "SpritesPacker.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"
#include "QtTools/Utils/QtThread.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QDialog>
POP_QT_WARNING_SUPRESSOR

namespace Ui
{
class DialogReloadSprites;
}

class DialogReloadSprites : public QDialog, public DAVA::InspBase
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
public:
    explicit DialogReloadSprites(SpritesPacker* packer, QWidget* parent = nullptr);
    ~DialogReloadSprites();

private slots:
    void OnStartClicked();
    void OnStopClicked();
    void OnRunningChangedQueued(bool running); //we can work with widgets only in application thread
    void OnRunningChangedDirect(bool running); //we can move to thead only from current thread
    void OnCheckboxShowConsoleToggled(bool checked);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void BlockingStop();

    DAVA::String GetGeometry() const;
    void SetGeometry(const DAVA::String& geometry);

    DAVA::uint8 GetCurrentGPU() const;
    void SetCurrentGPU(DAVA::uint8 gpu);

    DAVA::uint32 GetCurrentQuality() const;
    void SetCurrentQuality(DAVA::uint32 quality);

    bool IsForceRepackEnabled() const;
    void EnableForceRepack(bool enabled);

    DAVA::String GetConsoleState() const;
    void SetConsoleState(const DAVA::String& str);

    bool IsConsoleVisible() const;
    void SetConsoleVisible(bool visible);

    std::unique_ptr<Ui::DialogReloadSprites> ui;
    SpritesPacker* spritesPacker;
    QtThread workerThread; //we need this thread only for "cancel" button

public:
    INTROSPECTION(DialogReloadSprites,
                  PROPERTY("geometry", "DialogReloadSpritesInternal/Geometry", GetGeometry, SetGeometry, DAVA::I_PREFERENCE)
                  PROPERTY("currentGPU", "DialogReloadSpritesInternal/CurrentGPU", GetCurrentGPU, SetCurrentGPU, DAVA::I_PREFERENCE)
                  PROPERTY("quality", "DialogReloadSpritesInternal/Quality", GetCurrentQuality, SetCurrentQuality, DAVA::I_PREFERENCE)
                  PROPERTY("forceRepackEnabled", "DialogReloadSpritesInternal/ForceRepackEnabled", IsForceRepackEnabled, EnableForceRepack, DAVA::I_PREFERENCE)
                  PROPERTY("consoleState", "DialogReloadSpritesInternal/ConsoleState", GetConsoleState, SetConsoleState, DAVA::I_PREFERENCE)
                  PROPERTY("consoleVisible", "DialogReloadSpritesInternal/ConsoleVisible", IsConsoleVisible, SetConsoleVisible, DAVA::I_PREFERENCE)
                  );
};

#endif // __DIALOG_RELOAD_SPRITES_H__
