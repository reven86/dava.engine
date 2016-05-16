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


#include "Tools/LoggerOutput/ErrorDialogOutput.h"

#include "Concurrency/LockGuard.h"
#include "Utils/StringFormat.h"

#include "Main/mainwindow.h"
#include "QtTools/Updaters/LazyUpdater.h"

#include "Settings/SettingsManager.h"


#include <QApplication>
#include <QMessageBox>
#include <QTimer>

namespace ErrorDialogInternal
{
static const DAVA::uint32 maxErrorsPerDialog = 6;
static const DAVA::String errorDivideLine("--------------------\n");

bool ShouldWaitForUI()
{
    //should wait the wait bar until refactoring of wait bar

    return QtMainWindow::Instance()->IsWaitDialogOnScreen();
}

bool ShouldBeHiddenByUI()
{ //need be filled with context for special cases after Qa and Using
    return false;
}
}

ErrorDialogOutput::ErrorDialogOutput()
{
    DAVA::Function<void()> fn(this, &ErrorDialogOutput::ShowErrorDialog);
    dialogUpdater = new LazyUpdater(fn, nullptr);

    errors.reserve(ErrorDialogInternal::maxErrorsPerDialog);
}

ErrorDialogOutput::~ErrorDialogOutput()
{
    delete dialogUpdater;
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
        if (errors.size() < ErrorDialogInternal::maxErrorsPerDialog)
        {
            errors.insert(text);
        }
    }

    dialogUpdater->Update();
}

void ErrorDialogOutput::ShowErrorDialog()
{
    if (ErrorDialogInternal::ShouldWaitForUI())
    {
        dialogUpdater->Update();
        return;
    }

    if (ErrorDialogInternal::ShouldBeHiddenByUI())
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
            title = "Error occured";
            errorMessage = *errors.begin();
        }
        else
        {
            title = DAVA::Format("%u errors occured", totalErrors);
            DAVA::uint32 errorCounter = 0;
            for (const auto& message : errors)
            {
                errorMessage += message + ErrorDialogInternal::errorDivideLine;
                errorCounter++;

                if (errorCounter == ErrorDialogInternal::maxErrorsPerDialog)
                {
                    errorMessage += "\nSee console log for details.";
                    break;
                }
            }
        }

        errors.clear();
    }

    QTimer::singleShot(0, [title, errorMessage]() {
        QMessageBox::critical(QApplication::activeWindow(), title.c_str(), errorMessage.c_str());
    });
}
