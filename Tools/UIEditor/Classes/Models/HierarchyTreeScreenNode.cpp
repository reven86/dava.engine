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
	HierarchyTreeNode(name)
{
	this->parent = parent;
	this->screen = new ScreenControl();
	screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
	
	scale = 1.f;
	
	posX = POSITION_UNDEFINED;
	posY = POSITION_UNDEFINED;
}

HierarchyTreeScreenNode::HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const HierarchyTreeScreenNode* base):
	HierarchyTreeNode(base)
{
	this->parent = parent;
	this->screen = new ScreenControl();
	if (parent)
		screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
	
	scale = 1.f;
	posX = 0;
	posY = 0;

	unsavedChangesCounter = 0;

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& chilren = base->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = chilren.begin();
		 iter != chilren.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* baseControl = dynamic_cast<const HierarchyTreeControlNode* >((*iter));
		if (!baseControl)
			continue;
		
		HierarchyTreeControlNode* control = new HierarchyTreeControlNode(this, baseControl);
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

bool HierarchyTreeScreenNode::Load(const QString& path)
{
	ScopedPtr<UIYamlLoader> loader( new UIYamlLoader() );
	loader->Load(screen, path.toStdString());
	
	BuildHierarchyTree(this, screen->GetChildren());
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

	FontManager::Instance()->PrepareToSaveFonts();
	bool saveResult = UIYamlLoader::Save(screen, path.toStdString(), true);
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
		if (!control)
			continue;
		
		Rect controlRect = control->GetRect();
		
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
