#if defined(__DAVAENGINE_COREV2__)

#pragma once

namespace DAVA
{
namespace Private
{
Vector<String> InitializeEngine(int argc, char* argv[]);
void TerminateEngine();

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
