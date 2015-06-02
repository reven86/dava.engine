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


#include "EditorListDelegate.h"
#include "Utils/Utils.h"
#include "UI/UIAggregatorControl.h"

#include "HierarchyTreeAggregatorNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "HierarchyTreeController.h"
#include "EditorFontManager.h"

namespace DAVA
{
	EditorListDelegate::EditorListDelegate(UIList* list, bool rectInAbsoluteCoordinates/* = false*/)
	: UIControl(list->GetRect(), rectInAbsoluteCoordinates),
		aggregatorID(DEFAULT_AGGREGATOR_ID),
		cellsCount(CELL_COUNT),
		isElementsCountNeedUpdate(false)
	{
        currentList = list;
        
        const Rect& rect = list->GetRect();
		DVASSERT(cellsCount > 0);
		if (UIList::ORIENTATION_VERTICAL == list->GetOrientation())
		{
			cellSize = Vector2(rect.dx, (rect.dy <= DEFAULT_CELL_HEIGHT) ? rect.dy : (rect.dy / cellsCount));
		}
		else
		{
			cellSize = Vector2((rect.dx <= DEFAULT_CELL_WIDTH) ? rect.dx : (rect.dx / cellsCount), rect.dy);
		}
	}
	
	EditorListDelegate::~EditorListDelegate()
	{
        currentList->SetDelegate(NULL);
	}
	
	void EditorListDelegate::SetAggregatorID(int32 agId)
	{
		aggregatorID = agId;
		ResetElementsCount();
	}
	
	int32 EditorListDelegate::GetAggregatorID()
	{
		return aggregatorID;
	}
	
	void EditorListDelegate::SetCellSize(const Vector2 &size)
	{
		cellSize = size;
	}
	
	void EditorListDelegate::UpdateCellSize(UIList *forList)
	{		
		if (isElementsCountNeedUpdate && forList)
		{
			isElementsCountNeedUpdate = false;
			// Change cell size only if aggregator control is available
			UIControl *aggregatorControl = GetCurrentAggregatorControl();
			if (aggregatorControl)
			{
				Vector2 aggregatorSize = aggregatorControl->GetSize();
				SetCellSize(aggregatorSize);
			}
			
			Vector2 listSize = forList->GetSize();
			if(forList->GetOrientation() == UIList::ORIENTATION_HORIZONTAL)
			{
				DVASSERT(cellSize.x > 0);
				cellsCount =  ceilf( listSize.x / cellSize.x );
			}
			else
			{
				DVASSERT(cellSize.y > 0);
				cellsCount =  ceilf( listSize.y / cellSize.y );
			}
		}
	}
	
	void EditorListDelegate::ResetElementsCount()
	{
		isElementsCountNeedUpdate = true;
	}
	
	// UIListDelegate implementation	
	int32 EditorListDelegate::ElementsCount(UIList *forList)
	{
		UpdateCellSize(forList);
		DVASSERT(cellsCount > 0);
		return cellsCount;
	}
	
	UIListCell *EditorListDelegate::CellAtIndex(UIList *forList, int32 index)
	{
    	// Try to get cell from the reusable cells store
    	UIListCell *cell = forList->GetReusableCell("Cell");
		if (!cell)
		{
			cell = new UIListCell(Rect(0.0f, 0.0f, cellSize.x, cellSize.y), "Cell");
		}
		else
		{
			cell->SetSize(Vector2(cellSize.x, cellSize.y));
			// Reset reusable cells relative positions - new proper positions will be calculated at UIList::AddCellAtPost() method
			cell->SetPosition(Vector2(0.0f, 0.0f));
		}
	
		cell->RemoveAllControls();
		// Get aggregator control
		UIControl *aggregatorControl = GetCurrentAggregatorControl();
		if (aggregatorControl)
		{
			UIAggregatorControl* control = new UIAggregatorControl();
			control->CopyDataFrom(aggregatorControl);
			// DF-1770 - Reset aggregator's background draw type
			control->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
			cell->AddControl(control);
            SafeRelease(control);
		}
		else
		{			
			cell->SetStateFont(UIControl::STATE_NORMAL, EditorFontManager::Instance()->GetDefaultFont());
			cell->SetStateText(UIControl::STATE_NORMAL, StringToWString(Format("Cell %d",index)));
			cell->SetSelected(true);
		}
   
    	return cell;
	}

	float32 EditorListDelegate::CellHeight(UIList *, int32)
	{
    	return cellSize.y;
	}
	
	float32 EditorListDelegate::CellWidth(UIList *, int32)
	{
		return cellSize.x;
	}
	
	void EditorListDelegate::SaveToYaml(UIList *forList, YamlNode *)
	{
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(aggregatorID);
		HierarchyTreeAggregatorNode *aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregatorNode)
		{
			forList->SetAggregatorPath(aggregatorNode->GetPath());
		}
		else
		{
			forList->SetAggregatorPath(String());
		}
	}
	
	UIControl* EditorListDelegate::GetCurrentAggregatorControl()
	{
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(aggregatorID);
		HierarchyTreeAggregatorNode *aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		
		// Update cell size to fit new aggregator control size
		if (aggregatorNode)
		{
			return aggregatorNode->GetScreen();
		}
		
		return NULL;
	}
};