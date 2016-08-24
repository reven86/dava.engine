#pragma once

#include "Concurrency/Mutex.h"
#include "Logger/Logger.h"

#include <QObject>
#include <memory>

class GlobalOperations;
class ErrorDialogOutput final : public DAVA::LoggerOutput
{

public:
    ErrorDialogOutput(const std::shared_ptr<GlobalOperations>& globalOperations);

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;
    void Disable();

private:
    void ShowErrorDialog();
    void ShowErrorDialogImpl();

    class IgnoreHelper;
    std::unique_ptr<IgnoreHelper> ignoreHelper;
    std::shared_ptr<GlobalOperations> globalOperations;

    DAVA::UnorderedSet<DAVA::String> errors;
    DAVA::Mutex errorsLocker;

    std::atomic<bool> isJobStarted;
    std::atomic<bool> enabled;

    DAVA::SigConnectionID waitDialogConnectionId = 0;
};
