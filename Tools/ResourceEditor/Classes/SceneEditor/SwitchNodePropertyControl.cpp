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



#include "SwitchNodePropertyControl.h"

SwitchNodePropertyControl::SwitchNodePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

void SwitchNodePropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	propertyList->AddSection("Switch node", GetHeaderState("Switch node", true));

	Vector<String> switchIndeces;
	int32 childrenCount = sceneNode->GetChildrenCount();
	for(int32 i = 0; i < childrenCount; i++) 
	{
		switchIndeces.push_back(Format("%d", i));
	}
	if(switchIndeces.empty())
	{
		switchIndeces.push_back("0");
	}

    
    SwitchComponent *switchComponent = static_cast<SwitchComponent *>(sceneNode->GetComponent(Component::SWITCH_COMPONENT));
    if(switchComponent)
    {
        propertyList->AddComboProperty("Switch index", switchIndeces);
        propertyList->SetComboPropertyIndex("Switch index", switchComponent->GetSwitchIndex());
    }
}

void SwitchNodePropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    SwitchComponent *switchComponent = static_cast<SwitchComponent *>(currentSceneNode->GetComponent(Component::SWITCH_COMPONENT));
    if(switchComponent)
    {
        if("Switch index" == forKey)
        {
            switchComponent->SetSwitchIndex(newItemIndex);
        }
    }
    
	NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}
