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

static const QString AGGREGATOR_CONTROL_NAME = "UIAggregatorControl";

LibraryController::LibraryController()
{
	widget = NULL;
}

LibraryController::~LibraryController()
{
	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeControlNode* node = dynamic_cast<HierarchyTreeControlNode*>(iter->first);
        if(node)
        {
			SafeDelete(node);
        }
	}
}

void LibraryController::Init(LibraryWidget* widget)
{
	this->widget = widget;
    AddControl("UIControl"      , new UIControl());
    AddControl("UIButton"       , new UIButton());
	AddControl("UIStaticText"   , new UIStaticText());
	AddControl("UITextField"    , new UITextField());
	AddControl("UISlider"       , new UISlider());
	AddControl("UIList"         , new UIList());
	AddControl("UIScrollBar"    , new UIScrollBar());
	AddControl("UIScrollView"   , new UIScrollView());
	AddControl("UISpinner"      , new UISpinner());
	AddControl("UISwitch"       , new UISwitch());
    AddControl("UIParticles"    , new UIParticles());
	AddControl("UIWebView"      , new UIWebView());
    AddControl("UIMovieView"    , new UIMovieView());
    AddControl("UIJoypad"       , new UIJoypad());
    AddControl("UI3DView"       , new UI3DView());
    AddControl("UIListCell"     , new UIListCell());
}

void LibraryController::AddControl(const QString& name, UIControl* control)
{
	QString iconPath = IconHelper::GetIconPathForUIControl(control);
    HierarchyTreeControlNode *node = new HierarchyTreeControlNode(NULL, control, name);
	controls[node] = widget->AddControl(name, iconPath, node->GetId());
}

bool LibraryController::IsControlNameExists(const QString& name)
{
    CONTROLS::iterator iter;
    for (iter = controls.begin(); iter != controls.end(); ++iter)
    {
        if(iter->first->GetName() == name)
        {
            return true;
        }
    }
    return false;
}

HierarchyTreeControlNode* LibraryController::CreateNewControl(HierarchyTreeNode* parentNode,
																HierarchyTreeNode::HIERARCHYTREENODEID typeId,
                                                                const Vector2& position)
{
	HierarchyTreeControlNode* controlNode = NULL;
	UIControl* control = NULL;
	CONTROLS::iterator iter;
    
    for (iter = controls.begin(); iter != controls.end(); ++iter)
    {
    	HierarchyTreeNode* node = iter->first;
        if (node->GetId() == typeId)
        	break;
    }
    
    HierarchyTreeScreenNode *activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (iter == controls.end() || !activeScreen)
    	return NULL;
    
    // Get control name from library widget item
    QTreeWidgetItem *item = iter->second;
    String type = item->text(0).toStdString();
    String name = activeScreen->GetNewControlName(type);
    
    // Create aggregator
    HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->first);
    if (aggregator)
	{
       	controlNode = aggregator->CreateChild(parentNode, QString::fromStdString(name));
		control = controlNode->GetUIObject();
    }
	else
	{
		//create standart control
		BaseObject* object = ObjectFactory::Instance()->New<BaseObject>(type);
		control = dynamic_cast<UIControl*>(object);
		if (!control)
		{
			SafeRelease(object);
			return NULL;
		}
		 
		controlNode = new HierarchyTreeControlNode(parentNode, control, QString::fromStdString(name));
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

			HierarchyTreeControlNode* subControlNode = new HierarchyTreeControlNode(controlNode, subControl,
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
	newControlMetadata->InitializeControl(name, position);

	SAFE_DELETE(newControlMetadata);

	SafeRelease(control);
	return controlNode;
}

void LibraryController::UpdateLibrary()
{
    CONTROLS aggregatorControls(controls);
    // Remove current aggregators from widget
    for (CONTROLS::iterator iter = aggregatorControls.begin(); iter != aggregatorControls.end(); ++iter)
    {
        HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->first);
        
        if (!node)
        	continue;
        
        widget->RemoveControl(iter->second);
        controls.erase(node);
    }
    
	// No need to add aggregators into library if another aggregator is selected in tree
    if (dynamic_cast<HierarchyTreeAggregatorNode*>(HierarchyTreeController::Instance()->GetActiveScreen()))
    	return;
    
    List<HierarchyTreeAggregatorNode*> aggregatorsList = GetPlatformAggregators();
    // Add aggregators
    for (List<HierarchyTreeAggregatorNode*>::iterator it = aggregatorsList.begin(); it != aggregatorsList.end(); ++it)
    {
    	HierarchyTreeAggregatorNode* aggregatorNode = (*it);
        
        const QString& name = aggregatorNode->GetName();
        DVASSERT(controls.find(aggregatorNode) == controls.end());
        QTreeWidgetItem* item = widget->AddControl(name,
                                                    	IconHelper::GetIconPathForClassName(AGGREGATOR_CONTROL_NAME),
                                                        aggregatorNode->GetId());
        
        controls[aggregatorNode] = item;
	}
    
   	// DF-1488 - Reset treeview widget state, including selection
	widget->ResetSelection();
}

List<HierarchyTreeAggregatorNode*> LibraryController::GetPlatformAggregators() const
{
	HierarchyTreePlatformNode* activePlatform = HierarchyTreeController::Instance()->GetActivePlatform();
	List<HierarchyTreeAggregatorNode*> aggregatorsList;
    
    if (!activePlatform)
    	return aggregatorsList;

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = activePlatform->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>((*iter));
		if (!aggregatorNode)
			continue;
				
		aggregatorsList.push_back(aggregatorNode);
	}
    
	return aggregatorsList;
}