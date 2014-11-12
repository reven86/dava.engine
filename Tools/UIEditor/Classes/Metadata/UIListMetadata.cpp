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



#include "UIListMetadata.h"
#include "EditorListDelegate.h"

namespace DAVA {

UIListMetadata::UIListMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

UIList* UIListMetadata::GetActiveUIList() const
{
	return static_cast<UIList*>(GetActiveUIControl());
}

void UIListMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	BaseMetadata::InitializeControl(controlName, position);
	
	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UIList
        UIList* list = static_cast<UIList*>(this->treeNodeParams[i].GetUIControl());
        EditorListDelegate *editorList = new EditorListDelegate(list);
        list->SetDelegate(editorList);
        list->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    }	
}

int UIListMetadata::GetAggregatorID()
{
    if (!VerifyActiveParamID())
    	return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();	
	if (!editorList)
		return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	return editorList->GetAggregatorID();
}

void UIListMetadata::SetAggregatorID(int value)
{
	if (!VerifyActiveParamID())
    	return;

	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();
	if (!editorList)
		return;

	editorList->SetAggregatorID(value);
	GetActiveUIList()->Refresh();
}

int UIListMetadata::GetOrientation()
{
    if (!VerifyActiveParamID())
    {
        return UIList::ORIENTATION_VERTICAL;
    }

    return GetActiveUIList()->GetOrientation();
}
    
void UIListMetadata::SetOrientation(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }	
    
	UpdateListCellSize();
	GetActiveUIList()->SetOrientation((UIList::eListOrientation)value);
}

void UIListMetadata::SetActiveControlRect(const Rect& rect, bool restoreAlign, bool alignToIntegerPos)
{
	UIControlMetadata::SetActiveControlRect(rect, restoreAlign, alignToIntegerPos);
	UpdateListCellSize();
}

void UIListMetadata::SetLeftAlign(int value)
{
	UIControlMetadata::SetLeftAlign(value);
	UpdateListCellSize();
}

void UIListMetadata::SetHCenterAlign(int value)
{
	UIControlMetadata::SetHCenterAlign(value);
	UpdateListCellSize();
}

void UIListMetadata::SetRightAlign(int value)
{
	UIControlMetadata::SetRightAlign(value);
	UpdateListCellSize();
}

void UIListMetadata::SetTopAlign(int value)
{
	UIControlMetadata::SetTopAlign(value);
	UpdateListCellSize();
}

void UIListMetadata::SetVCenterAlign(int value)
{
	UIControlMetadata::SetVCenterAlign(value);
	UpdateListCellSize();
}

void UIListMetadata::SetBottomAlign(int value)
{
	UIControlMetadata::SetBottomAlign(value);
	UpdateListCellSize();
}

void UIListMetadata::SetLeftAlignEnabled(const bool value)
{
	UIControlMetadata::SetLeftAlignEnabled(value);
	UpdateListCellSize();
}

void UIListMetadata::SetHCenterAlignEnabled(const bool value)
{
	UIControlMetadata::SetHCenterAlignEnabled(value);
	UpdateListCellSize();
}

void UIListMetadata::SetRightAlignEnabled(const bool value)
{
	UIControlMetadata::SetRightAlignEnabled(value);
	UpdateListCellSize();
}

void UIListMetadata::SetTopAlignEnabled(const bool value)
{
	UIControlMetadata::SetTopAlignEnabled(value);
	UpdateListCellSize();
}

void UIListMetadata::SetVCenterAlignEnabled(const bool value)
{
	UIControlMetadata::SetVCenterAlignEnabled(value);
	UpdateListCellSize();
}

void UIListMetadata::SetBottomAlignEnabled(const bool value)
{
	UIControlMetadata::SetBottomAlignEnabled(value);
	UpdateListCellSize();
}


void UIListMetadata::UpdateListCellSize()
{
	// Get delegate for current list
	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();
	if (!editorList)
		return;
	
	editorList->ResetElementsCount();
	// Refresh list - and recreate cells
	GetActiveUIList()->Refresh();
}

};
