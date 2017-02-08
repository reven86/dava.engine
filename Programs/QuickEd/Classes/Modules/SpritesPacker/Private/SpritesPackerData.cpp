#include "Modules/SpritesPacker/SpritesPackerData.h"

#include <QtTools/ReloadSprites/SpritesPacker.h>

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
