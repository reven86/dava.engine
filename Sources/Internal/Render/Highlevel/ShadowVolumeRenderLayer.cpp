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


#include "Render/Highlevel/ShadowVolumeRenderLayer.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Renderer.h"
#include "Render/RenderCallbacks.h"

namespace DAVA
{

ShadowVolumeRenderLayer::ShadowVolumeRenderLayer(eRenderLayerID id, uint32 sortingFlags) : RenderLayer(id, sortingFlags)
{
    PrepareRenderData();
    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &ShadowVolumeRenderLayer::Restore));
}

ShadowVolumeRenderLayer::~ShadowVolumeRenderLayer()
{
    rhi::DeleteVertexBuffer(quadBuffer);
    SafeRelease(shadowRectMaterial);
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &ShadowVolumeRenderLayer::Restore));
}

const static uint32 VERTEX_COUNT = 6;
std::array<Vector3, VERTEX_COUNT> quad =
{
  Vector3(-1.f, -1.f, 1.f), Vector3(-1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f),
  Vector3(-1.f, 1.f, 1.f), Vector3(1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f),
};

void ShadowVolumeRenderLayer::Restore()
{
    if (rhi::NeedRestoreVertexBuffer(quadBuffer))
    {
        rhi::UpdateVertexBuffer(quadBuffer, quad.data(), 0, sizeof(Vector3) * VERTEX_COUNT);
    }
}

void ShadowVolumeRenderLayer::PrepareRenderData()
{
    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(Vector3) * VERTEX_COUNT;
    vDesc.initialData = quad.data();
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    rhi::VertexLayout vxLayout;
    vxLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    shadowRectPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(vxLayout);

    shadowRectPacket.vertexStreamCount = 1;
    shadowRectPacket.vertexStream[0] = quadBuffer;
    shadowRectPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    shadowRectPacket.primitiveCount = 2;


    shadowRectMaterial = new NMaterial();
    shadowRectMaterial->SetFXName(NMaterialName::SHADOWRECT);
    shadowRectMaterial->PreBuildMaterial(PASS_FORWARD);
}

void ShadowVolumeRenderLayer::Draw(Camera* camera, const RenderBatchArray & renderBatchArray, rhi::HPacketList packetList)
{	
    if(!QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_STENCIL_SHADOW))
    {
        return;
    }

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::SHADOWVOLUME_DRAW) && renderBatchArray.GetRenderBatchCount())
    {
        RenderLayer::Draw(camera, renderBatchArray, packetList);

        shadowRectMaterial->BindParams(shadowRectPacket);
        rhi::AddPacket(packetList, shadowRectPacket);
	}
}
    
};
