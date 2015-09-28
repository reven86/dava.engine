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

#include "SceneTree.h"

#include "core_data_model/i_tree_model.hpp"
#include "core_data_model/generic_tree_model.hpp"
#include "core_data_model/generic_tree_item.hpp"

#include "core_qt_common/helpers/qt_helpers.hpp"

#include "SceneUtils.h"
#include "SelectionSystem.h"

#include <QModelIndex>

namespace
{

using TFlushNodeFn = std::function<void (DAVA::Entity *)>;
void EnumerateSceneNodes(DAVA::Entity * entity, TFlushNodeFn const & flushFn)
{
    DVASSERT(flushFn != nullptr);
    for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        DAVA::Entity * childEntity = entity->GetChild(i);
        flushFn(childEntity);
    }
}

class EntitySceneItem : public GenericTreeItem
{
public:
    EntitySceneItem(DAVA::Entity * entity_, EntitySceneItem * parent_)
        : entity(entity_)
        , parent(parent_)
    {
        EnumerateSceneNodes(entity, [this](DAVA::Entity * childEntity)
        {
            children.push_back(new EntitySceneItem(childEntity, this));
        });
    }

    GenericTreeItem * getParent() const override
    {
        return parent;
    }

    GenericTreeItem * getChild(size_t index) const override
    {
        DVASSERT(index < children.size());
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

    const char * getDisplayText(int column) const override
    {
        if (entity)
            return entity->GetName().c_str();

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

    bool setData(int column, size_t roleId, const Variant & data) override
    {
        return false;
    }

    DAVA::Entity * GetEntity() { return entity; }

private:
    EntitySceneItem * parent;
    DAVA::Entity * entity;
    DAVA::Vector<EntitySceneItem *> children;
};

}

void SceneTree::Initialize(IUIFramework & uiFramework, IUIApplication & uiApplication)
{
    sceneTreeView = uiFramework.createView("qrc:/default/SceneTree.qml", IUIFramework::ResourceType::Url, this);

    uiApplication.addView(*sceneTreeView);
}

void SceneTree::Finilize()
{
    SetScene(nullptr);
    sceneTreeView.reset();
}

QVariant SceneTree::GetSceneTree()
{
    return QtHelpers::toQVariant(ObjectHandle(objectHandleStorage));
}

void SceneTree::OnSelectionChanged(QList<QVariant> const & selection)
{
    DVASSERT(scene != nullptr);
    foreach(QVariant s, selection)
    {
        if (!s.canConvert<QModelIndex>())
            continue;

        QModelIndex index = s.toModelIndex();
        if (!index.isValid())
            continue;

        EntitySceneItem * item = static_cast<EntitySceneItem *>(index.internalPointer());
        DAVA::Entity * entity = item->GetEntity();
        DVASSERT(entity != nullptr);

        SelectedEntityChanged.Emit(std::move(entity));
        break;
    }
}

void SceneTree::SetScene(DAVA::Scene * scene_)
{
    SafeRelease(scene);
    scene = scene_;
    SafeRetain(scene);

    using TModelPTr = std::unique_ptr<ITreeModel>;

    if (scene != nullptr)
        objectHandleStorage.reset(new ObjectHandleStorage<TModelPTr>(TModelPTr(CreateSceneModel()), nullptr));
    else
        objectHandleStorage.reset();

    emit SceneChanged();
}

void SceneTree::OnSceneSelectionChanged(DAVA::Scene * scene_, const EntityGroup * selected, const EntityGroup * deselected)
{
    DVASSERT(scene == scene_);
    DVASSERT(selected->Size() < 2);
    if (selected->Size() == 0)
    {
        return;
    }

    DAVA::Entity * entity = selected->GetEntity(0);
    SelectedEntityChanged.Emit(std::move(entity));
}

ITreeModel * SceneTree::CreateSceneModel()
{
    DVASSERT(scene != nullptr);
    GenericTreeModel * model = new GenericTreeModel();
    model->addRootItem(new EntitySceneItem(scene, nullptr));

    return model;
}
