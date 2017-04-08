#ifndef __DAVAENGINE_SKELETON_COMPONENT_H__
#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Math/AABBox3.h"

namespace DAVA
{
class Entity;
class SkeletonComponent : public Component
{
    friend class SkeletonSystem;

public:
    IMPLEMENT_COMPONENT_TYPE(SKELETON_COMPONENT);

    const static uint16 INVALID_JOINT_INDEX = 0xff; //same as INFO_PARENT_MASK
    const static uint16 MAX_TARGET_JOINTS = 32; //same as in shader

    struct JointTransform
    {
        Quaternion orientation;

        Vector3 position;
        float32 scale;

        inline JointTransform AppendTransform(const JointTransform& transform) const;
        inline JointTransform GetInverse() const;
        inline Vector3 TransformVector(const Vector3& inVec) const;
    };

    struct JointConfig : public InspBase
    {
        JointConfig();

        JointConfig(int32 parentIndex, int32 targetId, const FastName& name, const Vector3& position, const Quaternion& orientation, float32 scale, const AABBox3& bbox);

        int32 parentIndex;
        int32 targetId;
        FastName name;
        Quaternion orientation;
        Vector3 position;
        float32 scale;
        AABBox3 bbox;

        bool operator==(const JointConfig& other) const;

        INTROSPECTION(JointConfig,
                      MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(position, "Position", I_SAVE | I_VIEW | I_EDIT)
                      //MEMBER(orientation, "Orientation", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(scale, "Scale", I_SAVE | I_VIEW | I_EDIT)
                      MEMBER(bbox, "Bounding box", I_SAVE | I_VIEW | I_EDIT)
                      );

        DAVA_VIRTUAL_REFLECTION(JointConfig, InspBase);
    };

    void RebuildFromConfig();
    void SetConfigJoints(const Vector<JointConfig>& config);
    uint16 GetConfigJointsCount();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline void SetJointPosition(uint16 jointId, const Vector3& position);
    inline void SetJointOrientation(uint16 jointId, const Quaternion& orientation);
    inline void SetJointScale(uint16 jointId, float32 scale);

    inline uint16 GetJointId(const FastName& name) const;

    inline uint16 GetJointsCount() const;

    SkeletonComponent();
    ~SkeletonComponent();

private:
    /*config time*/
    Vector<JointConfig> configJoints;

    /*runtime*/
    const static uint32 INFO_PARENT_MASK = 0xff;
    const static uint32 INFO_TARGET_SHIFT = 8;
    const static uint32 INFO_FLAG_BASE = 0x10000;
    const static uint32 FLAG_UPDATED_THIS_FRAME = INFO_FLAG_BASE << 0;
    const static uint32 FLAG_MARKED_FOR_UPDATED = INFO_FLAG_BASE << 1;

    uint16 jointsCount;
    uint16 targetJointsCount; //amount of joints bound to skinnedMesh
    Vector<uint32> jointInfo; //flags and parent
    //transforms info
    Vector<JointTransform> localSpaceTransforms;
    Vector<JointTransform> objectSpaceTransforms;
    //bind pose
    Vector<JointTransform> inverseBindTransforms;
    //bounding boxes for bone
    Vector<AABBox3> jointSpaceBoxes;
    Vector<AABBox3> objectSpaceBoxes;

    Vector<Vector4> resultPositions; //stores final results
    Vector<Vector4> resultQuaternions;

    Map<FastName, uint16> jointMap;

    uint16 startJoint; //first joint in the list that was updated this frame - cache this value to optimize processing
    bool configUpdated = true;

public:
    INTROSPECTION_EXTEND(SkeletonComponent, Component,
                         COLLECTION(configJoints, "Root Joints", I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(SkeletonComponent, Component);
};

inline void SkeletonComponent::SetJointPosition(uint16 jointId, const Vector3& position)
{
    DVASSERT(jointId < GetJointsCount());
    jointInfo[jointId] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointId].position = position;
    startJoint = Min(startJoint, jointId);
}
inline void SkeletonComponent::SetJointOrientation(uint16 jointId, const Quaternion& orientation)
{
    DVASSERT(jointId < GetJointsCount());
    jointInfo[jointId] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointId].orientation = orientation;
    startJoint = Min(startJoint, jointId);
}
inline void SkeletonComponent::SetJointScale(uint16 jointId, float32 scale)
{
    DVASSERT(jointId < GetJointsCount());
    jointInfo[jointId] |= FLAG_MARKED_FOR_UPDATED;
    localSpaceTransforms[jointId].scale = scale;
    startJoint = Min(startJoint, jointId);
}

inline uint16 SkeletonComponent::GetJointId(const FastName& name) const
{
    Map<FastName, uint16>::const_iterator it = jointMap.find(name);
    if (jointMap.end() != it)
        return it->second;
    else
        return INVALID_JOINT_INDEX;
}

inline uint16 SkeletonComponent::GetJointsCount() const
{
    return jointsCount;
}

inline Vector3 SkeletonComponent::JointTransform::TransformVector(const Vector3& inVec) const
{
    return position + orientation.ApplyToVectorFast(inVec) * scale;
}

inline SkeletonComponent::JointTransform SkeletonComponent::JointTransform::AppendTransform(const JointTransform& transform) const
{
    JointTransform res;
    res.position = TransformVector(transform.position);
    res.orientation = orientation * transform.orientation;
    res.scale = scale * transform.scale;
    return res;
}

inline SkeletonComponent::JointTransform SkeletonComponent::JointTransform::GetInverse() const
{
    JointTransform res;
    res.scale = 1.0f / scale;
    res.orientation = orientation;
    res.orientation.Inverse();
    res.position = -res.orientation.ApplyToVectorFast(position) * res.scale;

    return res;
}

} //ns

#endif