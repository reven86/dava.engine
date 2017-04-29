#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class TransformComponent;
class AnimationComponent;
class TransformSingleComponent
{
public:
    Vector<TransformComponent*> localTransformChanged;
    Vector<TransformComponent*> transformParentChanged;
    Vector<TransformComponent*> worldTransformChanged;
    Vector<AnimationComponent*> animationTransformChanged;

    void Clear()
    {
        localTransformChanged.clear();
        transformParentChanged.clear();
        worldTransformChanged.clear();
        animationTransformChanged.clear();
    }
};
}
