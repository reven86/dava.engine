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


#include "HierarchyTreeScreenNode.h"
#include "ScreenManager.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include <QFile>
#include "Render/2D/FontManager.h"
#include <qmessagebox.h>

const float32 HierarchyTreeScreenNode::POSITION_UNDEFINED = -1.0f;

HierarchyTreeScreenNode::HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const QString& name) :
	HierarchyTreeNode(name),
    loaded(false)
{
	this->parent = parent;
	this->screen = new ScreenControl();
	screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
	
	scale = 1.f;
	
	posX = POSITION_UNDEFINED;
	posY = POSITION_UNDEFINED;
}

HierarchyTreeScreenNode::HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const HierarchyTreeScreenNode* base, bool needLoad/* = true*/):
	HierarchyTreeNode(base),
    loaded(!needLoad)
{
	this->parent = parent;
	this->screen = new ScreenControl();
	if (parent)
		screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
	
	scale = 1.f;
	posX = 0;
	posY = 0;

	unsavedChangesCounter = 0;

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& children = base->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = children.begin();
		 iter != children.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* baseControl = dynamic_cast<const HierarchyTreeControlNode* >((*iter));
		if (!baseControl)
			continue;
		
		HierarchyTreeControlNode* control = NULL;
		if (dynamic_cast<UIAggregatorControl*>(baseControl->GetUIObject()))
        {
        	const HierarchyTreeAggregatorControlNode* aggregatorBaseControl = static_cast<const HierarchyTreeAggregatorControlNode*>(baseControl);
			control = new HierarchyTreeAggregatorControlNode(this, aggregatorBaseControl);
        }
		else
        {
        	 control = new HierarchyTreeControlNode(this, baseControl);
        }
		AddTreeNode(control);
	}
}

HierarchyTreeScreenNode::~HierarchyTreeScreenNode()
{
	Cleanup();
	SafeRelease(screen);
}

void HierarchyTreeScreenNode::SetScale(float scale)
{
	this->scale = scale;
}

float HierarchyTreeScreenNode::GetScale() const
{
	return scale;
}

void HierarchyTreeScreenNode::SetPosX(int x)
{
	this->posX = x;
}

int HierarchyTreeScreenNode::GetPosX() const
{
	return posX;
}

void HierarchyTreeScreenNode::SetPosY(int y)
{
	this->posY = y;
}

int HierarchyTreeScreenNode::GetPosY() const
{
	return posY;
}

void HierarchyTreeScreenNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	HierarchyTreePlatformNode* newPlatform = dynamic_cast<HierarchyTreePlatformNode*>(node);
	DVASSERT(newPlatform);
	if (!newPlatform)
		return;

	HierarchyTreePlatformNode* oldPlatform = GetPlatform();
	if (oldPlatform)
	{
		oldPlatform->RemoveTreeNode(this, false, false);
	}
	
	parent = newPlatform;
	GetScreen()->SetRect(Rect(0, 0, newPlatform->GetWidth(), newPlatform->GetHeight()));
	newPlatform->AddTreeNode(this, insertAfter);
}

HierarchyTreeNode* HierarchyTreeScreenNode::GetParent()
{
	return this->parent;
}

String HierarchyTreeScreenNode::GetNewControlName(const String& baseName) const
{
	int i = 0;
	while (true)
	{
		QString newName = QString().fromStdString(baseName) + QString::number(++i);
		
		if (!IsNameExist(newName, this))
			return newName.toStdString();
	}
}
			
bool HierarchyTreeScreenNode::IsNameExist(const QString &name, const HierarchyTreeNode *parent) const
{
	if (!parent)
		return false;
	
	const HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HIERARCHYTREENODESLIST::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		if (node->GetName().compare(name) == 0)
			return true;

		if (IsNameExist(name, node))
			return true;
	}
	
	return false;
}

bool HierarchyTreeScreenNode::Unload()
{
    if(loaded && !IsNeedSave())
    {
        Cleanup();
        SafeRelease(screen);
        this->screen = new ScreenControl();
        if (parent)
        {
            screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
        }
        loaded = false;
        return true;
    }
    return false;
}

bool HierarchyTreeScreenNode::Load(const QString& path)
{
    if(!loaded)
    {
        UIYamlLoader::Load(screen, path.toStdString(), false);
        guides.Load(path.toStdString());
        
        BuildHierarchyTree(this, screen->GetChildren());
        loaded = true;
    }
	return true;
}

