#include "Base/GlobalEnum.h"
#include "FileSystem/KeyedArchive.h"
#include "Preferences/PreferencesRegistrator.h"

#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"

#include <Tools/TextureCompression/TextureConverter.h>

PUSH_QT_WARNING_SUPRESSOR
#include "ui_DialogReloadSprites.h"
#include <QTimer>
POP_QT_WARNING_SUPRESSOR

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(DialogReloadSprites,
                              PREF_ARG("currentGPU", static_cast<uint8>(GPU_ORIGIN)),
                              PREF_ARG("quality", static_cast<uint32>(TextureConverter::ECQ_VERY_HIGH)),
                              PREF_ARG("forceRepackEnabled", false),
                              PREF_ARG("consoleState", String()),
                              PREF_ARG("consoleVisible", true)
                              )

DialogReloadSprites::DialogReloadSprites(SpritesPacker* packer, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogReloadSprites)
    , spritesPacker(packer)
{
    ui->setupUi(this);
    DVASSERT(nullptr != spritesPacker);
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");

    workerThread.setStackSize(16 * 1024 * 1024);

    LoggerOutputObject* loggerOutput = new LoggerOutputObject(this); //will be removed by dialog
    connect(loggerOutput, &LoggerOutputObject::OutputReady, ui->logWidget, &LogWidget::AddMessage, Qt::DirectConnection);

    ui->pushButton_start->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_targetGPU->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_quality->setDisabled(spritesPacker->IsRunning());
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->pushButton_start, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_targetGPU, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_quality, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->checkBox_repack, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChangedQueued, Qt::QueuedConnection);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChangedDirect, Qt::DirectConnection);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &DialogReloadSprites::OnStopClicked);
    connect(ui->pushButton_start, &QPushButton::clicked, this, &DialogReloadSprites::OnStartClicked);
    connect(ui->checkBox_showConsole, &QCheckBox::toggled, this, &DialogReloadSprites::OnCheckboxShowConsoleToggled);

    const auto& gpuMap = GlobalEnumMap<eGPUFamily>::Instance();
    for (size_t i = 0; i < gpuMap->GetCount(); ++i)
    {
        int value;
        bool ok = gpuMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT(ok, "wrong enum used to create GPU list");
            break;
        }
        ui->comboBox_targetGPU->addItem(gpuMap->ToString(value), value);
    }

    const auto& qualityMap = GlobalEnumMap<TextureConverter::eConvertQuality>::Instance();
    for (size_t i = 0; i < qualityMap->GetCount(); ++i)
    {
        int value;
        bool ok = qualityMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT(ok, "wrong enum used to create quality list");
            break;
        }
        ui->comboBox_quality->addItem(qualityMap->ToString(value), value);
    }
    ui->comboBox_quality->setCurrentText(qualityMap->ToString(TextureConverter::ECQ_DEFAULT));

    PreferencesStorage::Instance()->RegisterPreferences(this);
}

DialogReloadSprites::~DialogReloadSprites()
{
    if (spritesPacker->IsRunning())
    {
        BlockingStop();
    }
    PreferencesStorage::Instance()->UnregisterPreferences(this);
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
    auto gpuType = static_cast<eGPUFamily>(gpuData.toInt());
    auto quality = static_cast<TextureConverter::eConvertQuality>(qualityData.toInt());
    QMetaObject::invokeMethod(spritesPacker, "ReloadSprites", Qt::QueuedConnection, Q_ARG(bool, true), Q_ARG(bool, ui->checkBox_repack->isChecked()), Q_ARG(DAVA::eGPUFamily, gpuType), Q_ARG(DAVA::TextureConverter::eConvertQuality, quality));
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

void DialogReloadSprites::OnRunningChangedDirect(bool /*running*/)
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

void DialogReloadSprites::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    BlockingStop();
}

uint8 DialogReloadSprites::GetCurrentGPU() const
{
    return ui->comboBox_targetGPU->currentData().toInt();
}

void DialogReloadSprites::SetCurrentGPU(uint8 gpu)
{
    for (int i = 0, k = ui->comboBox_targetGPU->count(); i < k; i++)
    {
        if (ui->comboBox_targetGPU->itemData(i).toInt() == gpu)
        {
            ui->comboBox_targetGPU->setCurrentIndex(i);
        }
    }
}

uint32 DialogReloadSprites::GetCurrentQuality() const
{
    return ui->comboBox_quality->currentData().toInt();
}

void DialogReloadSprites::SetCurrentQuality(uint32 quality)
{
    for (int i = 0, k = ui->comboBox_quality->count(); i < k; i++)
    {
        if (ui->comboBox_quality->itemData(i).toInt() == quality)
        {
            ui->comboBox_quality->setCurrentIndex(i);
        }
    }
}

String DialogReloadSprites::GetGeometry() const
{
    QByteArray geometry = saveGeometry().toBase64();
    return geometry.toStdString();
}

void DialogReloadSprites::SetGeometry(const String& str)
{
    QByteArray geometry = QByteArray::fromStdString(str);
    restoreGeometry(QByteArray::fromBase64(geometry));
}

bool DialogReloadSprites::IsForceRepackEnabled() const
{
    return ui->checkBox_repack->isChecked();
}

void DialogReloadSprites::EnableForceRepack(bool enabled)
{
    ui->checkBox_repack->setChecked(enabled);
}

String DialogReloadSprites::GetConsoleState() const
{
    QByteArray consoleState = ui->logWidget->Serialize().toBase64();
    return consoleState.toStdString();
}

void DialogReloadSprites::SetConsoleState(const String& str)
{
    QByteArray consoleState = QByteArray::fromStdString(str);
    ui->logWidget->Deserialize(QByteArray::fromBase64(consoleState));
}

bool DialogReloadSprites::IsConsoleVisible() const
{
    return ui->checkBox_showConsole->isChecked();
}

void DialogReloadSprites::SetConsoleVisible(bool visible)
{
    ui->checkBox_showConsole->setChecked(visible);
}

void DialogReloadSprites::BlockingStop()
{
    this->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QEventLoop loop;
    connect(&workerThread, &QThread::finished, &loop, &QEventLoop::quit);
    spritesPacker->Cancel();
    if (workerThread.isRunning())
    {
        loop.exec();
    }

    QApplication::restoreOverrideCursor();
    this->setEnabled(true);
}
