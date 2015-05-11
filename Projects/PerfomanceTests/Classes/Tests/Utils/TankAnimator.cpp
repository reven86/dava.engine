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

#include "Tests/Utils/TankAnimator.h"

const FastName TankAnimator::TURRET = FastName("turret");
const FastName TankAnimator::L_WHEELS = FastName("l_wheels");
const FastName TankAnimator::R_WHEELS = FastName("r_wheels");
const FastName TankAnimator::GUN_SHOT = FastName("GunShot");
const FastName TankAnimator::SKINNED_TANK = FastName("SKINNED_TANK");

TankAnimator::TankAnimator()
{
}

TankAnimator::~TankAnimator()
{
}

void TankAnimator::MakeSkinnedTank(Entity* sourceTank, Vector<uint16>& outJointIndexes)
{
    Entity* skinnedTank = new Entity(); 
    skinnedTank->SetName(SKINNED_TANK);
    
    Entity* lWheelsRoot = sourceTank->FindByName(L_WHEELS);
    Entity* rWheelsRoot = sourceTank->FindByName(R_WHEELS);

    Vector<Entity*> wheels;

    lWheelsRoot->GetChildNodes(wheels);
    rWheelsRoot->GetChildNodes(wheels);

    Vector<SkeletonComponent::JointConfig> tankJoints;
    RenderObject * skinnedRo = MeshUtils::CreateSkinnedMesh(sourceTank, tankJoints);
    ((RenderComponent *)skinnedTank->GetOrCreateComponent(Component::RENDER_COMPONENT))->SetRenderObject(skinnedRo);
    skinnedRo->Release();

    uint32 jointsCount = tankJoints.size();

    for (uint32 i = 0; i < wheels.size(); i++)
    {
        Entity* wheel = wheels[i];
        RenderComponent* renderComponent = (RenderComponent*) wheel->GetComponent(Component::RENDER_COMPONENT);
        Vector3 centerPos = renderComponent->GetRenderObject()->GetBoundingBox().GetCenter();

        for (uint32 j = 0; j < jointsCount; j++)
        {
            if (tankJoints[j].name == wheel->GetName())
            {
                outJointIndexes.push_back(j);
                tankJoints[j].position = centerPos;
            }

        }
    }

    for (uint32 i = 0; i < jointsCount; i++)
    {
        if (tankJoints[i].name == TURRET)
        {
            Entity* turret = sourceTank->FindByName(TURRET);
            Vector3 centerPos = ((RenderComponent*)turret->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject()->GetBoundingBox().GetCenter();

            tankJoints[i].position = centerPos;
        }
    }

    SkeletonComponent* conquerorSkeleton = new SkeletonComponent();
    conquerorSkeleton->SetConfigJoints(tankJoints);
    skinnedTank->AddComponent(conquerorSkeleton);
    conquerorSkeleton->RebuildFromConfig();

    SetReflectionRefractionVisibility(sourceTank, RenderObject::VISIBLE_REFLECTION | RenderObject::VISIBLE_REFRACTION);

    Vector<Entity*> sourceTankChildren;
    sourceTank->GetChildEntitiesWithComponent(sourceTankChildren, Component::RENDER_COMPONENT);

    Vector<Entity*>::iterator it = sourceTankChildren.begin();
    Vector<Entity*>::iterator end = sourceTankChildren.end();

    while (it != end)
    {
        (*it)->RemoveComponent(Component::RENDER_COMPONENT);
        ++it;
    }

    sourceTank->AddNode(skinnedTank);
    LodSystem::MergeChildLods(skinnedTank);
}

void TankAnimator::SetReflectionRefractionVisibility(Entity *node, DAVA::uint32 visibility)
{
    RenderComponent * rc = (RenderComponent*)node->GetComponent(Component::RENDER_COMPONENT);

    if (rc)
    {
        RenderObject *ro = rc->GetRenderObject();
        if (ro && (ro->GetType() == RenderObject::TYPE_MESH || ro->GetType() == RenderObject::TYPE_SKINNED_MESH))
        {
            ro->RemoveFlag(RenderObject::VISIBLE_REFLECTION | RenderObject::VISIBLE_REFRACTION);
            ro->AddFlag(visibility);
        }
    }

    int32 childrenCount = node->GetChildrenCount();
    for (int32 i = 0; i < childrenCount; i++)
    {
        SetReflectionRefractionVisibility(node->GetChild(i), visibility);
    }
}

void TankAnimator::Animate(Entity* tank, const Vector<uint16>& jointIndexes, float32 angle)
{
    Entity* skinnedTank = tank->FindByName(SKINNED_TANK);
    Entity* gunShotEffect = tank->FindByName(GUN_SHOT);

    SkeletonComponent* skeleton = (SkeletonComponent*)skinnedTank->GetComponent(Component::SKELETON_COMPONENT);

    Quaternion wheelsRotation = Quaternion::MakeRotation(Vector3::UnitX, angle);
    Quaternion turrentRotation = Quaternion::MakeRotation(Vector3::UnitZ, angle);

    for (uint32 i = 0; i < jointIndexes.size(); i++)
    {
        skeleton->SetJointOrientation(jointIndexes[i], wheelsRotation);
    }

    skeleton->SetJointOrientation(skeleton->GetJointId(TURRET), turrentRotation);

    Vector3 tranlation = gunShotEffect->GetLocalTransform().GetTranslationVector();
    gunShotEffect->SetLocalTransform(turrentRotation.GetMatrix() * Matrix4::MakeTranslation(tranlation));
}