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


#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"

#include "TextureCompression/TextureConverter.h"
#include "ui_DialogReloadSprites.h"
#include <QSettings>
#include <QTimer>
#include "Base/GlobalEnum.h"

using namespace DAVA;
namespace
{
    const QString GPU = "gpu";
    const QString QUALITY = "quality";
    const QString CLEAR_ON_START = "clear on start";
    const QString SHOW_CONSOLE = "show console";
    const QString CONSOLE_STATE = "console state";
}

DialogReloadSprites::DialogReloadSprites(SpritesPacker* packer, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogReloadSprites)
    , spritesPacker(packer)
{
    DVASSERT(nullptr != spritesPacker);
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");
    
    workerThread.setStackSize(16 * 1024 * 1024);

    ui->setupUi(this);

    LoggerOutputObject* loggerOutput = new LoggerOutputObject(this); //will be removed by dialog
    connect(loggerOutput, &LoggerOutputObject::OutputReady, ui->logWidget, &LogWidget::AddMessage, Qt::DirectConnection);

    ui->pushButton_start->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_targetGPU->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_quality->setDisabled(spritesPacker->IsRunning());
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->pushButton_start, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_targetGPU, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_quality, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->checkBox_clean, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChangedQueued, Qt::QueuedConnection);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChangedDirect, Qt::DirectConnection);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &DialogReloadSprites::OnStopClicked);
    connect(ui->pushButton_start, &QPushButton::clicked, this, &DialogReloadSprites::OnStartClicked);
    connect(ui->checkBox_showConsole, &QCheckBox::toggled, this, &DialogReloadSprites::OnCheckboxShowConsoleToggled);

    const auto &gpuMap = GlobalEnumMap<eGPUFamily>::Instance();
    for (size_t i = 0; i < gpuMap->GetCount(); ++i)
    {
        int value;
        bool ok = gpuMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create GPU list");
            break;
        }
        ui->comboBox_targetGPU->addItem(gpuMap->ToString(value), value);
    }

    const auto &qualityMap = GlobalEnumMap<TextureConverter::eConvertQuality>::Instance();
    for (size_t i = 0; i < qualityMap->GetCount(); ++i)
    {
        int value;
        bool ok = qualityMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create quality list");
            break;
        }
        ui->comboBox_quality->addItem(qualityMap->ToString(value), value);
    }
    ui->comboBox_quality->setCurrentText(qualityMap->ToString(TextureConverter::ECQ_DEFAULT));
    LoadSettings();
}

DialogReloadSprites::~DialogReloadSprites()
{
    SaveSettings();
    if(spritesPacker->IsRunning())
    {
        BlockingStop();
    }
}

void DialogReloadSprites::OnStartClicked()
{
    const auto gpuData = ui->comboBox_targetGPU->currentData();
    const auto qualityData = ui->comboBox_quality->currentData();
    if (!gpuData.isValid() || !qualityData.isValid())
    {
        return;
    }
    spritesPacker->moveToThread(&workerThread);
    workerThread.start();
    auto gpuType = static_cast<DAVA::eGPUFamily>(gpuData.toInt());
    auto quality = static_cast<TextureConverter::eConvertQuality>(qualityData.toInt());
    QMetaObject::invokeMethod(spritesPacker, "ReloadSprites", Qt::QueuedConnection, Q_ARG(bool, ui->checkBox_clean->isChecked()), Q_ARG(DAVA::eGPUFamily, gpuType), Q_ARG(DAVA::TextureConverter::eConvertQuality, quality));
}

void DialogReloadSprites::OnStopClicked()
{
    if (spritesPacker->IsRunning())
    {
        BlockingStop();
    }
    else
    {
        close();
    }
}

void DialogReloadSprites::OnRunningChangedQueued(bool running)
{
    ui->pushButton_cancel->setText(running ? "Cancel" : "Close");
    if (!running)
    {
        workerThread.quit();
        workerThread.wait();
    }
}

void DialogReloadSprites::OnRunningChangedDirect(bool running)
{
    spritesPacker->moveToThread(qApp->thread());
}

void DialogReloadSprites::OnCheckboxShowConsoleToggled(bool checked)
{
    ui->logWidget->setVisible(checked);
    if (!checked)
    {
        setFixedHeight(minimumSizeHint().height());
    }
    else
    {
        setMinimumHeight(minimumSizeHint().height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    }
}

void DialogReloadSprites::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    BlockingStop();
}

void DialogReloadSprites::LoadSettings()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("DialogReloadSprites");
    const auto &targetGPU = settings.value(GPU);
    if (targetGPU.isValid())
    {
        for (int i = 0, k = ui->comboBox_targetGPU->count(); i < k; i++)
        {
            if (ui->comboBox_targetGPU->itemData(i) == targetGPU)
            {
                ui->comboBox_targetGPU->setCurrentIndex(i);
            }
        }
    }
    const auto &quality = settings.value(QUALITY);
    if (quality.isValid())
    {
        for (int i = 0, k = ui->comboBox_quality->count(); i < k; i++)
        {
            if (ui->comboBox_quality->itemData(i) == quality)
            {
                ui->comboBox_quality->setCurrentIndex(i);
            }
        }
    }
    const auto &clear = settings.value(CLEAR_ON_START);
    if (clear.isValid())
    {
        ui->checkBox_clean->setChecked(clear.toBool());
    }
    const auto& consoleState = settings.value(CONSOLE_STATE);
    if (consoleState.canConvert<QByteArray>())
    {
        ui->logWidget->Deserialize(consoleState.toByteArray());
    }

    const auto& showConsole = settings.value(SHOW_CONSOLE);
    if (showConsole.canConvert<bool>())
    {
        ui->checkBox_showConsole->setChecked(showConsole.toBool());
    }
    settings.endGroup();
}

void DialogReloadSprites::SaveSettings() const
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("DialogReloadSprites");
    settings.setValue(GPU, ui->comboBox_targetGPU->currentData());
    settings.setValue(QUALITY, ui->comboBox_quality->currentData());
    settings.setValue(CLEAR_ON_START, ui->checkBox_clean->isChecked());
    settings.setValue(CONSOLE_STATE, ui->logWidget->Serialize());
    settings.setValue(SHOW_CONSOLE, ui->checkBox_showConsole->isChecked());
    settings.endGroup();
}

void DialogReloadSprites::BlockingStop()
{
    this->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QEventLoop loop;
    connect(spritesPacker, &SpritesPacker::Finished, &loop, &QEventLoop::quit);
    spritesPacker->Cancel();
    if (spritesPacker->IsRunning())
    {
        loop.exec();
    }
    QApplication::restoreOverrideCursor();
    this->setEnabled(true);
}
