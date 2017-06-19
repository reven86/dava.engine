#pragma once

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class ModuleManager;

class SampleModuleUIComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(SampleModuleUIComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(SampleModuleUIComponent);

    UIComponent* Clone() const override
    {
        return new SampleModuleUIComponent(*this);
    }
};

class SampleModule : public IModule
{
public:
    enum eStatus
    {
        ES_UNKNOWN,
        ES_INIT,
        ES_SHUTDOWN
    };

    const Vector<eStatus>& StatusList() const
    {
        return statusList;
    }

    SampleModule(Engine* engine);

    void Init() override;
    void Shutdown() override;

private:
    Vector<eStatus> statusList;

    DAVA_VIRTUAL_REFLECTION(SampleModule, IModule);
};
};