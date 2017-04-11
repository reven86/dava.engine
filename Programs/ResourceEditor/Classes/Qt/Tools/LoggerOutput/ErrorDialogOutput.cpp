#include "Tools/LoggerOutput/ErrorDialogOutput.h"

#include "Classes/Qt/GlobalOperations.h"

#include "TArc/Utils/AssertGuard.h"

#include "Concurrency/LockGuard.h"
#include "Utils/StringFormat.h"

#include "Settings/SettingsManager.h"

#include "Debug/DVAssertDefaultHandlers.h"

#include <QMessageBox>

namespace ErrorDialogDetail
{
static const DAVA::uint32 maxErrorsPerDialog = 6;
static const DAVA::String errorDivideLine("--------------------\n");

bool ShouldBeHiddenByUI()
{ //need be filled with context for special cases after Qa and Using
    return false;
}
}

class ErrorDialogOutput::IgnoreHelper
{
public:
    bool ShouldIgnoreMessage(DAVA::Logger::eLogLevel ll, const DAVA::String& textMessage)
    {
        bool enabled = (SettingsManager::Instance() != nullptr) ? SettingsManager::GetValue(Settings::General_ShowErrorDialog).AsBool() : false;
        if ((ll < DAVA::Logger::LEVEL_ERROR) || !enabled)
        {
            return true;
        }

        if (DAVA::TArc::IsInsideAssertHandler())
        {
            return true;
        }

        return HasIgnoredWords(textMessage);
    }

private:
    bool HasIgnoredWords(const DAVA::String& testedString)
    {
        static const DAVA::String callstackProlog = "==== callstack ====";
        static const DAVA::String callstackEpilog = "==== callstack end ====";

        if (testedString.find(callstackEpilog) != DAVA::String::npos)
        {
            callstackPrinting = false;
            return true;
        }

        if (callstackPrinting == true)
        {
            return true;
        }

        if (testedString.find(callstackProlog) != DAVA::String::npos)
        {
            callstackPrinting = true;
            return true;
        }

        return false;
    }

private:
    bool callstackPrinting = false;
};

ErrorDialogOutput::ErrorDialogOutput(const std::shared_ptr<GlobalOperations>& globalOperations_)
    : ignoreHelper(new IgnoreHelper())
    , globalOperations(globalOperations_)
    , isJobStarted(false)
    , enabled(true)
{
    errors.reserve(ErrorDialogDetail::maxErrorsPerDialog);
}

void ErrorDialogOutput::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    if (!enabled || ignoreHelper->ShouldIgnoreMessage(ll, text))
    {
        return;
    }

    { //lock container to add new text
        DAVA::LockGuard<DAVA::Mutex> lock(errorsLocker);
        if (errors.size() < ErrorDialogDetail::maxErrorsPerDialog)
        {
            errors.insert(text);
        }
    }

    if (isJobStarted == false)
    {
        DVASSERT(waitDialogConnectionToken.IsEmpty());

        isJobStarted = true;
        DelayedExecute(DAVA::MakeFunction(this, &ErrorDialogOutput::ShowErrorDialog));
    }
}

void ErrorDialogOutput::ShowErrorDialog()
{
    DVASSERT(isJobStarted == true);

    if (globalOperations->IsWaitDialogVisible())
    {
        DVASSERT(waitDialogConnectionToken.IsEmpty());
        waitDialogConnectionToken = globalOperations->waitDialogClosed.Connect(this, &ErrorDialogOutput::ShowErrorDialog);
        return;
    }

    { // disconnect from
        if (!waitDialogConnectionToken.IsEmpty())
        {
            globalOperations->waitDialogClosed.Disconnect(waitDialogConnectionToken);
            waitDialogConnectionToken.Clear();
        }
        isJobStarted = false;
    }

    DelayedExecute(DAVA::MakeFunction(this, &ErrorDialogOutput::ShowErrorDialogImpl));
}

void ErrorDialogOutput::ShowErrorDialogImpl()
{
    if (ErrorDialogDetail::ShouldBeHiddenByUI() || enabled == false)
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

    QMessageBox::critical(globalOperations->GetGlobalParentWidget(), title.c_str(), errorMessage.c_str());
}

void ErrorDialogOutput::Disable()
{
    enabled = false;
}
