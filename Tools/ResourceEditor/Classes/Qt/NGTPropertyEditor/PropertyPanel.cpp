#include "PropertyPanel.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "NgtTools/Reflection/ReflectionBridge.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Scene3D/Entity.h"

#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/interfaces/i_reflection_controller.hpp>
#include <core_data_model/reflection/reflected_tree_model.hpp>
#include <core_data_model/i_tree_model.hpp>
#include <core_qt_common/helpers/qt_helpers.hpp>
#include "metainfo/PropertyPanel.mpp"

#include "Debug/DVAssert.h"

PropertyPanel::PropertyPanel()
{
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::Initialize(wgt::IUIFramework& uiFramework, wgt::IUIApplication& uiApplication)
{
    wgt::IDefinitionManager* defMng = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defMng != nullptr);
    defMng->registerDefinition(std::unique_ptr<wgt::IClassDefinitionDetails>(new wgt::TypeClassDefinition<PropertyPanel>()));

    view = uiFramework.createView("Views/PropertyPanel.qml", wgt::IUIFramework::ResourceType::Url, this);
    view->registerListener(this);
    uiApplication.addView(*view);
}

void PropertyPanel::Finalize(wgt::IUIApplication& uiApplication)
{
    SetObject(nullptr);
    uiApplication.removeView(*view);
    view->deregisterListener(this);
    view.reset();
}

wgt::ObjectHandle PropertyPanel::GetPropertyTree() const
{
    return wgt::ObjectHandleT<wgt::ITreeModel>(model.get());
}

void PropertyPanel::SetPropertyTree(const wgt::ObjectHandle& /*dummyTree*/)
{
}

void PropertyPanel::SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    if (selected->IsEmpty())
    {
        selectedObject = nullptr;
        isSelectionDirty = false;
        SetObject(nullptr);
    }
    else
    {
        selectedObject = selected->GetFirst().GetContainedObject();
        if (visible)
        {
            SetObject(selectedObject);
        }
        else
        {
            isSelectionDirty = true;
        }
    }
}

void PropertyPanel::SetObject(DAVA::InspBase* object)
{
    return;

    std::shared_ptr<wgt::ITreeModel> tempTreeModel(model);

    wgt::IDefinitionManager* defMng = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defMng != nullptr);
    wgt::IReflectionController* controller = NGTLayer::queryInterface<wgt::IReflectionController>();
    wgt::ITreeModel* newModel = nullptr;
    if (object != nullptr)
    {
        const DAVA::InspInfo* info = object->GetTypeInfo();
        NGTLayer::RegisterType(*defMng, info);

        wgt::IClassDefinition* definition = defMng->getDefinition(info->Type()->GetTypeName());
        newModel = new wgt::ReflectedTreeModel(NGTLayer::CreateObjectHandle(*defMng, info, object), *defMng, controller);
    }

    model.reset(newModel);

    wgt::IClassDefinition* definition = defMng->getDefinition(wgt::getClassIdentifier<PropertyPanel>());
    definition->bindProperty("PropertyTree", this).setValue(wgt::Variant());
}

void PropertyPanel::onFocusIn(wgt::IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = true;
    if (isSelectionDirty)
    {
        SetObject(selectedObject);
        isSelectionDirty = false;
    }
}

void PropertyPanel::onFocusOut(wgt::IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = false;
}
