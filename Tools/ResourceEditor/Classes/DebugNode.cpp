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




#include "DebugNode.h"


DebugNode::DebugNode(std::vector<float32> &data)
    : Entity()
{
	verts = data;
	
	for (int32 i = 0; i < (int32)verts.size() / 3; i++)
	{
		for (int j = 0; j < 4; j++)
			colors.push_back(1.0f);
	}
	renderData = NULL;
	renderData = new RenderDataObject();
	renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, verts.data());
	renderData->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 0, colors.data());
	isDraw = true;
}

DebugNode::~DebugNode()
{
	SafeRelease(renderData); 
}

void DebugNode::Draw()
{
	Entity::Draw();
	if (!isDraw)
		return;
	if (verts.size() == 0)
		return;
	
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 meshFinalMatrix = GetWorldTransform() * prevMatrix;
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);	
    RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);

    RenderManager::Instance()->SetRenderData(renderData);
	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINELIST, 0, verts.size()/3);
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);    
}

