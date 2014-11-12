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


#include "CopyPasteHelper.h"
#include "CommandsController.h"
#include "ItemsCommand.h"
#include "HierarchyTreeAggregatorControlNode.h"

namespace DAVA {

QString CopyPasteHelper::FormatCopyName(QString baseName, const HierarchyTreeNode* parent)
{
	QString name = baseName;
	QString numberName;
	const char* cName = name.toStdString().c_str();
	for (int i = name.length() - 1; i >= 0; --i)
	{
		char a = cName[i];
		if (a >= '0' && a <= '9')
			numberName = a + numberName;
		else
			break;
	}
	int id = numberName.toInt();
	baseName = name.left(name.length() - numberName.length());
	Logger::Debug(baseName.toStdString().c_str());
	
	const HierarchyTreeRootNode* parentRoot = dynamic_cast<const HierarchyTreeRootNode*>(parent);
	const HierarchyTreePlatformNode* parentPlatform = dynamic_cast<const HierarchyTreePlatformNode*>(parent);
	const HierarchyTreeScreenNode* parentScreen = dynamic_cast<const HierarchyTreeScreenNode*>(parent);
	if (!parentScreen)
	{
		const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode*>(parent);
		if (parentControl)
		{
			parentScreen = parentControl->GetScreenNode();
		}
	}
	
	for (int i = 0; i < 1000; i++)
	{
		name = QString("%1%2").arg(baseName).arg(++id);
		Logger::Debug(name.toStdString().c_str());

		bool bFind = false;
		
		if (parentPlatform || parentRoot)
		{
			// check name only for one child level for screen and platform copy
			const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = parent->GetChildNodes();
			for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
				 iter != child.end();
				 ++iter)
			{
				const HierarchyTreeNode* node = (*iter);
				if (node->GetName().compare(name) == 0)
					bFind = true;
			}
			if (!bFind)
				return name;
		}
		else
		{
			// copy control
			if (parentScreen)
			{
				if (!parentScreen->IsNameExist(name, parentScreen))
					return name;
			}
		}
	}
	return baseName;
}

void CopyPasteHelper::UpdateAggregators(HierarchyTreeNode* controlNode, HierarchyTreeNode* targetNode)
{
    HierarchyTreePlatformNode* activePlatform = NULL;
    HierarchyTreeControlNode* targetControl = dynamic_cast<HierarchyTreeControlNode*>(targetNode);
    HierarchyTreeScreenNode* targetScreen = dynamic_cast<HierarchyTreeScreenNode*>(targetNode);
    HierarchyTreePlatformNode* targetPlatform = dynamic_cast<HierarchyTreePlatformNode*>(targetNode);
    
    if (targetControl)
    {
    	activePlatform = targetControl->GetScreenNode()->GetPlatform();
    }
    else if (targetScreen)
    {
    	activePlatform = targetScreen->GetPlatform();
    }
    else if (targetPlatform)
    {
    	activePlatform = targetPlatform;
    }
                
    if (activePlatform)
    {
    	UpdateAggregatorControls(controlNode, activePlatform);
    }
}

void CopyPasteHelper::UpdateAggregatorControls(HierarchyTreeNode* control, HierarchyTreePlatformNode* activePlatform)
{
	HierarchyTreeAggregatorControlNode* aggregator = dynamic_cast<HierarchyTreeAggregatorControlNode*>(control);
    if (aggregator)
    {
    	const HierarchyTreeAggregatorNode* parentAggregator = aggregator->GetAggregatorNode();
        // Check if we aggregator is present of active platform - if not - create new with such name
        if (parentAggregator)
        {
        	HierarchyTreeAggregatorNode *aggregatorNode = NULL;
            QString aggregatorName = parentAggregator->GetName();
        	if (!activePlatform->IsAggregatorOrScreenNamePresent(aggregatorName))
			{
                CreateAggregatorCommand* cmd = new CreateAggregatorCommand(aggregatorName, activePlatform->GetId(), Rect());
				CommandsController::Instance()->ExecuteCommand(cmd);
				SafeRelease(cmd);
            }

 			aggregatorNode = activePlatform->GetAggregatorNodeByName(aggregatorName);
            if (!aggregatorNode) return;
            
            aggregator->SetAggregatorNode(aggregatorNode);
            aggregatorNode->UpdateChilds();
        }
    }
    
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = control->GetChildNodes().begin();
			 iter != control->GetChildNodes().end();
			 ++iter)
	{
		HierarchyTreeNode* childControl = (*iter);
		UpdateAggregatorControls(childControl, activePlatform);
	}
}

};