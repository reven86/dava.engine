#pragma once

#include "Base/BaseTypes.h"
#include "Logger/Logger.h"

namespace SceneValidation
{
using namespace DAVA;

class ValidationOutput
{
public:
    virtual void ValidationStarted(){};
    virtual void ValidationAlert(String alertMessage){};
    virtual void ValidationDone(){};
};

struct LogValidationOutput : public ValidationOutput
{
public:
    explicit LogValidationOutput(String validationTitle);

protected:
    void ValidationAlert(String alertMessage) override;
    void ValidationDone() override;

private:
    String validationTitle;
};
}