void HierarchyTreeScreenNode::BuildHierarchyTree(HierarchyTreeNode* parent, List<UIControl*> child)
{
	for (List<UIControl*>::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		UIControl* uiControl = (*iter);
		
		HierarchyTreeControlNode* node = NULL;
		if (dynamic_cast<UIAggregatorControl*>(uiControl))
			node = new HierarchyTreeAggregatorControlNode(NULL, parent, uiControl, QString::fromStdString(uiControl->GetName()));
		else
			node = new HierarchyTreeControlNode(parent, uiControl, QString::fromStdString(uiControl->GetName()));

		// Build hierarchy tree for all control's children. Subcontrols are loaded separately
        InitializeControlBeforeAddingToTree(uiControl);
		BuildHierarchyTree(node, uiControl->GetRealChildren());
		parent->AddTreeNode(node);
	}
}

bool HierarchyTreeScreenNode::Save(const QString& path, bool saveAll)
{
	// Do not save the screen if it wasn't changed.
	if (!saveAll && !IsNeedSave())
	{
		return true;
	}

    //TODO: if there is still any reason to group fonts by IsEqual and save using one name (instead of using registered font name assuming all fonts are registered), use FontManager::Instance()->PrepareToSaveFonts();, otherwise:
    FontManager::Instance()->PrepareToSaveFonts(true);
    
	bool saveResult = UIYamlLoader::Save(screen, path.toStdString(), true);
	if (saveResult)
	{
        // Save the Guides - append their data to the existing screen YAML.
        saveResult = guides.Save(path.toStdString(), File::APPEND | File::WRITE);
    }

    if (saveResult)
    {
		ResetUnsavedChanges();
	}

	return saveResult;
}

void HierarchyTreeScreenNode::ReturnTreeNodeToScene()
{
	if (!this->redoParentNode)
	{
		return;
	}
	
	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);
}

void HierarchyTreeScreenNode::CombineRectWithChild(Rect& rect) const
{
	const HIERARCHYTREENODESLIST& childs = GetChildNodes();
	for (HIERARCHYTREENODESLIST::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
		if (!control || !control->GetUIObject())
			continue;
		
		Rect controlRect = control->GetUIObject()->GetGeometricData().GetAABBox();
		rect = rect.Combine(controlRect);
	}
}

Rect HierarchyTreeScreenNode::GetRect() const
{
	Rect rect(0, 0, GetPlatform()->GetWidth(), GetPlatform()->GetHeight());
	
	CombineRectWithChild(rect);
	
	return rect;
}

bool HierarchyTreeScreenNode::IsNeedSave() const
{
	return IsMarked() | (this->unsavedChangesCounter != 0);
}

void HierarchyTreeScreenNode::StartNewGuide(GuideData::eGuideType guideType)
{
    guides.StartNewGuide(guideType, GetControlRectsList(true));
}

void HierarchyTreeScreenNode::MoveNewGuide(const Vector2& pos)
{
    guides.MoveNewGuide(pos);
}

bool HierarchyTreeScreenNode::CanAcceptNewGuide() const
{
    return guides.CanAcceptNewGuide();
}

const GuideData* HierarchyTreeScreenNode::AcceptNewGuide()
{
    return guides.AcceptNewGuide();
}

void HierarchyTreeScreenNode::CancelNewGuide()
{
    guides.CancelNewGuide();
}

bool HierarchyTreeScreenNode::StartMoveGuide(const Vector2& pos)
{
    return guides.StartMoveGuide(pos, GetControlRectsList(true));
}

void HierarchyTreeScreenNode::MoveGuide(const Vector2& pos)
{
    guides.MoveGuide(pos);
}

const GuideData* HierarchyTreeScreenNode::GetMoveGuide() const
{
    return guides.GetMoveGuide();
}

const GuideData* HierarchyTreeScreenNode::AcceptMoveGuide()
{
    return guides.AcceptMoveGuide();
}

const GuideData* HierarchyTreeScreenNode::CancelMoveGuide()
{
    return guides.CancelMoveGuide();
}

Vector2 HierarchyTreeScreenNode::GetMoveGuideStartPos() const
{
    return guides.GetMoveGuideStartPos();
}

const List<GuideData*> HierarchyTreeScreenNode::GetSelectedGuides(bool includeNewGuide) const
{
    return guides.GetSelectedGuides(includeNewGuide);
}

