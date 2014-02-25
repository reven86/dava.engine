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



#include "LibraryController.h"
#include "HierarchyTreeControlNode.h"
#include "MetadataFactory.h"
#include "ScreenWrapper.h"
#include "DefaultScreen.h"
#include "IconHelper.h"

using namespace DAVA;

LibraryController::LibraryController()
{
	widget = NULL;
	aggregatorTemp = new UIAggregatorControl();
}

LibraryController::~LibraryController()
{
	SafeRelease(aggregatorTemp);
	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeNode* node = iter->first;
		SafeDelete(node);
	}
}

void LibraryController::Init(LibraryWidget* widget)
{
	this->widget = widget;
	AddControl("UIControl", new UIControl());
	AddControl("UIButton", new UIButton());
	AddControl("UIStaticText", new UIStaticText());
	AddControl("UITextField", new UITextField());
	AddControl("UISlider", new UISlider());
	AddControl("UIList", new UIList());
	AddControl("UIScrollBar", new UIScrollBar());
	AddControl("UIScrollView", new UIScrollView());
	AddControl("UISpinner", new UISpinner());
	AddControl("UISwitch", new UISwitch());
    AddControl("UIParticles", new UIParticles());
}

void LibraryController::AddControl(HierarchyTreeAggregatorNode* node)
{
	const QString& name = node->GetName();
	DVASSERT(controls.find(node) == controls.end());
	controls[node] = widget->AddControl(name, IconHelper::GetIconPathForClassName("UIAggregatorControl"));
}

void LibraryController::RemoveControl(HierarchyTreeAggregatorNode* node)
{
	CONTROLS::iterator iter = controls.find(node);
	if (iter == controls.end())
		return;
	
	widget->RemoveControl(iter->second);
	controls.erase(node);
}

void LibraryController::UpdateControl(HierarchyTreeAggregatorNode* node)
{
	CONTROLS::iterator iter = controls.find(node);
	if (iter == controls.end())
		return;

	widget->UpdateControl(iter->second, node->GetName());
}

void LibraryController::AddControl(const QString& name, UIControl* control)
{
	QString iconPath = IconHelper::GetIconPathForUIControl(control);
	controls[new HierarchyTreeControlNode(NULL, control, name)] = widget->AddControl(name, iconPath);
}

HierarchyTreeControlNode* LibraryController::CreateNewControl(HierarchyTreeNode* parentNode, const QString& strType, const QString& name, const Vector2& position)
{
	String type = strType.toStdString();

	HierarchyTreeControlNode* controlNode = NULL;
	UIControl* control = NULL;
	CONTROLS::iterator iter;
	
	for (iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeNode* node = iter->first;
		if (strType == node->GetName())
			break;
	}
	
	if (iter == controls.end() ||
		dynamic_cast<HierarchyTreeControlNode*>(iter->first))
	{
		//create standart control
		BaseObject* object = ObjectFactory::Instance()->New<BaseObject>(type);
		control = dynamic_cast<UIControl*>(object);
		if (!control)
		{
			SafeRelease(object);
			return NULL;
		}
		 
		controlNode = new HierarchyTreeControlNode(parentNode, control, name);
	}
	else
	{
		//create aggregator
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->first);
		if (aggregator)
		{
			controlNode = aggregator->CreateChild(parentNode, name);
			control = controlNode->GetUIObject();
		}
	}
	
	parentNode->AddTreeNode(controlNode);
	
	// In case the control has subcontrols - they should be added to the control node too.
	if (control && !control->GetSubcontrols().empty())
	{
		List<UIControl*> subControls = control->GetSubcontrols();
		for (List<UIControl*>::iterator iter = subControls.begin(); iter != subControls.end(); iter ++)
		{
			UIControl* subControl = (*iter);
			if (!subControl)
			{
				continue;
			}

			HierarchyTreeControlNode* subControlNode =
				new HierarchyTreeControlNode(controlNode, subControl,
											 QString::fromStdString(subControl->GetName()));
			controlNode->AddTreeNode(subControlNode);
		}
	}

	// Initialize a control through its metadata.
	BaseMetadata* newControlMetadata = MetadataFactory::Instance()->GetMetadataForUIControl(control);

	METADATAPARAMSVECT params;
	params.push_back(BaseMetadataParams(controlNode->GetId(), control));
	newControlMetadata->SetupParams(params);

	// Ready to do initialization!
	newControlMetadata->InitializeControl(name.toStdString(), position);

	SAFE_DELETE(newControlMetadata);

	SafeRelease(control);
	return controlNode;
}

void LibraryController::UpdateLibrary()
{
	HierarchyTreePlatformNode* activePlatform = HierarchyTreeController::Instance()->GetActivePlatform();
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->first);
		if (!node)
			continue;
		
		bool visible = node->GetPlatform() == activePlatform;
		if (dynamic_cast<HierarchyTreeAggregatorNode*>(activeScreen))
		{
			visible = false;
		}
		widget->SetItemVisible(iter->second, visible);
	}
	// DF-1488 - Reset treeview widget state, including selection
	widget->ResetSelection();
}