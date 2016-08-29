#pragma once

#include "TArcCore/ClientModule.h"
#include "TArcCore/ContextManager.h"

namespace DAVA
{
namespace TArc
{
class ControllerModule : public ClientModule
{
protected:
    ContextManager& GetContextManager();

private:
    void SetContextManager(ContextManager* contextManager);

private:
    friend class Core;
    ContextManager* contextManager = nullptr;
};
} // namespace TArc
} // namespace DAVA
