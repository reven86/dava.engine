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

#include "GuideCommands.h"

namespace DAVA {

BaseGuideCommand::BaseGuideCommand(const HierarchyTreeScreenNode* screenNode) :
    BaseCommand()
{
    if (screenNode)
    {
        activeScreen = screenNode->GetId();
    }
}

void BaseGuideCommand::AddRememberedGuide(const GuideData& data)
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    screen->AddGuide(data);
}

void BaseGuideCommand::RemoveRememberedGuide(const GuideData& data)
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    screen->RemoveGuide(data);
}

/////////////////////////////////////////////////////////////////////////////////////////

AddNewGuideCommand::AddNewGuideCommand(const HierarchyTreeScreenNode* screenNode) :
    BaseGuideCommand(screenNode),
    isFirstAdd(false)
{
}

void AddNewGuideCommand::Execute()
{
    if (!isFirstAdd)
    {
        // Adding the new guide for the first time - need to accept it.
        AcceptNewGuide();
        isFirstAdd = true;
    }
    else
    {
        // The Rollback was already called, so just adding the guide remembered.
        AddRememberedGuide(guideData);
    }
    
    IncrementUnsavedChanges();
    HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void AddNewGuideCommand::Rollback()
{
    if (isFirstAdd)
    {
        RemoveRememberedGuide(guideData);
        DecrementUnsavedChanges();
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}

void AddNewGuideCommand::AcceptNewGuide()
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    // Remember this guide as reference one for Undo/Redo operations. Store the data themselves,
    // not the pointer - the pointer might be deleted.
    const GuideData* newGuideData = screen->AcceptNewGuide();
    if (newGuideData)
    {
        guideData = *newGuideData;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
    
MoveGuideCommand::MoveGuideCommand(const HierarchyTreeScreenNode* screenNode) :
    BaseGuideCommand(screenNode),
    isFirstMovePerformed(false)
{
}

void MoveGuideCommand::Execute()
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    if (!isFirstMovePerformed)
    {
        // Remember the move guide position before accepting. Again - no pointers storing because of
        // possible deletion.
        startGuidePos = screen->GetMoveGuideStartPos();
        const GuideData* guideData = screen->AcceptMoveGuide();
        if (guideData)
        {
            movedGuideData = *guideData;
        }

        isFirstMovePerformed = true;
    }
    else
    {
        // Called after Rollback. Restore the guide position.
        GuideData origGuideData(movedGuideData.GetType(), startGuidePos);
        UpdateGuidePosition(origGuideData, movedGuideData.GetPosition());
    }
    
    IncrementUnsavedChanges();
    HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}
    
void MoveGuideCommand::Rollback()
{
    if (isFirstMovePerformed)
    {
        UpdateGuidePosition(movedGuideData, startGuidePos);
        DecrementUnsavedChanges();
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}

void MoveGuideCommand::UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos)
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    screen->UpdateGuidePosition(guideData, newPos);
}

/////////////////////////////////////////////////////////////////////////////////////////

DeleteGuidesCommand::DeleteGuidesCommand(const HierarchyTreeScreenNode* screenNode) :
    BaseGuideCommand(screenNode),
    isFirstDeletePerformed(false)
{
}

void DeleteGuidesCommand::Execute()
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    if (!isFirstDeletePerformed)
    {
        deletedGuidesData = screen->DeleteSelectedGuides();
        isFirstDeletePerformed = true;
    }
    else
    {
        RemoveRememberedGuides();
    }
    
    IncrementUnsavedChanges();
    HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void DeleteGuidesCommand::Rollback()
{
    if (isFirstDeletePerformed)
    {
        AddRememberedGuides();
        DecrementUnsavedChanges();
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}
    
void DeleteGuidesCommand::AddRememberedGuides()
{
    for (List<GuideData>::iterator iter = deletedGuidesData.begin(); iter != deletedGuidesData.end(); iter ++)
    {
        AddRememberedGuide(*iter);
    }
}

void DeleteGuidesCommand::RemoveRememberedGuides()
{
    for (List<GuideData>::iterator iter = deletedGuidesData.begin(); iter != deletedGuidesData.end(); iter ++)
    {
        RemoveRememberedGuide(*iter);
    }
}

};