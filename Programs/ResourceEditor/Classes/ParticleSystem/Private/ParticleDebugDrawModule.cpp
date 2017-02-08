#include "Classes/ParticleSystem/ParticleDebugDrawModule.h"
#include "Classes/SceneManager/SceneData.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataNode.h>

#include <QWidget>

ENUM_DECLARE(eParticleDebugDrawMode)
{
    ENUM_ADD_DESCR(eParticleDebugDrawMode::LOW_ALPHA, "Low alpha");
    ENUM_ADD_DESCR(eParticleDebugDrawMode::WIREFRAME, "Wireframe");
    ENUM_ADD_DESCR(eParticleDebugDrawMode::OVERDRAW, "Overdraw");
}

namespace ParticleDebugDrawModuleDetail
{
DAVA::String ParticlesDebugStytemState(const DAVA::Any& isSystemOn)
{
    return "Particles debug";
//     if (isSystemOn.Get<bool>() == true)
//         return "Disable particles debug";
//     else
//         return "Enable particles debug";
}
}

class ParticleDebugDrawData : public DAVA::TArc::DataNode
{
public:
    bool isSystemOn = true;
    eParticleDebugDrawMode drawMode = eParticleDebugDrawMode::OVERDRAW;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ParticleDebugDrawData, DAVA::TArc::DataNode)
    {
    }
};

void ParticleDebugDrawModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
}

void ParticleDebugDrawModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void ParticleDebugDrawModule::PostInit()
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    accessor->GetGlobalContext()->CreateData(std::make_unique<ParticleDebugDrawData>());

    QWidget* w = new QWidget();
    QtHBoxLayout* layout = new QtHBoxLayout(w);

    {
        ControlDescriptorBuilder<CheckBox::Fields> fields;
        fields[CheckBox::Fields::IsReadOnly] = "readOnly";
        fields[CheckBox::Fields::Checked] = "boolProperty";
        layout->AddWidget(new CheckBox(fields, accessor, DAVA::Reflection::Create(this), w));
    }

    {
        ControlDescriptorBuilder<ComboBox::Fields> fields;
        fields[ComboBox::Fields::IsReadOnly] = "readOnly";
        fields[ComboBox::Fields::Value] = "enumProperty";
        layout->AddWidget(new ComboBox(fields, accessor, DAVA::Reflection::Create(this), w));
    }

    QAction* action = new QAction(nullptr);
    AttachWidgetToAction(action, w);

    ActionPlacementInfo placementInfo(CreateToolbarPoint("ParticleSystemToolbar"));
    GetUI()->AddAction(REGlobal::MainWindowKey, placementInfo, action);
}

bool ParticleDebugDrawModule::GetSystemEnabledState() const
{
    return GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->isSystemOn;
}

void ParticleDebugDrawModule::SetSystemEnabledState(bool v)
{
    GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->isSystemOn = v;
    UpdateSceneSystem();
}

bool ParticleDebugDrawModule::IsDisabled() const
{
    return GetAccessor()->GetContextCount() == 0;
}

eParticleDebugDrawMode ParticleDebugDrawModule::GetDrawMode() const
{
    return GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->drawMode;
}

void ParticleDebugDrawModule::SetDrawMode(eParticleDebugDrawMode mode)
{
    GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->drawMode = mode;
    UpdateSceneSystem();
}

void ParticleDebugDrawModule::UpdateSceneSystem()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    ParticleDebugDrawData* data = GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>();
    accessor->ForEachContext([data](DataContext& ctx)
    {
        SceneData::TSceneType scene = ctx.GetData<SceneData>()->GetScene();
        ParticleEffectDebugDrawSystem* particleEffectDebugDrawSystem = scene->GetParticleDebugSystem();
        if (particleEffectDebugDrawSystem != nullptr)
        {
            particleEffectDebugDrawSystem->SetIsEnabled(data->isSystemOn);
            particleEffectDebugDrawSystem->SetDrawMode(data->drawMode);
        }
    });
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleDebugDrawModule)
{
    DAVA::ReflectionRegistrator<ParticleDebugDrawModule>::Begin()
        .ConstructorByPointer()
        .Field("boolProperty", &ParticleDebugDrawModule::GetSystemEnabledState, &ParticleDebugDrawModule::SetSystemEnabledState)
            [DAVA::M::ValueDescription(&ParticleDebugDrawModuleDetail::ParticlesDebugStytemState)]
        .Field("enumProperty", &ParticleDebugDrawModule::GetDrawMode, &ParticleDebugDrawModule::SetDrawMode)[DAVA::M::EnumT<eParticleDebugDrawMode>()]
        .Field("readOnly", &ParticleDebugDrawModule::IsDisabled, nullptr)
        .End();
}

DECL_GUI_MODULE(ParticleDebugDrawModule);
