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
#include "TextureCompression/TextureConverter.h"
#include "ui_DialogReloadSprites.h"
#include <QSettings>
#include "Base/GlobalEnum.h"

using namespace DAVA;
namespace
{
    const QString GPU = "gpu";
    const QString QUALITY = "quality";
    const QString CLEAR_ON_START = "clear on start";
}

DialogReloadSprites::DialogReloadSprites(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogReloadSprites)
    , actionReloadSprites(new QAction(QIcon(":/QtTools/Icons/reload.png"), tr("Reload Sprites"), this))
{
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");
    
    connect(actionReloadSprites, &QAction::triggered, this, &DialogReloadSprites::exec);
    
    spritesPacker = new SpritesPacker(this);
    workerThread.setStackSize(16 * 1024 * 1024);
    spritesPacker->moveToThread(&workerThread);
    ui->setupUi(this);
    OnRunningChanged(spritesPacker->IsRunning());
    ui->pushButton_start->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_targetGPU->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_quality->setDisabled(spritesPacker->IsRunning());
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->pushButton_start, &QPushButton::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_targetGPU, &QComboBox::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_quality, &QComboBox::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChanged);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &DialogReloadSprites::OnStopClicked);
    connect(ui->pushButton_start, &QPushButton::clicked, this, &DialogReloadSprites::OnStartClicked);

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
        OnStopClicked();
    }
    delete ui;
}

void DialogReloadSprites::OnStartClicked()
{
    if (!StarPackProcess())
    {
        return;
    }
    const auto gpuData = ui->comboBox_targetGPU->currentData();
    const auto qualityData = ui->comboBox_quality->currentData();
    if (!gpuData.isValid() || !qualityData.isValid())
    {
        return;
    }
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

void DialogReloadSprites::OnRunningChanged(bool running)
{
    ui->pushButton_cancel->setText(running ? "Cancel" : "Close");
}

void DialogReloadSprites::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    BlockingStop();
}

void DialogReloadSprites::LoadSettings()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    const auto &targetGPU = settings.value(GPU);
    if (targetGPU.isValid())
    {
        const auto &string = GlobalEnumMap<eGPUFamily>::Instance()->ToString(targetGPU.toInt());
        ui->comboBox_targetGPU->setCurrentText(string);
    }
    const auto &quality = settings.value(QUALITY);
    if (quality.isValid())
    {
        const auto &string = GlobalEnumMap<TextureConverter::eConvertQuality>::Instance()->ToString(quality.toInt());
        ui->comboBox_quality->setCurrentText(string);
    }
    const auto &clear = settings.value(CLEAR_ON_START);
    if (clear.isValid())
    {
        ui->checkBox_clean->setChecked(clear.toBool());
    }
}

void DialogReloadSprites::SaveSettings() const
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.setValue(GPU, ui->comboBox_targetGPU->currentData().toInt());
    settings.setValue(QUALITY, ui->comboBox_quality->currentData().toInt());
    settings.setValue(CLEAR_ON_START, ui->checkBox_clean->isChecked());
}

void DialogReloadSprites::BlockingStop()
{
    this->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QEventLoop loop;
    connect(spritesPacker, &SpritesPacker::Finished, &loop, &QEventLoop::quit);
    spritesPacker->Cancel();
    workerThread.quit();
    if (spritesPacker->IsRunning())
    {
        loop.exec();
    }
    workerThread.wait();
    QApplication::restoreOverrideCursor();
    workerThread.quit();
    this->setEnabled(true);
}