const GuideData* HierarchyTreeScreenNode::GetActiveGuide() const
{
    // Firstly try if exactly one guide is selected.
    const List<GuideData*>& selectedGuides = guides.GetSelectedGuides(true);
    if (selectedGuides.empty())
    {
        return NULL;
    }
    
    if (selectedGuides.size() == 1)
    {
        return selectedGuides.front();
    }
    
    // Retry with the move guide.
    return guides.GetMoveGuide();
}

bool HierarchyTreeScreenNode::AreGuidesSelected() const
{
    return guides.AreGuidesSelected();
}

List<GuideData> HierarchyTreeScreenNode::DeleteSelectedGuides()
{
    return guides.DeleteSelectedGuides();
}

int32 HierarchyTreeScreenNode::CalculateStickToGuides(const List<Rect>& controlsRectList, Vector2& offset) const
{
    return guides.CalculateStickToGuides(controlsRectList, offset);
}

int32 HierarchyTreeScreenNode::GetGuideStickTreshold() const
{
    return guides.GetGuideStickTreshold();
}

void HierarchyTreeScreenNode::AddGuide(const GuideData& guideData)
{
    guides.AddGuide(guideData);
}

bool HierarchyTreeScreenNode::RemoveGuide(const GuideData& guideData)
{
    return guides.RemoveGuide(guideData);
}

bool HierarchyTreeScreenNode::UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos)
{
    return guides.UpdateGuidePosition(guideData, newPos);
}

void HierarchyTreeScreenNode::SetGuidePosition(GuideData* guideData, const Vector2& newPos)
{
    return guides.SetGuidePosition(guideData, newPos);
}

const List<GuideData*> HierarchyTreeScreenNode::GetGuides(bool includeNewGuide) const
{
    return guides.GetGuides(includeNewGuide);
}

int32 HierarchyTreeScreenNode::GetStickMode() const
{
    return guides.GetStickMode();
}

void HierarchyTreeScreenNode::SetStickMode(int32 stickMode)
{
    guides.SetStickMode(stickMode);
}

bool HierarchyTreeScreenNode::AreGuidesEnabled() const
{
    return guides.AreGuidesEnabled();
}

void HierarchyTreeScreenNode::SetGuidesEnabled(bool value)
{
    guides.SetGuidesEnabled(value);
}

void HierarchyTreeScreenNode::InitializeControlBeforeAddingToTree(UIControl* uiControl)
{
    // Hide WebView native control during load.
    UIWebView* webViewControl = dynamic_cast<UIWebView*>(uiControl);
    if (webViewControl)
    {
        webViewControl->SetNativeControlVisible(false);
    }
}

bool HierarchyTreeScreenNode::AreGuidesLocked() const
{
    return guides.AreGuidesLocked();
}

void HierarchyTreeScreenNode::LockGuides(bool value)
{
    guides.LockGuides(value);
}

const GuidesManager& HierarchyTreeScreenNode::GetGuidesManager() const
{
    return guides;
}

List<GuidesManager::StickedRect> HierarchyTreeScreenNode::GetControlRectsList(bool includeScreenBounds) const
{
    List<GuidesManager::StickedRect> rectsList;
    
    const HierarchyTreeNode::HIERARCHYTREENODESLIST& children = GetChildNodes();

    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = children.begin(); iter != children.end(); iter ++)
    {
        const HierarchyTreeControlNode* controlNode = static_cast<const HierarchyTreeControlNode*>(*iter);
        GetControlRectsListRecursive(controlNode, rectsList);
    }

    if (includeScreenBounds)
    {
        // Screen/platform bounds are always forced to be sticked.
        rectsList.push_back(GuidesManager::StickedRect(GetOwnRect(), true));
    }

    return rectsList;
}

Rect HierarchyTreeScreenNode::GetOwnRect() const
{
    const Vector2& screenSize = GetPlatform()->GetSize();
    return Rect(0, 0, screenSize.x, screenSize.y);
}

void HierarchyTreeScreenNode::GetControlRectsListRecursive(const HierarchyTreeControlNode* rootNode, List<GuidesManager::StickedRect>& rectsList) const
{
    // Inner controls aren't forced to be sticked.
    rectsList.push_back(GuidesManager::StickedRect(rootNode->GetUIObject()->GetGeometricData().GetAABBox(), false));

    const HierarchyTreeNode::HIERARCHYTREENODESLIST& children = rootNode->GetChildNodes();
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = children.begin(); iter != children.end(); iter ++)
    {
        const HierarchyTreeControlNode* childNode = static_cast<const HierarchyTreeControlNode*>(*iter);
        GetControlRectsListRecursive(childNode, rectsList);
    }
}