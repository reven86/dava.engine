#pragma once

#include "Concurrency/Mutex.h"
#include "Logger/Logger.h"

class LazyUpdater;
class ErrorDialogOutput final : public DAVA::LoggerOutput
{
public:
    ErrorDialogOutput();
    ~ErrorDialogOutput() override;

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;

private:
    void ShowErrorDialog();

    DAVA::UnorderedSet<DAVA::String> errors;
    DAVA::Mutex errorsLocker;

    LazyUpdater* dialogUpdater = nullptr;
};
