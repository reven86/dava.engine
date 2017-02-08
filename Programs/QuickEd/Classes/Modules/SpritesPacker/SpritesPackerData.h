#pragma once

#include <TArc/DataProcessing/DataNode.h>

class SpritesPacker;

class SpritesPackerData : public DAVA::TArc::DataNode
{
public:
    SpritesPackerData();
    ~SpritesPackerData() override;

    SpritesPacker* GetSpritesPacker();
    const SpritesPacker* GetSpritesPacker() const;

private:
    std::unique_ptr<SpritesPacker> spritesPacker;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SpritesPackerData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SpritesPackerData>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
