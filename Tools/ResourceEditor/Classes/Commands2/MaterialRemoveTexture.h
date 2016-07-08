#ifndef __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__
#define __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__

#include "Commands2/Base/Command2.h"

class MaterialRemoveTexture : public Command2
{
public:
    MaterialRemoveTexture(const DAVA::FastName& textureSlot_, DAVA::NMaterial* material_);
    ~MaterialRemoveTexture() override;

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const override;

private:
    DAVA::NMaterial* material = nullptr;
    DAVA::Texture* texture = nullptr;
    DAVA::FastName textureSlot;
};



#endif // __RESOURCEEDITOR_MATERIALREMOVETEXTURE_H__