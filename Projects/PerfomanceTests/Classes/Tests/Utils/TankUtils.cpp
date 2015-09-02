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

#include "Tests/Utils/TankUtils.h"

using namespace DAVA;

const FastName TankUtils::TankNode::TURRET = FastName("turret");
const FastName TankUtils::TankNode::L_WHEELS = FastName("l_wheels");
const FastName TankUtils::TankNode::R_WHEELS = FastName("r_wheels");
const FastName TankUtils::TankNode::SKINNED_TANK = FastName("SKINNED_TANK");

void TankUtils::MakeSkinnedTank(Entity* sourceTank, Vector<uint16>& outJointIndexes)
{
    ScopedPtr<Entity> skinnedTank(new Entity());
    skinnedTank->SetName(TankUtils::TankNode::SKINNED_TANK);
    
    Entity* lWheelsRoot = sourceTank->FindByName(TankNode::L_WHEELS);
    Entity* rWheelsRoot = sourceTank->FindByName(TankNode::R_WHEELS);

    Vector<Entity*> wheels;

    lWheelsRoot->GetChildNodes(wheels);
    rWheelsRoot->GetChildNodes(wheels);

    Vector<SkeletonComponent::JointConfig> tankJoints;
    ScopedPtr<RenderObject> skinnedRo(MeshUtils::CreateSkinnedMesh(sourceTank, tankJoints));
    skinnedRo->AddFlag(RenderObject::VISIBLE_REFLECTION | RenderObject::VISIBLE_REFRACTION);
    
    RenderComponent* renderComponent = static_cast<RenderComponent*>(skinnedTank->GetOrCreateComponent(Component::RENDER_COMPONENT));
    renderComponent->SetRenderObject(skinnedRo);

    uint32 jointsCount = tankJoints.size();

    for (Entity* wheel : wheels)
    {
        RenderComponent* renderComponent = static_cast<RenderComponent*>(wheel->GetComponent(Component::RENDER_COMPONENT));
        const Vector3& centerPos = renderComponent->GetRenderObject()->GetBoundingBox().GetCenter();

        for (uint32 i = 0; i < jointsCount; i++)
        {
            if (tankJoints[i].name == wheel->GetName())
            {
                outJointIndexes.push_back(i);
                tankJoints[i].position = centerPos;
            }

        }
    }

    for (uint32 i = 0; i < jointsCount; i++)
    {
        if (tankJoints[i].name == TankNode::TURRET)
        {
            Entity* turret = sourceTank->FindByName(TankNode::TURRET);
            RenderComponent* renderComponent = static_cast<RenderComponent*>(turret->GetComponent(Component::RENDER_COMPONENT));
            
            tankJoints[i].position = renderComponent->GetRenderObject()->GetBoundingBox().GetCenter();
        }
    }

    SkeletonComponent* conquerorSkeleton = new SkeletonComponent();
    conquerorSkeleton->SetConfigJoints(tankJoints);
    conquerorSkeleton->RebuildFromConfig();
    
    skinnedTank->AddComponent(conquerorSkeleton);

    Vector<Entity*> sourceTankChildren;
    sourceTank->GetChildEntitiesWithComponent(sourceTankChildren, Component::RENDER_COMPONENT);

    for (auto *child : sourceTankChildren)
    {
        child->RemoveComponent(Component::RENDER_COMPONENT);
    }

    LodSystem::MergeChildLods(skinnedTank);
    sourceTank->AddNode(skinnedTank);
}

void TankUtils::Animate(Entity* tank, const Vector<uint16>& jointIndexes, float32 angle)
{
    Entity* skinnedTank = tank->FindByName(TankUtils::TankNode::SKINNED_TANK);
    Entity* turret = tank->FindByName(TankUtils::TankNode::TURRET);

    SkeletonComponent* skeleton = (SkeletonComponent*)skinnedTank->GetComponent(Component::SKELETON_COMPONENT);

    const Quaternion& wheelsRotation = Quaternion::MakeRotation(Vector3::UnitX, angle);
    const Quaternion& turrentRotation = Quaternion::MakeRotation(Vector3::UnitZ, angle);

    for (uint32 i = 0; i < jointIndexes.size(); i++)
    {
        skeleton->SetJointOrientation(jointIndexes[i], wheelsRotation);
    }

    // rotate gun shot effect
    turret->SetLocalTransform(turrentRotation.GetMatrix());
    skeleton->SetJointOrientation(skeleton->GetJointId(TankUtils::TankNode::TURRET), turrentRotation);
}