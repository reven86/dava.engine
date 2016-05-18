#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EngineBackend.h"

namespace DAVA
{
namespace Private
{
void InitFramework()
{
    new EngineBackend;
}

void TermFramework()
{
    delete EngineBackend::instance;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
