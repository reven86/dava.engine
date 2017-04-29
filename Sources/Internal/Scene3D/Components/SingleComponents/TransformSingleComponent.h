#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
class TransformSingleComponent
{
public:
    Vector<Entity*> localTransformChanged;
    Vector<Entity*> transformParentChanged;
    Vector<Entity*> worldTransformChanged;
    Vector<Entity*> animationTransformChanged;

    void Clear()
    {
        localTransformChanged.clear();
        transformParentChanged.clear();
        worldTransformChanged.clear();
        animationTransformChanged.clear();
    }
};
}
