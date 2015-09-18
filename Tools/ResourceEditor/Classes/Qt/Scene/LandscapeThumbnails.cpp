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


#include "LandscapeThumbnails.h"
#include "Base/Platform.h"
#include "Concurrency/LockGuard.h"
#include "Render/RenderCallbacks.h"
#include "Render/ShaderCache.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderPassNames.h"

using namespace DAVA;

namespace
{
struct ThumbnailRequest
{
	rhi::HSyncObject syncObject;
	Landscape* landscape = nullptr;
	Texture* texture = nullptr;
	LandscapeThumbnails::Callback callback;

	ThumbnailRequest(rhi::HSyncObject so, Landscape* l, Texture* tex, LandscapeThumbnails::Callback cb) :
		syncObject(so), landscape(l), texture(tex), callback(cb) { }
};

struct Requests
{
	DAVA::Mutex mutex;
	Vector<ThumbnailRequest> list;
};

static Requests requests;

void OnCreateLandscapeTextureCompleted(rhi::HSyncObject syncObject)
{
	Vector<ThumbnailRequest> completedRequests;
	completedRequests.reserve(requests.list.size());
	{
		DAVA::LockGuard<DAVA::Mutex> lock(requests.mutex);
		auto i = requests.list.begin();
		while (i != requests.list.end())
		{
			if (i->syncObject == syncObject)
			{
				completedRequests.push_back(*i);
				i = requests.list.erase(i);
			}
			else 
			{
				++i;
			}
		}
	}

	for (const auto& req : completedRequests)
	{
		req.callback(req.landscape, req.texture);
	}
}

}

void LandscapeThumbnails::Create(DAVA::Landscape* landscape, LandscapeThumbnails::Callback handler)
{
	const uint32 TEXTURE_TILE_FULL_SIZE = 2048;

    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	ScopedPtr<PolygonGroup> renderData(new PolygonGroup());
	renderData->AllocateData(EVF_VERTEX | EVF_TEXCOORD0, 4, 6);
	renderData->SetPrimitiveType(rhi::PrimitiveType::PRIMITIVE_TRIANGLELIST);
	renderData->SetCoord(0, Vector3(-1.0f, -1.0f, 0.0f));
    renderData->SetCoord(1, Vector3( 1.0f, -1.0f, 0.0f));
    renderData->SetCoord(2, Vector3(-1.0f,  1.0f, 0.0f));
    renderData->SetCoord(3, Vector3( 1.0f,  1.0f, 0.0f));
    renderData->SetTexcoord(0, 0, Vector2(0.0f, 0.0f));
    renderData->SetTexcoord(0, 1, Vector2(1.0f, 0.0f));
    renderData->SetTexcoord(0, 2, Vector2(0.0f, 1.0f));
    renderData->SetTexcoord(0, 3, Vector2(1.0f, 1.0f));
	renderData->SetIndex(0, 0);
	renderData->SetIndex(1, 1);
	renderData->SetIndex(2, 2);
	renderData->SetIndex(3, 2);
	renderData->SetIndex(4, 1);
	renderData->SetIndex(5, 3);
	renderData->BuildBuffers();

	rhi::HSyncObject syncObject = rhi::CreateSyncObject();
	Texture* texture = Texture::CreateFBO(TEXTURE_TILE_FULL_SIZE, TEXTURE_TILE_FULL_SIZE, FORMAT_RGBA8888);
	{
		DAVA::LockGuard<DAVA::Mutex> lock(requests.mutex);
		requests.list.emplace_back(syncObject, landscape, texture, handler);
	}
	RenderCallbacks::RegisterSyncCallback(syncObject, MakeFunction(&OnCreateLandscapeTextureCompleted));

	const auto identityMatrix = &Matrix4::IDENTITY;
	DAVA::ShaderDescriptorCache::ClearDynamicBindigs();
	Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, identityMatrix, (pointer_size)(identityMatrix));
	Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEW, identityMatrix, (pointer_size)(identityMatrix));
	Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, identityMatrix, (pointer_size)(identityMatrix));

	rhi::Packet packet = { };
    packet.vertexStreamCount = 1;
    packet.vertexStream[0] = renderData->vertexBuffer;
    packet.vertexCount = renderData->vertexCount;
    packet.indexBuffer = renderData->indexBuffer;
    packet.primitiveType = renderData->primitiveType;
    packet.primitiveCount = GetPrimitiveCount(renderData->indexCount, renderData->primitiveType);
    packet.vertexLayoutUID = renderData->vertexLayoutId;

	landscape->GetMaterial()->BindParams(packet);

	rhi::RenderPassConfig passDesc = { };
	passDesc.colorBuffer[0].texture = texture->handle;
	passDesc.priority = PRIORITY_SERVICE_3D;
	passDesc.viewport.width = TEXTURE_TILE_FULL_SIZE;
	passDesc.viewport.height = TEXTURE_TILE_FULL_SIZE;

	rhi::HPacketList packetList = { };
	rhi::HRenderPass renderPass = rhi::AllocateRenderPass(passDesc, 1, &packetList);
	rhi::BeginRenderPass(renderPass);
	rhi::BeginPacketList(packetList);
	rhi::AddPacket(packetList, packet);
	rhi::EndPacketList(packetList, syncObject);
	rhi::EndRenderPass(renderPass);
}
