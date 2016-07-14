#ifndef __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__
#define __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "Base/FastName.h"

namespace DAVA
{
    class Entity;
    class Texture;
    class NMaterial;
}

class MaterialRemoveTexture : public CommandWithoutExecute
{
public:
    MaterialRemoveTexture(const DAVA::FastName& textureSlot_, DAVA::NMaterial* material_);
    ~MaterialRemoveTexture() override;

    void Undo() override;
    void Redo() override;

private:
    DAVA::NMaterial* material = nullptr;
    DAVA::Texture* texture = nullptr;
    DAVA::FastName textureSlot;
};



#endif // __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__