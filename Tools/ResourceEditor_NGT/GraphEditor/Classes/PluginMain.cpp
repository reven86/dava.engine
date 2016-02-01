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

#include "TypeRegistration.h"
#include "GraphEditor.h"

#include <core_generic_plugin/interfaces/i_component_context.hpp>
#include <core_generic_plugin/generic_plugin.hpp>

#include <core_variant/variant.hpp>
#include <core_variant/interfaces/i_meta_type_manager.hpp>

#include <core_reflection/i_definition_manager.hpp>

#include <core_ui_framework/i_ui_framework.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_ui_framework/i_view.hpp>

class GraphEditorPlugin : public PluginMain
{
public:
    GraphEditorPlugin(IComponentContext& context)
    {
    }

    bool PostLoad(IComponentContext& context) override
    {
        return true;
    }

    void Initialise(IComponentContext& context) override
    {
        IUIFramework* uiFramework = context.queryInterface<IUIFramework>();
        IUIApplication* uiapplication = context.queryInterface<IUIApplication>();
        IDefinitionManager* defMng = context.queryInterface<IDefinitionManager>();

        assert(uiFramework != nullptr);
        assert(uiapplication != nullptr);
        assert(defMng != nullptr);

        Variant::setMetaTypeManager(context.queryInterface<IMetaTypeManager>());

        RegisterGrapEditorTypes(*defMng);
        editor = defMng->create<GraphEditor>(false);

        view = uiFramework->createView("GE/GraphEditorView.qml", IUIFramework::ResourceType::Url, ObjectHandle(editor));
        uiapplication->addView(*view);
    }

    bool Finalise(IComponentContext& context) override
    {
        view.reset();
        editor = ObjectHandleT<GraphEditor>();
        return true;
    }

    void Unload(IComponentContext& context) override
    {
    }

private:
    std::unique_ptr<IView> view;
    ObjectHandleT<GraphEditor> editor;
};

PLG_CALLBACK_FUNC(GraphEditorPlugin)