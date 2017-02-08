#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>
#include <Base/GlobalEnum.h>

enum DebugDrawEnum
{
    First,
    Second,
    Third
};

class ParticleDebugDrawModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    bool IsBool() const;
    void SetBool(bool v);

    bool IsDisabled() const;

    DebugDrawEnum GetEnumValue() const;
    void SetEnumValue(DebugDrawEnum v);

    void UpdateSceneSystem();

DAVA_VIRTUAL_REFLECTION(ParticleDebugDrawModule, DAVA::TArc::ClientModule);
};