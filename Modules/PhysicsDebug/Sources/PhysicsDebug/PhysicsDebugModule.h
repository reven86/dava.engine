#pragma once

#include <ModuleManager/IModule.h>

namespace physx
{
namespace debugger
{
namespace comm
{
class PvdConnection;
} // namespace comm
} // namespace debugger
} // namespace physx

namespace DAVA
{
class PhysicsDebug : public IModule
{
public:
    PhysicsDebug(Engine* engine);
    void Init() override;
    void Shutdown() override;

private:
    physx::debugger::comm::PvdConnection* pvdConnection = nullptr;
};
}