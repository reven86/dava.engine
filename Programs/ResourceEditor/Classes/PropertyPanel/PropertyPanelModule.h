#pragma once

#include "Base/BaseTypes.h"
#include "TArc/Core/ClientModule.h"

#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;
namespace TArc
{
class FieldBinder;
}
}

class PropertyPanelModule final : public DAVA::TArc::ClientModule
{
public:
    PropertyPanelModule() = default;
    void PostInit() override;

private:
    void SceneSelectionChanged(const DAVA::Any& newSelection);

private:
    std::unique_ptr<DAVA::TArc::FieldBinder> binder;

    DAVA_VIRTUAL_REFLECTION(PropertyPanelModule, DAVA::TArc::ClientModule);
};