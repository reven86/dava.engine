#include "ValidationOutput.h"

namespace SceneValidation
{
using namespace DAVA;

LogValidationOutput::LogValidationOutput(String validationTitle)
    : validationTitle(validationTitle)
{
}

void LogValidationOutput::ValidationAlert(String alertMessage)
{
    Logger::Warning("%s: %s", validationTitle.c_str(), alertMessage.c_str());
}

void LogValidationOutput::ValidationDone()
{
    Logger::Info("%s: done ", validationTitle.c_str());
}
}
