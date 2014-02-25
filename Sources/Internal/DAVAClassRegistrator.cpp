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

#include "DAVAEngine.h"
#include "DAVAClassRegistrator.h"

using namespace DAVA;

void DAVA::RegisterDAVAClasses()
{
    //this code do nothing. Needed to compiler generate code from this cpp file
    Logger * log = Logger::Instance();
    if(log)
        log->Log(Logger::LEVEL__DISABLE, "");
}

#if !defined(__DAVAENGINE_ANDROID__)
REGISTER_CLASS(TheoraPlayer);
#endif

REGISTER_CLASS(PolygonGroup);
REGISTER_CLASS(StaticMesh);
REGISTER_CLASS(Camera);
REGISTER_CLASS(UIScrollViewContainer);
REGISTER_CLASS(UISlider);
REGISTER_CLASS(UISpinner);
REGISTER_CLASS(UIStaticText);
REGISTER_CLASS(LandscapeChunk);
REGISTER_CLASS(UISwitch);
REGISTER_CLASS(UITextField);
REGISTER_CLASS(Landscape);
REGISTER_CLASS(UIAggregatorControl);
REGISTER_CLASS(Light);
REGISTER_CLASS(Mesh);
REGISTER_CLASS(SpeedTreeObject);
REGISTER_CLASS(RenderBatch);
REGISTER_CLASS(RenderObject);
REGISTER_CLASS(ShadowVolume);
REGISTER_CLASS(SkyboxRenderObject);
REGISTER_CLASS(InstanceMaterialState);
REGISTER_CLASS(Material);
REGISTER_CLASS(NMaterial);
REGISTER_CLASS(ImposterNode);
REGISTER_CLASS(BillboardNode);
REGISTER_CLASS(BoneNode);
REGISTER_CLASS(DataNode);
REGISTER_CLASS(Entity);
REGISTER_CLASS(LodNode);
REGISTER_CLASS(MeshInstanceNode);
REGISTER_CLASS(ParticleEffectNode);
REGISTER_CLASS(ParticleEmitterNode);
REGISTER_CLASS(ProxyNode);
REGISTER_CLASS(Scene);
REGISTER_CLASS(ShadowVolumeNode);
REGISTER_CLASS(SkeletonNode);
REGISTER_CLASS(SwitchNode);
REGISTER_CLASS(UserNode);
REGISTER_CLASS(UIButton);
REGISTER_CLASS(UIControl);
REGISTER_CLASS(UIList);
REGISTER_CLASS(UIListCell);
REGISTER_CLASS(UIScrollBar);
REGISTER_CLASS(UIScrollView);
REGISTER_CLASS(StaticOcclusionComponent);
REGISTER_CLASS(StaticOcclusionDataComponent);
REGISTER_CLASS_WITH_ALIAS(PartilceEmitterLoadProxy, "ParticleEmitter3D");
REGISTER_CLASS(UIWebView);
REGISTER_CLASS(UIParticles);
