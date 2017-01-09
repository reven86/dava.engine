#pragma once

#include "Base/BaseTypes.h"
#include "TArc/Core/ClientModule.h"

#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;
}

class SceneEditor2;
class SelectableGroup;
class PropertyPanelModule final : public DAVA::TArc::ClientModule
{
public:
    PropertyPanelModule() = default;
    ~PropertyPanelModule();
    void PostInit() override;

private:
    void SceneSelectionChanged(const Any& newSelection);

private:
    std::unique_ptr<DAVA::TArc::FieldBinder> binder;
    DAVA::Vector<DAVA::Entity*> selection;

    DAVA_VIRTUAL_REFLECTION(PropertyPanelModule, DAVA::TArc::ClientModule);
};