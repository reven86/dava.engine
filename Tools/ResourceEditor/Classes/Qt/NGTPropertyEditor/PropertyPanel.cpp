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
#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "NgtTools/Reflection/ReflectionBridge.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "metainfo/PropertyPanel.mpp"

#include <core_command_system/i_command_manager.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/interfaces/i_reflection_controller.hpp>
#include <core_data_model/reflection/reflected_property_model.hpp>
#include <core_data_model/i_tree_model.hpp>
#include <core_qt_common/helpers/qt_helpers.hpp>

#include <QTimerEvent>

PropertyPanel::PropertyPanel()
    : updater(DAVA::MakeFunction(this, &PropertyPanel::UpdateModel))
{
    ICommandManager* commandManager = NGTLayer::queryInterface<ICommandManager>();
    DVASSERT(commandManager != nullptr);

    IDefinitionManager* definitionManager = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(definitionManager != nullptr);

    IReflectionController* reflectionController = NGTLayer::queryInterface<IReflectionController>();
    DVASSERT(reflectionController != nullptr);

    model.reset(new ReflectedPropertyModel(*definitionManager, *commandManager, *reflectionController));
    model->registerExtension(new EntityChildCreatorExtension());
    model->registerExtension(new EntityMergeValueExtension());
    model->registerExtension(new PropertyPanelGetExtension());
    model->registerExtension(new EntityInjectDataExtension(*this, *definitionManager));
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

void PropertyPanel::Finalize()
{
    selectedObjects.clear();
    SetObject(selectedObjects);
    view->deregisterListener(this);
    view.reset();
    model.reset();
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
    selectedObjects.clear();
    const SelectableGroup::CollectionType& selectedContent = selected->GetContent();
    selectedObjects.reserve(selectedContent.size());
    std::transform(selectedContent.begin(), selectedContent.end(),
                   std::back_inserter(selectedObjects),
                   std::bind(&Selectable::GetContainedObject, std::placeholders::_1));

    if (visible || selected->IsEmpty())
    {
        SetObject(selectedObjects);
    }
    else
    {
        isSelectionDirty = true;
    }
}

void PropertyPanel::SetObject(const std::vector<DAVA::InspBase*>& davaObjects)
{
    DVASSERT(model != nullptr);

    IDefinitionManager* defMng = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(defMng != nullptr);

    std::vector<ObjectHandle> objects;
    for (DAVA::InspBase* object : davaObjects)
    {
        const DAVA::InspInfo* info = object->GetTypeInfo();
        NGTLayer::RegisterType(*defMng, info);

        IClassDefinition* definition = defMng->getDefinition(info->Type()->GetTypeName());
        objects.push_back(NGTLayer::CreateObjectHandle(*defMng, info, object));
    }

    model->setObjects(objects);
}

void PropertyPanel::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == updateTimerId)
    {
        UpdateModel();
        e->accept();
    }
}

void PropertyPanel::onFocusIn(IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = true;
    if (isSelectionDirty)
    {
        SetObject(selectedObjects);
        isSelectionDirty = false;
    }

    if (updateTimerId == -1)
    {
        updateTimerId = startTimer(1000);
    }
}

void PropertyPanel::onFocusOut(IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = false;

    if (updateTimerId != -1)
    {
        killTimer(updateTimerId);
        updateTimerId = -1;
    }
}

void PropertyPanel::UpdateModel()
{
    model->update();
}

void PropertyPanel::StartBatch(const DAVA::String& name, DAVA::uint32 commandCount)
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);

    scene->BeginBatch(name, commandCount);
}

void PropertyPanel::Exec(Command2::Pointer&& command)
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);

    scene->Exec(std::move(command));
}

void PropertyPanel::EndBatch()
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);

    scene->EndBatch();
}
