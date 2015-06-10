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
    
MoveGuideByMouseCommand::MoveGuideByMouseCommand(const HierarchyTreeScreenNode* screenNode) :
    BaseGuideCommand(screenNode),
    isFirstMovePerformed(false)
{
}

void MoveGuideByMouseCommand::Execute()
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
    
void MoveGuideByMouseCommand::Rollback()
{
    if (isFirstMovePerformed)
    {
        UpdateGuidePosition(movedGuideData, startGuidePos);
        DecrementUnsavedChanges();
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}

void MoveGuideByMouseCommand::UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos)
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    screen->UpdateGuidePosition(guideData, newPos);
}

/////////////////////////////////////////////////////////////////////////////////////////

MoveGuideCommand::MoveGuideCommand(const HierarchyTreeScreenNode* screenNode, const Vector2& delta) :
    BaseGuideCommand(screenNode),
    isFirstMovePerformed(false),
    moveDelta(delta)
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
        // Gather all the selected guides and remember their original positions.
        const List<GuideData*>& selectedGuides = screen->GetSelectedGuides();
        for (List<GuideData*>::const_iterator iter = selectedGuides.begin(); iter != selectedGuides.end(); iter ++)
        {
            GuideData* guideData = *iter;
            guidesPositions.push_back(guideData->GetPosition());
            screen->SetGuidePosition(guideData, guideData->GetPosition() + moveDelta);
        }
    
        isFirstMovePerformed = true;
    }
    else
    {
        // Called after Rollback. Restore the guide position.
        const List<GuideData*>& allGuides = screen->GetGuides(false);
        for (List<GuideData*>::const_iterator iter = allGuides.begin(); iter != allGuides.end(); iter ++)
        {
            GuideData* guideData = *iter;
            
            for (List<Vector2>::iterator innerIter = guidesPositions.begin(); innerIter != guidesPositions.end(); innerIter ++)
            {
                const Vector2& origPos = *innerIter;
                if (guideData->GetPosition() == origPos)
                {
                    screen->SetGuidePosition(guideData, guideData->GetPosition() + moveDelta);
                }
            }
        }
    }

    IncrementUnsavedChanges();
    HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void MoveGuideCommand::Rollback()
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen || !isFirstMovePerformed)
    {
        return;
    }

    const List<GuideData*>& allGuides = screen->GetGuides(false);
    for (List<GuideData*>::const_iterator iter = allGuides.begin(); iter != allGuides.end(); iter ++)
    {
        GuideData* guideData = *iter;

        for (List<Vector2>::iterator innerIter = guidesPositions.begin(); innerIter != guidesPositions.end(); innerIter ++)
        {
            const Vector2& origPos = *innerIter;
            if (guideData->GetPosition() - moveDelta == origPos)
            {
                screen->SetGuidePosition(guideData, origPos);
            }
        }
    }

    DecrementUnsavedChanges();
    HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

/////////////////////////////////////////////////////////////////////////////////////////

DeleteSingleGuideCommand::DeleteSingleGuideCommand(const HierarchyTreeScreenNode* screenNode,
    const GuideData& guideData) :
    BaseGuideCommand(screenNode),
    isFirstDeletePerformed(false),
    guideDataToDelete(guideData)
{
}

void DeleteSingleGuideCommand::Execute()
{
    HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
    if (!screen)
    {
        return;
    }

    if (!isFirstDeletePerformed)
    {
        screen->RemoveGuide(guideDataToDelete);
        isFirstDeletePerformed = true;
    }
    else
    {
        RemoveRememberedGuide(guideDataToDelete);
    }
    
    IncrementUnsavedChanges();
    HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void DeleteSingleGuideCommand::Rollback()
{
    if (isFirstDeletePerformed)
    {
        AddRememberedGuide(guideDataToDelete);
        DecrementUnsavedChanges();
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
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
        (*iter).SetSelected(false);
        RemoveRememberedGuide(*iter);
    }
}

};