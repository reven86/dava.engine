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



#include "ImposterManager.h"
#include "Render/SharedFBO.h"
#include "Utils/Utils.h"
#include "Debug/Stats.h"

namespace DAVA
{

ImposterManager::ImposterManager(Scene * _scene)
    :	sharedFBO(0)
    ,   scene(_scene)
{
	CreateFBO();
}

void ImposterManager::CreateFBO()
{
	if(!sharedFBO)
	{
		SharedFBO::Setup setup;
		setup.size = Vector2(2048, 1024);
		setup.pixelFormat = FORMAT_RGBA4444;
		setup.depthFormat = Texture::DEPTH_RENDERBUFFER;
		setup.blocks.push_back(std::pair<int32, Vector2>(16, Vector2(256.f, 256.f)));
		setup.blocks.push_back(std::pair<int32, Vector2>(32, Vector2(128.f, 128.f)));
		setup.blocks.push_back(std::pair<int32, Vector2>(112, Vector2(64.f, 64.f)));
		setup.blocks.push_back(std::pair<int32, Vector2>(64, Vector2(32.f, 32.f)));

		sharedFBO = new SharedFBO(&setup);
	}
}

void ImposterManager::ReleaseFBO()
{
	SafeRelease(sharedFBO);
	queue.clear();
	List<ImposterNode*>::iterator end = imposters.end();
	for(List<ImposterNode*>::iterator iter = imposters.begin(); iter != end; ++iter)
	{
		ImposterNode * node = *iter;
		node->ZeroOutBlock();
	}
}

ImposterManager::~ImposterManager()
{
	ReleaseFBO();
}

bool ImposterManager::IsEmpty()
{
	return imposters.empty();
}

void ImposterManager::Update(float32 frameTime)
{
	List<ImposterNode*>::iterator end = imposters.end();
	for(List<ImposterNode*>::iterator iter = imposters.begin(); iter != end; ++iter)
	{
		ImposterNode * node = *iter;
		node->UpdateState();
	}
}

void ImposterManager::Draw()
{
	ProcessQueue();

	List<ImposterNode*>::iterator end = imposters.end();
	for(List<ImposterNode*>::iterator iter = imposters.begin(); iter != end; ++iter)
	{
		(*iter)->GeneralDraw();
	}
}

void ImposterManager::ProcessQueue()
{
	if(!queue.empty())
	{
		Camera * camera = scene->GetCurrentCamera();
		RenderManager::Instance()->SetRenderTarget(sharedFBO->GetTexture());

		int32 maxCount = Min(MAX_UPDATES_PER_FRAME, (int32)queue.size());
		for(int32 i = 0; i < maxCount; ++i)
		{
			ImposterNode * node = queue.front();
			queue.pop_front();
			node->UpdateImposter();
		}

#if defined(__DAVAENGINE_OPENGL__)
		RenderManager::Instance()->HWglBindFBO(RenderManager::Instance()->GetFBOViewFramebuffer());
#endif //#if defined(__DAVAENGINE_OPENGL__)
		camera->SetupDynamicParameters();
	}
}

void ImposterManager::Add(ImposterNode * node)
{
	//Add can be called multiple times for one node, because ImposterNode::SetScene can be called multiple times
	List<ImposterNode*>::iterator iter = find(imposters.begin(), imposters.end(), node);
	if(imposters.end() == iter)
	{
		node->SetManager(this);
		imposters.push_back(node);
	}
}

void ImposterManager::Remove(ImposterNode * node)
{
	List<ImposterNode*>::iterator iter = find(imposters.begin(), imposters.end(), node);
	if(imposters.end() == iter)
	{
		Logger::Error("ImposterManager::Remove node not found");
	}
	else
	{
		node->SetManager(0);
		imposters.erase(iter);
	}
}

void ImposterManager::AddToQueue(ImposterNode * node)
{
	AddToPrioritizedPosition(node);
}

void ImposterManager::UpdateQueue(ImposterNode * node)
{
	RemoveFromQueue(node);
	AddToPrioritizedPosition(node);
}

void ImposterManager::AddToPrioritizedPosition(ImposterNode * node)
{
	List<ImposterNode*>::iterator endIterator = queue.end();
	for(List<ImposterNode*>::iterator iter = queue.begin(); iter != endIterator; ++iter)
	{
		if(node->GetPriority() < (*iter)->GetPriority())
		{
			queue.insert(++iter, node);
			return;
		}
	}
	
	//if queue is empty or node has highest priority
	queue.push_back(node);
}

void ImposterManager::RemoveFromQueue(ImposterNode * node)
{
	List<ImposterNode*>::iterator endIterator = queue.end();
	for(List<ImposterNode*>::iterator iter = queue.begin(); iter != endIterator; ++iter)
	{
		if((*iter) == node)
		{
			queue.erase(iter);
			return;
		}
	}
}

SharedFBO * ImposterManager::GetFBO()
{
	return sharedFBO;
}

};
