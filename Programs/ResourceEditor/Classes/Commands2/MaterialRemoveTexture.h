#ifndef __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__
#define __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__

#include "Commands2/Base/RECommand.h"
#include "Base/FastName.h"

namespace DAVA
{
class Entity;
class Texture;
class NMaterial;
}

class MaterialRemoveTexture : public RECommand
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