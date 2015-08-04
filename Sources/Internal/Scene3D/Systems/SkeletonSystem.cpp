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


#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"


namespace DAVA
{

SkeletonSystem::SkeletonSystem(Scene * scene):SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED); 
}

SkeletonSystem::~SkeletonSystem()
{    
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

void SkeletonSystem::AddEntity(Entity * entity)
{
    entities.push_back(entity);
    RebuildSkeleton(entity);
}

void SkeletonSystem::RemoveEntity(Entity * entity)
{
    uint32 size = static_cast<uint32>(entities.size());
    for(uint32 i = 0; i < size; ++i)
    {
        if(entities[i] == entity)
        {
            entities[i] = entities[size-1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void SkeletonSystem::ImmediateEvent(Component * component, uint32 event)
{    
    if (event == EventSystem::SKELETON_CONFIG_CHANGED)    
        RebuildSkeleton(component->GetEntity());
        
}

void SkeletonSystem::Process(float32 timeElapsed)
{
    for (int32 i=0, sz=static_cast<int32>(entities.size()); i<sz; ++i)
    {
        SkeletonComponent *component = GetSkeletonComponent(entities[i]);
        /*test/
        {
            static float32 t=0;
            t+=timeElapsed;
            for (int32 i=0, sz = component->GetJointsCount(); i<sz; ++i)
            {
                float32 fi = t;
                Quaternion q;
                q.Construct(Vector3(0,1,0), fi);
                component->SetJointOrientation(i, q);
            }
        }*/
        if (component&&(component->startJoint!=SkeletonComponent::INVALID_JOINT_INDEX))
        {
            UpdatePose(component);     
            RenderObject *ro = GetRenderObject(entities[i]);
            if (ro && (RenderObject::TYPE_SKINNED_MESH == ro->GetType()))
            {
                SkinnedMesh *skinnedMeshObject = static_cast<SkinnedMesh*>(ro);
                DVASSERT(skinnedMeshObject);
                UpdateSkinnedMesh(component, skinnedMeshObject);
            }
        }        
    }    
}

void SkeletonSystem::UpdatePose(SkeletonComponent *component)
{    
    uint16 count = component->GetJointsCount();
    for (uint16 currJoint = component->startJoint; currJoint<count; ++currJoint)
    {
        uint16 parentJoint = component->jointInfo[currJoint]&SkeletonComponent::INFO_PARENT_MASK;
        if ((component->jointInfo[currJoint]&SkeletonComponent::FLAG_MARKED_FOR_UPDATED)||((parentJoint!=SkeletonComponent::INVALID_JOINT_INDEX)&&(component->jointInfo[parentJoint]&SkeletonComponent::FLAG_UPDATED_THIS_FRAME)))
        {
            //calculate local pose
            if (parentJoint == SkeletonComponent::INVALID_JOINT_INDEX) //root
            {
                component->objectSpaceTransforms[currJoint] = component->localSpaceTransforms[currJoint]; //just copy
            }
            else
            {
                component->objectSpaceTransforms[currJoint] = component->objectSpaceTransforms[parentJoint].AppendTransform(component->localSpaceTransforms[currJoint]);
            }              
            //calculate final transform including bindTransform
            
            uint16 targetId = (component->jointInfo[currJoint]>>SkeletonComponent::INFO_TARGET_SHIFT)&SkeletonComponent::INFO_PARENT_MASK;
            if (targetId!=SkeletonComponent::INVALID_JOINT_INDEX)
            {
                SkeletonComponent::JointTransform finalTransform = component->objectSpaceTransforms[currJoint].AppendTransform(component->inverseBindTransforms[currJoint]);
                component->resultPositions[targetId].Set(finalTransform.position.x, finalTransform.position.y, finalTransform.position.z, finalTransform.scale);
                component->resultQuaternions[targetId].Set(finalTransform.orientation.x, finalTransform.orientation.y, finalTransform.orientation.z, finalTransform.orientation.w);
                const Vector3& min = component->jointSpaceBoxes[currJoint].min;
                const Vector3& max = component->jointSpaceBoxes[currJoint].max;
                AABBox3& box = component->objectSpaceBoxes[currJoint];
                box.Empty();
                /*rework analogically to box.applytransform later*/
                box.AddPoint(finalTransform.TransformVector(min));
                box.AddPoint(finalTransform.TransformVector(max));
                box.AddPoint(finalTransform.TransformVector(Vector3(min.x, min.y, max.z)));
                box.AddPoint(finalTransform.TransformVector(Vector3(min.x, max.y, min.z)));
                box.AddPoint(finalTransform.TransformVector(Vector3(min.x, max.y, max.z)));
                box.AddPoint(finalTransform.TransformVector(Vector3(max.x, min.y, min.z)));
                box.AddPoint(finalTransform.TransformVector(Vector3(max.x, min.y, max.z)));
                box.AddPoint(finalTransform.TransformVector(Vector3(max.x, max.y, min.z)));
            }                        
            
            
            //  add [was updated]  remove [marked for update]            
            component->jointInfo[currJoint] &=~ SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
            component->jointInfo[currJoint] |= SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
        else
        {
            /*  remove was updated  - note that as bones come in descending order we do not care that was updated flag would be cared to next frame*/
            component->jointInfo[currJoint] &=~ SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
    }
    component->startJoint = SkeletonComponent::INVALID_JOINT_INDEX;    
}

void SkeletonSystem::UpdateSkinnedMesh(SkeletonComponent *component, SkinnedMesh *skinnedMeshObject)
{    
    //recalculate object box
    uint16 count = component->GetJointsCount();
    AABBox3 resBox;
    for (uint16 currJoint=0; currJoint<count; ++currJoint)
    {
        uint16 targetId = (component->jointInfo[currJoint]>>SkeletonComponent::INFO_TARGET_SHIFT)&SkeletonComponent::INFO_PARENT_MASK;
        if (targetId!=SkeletonComponent::INVALID_JOINT_INDEX)
            resBox.AddAABBox(component->objectSpaceBoxes[currJoint]);
    }

    //set data to SkinnedMesh    
    skinnedMeshObject->SetJointsPtr(&component->resultPositions[0], &component->resultQuaternions[0], component->targetJointsCount);
    skinnedMeshObject->SetObjectSpaceBoundingBox(resBox);
    GetScene()->renderSystem->MarkForUpdate(skinnedMeshObject);        
}


void SkeletonSystem::RebuildSkeleton(Entity *entity)
{
    
    SkeletonComponent *component = GetSkeletonComponent(entity);
    DVASSERT(component);
    /*convert joint configs to joints*/
    component->jointsCount = component->GetConfigJointsCount();
        
    component->jointInfo.resize(component->jointsCount);
    component->localSpaceTransforms.resize(component->jointsCount);
    component->objectSpaceTransforms.resize(component->jointsCount);
    component->inverseBindTransforms.resize(component->jointsCount);
    component->jointSpaceBoxes.resize(component->jointsCount);
    component->objectSpaceBoxes.resize(component->jointsCount);    
    component->jointMap.clear();
    
    component->targetJointsCount = 0;
    int32 maxTargetJoint = 0;
    DVASSERT(component->configJoints.size()<SkeletonComponent::INFO_PARENT_MASK);
    for (int32 i=0, sz = static_cast<int32>(component->configJoints.size()); i<sz; ++i)
    {
        DVASSERT((component->configJoints[i].parentIndex==SkeletonComponent::INVALID_JOINT_INDEX)||(component->configJoints[i].parentIndex<i)); //order
        DVASSERT((component->configJoints[i].parentIndex==SkeletonComponent::INVALID_JOINT_INDEX)||((component->configJoints[i].parentIndex&SkeletonComponent::INFO_PARENT_MASK)==component->configJoints[i].parentIndex)); //parent fits mask
        DVASSERT((component->configJoints[i].targetId==SkeletonComponent::INVALID_JOINT_INDEX)||((component->configJoints[i].targetId&SkeletonComponent::INFO_PARENT_MASK)==component->configJoints[i].targetId)); //target fits mask
        DVASSERT((component->configJoints[i].targetId==SkeletonComponent::INVALID_JOINT_INDEX)||(component->configJoints[i].targetId<SkeletonComponent::MAX_TARGET_JOINTS));
        DVASSERT(component->jointMap.find(component->configJoints[i].name) == component->jointMap.end()); //duplicate bone name
        
        component->jointInfo[i] = component->configJoints[i].parentIndex | (component->configJoints[i].targetId<<SkeletonComponent::INFO_TARGET_SHIFT) | SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
        if ((component->configJoints[i].targetId!=SkeletonComponent::INVALID_JOINT_INDEX)&&component->configJoints[i].targetId>maxTargetJoint)
            maxTargetJoint = component->configJoints[i].targetId;            
        
        component->jointMap[component->configJoints[i].name] = i;

        SkeletonComponent::JointTransform localTransform;
        localTransform.position = component->configJoints[i].position;
        localTransform.orientation = component->configJoints[i].orientation;
        localTransform.scale = component->configJoints[i].scale;

        component->localSpaceTransforms[i] = localTransform;    
        if (component->configJoints[i].parentIndex==SkeletonComponent::INVALID_JOINT_INDEX)
            component->objectSpaceTransforms[i] = localTransform;
        else
            component->objectSpaceTransforms[i] = component->objectSpaceTransforms[component->configJoints[i].parentIndex].AppendTransform(localTransform);
        component->inverseBindTransforms[i] = component->objectSpaceTransforms[i].GetInverse();
        component->jointSpaceBoxes[i] = component->configJoints[i].bbox;        
    }        
    component->targetJointsCount = maxTargetJoint+1;
    component->resultPositions.resize(component->targetJointsCount);
    component->resultQuaternions.resize(component->targetJointsCount);

    component->startJoint = 0;
    UpdatePose(component);

    RenderObject *ro = GetRenderObject(entity);
    if (ro && (RenderObject::TYPE_SKINNED_MESH == ro->GetType()))
    {
        SkinnedMesh *skinnedMeshObject = static_cast<SkinnedMesh*>(ro);
        DVASSERT(skinnedMeshObject);
        UpdateSkinnedMesh(component, skinnedMeshObject);
    }
}


}