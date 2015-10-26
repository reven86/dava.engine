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

#include "Classes/SceneTree.h"
#include "Classes/Metadata/SceneTree.mpp"

#include "Interfaces/ISceneEnumerator.h"
#include "Interfaces/ISceneObserver.h"

#include <core_ui_framework/i_ui_application.hpp>
#include <core_ui_framework/i_ui_framework.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_data_model/generic_tree_item.hpp>

#include <core_logging/logging.hpp>

using namespace std::placeholders;

template <typename T>
T* queryInterface(IComponentContext& context, const char* interfaceName)
{
    T* result = context.queryInterface<T>();
    if (result == nullptr)
    {
        NGT_ERROR_MSG("Can't query %s interface\n", interfaceName);
    }

    return result;
}

namespace
{
using TFlushNodeFn = std::function<void(DAVA::Entity*)>;
void EnumerateSceneNodes(DAVA::Entity* entity, ISceneEnumerator& enumerator, TFlushNodeFn const& flushFn)
{
    for (size_t i = 0; i < enumerator.GetChildCount(entity); ++i)
    {
        DAVA::Entity* childEntity = enumerator.GetChild(entity, i);
        flushFn(childEntity);
    }
}
}

class SceneTree::EntitySceneItem : public GenericTreeItem
{
public:
    EntitySceneItem(DAVA::Entity* entity_, EntitySceneItem* parent_, ISceneEnumerator& enumerator_)
        : entity(entity_)
        , parent(parent_)
        , enumerator(enumerator_)
    {
        EnumerateSceneNodes(entity, enumerator, [this](DAVA::Entity* childEntity) {
            children.push_back(new EntitySceneItem(childEntity, this, enumerator));
        });
    }

    ~EntitySceneItem()
    {
        for (EntitySceneItem* item : children)
            delete item;
    }

    GenericTreeItem* getParent() const override
    {
        return parent;
    }

    GenericTreeItem* getChild(size_t index) const override
    {
        return children[index];
    }

    size_t size() const override
    {
        return children.size();
    }

    int columnCount() const override
    {
        return 1;
    }

    const char* getDisplayText(int column) const override
    {
        if (entity)
            return enumerator.GetEntityName(entity);

        return nullptr;
    }

    ThumbnailData getThumbnail(int column) const override
    {
        return nullptr;
    }

    Variant getData(int column, size_t roleId) const override
    {
        return Variant();
    }

    bool setData(int column, size_t roleId, const Variant& data) override
    {
        return false;
    }

    DAVA::Entity* GetEntity()
    {
        return entity;
    }

private:
    EntitySceneItem* parent;
    DAVA::Entity* entity;
    std::vector<EntitySceneItem*> children;
    ISceneEnumerator& enumerator;
};

SceneTree::SceneTree()
{
}

SceneTree::~SceneTree()
{
}

void SceneTree::Initialise(IComponentContext& context)
{
    IUIApplication* uiApplication = queryInterface<IUIApplication>(context, "IUIApplication");
    IUIFramework* uiFramework = queryInterface<IUIFramework>(context, "IUIFramework");
    ISceneObserver* observer = queryInterface<ISceneObserver>(context, "ISceneObserver");
    enumerator = queryInterface<ISceneEnumerator>(context, "ISceneEnumerator");
    IDefinitionManager& definitionManager = *queryInterface<IDefinitionManager>(context, "IDefinitionManager");

    REGISTER_DEFINITION(SceneTree);

    observer->Opened.connect(std::bind(&SceneTree::OnOpened, this, _1));
    observer->Closed.connect(std::bind(&SceneTree::OnClosed, this, _1));
    observer->Activated.connect(std::bind(&SceneTree::OnActivated, this, _1));
    observer->Deactivated.connect(std::bind(&SceneTree::OnDeactivated, this, _1));

    sceneTreeView = uiFramework->createView("qrc:/SC/SceneTree.qml", IUIFramework::ResourceType::Url, ObjectHandle(this));
    uiApplication->addView(*sceneTreeView);
}

void SceneTree::Finilise(IComponentContext& context)
{
}

void SceneTree::OnOpened(DAVA::Entity* scene)
{
}

void SceneTree::OnClosed(DAVA::Entity* scene)
{
}

void SceneTree::OnActivated(DAVA::Entity* scene)
{
    root = new EntitySceneItem(scene, nullptr, *enumerator);
    model.addRootItem(root);
}

void SceneTree::OnDeactivated(DAVA::Entity* scene)
{
    model.removeRootItem(root);
}

const ITreeModel* SceneTree::GetSceneModel() const
{
    return &model;
}
