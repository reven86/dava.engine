#include "Classes/ParticleSystem/ParticleDebugDrawModule.h"

#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleEffectDebugDrawSystem.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Selection/SelectableGroup.h"

#include "Scene3D/Entity.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/FieldBinder.h>
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
}

DAVA::String ParticlesDebugStytemDrawSelectedState(const DAVA::Any& drawOnlySelected)
{
    return "Show only selected";
}
}

class ParticleDebugDrawData : public DAVA::TArc::DataNode
{
public:
    bool isSystemOn = false;
    bool drawOnlySelected = false;
    eParticleDebugDrawMode drawMode = eParticleDebugDrawMode::OVERDRAW;
    DAVA::UnorderedSet<RenderObject*> selectedParticles;

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
        fields[CheckBox::Fields::Checked] = "isEnabledProperty";
        layout->AddWidget(new CheckBox(fields, accessor, DAVA::Reflection::Create(this), w));
    }

    {
        ControlDescriptorBuilder<ComboBox::Fields> fields;
        fields[ComboBox::Fields::IsReadOnly] = "readOnly";
        fields[ComboBox::Fields::Value] = "drawModeProperty";
        layout->AddWidget(new ComboBox(fields, accessor, DAVA::Reflection::Create(this), w));
    }

    {
        ControlDescriptorBuilder<CheckBox::Fields> fields;
        fields[CheckBox::Fields::IsReadOnly] = "readOnly";
        fields[CheckBox::Fields::Checked] = "drawSelectedProperty";
        layout->AddWidget(new CheckBox(fields, accessor, DAVA::Reflection::Create(this), w));
    }

    QAction* action = new QAction(nullptr);
    AttachWidgetToAction(action, w);

    ActionPlacementInfo placementInfo(CreateToolbarPoint("ParticleSystemToolbar"));
    GetUI()->AddAction(REGlobal::MainWindowKey, placementInfo, action);

    filedBinder.reset(new FieldBinder(accessor));
    DAVA::TArc::FieldDescriptor descr;
    descr.type = DAVA::ReflectedTypeDB::Get<SelectionData>();
    descr.fieldName = DAVA::FastName(SelectionData::selectionPropertyName);
    filedBinder->BindField(descr, DAVA::MakeFunction(this, &ParticleDebugDrawModule::OnSelectionChanged));
}

void ParticleDebugDrawModule::OnSelectionChanged(const DAVA::Any selection)
{
    SelectableGroup group = selection.Cast<SelectableGroup>(SelectableGroup());
    GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->selectedParticles = ProcessSelection(group);
    UpdateSceneSystem();
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

bool ParticleDebugDrawModule::GetDrawOnlySelected() const
{
    return GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->drawOnlySelected;
}

void ParticleDebugDrawModule::SetDrawOnlySelected(bool drawOnlySelected)
{
    GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->drawOnlySelected = drawOnlySelected;
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
            particleEffectDebugDrawSystem->SetIsDrawOnlySelected(data->drawOnlySelected);
            particleEffectDebugDrawSystem->SetSelectedParticles(data->selectedParticles);
        }
    });
}

DAVA::UnorderedSet<RenderObject*> ParticleDebugDrawModule::ProcessSelection(const SelectableGroup& group)
{
    uint32 count = static_cast<uint32>(group.GetSize());
    UnorderedSet<RenderObject*> particleObjects;
    for (auto entity : group.ObjectsOfType<DAVA::Entity>())
    {
        ParticleEffectComponent* particleComponent = static_cast<ParticleEffectComponent*>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
        if (particleComponent != nullptr)
            particleObjects.insert(particleComponent->GetRenderObject());
    }
    return particleObjects;
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleDebugDrawModule)
{
    DAVA::ReflectionRegistrator<ParticleDebugDrawModule>::Begin()
        .ConstructorByPointer()
        .Field("isEnabledProperty", &ParticleDebugDrawModule::GetSystemEnabledState, &ParticleDebugDrawModule::SetSystemEnabledState)
            [DAVA::M::ValueDescription(&ParticleDebugDrawModuleDetail::ParticlesDebugStytemState)]
        .Field("drawModeProperty", &ParticleDebugDrawModule::GetDrawMode, &ParticleDebugDrawModule::SetDrawMode)[DAVA::M::EnumT<eParticleDebugDrawMode>()]
        .Field("drawSelectedProperty", &ParticleDebugDrawModule::GetDrawOnlySelected, &ParticleDebugDrawModule::SetDrawOnlySelected)
            [DAVA::M::ValueDescription(&ParticleDebugDrawModuleDetail::ParticlesDebugStytemDrawSelectedState)]
        .Field("readOnly", &ParticleDebugDrawModule::IsDisabled, nullptr)
        .End();
}

DECL_GUI_MODULE(ParticleDebugDrawModule);
