#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EngineBackend.h"

namespace DAVA
{
namespace Private
{
Vector<String> InitializeEngine(int argc, char* argv[])
{
    EngineBackend* engineBackend = new EngineBackend(argc, argv);
    return engineBackend->GetCommandLine();
}

void TerminateEngine()
{
    delete EngineBackend::instance;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
