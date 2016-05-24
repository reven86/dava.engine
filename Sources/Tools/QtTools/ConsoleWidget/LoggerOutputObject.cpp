#include "LoggerOutputObject.h"
#include "Debug/DVAssert.h"

class LoggerOutputObject::LoggerOutputContainer : private DAVA::LoggerOutput, public QObject
{
public:
    LoggerOutputContainer(LoggerOutputObject* parent);

private slots:
    void AboutToBeDestroyed();

private:
    ~LoggerOutputContainer() override;
    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;
    LoggerOutputObject* parent = nullptr;
};

LoggerOutputObject::LoggerOutputContainer::LoggerOutputContainer(LoggerOutputObject* parent_)
    : parent(parent_)
{
    DAVA::Logger::AddCustomOutput(this);
    DVASSERT(nullptr != parent);
    connect(parent, &QObject::destroyed, this, &LoggerOutputContainer::AboutToBeDestroyed);
}

void LoggerOutputObject::LoggerOutputContainer::AboutToBeDestroyed()
{
    DAVA::Logger::RemoveCustomOutput(this); //as a static method, must be safe for Logger::Instance() == nullptr
}

LoggerOutputObject::LoggerOutputContainer::~LoggerOutputContainer()
{
    parent->outputContainer = nullptr;
}

void LoggerOutputObject::LoggerOutputContainer::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    parent->OutputReady(ll, text);
}

LoggerOutputObject::LoggerOutputObject(QObject* parent)
    : QObject(parent)
    , outputContainer(new LoggerOutputContainer(this))
{
    qRegisterMetaType<DAVA::Logger::eLogLevel>("DAVA::Logger::eLogLevel");
}