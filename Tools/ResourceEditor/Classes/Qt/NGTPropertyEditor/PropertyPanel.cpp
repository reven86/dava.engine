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
#include "Classes/Qt/Scene/EntityGroup.h"
#include "QtTools/NGTUtils/ReflectionBridge.h"
#include "QtTools/NGTUtils/GlobalContext.h"

#include "Scene3D/Entity.h"

#include "core_reflection/i_definition_manager.hpp"
#include "core_reflection/interfaces/i_reflection_controller.hpp"
#include "core_data_model/reflection/reflected_tree_model.hpp"
#include "core_data_model/i_tree_model.hpp"
#include "core_qt_common/helpers/qt_helpers.hpp"


#include "Debug/DVAssert.h"

PropertyPanel::PropertyPanel()
{
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::Initialize(IUIFramework& uiFramework, IUIApplication& uiApplication)
{
    view = uiFramework.createView("Views/PropertyPanel.qml", IUIFramework::ResourceType::Url, this);
    uiApplication.addView(*view);
}

void PropertyPanel::Finalize()
{
    SetObject(nullptr);
    view.reset();
}

Q_INVOKABLE QVariant PropertyPanel::GetPropertyTree()
{
    return QtHelpers::toQVariant(ObjectHandle(objectHandleStorage));
}

void PropertyPanel::SceneSelectionChanged(SceneEditor2* scene, const EntityGroup* selected, const EntityGroup* deselected)
{
    int x = 0;
    x++;
}

void PropertyPanel::SetObject(DAVA::InspBase* object)
{
    using TModelPTr = std::unique_ptr<ITreeModel>;

    std::shared_ptr<IObjectHandleStorage> temporaryHandle(objectHandleStorage);

    if (object != nullptr)
    {
        IDefinitionManager* defMng = DAVA::queryInterface<IDefinitionManager>();
        DVASSERT(defMng != nullptr);
        IReflectionController* controller = DAVA::queryInterface<IReflectionController>();
        const DAVA::InspInfo* info = object->GetTypeInfo();
        DAVA::RegisterType(*defMng, info);

        IClassDefinition* definition = defMng->getDefinition(info->Type()->GetTypeName());

        TModelPTr model(new ReflectedTreeModel(DAVA::CreateObjectHandle(*defMng, info, object), *defMng, controller));
        objectHandleStorage.reset(new ObjectHandleStorage<TModelPTr>(std::move(model), nullptr));
    }
    else
    {
        IDefinitionManager* defMng = DAVA::queryInterface<IDefinitionManager>();
        IReflectionController* controller = DAVA::queryInterface<IReflectionController>();
        TModelPTr model(new ReflectedTreeModel(ObjectHandle(), *defMng, controller));

        objectHandleStorage.reset(new ObjectHandleStorage<TModelPTr>(std::move(model), nullptr));
    }

    emit EntityChanged();
}
