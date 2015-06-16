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



#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
    
REGISTER_CLASS(SkeletonComponent)


SkeletonComponent::SkeletonComponent()
{   
}

SkeletonComponent::~SkeletonComponent()
{

}
        
SkeletonComponent::JointConfig::JointConfig()
    : parentIndex(INVALID_JOINT_INDEX)
    , targetId(INVALID_JOINT_INDEX)
    , orientation(0.0f,0.0f,0.0f,1.0f)
    , position(0.0f,0.0f,0.0f)
    , scale(1.0f)
{    
}
SkeletonComponent::JointConfig::JointConfig(int32 _parentIndex, int32 _targetId, const FastName& _name, const Vector3& _position, const Quaternion& _orientation, float32 _scale, const AABBox3& _bbox)
    : parentIndex(_parentIndex)
    , targetId(_targetId)
    , name(_name)
    , orientation(_orientation)
    , position(_position)
    , scale(_scale)
    , bbox(_bbox)
{
}



uint16 SkeletonComponent::GetConfigJointsCount()
{
    return configJoints.size();
}
void SkeletonComponent::SetConfigJoints(const Vector<JointConfig>& config)
{
    configJoints = config;
}


void SkeletonComponent::RebuildFromConfig()
{
    GlobalEventSystem::Instance()->Event(this, EventSystem::SKELETON_CONFIG_CHANGED);	
}

Component * SkeletonComponent::Clone(Entity * toEntity)
{
    SkeletonComponent * newComponent = new SkeletonComponent();      
    newComponent->SetEntity(toEntity);
    newComponent->configJoints = configJoints;
    return newComponent;
}
void SkeletonComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Serialize(archive, serializationContext);    
    archive->SetUInt32("skeletoncomponent.jointsCount", static_cast<uint32>(configJoints.size()));
    ScopedPtr<KeyedArchive> jointsArch (new KeyedArchive());	
    for (size_t i=0, sz = configJoints.size(); i<sz; ++i)
    {		
        const JointConfig& joint = configJoints[i];
        ScopedPtr<KeyedArchive> jointArch (new KeyedArchive());	  
        jointArch->SetFastName("joint.name", joint.name);
        jointArch->SetInt32("joint.parentIndex", joint.parentIndex);
        jointArch->SetInt32("joint.targetId", joint.targetId);
        jointArch->SetVector3("joint.position", joint.position);
        jointArch->SetVector4("joint.orientation", Vector4(joint.orientation.x, joint.orientation.y, joint.orientation.z, joint.orientation.w));
        jointArch->SetFloat("joint.scale", joint.scale);
        jointArch->SetVector3("joint.bbox.min", joint.bbox.min);
        jointArch->SetVector3("joint.bbox.max", joint.bbox.max);

        jointsArch->SetArchive(KeyedArchive::GenKeyFromIndex(static_cast<int32>(i)), jointArch);
    }     

    archive->SetArchive("skeletoncomponent.joints", jointsArch);

}
void SkeletonComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Deserialize(archive, serializationContext);

   
    uint32 configJointsCount = archive->GetUInt32("skeletoncomponent.jointsCount", static_cast<uint32>(configJoints.size()));
    configJoints.resize(configJointsCount);
    KeyedArchive *jointsArch = archive->GetArchive("skeletoncomponent.joints");    
    for (uint32 i=0; i<configJointsCount; ++i)
    {		
        JointConfig& joint = configJoints[i];
        KeyedArchive *jointArch = jointsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
        joint.name = jointArch->GetFastName("joint.name");
        joint.parentIndex = jointArch->GetInt32("joint.parentIndex");
        joint.targetId = jointArch->GetInt32("joint.targetId");
        joint.position = jointArch->GetVector3("joint.position");
        Vector4 qv = jointArch->GetVector4("joint.orientation");
        joint.orientation = Quaternion(qv.x, qv.y, qv.z, qv.w);
        joint.scale = jointArch->GetFloat("joint.scale");
        joint.bbox.min = jointArch->GetVector3("joint.bbox.min");
        joint.bbox.max = jointArch->GetVector3("joint.bbox.max");      
    } 	   
}

}