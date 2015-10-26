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

#ifndef SCENETREE_SCENETREE_H
#define SCENETREE_SCENETREE_H

#include "Interfaces/ISceneEnumerator.h"

#include <core_generic_plugin/interfaces/i_component_context.hpp>
#include <core_ui_framework/i_view.hpp>
#include <core_reflection/object_handle.hpp>
#include <core_reflection/reflected_object.hpp>
#include <core_data_model/generic_tree_model.hpp>
#include <core_dependency_system/interface_helpers.hpp>

namespace DAVA
{
class Entity;
}

class SceneTree
{
    DECLARE_REFLECTED

public:
    SceneTree();
    ~SceneTree();

    void Initialise(IComponentContext& context);
    void Finilise(IComponentContext& context);

    const ITreeModel* GetSceneModel() const;

private:
    void OnOpened(DAVA::Entity* scene);
    void OnClosed(DAVA::Entity* scene);
    void OnActivated(DAVA::Entity* scene);
    void OnDeactivated(DAVA::Entity* scene);

private:
    std::unique_ptr<IView> sceneTreeView;
    GenericTreeModel model;
    class EntitySceneItem;
    EntitySceneItem* root = nullptr;
    ISceneEnumerator* enumerator;
};

#endif // SCENETREE_SCENETREE_H