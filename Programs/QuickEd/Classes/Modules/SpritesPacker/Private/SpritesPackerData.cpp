#include "Modules/SpritesPacker/SpritesPackerData.h"

#include <QtTools/ReloadSprites/SpritesPacker.h>

DAVA_VIRTUAL_REFLECTION_IMPL(SpritesPackerData)
{
    DAVA::ReflectionRegistrator<SpritesPackerData>::Begin()
    .ConstructorByPointer()
    .End();
}

SpritesPackerData::SpritesPackerData()
{
    spritesPacker.reset(new SpritesPacker());
}

SpritesPackerData::~SpritesPackerData() = default;

SpritesPacker* SpritesPackerData::GetSpritesPacker()
{
    return spritesPacker.get();
}

const SpritesPacker* SpritesPackerData::GetSpritesPacker() const
{
    return spritesPacker.get();
}
