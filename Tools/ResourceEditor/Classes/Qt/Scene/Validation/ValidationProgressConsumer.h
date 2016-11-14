#pragma once

#include "Base/BaseTypes.h"
#include "Logger/Logger.h"

namespace SceneValidation
{
using namespace DAVA;

class ValidationProgressConsumer
{
public:
    virtual void ValidationStarted(const String& validationTitle){};
    virtual void ValidationAlert(const String& alertMessage){};
    virtual void ValidationDone(){};
};

class ValidationProgressToLog : public ValidationProgressConsumer
{
protected:
    void ValidationStarted(const String& title) override;
    void ValidationAlert(const String& alertMessage) override;
    void ValidationDone() override;

private:
    String validationTitle;
};

inline void ValidationProgressToLog::ValidationStarted(const String& title)
{
    validationTitle = title;
}

inline void ValidationProgressToLog::ValidationAlert(const String& alertMessage)
{
    Logger::Warning("%s: %s", validationTitle.c_str(), alertMessage.c_str());
}

inline void ValidationProgressToLog::ValidationDone()
{
    Logger::Info("%s: done ", validationTitle.c_str());
}
}
