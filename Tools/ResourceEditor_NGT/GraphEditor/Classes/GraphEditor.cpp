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

#include "GraphEditor.h"
#include "Metadata/GraphEditor.mpp"

#include "ConnectionManager.h"

#include <core_dependency_system/i_interface.hpp>
#include <core_reflection/i_definition_manager.hpp>

GraphEditor::GraphEditor()
{
    IDefinitionManager* defMng = Context::queryInterface<IDefinitionManager>();
    assert(defMng != nullptr);

    ConnectionManager& mng = ConnectionManager::Instance();
    mng.Initialize(defMng);
}

GraphEditor::~GraphEditor()
{
    ConnectionManager::Instance().Finilize();
}

IListModel* GraphEditor::GetConnectorsModel() const
{
    return ConnectionManager::Instance().GetConnectorsModel();
}

IListModel* GraphEditor::GetNodeModel() const
{
    return ConnectionManager::Instance().GetNodeModel();
}

IListModel* GraphEditor::GetRootContextMenuModel() const
{
    return ConnectionManager::Instance().GetRootContextMenuModel();
}

IListModel* GraphEditor::GetNodeContextMenuModel() const
{
    return ConnectionManager::Instance().GetNodeContextMenuModel();
}

IListModel* GraphEditor::GetSlotContextMenuModel() const
{
    return ConnectionManager::Instance().GetSlotContextMenuModel();
}

void GraphEditor::CreateConnection(size_t outputUID, size_t inputUID)
{
    ConnectionManager::Instance().CreateConnection(outputUID, inputUID);
}
