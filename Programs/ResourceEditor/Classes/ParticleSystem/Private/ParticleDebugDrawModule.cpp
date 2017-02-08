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

ENUM_DECLARE(DebugDrawEnum)
{
    ENUM_ADD_DESCR(DebugDrawEnum::First, "First");
    ENUM_ADD_DESCR(DebugDrawEnum::Second, "Second");
    ENUM_ADD_DESCR(DebugDrawEnum::Third, "Third");
}

namespace ParticleDebugDrawModuleDetail
{
DAVA::String BoolValueDescription(const DAVA::Any&v)
{
    if (v.Get<bool>() == true)
        return "True";
    else
        return "False";
}
}

class ParticleDebugDrawData : public DAVA::TArc::DataNode
{
public:
    bool isBool = true;
    DebugDrawEnum enumValue = First;

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

bool ParticleDebugDrawModule::IsBool() const
{
    return GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->isBool;
}

void ParticleDebugDrawModule::SetBool(bool v)
{
    GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->isBool = v;
    UpdateSceneSystem();
}

bool ParticleDebugDrawModule::IsDisabled() const
{
    return GetAccessor()->GetContextCount() == 0;
}

DebugDrawEnum ParticleDebugDrawModule::GetEnumValue() const
{
    return GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->enumValue;
}

void ParticleDebugDrawModule::SetEnumValue(DebugDrawEnum v)
{
    GetAccessor()->GetGlobalContext()->GetData<ParticleDebugDrawData>()->enumValue = v;
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
        //scene->
    });
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleDebugDrawModule)
{
    DAVA::ReflectionRegistrator<ParticleDebugDrawModule>::Begin()
        .ConstructorByPointer()
        .Field("boolProperty", &ParticleDebugDrawModule::IsBool, &ParticleDebugDrawModule::SetBool)
            [DAVA::M::ValueDescription(&ParticleDebugDrawModuleDetail::BoolValueDescription)]
        .Field("enumProperty", &ParticleDebugDrawModule::GetEnumValue, &ParticleDebugDrawModule::SetEnumValue)[DAVA::M::EnumT<DebugDrawEnum>()]
        .Field("readOnly", &ParticleDebugDrawModule::IsDisabled, nullptr)
        .End();
}


DECL_GUI_MODULE(ParticleDebugDrawModule);