#pragma once

#include "Base/BaseTypes.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

//#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;
}

class SceneEditor2;
class SelectableGroup;
class PropertyPanelModule final : public DAVA::TArc::ClientModule
{
public:
    void PostInit() override;

private:
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);

private:
    DAVA::TArc::QtConnections connections;
    DAVA::Vector<DAVA::Entity*> selection;

    //DAVA_VIRTUAL_REFLECTION(PropertyPanelModule, DAVA::TArc::ClientModule)
};