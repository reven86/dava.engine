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

#include "Model/YamlPackageSerializer.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "Document.h"
#include "UI/QtModelPackageCommandExecutor.h"

#include "Systems/TreeSystem.h"
#include <QApplication>
#include <QClipboard>


TreeSystem::TreeSystem(Document* parent)
    : document(parent)
{

}

bool TreeSystem::OnInput(DAVA::UIEvent *currentInput)
{
    /*if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->matches(QKeySequence::Copy))
    {
        OnCopy();
        return true;
    }
    if(keyEvent->matches(QKeySequence::Paste))
    {
        OnPaste();
        return true;
    }
    if(keyEvent->matches(QKeySequence::Delete))
    {
        OnDelete();
        return true;
    }
    if(keyEvent->matches(QKeySequence::Cut))
    {
        OnCopy();
        OnDelete();
        return true;
    }
    }*/
    return false;
}

void TreeSystem::SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    selectionList.insert(selected.begin(), selected.end());
    selectionList.erase(deselected.begin(), deselected.end());
}

void TreeSystem::OnCopy()
{
    DAVA::Vector<ControlNode*> nodesToCopy;
    for (auto node : selectionList)
    {
        if (node->CanCopy())
        {
            nodesToCopy.push_back(node);
        }
    }
    if (nodesToCopy.empty())
    {
        return;
    }
    YamlPackageSerializer serializer;
    serializer.SerializePackageNodes(document->GetPackage(), nodesToCopy);
    DAVA::String str = serializer.WriteToString();
    QApplication::clipboard()->setText(QString::fromStdString(str));
}

void TreeSystem::OnPaste()
{
    if (QApplication::clipboard()->text().isEmpty())
    {
        return;
    }
    for (auto &node : selectionList)
    {
        DVASSERT(nullptr != node);
        if (!node->IsReadOnly())
        {
            DAVA::String string = QApplication::clipboard()->text().toStdString();
            document->GetCommandExecutor()->Paste(document->GetPackage(), node, node->GetCount(), string);
        }
    }
}

void TreeSystem::OnDelete()
{
    DAVA::Vector<ControlNode*> nodesToRemove;
    for (auto node : selectionList)
    {
        if (node->CanRemove())
        {
            nodesToRemove.push_back(node);
        }
    }
    if (!nodesToRemove.empty())
    {
        document->GetCommandExecutor()->RemoveControls(nodesToRemove);
    }
}
