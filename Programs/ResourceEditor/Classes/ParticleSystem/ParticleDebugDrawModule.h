#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>
#include <Base/GlobalEnum.h>

#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleEffectDebugDrawSystem.h"

class ParticleDebugDrawModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    bool GetSystemEnabledState() const;
    void SetSystemEnabledState(bool enabled);

    bool IsDisabled() const;

    eParticleDebugDrawMode GetDrawMode() const;
    void SetDrawMode(eParticleDebugDrawMode drawMode);

    void UpdateSceneSystem();

DAVA_VIRTUAL_REFLECTION(ParticleDebugDrawModule, DAVA::TArc::ClientModule);
};