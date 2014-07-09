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



#include "UI/UIParticles.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Core/Core.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA {

/* this camera is required just for preparing draw data*/
UIParticles::ParticleCameraWrap UIParticles::defaultCamera;
UIParticles::ParticleCameraWrap::ParticleCameraWrap():camera(new Camera())
{
    camera->SetPosition(Vector3(0,0,-1));
    camera->SetUp(Vector3(0,-1,0));    
    camera->RebuildCameraFromValues();
    camera->RebuildViewMatrix();

}
UIParticles::ParticleCameraWrap::~ParticleCameraWrap()
{
    SafeRelease(camera);
}

UIParticles::UIParticles(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)    
    , effect(NULL)
    , system(new ParticleEffectSystem(NULL, true))
    , updateTime(0)
    , isAutostart(false)
    , startDelay(0.0f)
    , delayedActionType(UIParticles::actionNone)
    , delayedActionTime(0.0f)
    , delayedDeleteAllParticles(false)
{
    matrix.Identity();    
}

UIParticles::~UIParticles()
{    
    if (effect&&effect->state!=ParticleEffectComponent::STATE_STOPPED)
        system->RemoveFromActive(effect);
    SafeDelete(system);
    SafeDelete(effect);
}

void UIParticles::WillAppear()
{
    updateTime = 0.0f;
}

void UIParticles::Start()
{
    if (FLOAT_EQUAL(startDelay, 0.0f))
    {
        DoStart();
    }
    else
    {
        delayedActionType = actionStart;
        delayedActionTime = 0.0f;
    }
}
    
void UIParticles::DoStart()
{
    DVASSERT(effect);
    updateTime = 0;

    if (effect->state == ParticleEffectComponent::STATE_STARTING ||
        effect->state == ParticleEffectComponent::STATE_PLAYING)
    {
        return;
    }

    effect->isPaused = false;
    system->AddToActive(effect);
    effect->effectRenderObject->SetEffectMatrix(&matrix);
    system->RunEffect(effect);
}

void UIParticles::Stop(bool isDeleteAllParticles)
{
    DVASSERT(effect);
    updateTime = 0;
    
    if (effect->state == ParticleEffectComponent::STATE_STOPPED) return;
    
    if (isDeleteAllParticles)
    {
        effect->ClearCurrentGroups();		
        effect->effectData.infoSources.resize(1);        
        effect->isPaused = false;
        system->RemoveFromActive(effect);
    }
    else
    {
        effect->state = ParticleEffectComponent::STATE_STOPPING;
    }
}

void UIParticles::Pause(bool isPaused /*= true*/)
{	
    DVASSERT(effect);
    effect->isPaused = isPaused;
}

bool UIParticles::IsStopped()
{
    DVASSERT(effect);
    return effect->state == ParticleEffectComponent::STATE_STOPPED;
}

bool UIParticles::IsPaused()
{
    DVASSERT(effect);
    return effect->isPaused;
}

void UIParticles::Restart(bool isDeleteAllParticles)
{
    delayedDeleteAllParticles = isDeleteAllParticles;
    if (FLOAT_EQUAL(startDelay, 0.0f))
    {
        DoRestart();
    }
    else
    {
        delayedActionType = actionRestart;
        delayedActionTime = 0.0f;
    }
}
    
void UIParticles::DoRestart()
    {
    DVASSERT(effect);
    effect->isPaused = false;
    if (delayedDeleteAllParticles)
    {
        effect->ClearCurrentGroups();
    }

    effect->currRepeatsCont = 0;
    system->RunEffect(effect);
}

void UIParticles::AddControl(UIControl *control)
{
    DVASSERT(0 && "UIParticles do not support children");
}

    
void UIParticles::Update(float32 timeElapsed)
{
    updateTime = timeElapsed;        
    if (delayedActionType != UIParticles::actionNone)
    {
        HandleDelayedAction(timeElapsed);
    }
}

void UIParticles::Draw(const UIGeometricData & geometricData)
{
    if ((!effect)||(effect->state == ParticleEffectComponent::STATE_STOPPED)) 
        return;

    matrix.CreateRotation(Vector3(0,0,1), -geometricData.angle);
    matrix.SetTranslationVector(Vector3(geometricData.position.x, geometricData.position.y, 0));
    system->Process(updateTime);
    updateTime = 0;    		        
    
    effect->effectRenderObject->PrepareToRender(defaultCamera.camera);
    for (int32 i=0, sz = effect->effectRenderObject->GetActiveRenderBatchCount(); i<sz; ++i)
        effect->effectRenderObject->GetActiveRenderBatch(i)->Draw(PASS_FORWARD, defaultCamera.camera);
}

