/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "PropertyPanel.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "NgtTools/Reflection/ReflectionBridge.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Scene3D/Entity.h"

#include "core_reflection/i_definition_manager.hpp"
#include "core_reflection/interfaces/i_reflection_controller.hpp"
#include "core_data_model/reflection/reflected_tree_model.hpp"
#include "core_data_model/i_tree_model.hpp"
#include "core_qt_common/helpers/qt_helpers.hpp"
#include "metainfo/PropertyPanel.mpp"

#include "Debug/DVAssert.h"

PropertyPanel::PropertyPanel()
{
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::Initialize(IUIFramework& uiFramework, IUIApplication& uiApplication)
{
    IDefinitionManager* defMng = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(defMng != nullptr);
    defMng->registerDefinition(new TypeClassDefinition<PropertyPanel>());

    view = uiFramework.createView("Views/PropertyPanel.qml", IUIFramework::ResourceType::Url, this);
    view->registerListener(this);
    uiApplication.addView(*view);
}

void PropertyPanel::Finalize(IUIApplication& uiApplication)
{
    SetObject(nullptr);
    uiApplication.removeView(*view);
    view->deregisterListener(this);
    view.reset();
}

ObjectHandle PropertyPanel::GetPropertyTree() const
{
    return ObjectHandleT<ITreeModel>(model.get());
}

void PropertyPanel::SetPropertyTree(const ObjectHandle& /*dummyTree*/)
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
    std::shared_ptr<ITreeModel> tempTreeModel(model);

    IDefinitionManager* defMng = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(defMng != nullptr);
    IReflectionController* controller = NGTLayer::queryInterface<IReflectionController>();
    ITreeModel* newModel = nullptr;
    if (object != nullptr)
    {
        const DAVA::InspInfo* info = object->GetTypeInfo();
        NGTLayer::RegisterType(*defMng, info);

        IClassDefinition* definition = defMng->getDefinition(info->Type()->GetTypeName());
        newModel = new ReflectedTreeModel(NGTLayer::CreateObjectHandle(*defMng, info, object), *defMng, controller);
    }

    model.reset(newModel);

    IClassDefinition* definition = defMng->getDefinition(getClassIdentifier<PropertyPanel>());
    definition->bindProperty("PropertyTree", this).setValue(Variant());
}

void PropertyPanel::onFocusIn(IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = true;
    if (isSelectionDirty)
    {
        SetObject(selectedObject);
        isSelectionDirty = false;
    }
}

void PropertyPanel::onFocusOut(IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = false;
}
