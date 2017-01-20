#ifndef __DAVAENGINE_ANIMATION_COMPONENT_H__
#define __DAVAENGINE_ANIMATION_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/AnimationSystem.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Base/Message.h"
#include "Functional/Function.h"

namespace DAVA
{
class AnimationData;

class AnimationComponent : public Component
{
protected:
    virtual ~AnimationComponent();

public:
    AnimationComponent();

    IMPLEMENT_COMPONENT_TYPE(ANIMATION_COMPONENT);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void GetDataNodes(Set<DataNode*>& dataNodes) override;

    void SetAnimation(AnimationData* animation);
    AnimationData* GetAnimation() const;

    bool GetIsPlaying() const;
    void SetIsPlaying(bool value);

    void Start();
    void Stop();
    void StopAfterNRepeats(int32 numberOfRepeats);
    /**
        \brief Move animation to the last frame and stop the animation.
        \ Will not call the playback complete callback.
    */
    void MoveAnimationToTheLastFrame();
    /**
        \brief Move animation to the first frame and leave animation in current state (Playing or Stopped).
    */
    void MoveAnimationToTheFirstFrame();

    void SetPlaybackCompleteCallback(Function<void(Component* const)> callback);

    enum eState
    {
        STATE_PLAYING,
        STATE_PAUSED,
        STATE_STOPPED
    };

private:
    friend class AnimationSystem;
    friend class TransformSystem;
    AnimationData* animation;
    float32 time;
    float32 animationTimeScale;
    uint32 frameIndex;
    uint32 repeatsCount;
    uint32 currRepeatsCont;
    eState state;

    /*completion callback stuff*/
    Function<void(Component* const)> playbackComplete;

    Matrix4 animationTransform;

public:
    INTROSPECTION_EXTEND(AnimationComponent, Component,
                         MEMBER(repeatsCount, "repeatsCount", I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("isPlaying", "isPlaying", GetIsPlaying, SetIsPlaying, I_SAVE | I_EDIT | I_VIEW)
                         MEMBER(animationTimeScale, "animationTimeScale", I_VIEW | I_EDIT | I_SAVE)
                         );
};

inline AnimationData* AnimationComponent::GetAnimation() const
{
    return animation;
}
};

#endif //__DAVAENGINE_ANIMATION_COMPONENT_H__
