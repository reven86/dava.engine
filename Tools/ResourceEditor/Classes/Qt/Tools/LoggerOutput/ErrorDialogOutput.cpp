#include "Tools/LoggerOutput/ErrorDialogOutput.h"

#include "Concurrency/LockGuard.h"
#include "Utils/StringFormat.h"

#include "Main/mainwindow.h"
#include "QtTools/Updaters/LazyUpdater.h"

#include "Settings/SettingsManager.h"


#include <QApplication>
#include <QMessageBox>
#include <QTimer>

namespace ErrorDialogDetail
{
static const DAVA::uint32 maxErrorsPerDialog = 6;
static const DAVA::String errorDivideLine("--------------------\n");

bool ShouldWaitForUI()
{
    //should wait the wait bar until refactoring of wait bar
    return QtMainWindow::Instance()->IsWaitDialogOnScreen() || QtMainWindow::Instance()->ParticlesArePacking();
}

bool ShouldBeHiddenByUI()
{ //need be filled with context for special cases after Qa and Using
    return false;
}
}

ErrorDialogOutput::ErrorDialogOutput()
    : QObject(nullptr)
{
    connect(this, &ErrorDialogOutput::FireError, this, &ErrorDialogOutput::OnError, Qt::QueuedConnection);

    errors.reserve(ErrorDialogDetail::maxErrorsPerDialog);
}

ErrorDialogOutput::~ErrorDialogOutput()
{
    disconnect(this, &ErrorDialogOutput::FireError, this, &ErrorDialogOutput::OnError);
}

void ErrorDialogOutput::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    bool enabled = (SettingsManager::Instance() != nullptr) ? SettingsManager::GetValue(Settings::General_ShowErrorDialog).AsBool() : false;
    if ((ll < DAVA::Logger::LEVEL_ERROR) || !enabled)
    {
        return;
    }

    {
        DAVA::LockGuard<DAVA::Mutex> lock(errorsLocker);
        if (errors.size() < ErrorDialogDetail::maxErrorsPerDialog)
        {
            errors.insert(text);
        }
    }

    ++firedErrorsCount;
    emit FireError();
}

void ErrorDialogOutput::ShowErrorDialog()
{
    if (ErrorDialogDetail::ShouldWaitForUI())
    {
        ++firedErrorsCount;
        emit FireError();
        return;
    }

    if (ErrorDialogDetail::ShouldBeHiddenByUI())
    {
        DAVA::LockGuard<DAVA::Mutex> lock(errorsLocker);
        errors.clear();
        return;
    }

    DAVA::String title;
    DAVA::String errorMessage;

    {
        DAVA::LockGuard<DAVA::Mutex> lock(errorsLocker);
        DAVA::uint32 totalErrors = static_cast<DAVA::uint32>(errors.size());

        if (totalErrors == 1)
        {
            title = "Error occurred";
            errorMessage = *errors.begin();
        }
        else
        {
            title = DAVA::Format("%u errors occurred", totalErrors);
            for (const auto& message : errors)
            {
                errorMessage += message + ErrorDialogDetail::errorDivideLine;
            }

            if (totalErrors == ErrorDialogDetail::maxErrorsPerDialog)
            {
                errorMessage += "\nSee console log for details.";
            }
        }

        errors.clear();
    }

    QMessageBox::critical(QtMainWindow::Instance(), title.c_str(), errorMessage.c_str());
}

void ErrorDialogOutput::OnError()
{
    --firedErrorsCount;
    if (firedErrorsCount == 0)
    {
        ShowErrorDialog();
    }
}
