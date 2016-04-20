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


#ifndef __DIALOG_RELOAD_SPRITES_H__
#define __DIALOG_RELOAD_SPRITES_H__


#include "Base/Introspection.h"
#include "FileSystem/KeyedArchive.h"
#include "QtTools/EditorPreferences/PreferencesRegistrator.h"
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

    DAVA::int64 GetCurrentGPU() const;
    void SetCurrentGPU(DAVA::int64 gpu);

    DAVA::int64 GetCurrentQuality() const;
    void SetCurrentQuality(DAVA::int64 quality);

    bool IsForceRepackEnabled() const;
    void EnableForseRepack(bool enabled);

    DAVA::String GetConsoleState() const;
    void SetConsoleState(const DAVA::String& str);

    bool IsConsoleVisible() const;
    void SetConsoleVisible(bool visible);

    std::unique_ptr<Ui::DialogReloadSprites> ui;
    SpritesPacker* spritesPacker;
    QThread workerThread; //we need this thread only for "cancel" button

public:
    INTROSPECTION(DialogReloadSprites,
                  PROPERTY("currentGPU", "DialogReloadSpritesInternal/CurrentGPU", GetCurrentGPU, SetCurrentGPU, DAVA::I_PREFERENCE)
                  PROPERTY("quality", "DialogReloadSpritesInternal/Quality", GetCurrentQuality, SetCurrentQuality, DAVA::I_PREFERENCE)
                  PROPERTY("forceRepackEnabled", "DialogReloadSpritesInternal/ForceRepackEnabled", IsForceRepackEnabled, EnableForseRepack, DAVA::I_PREFERENCE)
                  PROPERTY("consoleState", "DialogReloadSpritesInternal/ConsoleState", GetConsoleState, SetConsoleState, DAVA::I_PREFERENCE)
                  PROPERTY("consoleVisible", "DialogReloadSpritesInternal/ConsoleVisible", IsConsoleVisible, SetConsoleVisible, DAVA::I_PREFERENCE)
                  );
};

#endif // __DIALOG_RELOAD_SPRITES_H__
