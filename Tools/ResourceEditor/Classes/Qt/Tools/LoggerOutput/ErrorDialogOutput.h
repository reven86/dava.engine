#pragma once

#include "Concurrency/Mutex.h"
#include "Logger/Logger.h"

#include <QObject>
#include <memory>

class GlobalOperations;
class ErrorDialogOutput final : public QObject, public DAVA::LoggerOutput
{
    Q_OBJECT

public:
    ErrorDialogOutput(const std::shared_ptr<GlobalOperations>& globalOperations, QObject* parent);
    ~ErrorDialogOutput() override;

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;

private slots:
    void OnError();

signals:

    void FireError();

private:
    void ShowErrorDialog();

    DAVA::UnorderedSet<DAVA::String> errors;
    DAVA::Mutex errorsLocker;

    DAVA::uint32 firedErrorsCount = 0;
    std::shared_ptr<GlobalOperations> globalOperations;
};
