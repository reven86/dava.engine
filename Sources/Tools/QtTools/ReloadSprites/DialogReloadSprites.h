#ifndef __DIALOG_RELOAD_SPRITES_H__
#define __DIALOG_RELOAD_SPRITES_H__

#include "QtTools/WarningGuard/QtWarningsHandler.h"
#include "SpritesPacker.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QDialog>
#include <QThread>
POP_QT_WARNING_SUPRESSOR

namespace Ui
{
class DialogReloadSprites;
}

class DialogReloadSprites : public QDialog
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
    void LoadSettings();
    void SaveSettings() const;
    void BlockingStop();

    std::unique_ptr<Ui::DialogReloadSprites> ui;
    SpritesPacker* spritesPacker;
    QThread workerThread; //we need this thread only for "cancel" button
};

#endif // __DIALOG_RELOAD_SPRITES_H__