void UIParticles::Load(const FilePath& path)
{
    SceneFileV2 *sceneFile = new SceneFileV2();
    sceneFile->EnableDebugLog(false);
    SceneArchive *archive = sceneFile->LoadSceneArchive(path);    
    SafeRelease(sceneFile);
    if (archive && archive->children.size()>0)
    {
        Entity *e = new Entity();
        SerializationContext serializationContext;
        serializationContext.SetRootNodePath(path);
        serializationContext.SetScenePath(FilePath(path.GetDirectory()));
        serializationContext.SetVersion(10);
        serializationContext.SetScene(NULL);
        serializationContext.SetDefaultMaterialQuality(NMaterial::DEFAULT_QUALITY_NAME);
        e->Load(archive->children[0]->archive, &serializationContext);
        ParticleEffectComponent *effSrc = GetEffectComponent(e);
        if (effSrc)
            effect = (ParticleEffectComponent*)effSrc->Clone(NULL);
        SafeRelease(e);
    }

    SafeRelease(archive);

    if (effect)
    {
        effect->effectRenderObject->SetEffectMatrix(&matrix);
        effect->effectRenderObject->Set2DMode(true);
        effectPath = path;
        
        HandleAutostart();
    }
    else
    {
        effectPath = FilePath();
    }
}

void UIParticles::Reload()
{
    if (effectPath.IsEmpty())
    {
        DVASSERT_MSG(false, "You have to load UIPartilces effect prior to calling Reload()");
        return;
    }

    Load(effectPath);
}

const FilePath& UIParticles::GetEffectPath() const
{
    return effectPath;
}

void UIParticles::SetAutostart(bool value)
{
    isAutostart = value;
    HandleAutostart();
}

bool UIParticles::IsAutostart() const
{
    return isAutostart;
}
   
YamlNode * UIParticles::SaveToYamlNode(UIYamlLoader * loader)
{
    UIParticles* baseControl = new UIParticles();

    YamlNode *node = UIControl::SaveToYamlNode(loader);
    
    if (baseControl->GetEffectPath() != effectPath)
    {
        node->Set("effectPath", effectPath.GetFrameworkPath());
    }
    
    if (baseControl->IsAutostart() != isAutostart)
    {
        node->Set("autoStart", isAutostart);
    }
    
    if (baseControl->GetStartDelay() != startDelay)
    {
        node->Set("startDelay", startDelay);
    }

    return node;
}

UIControl* UIParticles::Clone()
{
    UIParticles *particles = new UIParticles(GetRect());
    particles->CopyDataFrom(this);
    return particles;
}
    
void UIParticles::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIParticles* particles = (UIParticles*) srcControl;

    SetStartDelay(particles->GetStartDelay());
    SetAutostart(particles->IsAutostart());

    if (!particles->effectPath.IsEmpty())
    {
        Load(particles->effectPath);
    }
}

void UIParticles::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * effectPathNode = node->Get("effectPath");
	const YamlNode * autoStartNode = node->Get("autoStart");
    const YamlNode * startDelayNode = node->Get("startDelay");

    if (effectPathNode)
    {
        Load(effectPathNode->AsString());
    }

    if (startDelayNode)
    {
        SetStartDelay(startDelayNode->AsFloat());
    }
    
    if (autoStartNode)
    {
        SetAutostart(autoStartNode->AsBool());
    }
}

void UIParticles::HandleAutostart()
{
    if (isAutostart && effect)
    {
        Start();
    }
}

float32 UIParticles::GetStartDelay() const
{
    return startDelay;
}

void UIParticles::SetStartDelay(float32 value)
{
    startDelay = value;
}

void UIParticles::SetInheritPosition(bool inheritPosition)
{
	effect->SetInheritPosition(inheritPosition);
}
    
void UIParticles::HandleDelayedAction(float32 timeElapsed)
{
    delayedActionTime += timeElapsed;
    if (delayedActionTime >= startDelay)
    {
        switch (delayedActionType)
        {
            case UIParticles::actionStart:
            {
                DoStart();
                break;
            }

            case UIParticles::actionRestart:
            {
                DoRestart();
                break;
            }

            default:
            {
                break;
            }
        }

        delayedActionType = UIParticles::actionNone;
        delayedActionTime = 0.0f;
    }
}

};
