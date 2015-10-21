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
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"

namespace DAVA {

/* this camera is required just for preparing draw data*/
Camera *UIParticles::defaultCamera = nullptr;

UIParticles::UIParticles(const Rect &rect)
    : UIControl(rect)
    , isAutostart(false)
    , startDelay(0.0f)
    , effect(nullptr)
    , system(new ParticleEffectSystem(nullptr, true))
    , updateTime(0)
    , delayedActionType(UIParticles::actionNone)
    , delayedActionTime(0.0f)
    , delayedDeleteAllParticles(false)
    , needHandleAutoStart(false)
{
    if (defaultCamera != nullptr)
    {
        defaultCamera->Retain();
    }
    else
    {
        defaultCamera = new Camera();
        defaultCamera->SetPosition(-Vector3::UnitZ);
        defaultCamera->SetUp(-Vector3::UnitY);
        defaultCamera->RebuildCameraFromValues();
        defaultCamera->RebuildViewMatrix(); 
    }
}

UIParticles::~UIParticles()
{
    UnloadEffect();
    SafeDelete(system);

    if (defaultCamera->GetRetainCount() != 1)
    {
        defaultCamera->Release();
    }
    else
    {
        SafeRelease(defaultCamera);
    }
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
    if (!effect)
    {
        return;
    }

    updateTime = 0.0f;

    if (effect->state == ParticleEffectComponent::STATE_STARTING ||
        effect->state == ParticleEffectComponent::STATE_PLAYING)
    {
        return;
    }

    effect->isPaused = false;
    system->AddToActive(effect);
    effect->effectRenderObject->SetWorldTransformPtr(&matrix);
    system->RunEffect(effect);
}

void UIParticles::Stop(bool isDeleteAllParticles)
{
    if (!effect)
    {
        return;
    }

    updateTime = 0.0f;

    if (effect->state == ParticleEffectComponent::STATE_STOPPED)
        return;

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
    if (!effect)
    {
        return;
    }

    effect->isPaused = isPaused;
}

bool UIParticles::IsStopped() const
{
    if (!effect)
    {
        return false;
    }

    return effect->state == ParticleEffectComponent::STATE_STOPPED;
}

bool UIParticles::IsPaused() const
{
    if (!effect)
    {
        return false;
    }

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
    if (!effect)
    {
        return;
    }

    effect->isPaused = false;
    if (delayedDeleteAllParticles)
    {
        effect->ClearCurrentGroups();
    }

    effect->currRepeatsCont = 0;
    system->RunEffect(effect);
}

void UIParticles::Update(float32 timeElapsed)
{
    updateTime = timeElapsed;
    if (needHandleAutoStart)
    {
        needHandleAutoStart = false;
        HandleAutostart();
    }

    if (delayedActionType != UIParticles::actionNone)
    {
        HandleDelayedAction(timeElapsed);
    }
}

void UIParticles::Draw(const UIGeometricData & geometricData)
{
    if ( !effect || effect->state == ParticleEffectComponent::STATE_STOPPED)
        return;

    RenderSystem2D::Instance()->Flush();

    matrix.CreateRotation(Vector3::UnitZ, -geometricData.angle);
    matrix.SetTranslationVector(Vector3(geometricData.position.x, geometricData.position.y, 0));
    effect->SetExtertnalValue("scale", geometricData.scale.x);
    system->Process(updateTime);
    updateTime = 0.0f;
    
    RenderSystem2D::Instance()->UpdateClip();

    effect->effectRenderObject->PrepareToRender(defaultCamera);
    for (int32 i=0, sz = effect->effectRenderObject->GetActiveRenderBatchCount(); i<sz; ++i)
        effect->effectRenderObject->GetActiveRenderBatch(i)->Draw(PASS_FORWARD, defaultCamera);
}

void UIParticles::SetExtertnalValue(const String& name, float32 value)
{
    if (effect != nullptr)
        effect->SetExtertnalValue(name, value);
}

void UIParticles::LoadEffect(const FilePath& path)
{
    ScopedPtr<SceneFileV2> sceneFile(new SceneFileV2());
    sceneFile->EnableDebugLog(false);

    ScopedPtr<SceneArchive> archive(sceneFile->LoadSceneArchive(path));
    ParticleEffectComponent *newEffect = nullptr;
    if ((SceneArchive *)archive != nullptr && !archive->children.empty())
    {
        ScopedPtr<Entity> entity(new Entity());
        SerializationContext serializationContext;
        serializationContext.SetRootNodePath(path);
        serializationContext.SetScenePath(FilePath(path.GetDirectory()));
        serializationContext.SetVersion(10);
        serializationContext.SetScene(nullptr);
        serializationContext.SetDefaultMaterialQuality(NMaterial::DEFAULT_QUALITY_NAME);
        entity->Load(archive->children[0]->archive, &serializationContext);
        ParticleEffectComponent *effSrc = GetEffectComponent(entity);
        if (effSrc)
        {
            newEffect = (ParticleEffectComponent*)effSrc->Clone(NULL);
        }
    }

    if (newEffect)
    {
        DVASSERT(!effect);
        effect = newEffect;
        effect->effectRenderObject->SetWorldTransformPtr(&matrix);
        effect->effectRenderObject->Set2DMode(true);
        needHandleAutoStart = true;
    }
}

void UIParticles::UnloadEffect()
{
    if (!effect)
        return;

    if (effect->state != ParticleEffectComponent::STATE_STOPPED)
        system->RemoveFromActive(effect);

    SafeDelete(effect);
}

void UIParticles::ReloadEffect()
{
    if (effectPath.IsEmpty())
    {
        DVASSERT_MSG(false, "You have to load UIPartilces effect prior to calling Reload()");
        return;
    }

    UnloadEffect();
    LoadEffect(effectPath);
}
    
void UIParticles::SetEffectPath(const FilePath& path)
{
    effectPath = path;
    UnloadEffect();
    if (!effectPath.IsEmpty())
    {
        LoadEffect(effectPath);
    }
}

const FilePath& UIParticles::GetEffectPath() const
{
    return effectPath;
}

void UIParticles::SetAutostart(bool value)
{
    isAutostart = value;
    needHandleAutoStart = true;
}

bool UIParticles::IsAutostart() const
{
    return isAutostart;
}
   
YamlNode * UIParticles::SaveToYamlNode(UIYamlLoader * loader)
{
    ScopedPtr<UIParticles> baseControl(new UIParticles());

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

UIParticles* UIParticles::Clone()
{
    UIParticles *particles = new UIParticles(GetRect());
    particles->CopyDataFrom(this);
    return particles;
}
    
void UIParticles::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIParticles* src = (UIParticles*) srcControl;

    SetEffectPath(src->GetEffectPath());
    SetStartDelay(src->GetStartDelay());
    SetAutostart(src->IsAutostart());
}

void UIParticles::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * effectPathNode = node->Get("effectPath");
	const YamlNode * autoStartNode = node->Get("autoStart");
    const YamlNode * startDelayNode = node->Get("startDelay");

    if (effectPathNode)
    {
        SetEffectPath(effectPathNode->AsString());
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
    
void UIParticles::HandleDelayedAction(float32 timeElapsed)
{
    if(IsOnScreen())
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
}

};
