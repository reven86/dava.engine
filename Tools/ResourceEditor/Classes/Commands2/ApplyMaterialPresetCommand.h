#pragma once
#include "Base/RECommand.h"
#include "Base/ScopedPtr.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class KeyedArchive;
class Scene;
class NMaterial;
class SerializationContext;
class FilePath;
}

class ApplyMaterialPresetCommand : public RECommand
{
public:
    enum eMaterialPart : DAVA::uint32
    {
        NOTHING = 0x0,

        TEMPLATE = 0x1,
        GROUP = 0x2,
        PROPERTIES = 0x4,
        TEXTURES = 0x8,

        ALL = 0xff
    };

    ApplyMaterialPresetCommand(const DAVA::FilePath& presetPath, DAVA::NMaterial* material, DAVA::Scene* scene);

    bool IsValidPreset() const;
    void Init(DAVA::uint32 materialParts);

    void Undo() override;
    void Redo() override;
    bool IsClean() const override;

    static void StoreMaterialPreset(DAVA::KeyedArchive* archive, DAVA::NMaterial* material, const DAVA::SerializationContext& context);

private:
    void LoadMaterialPreset(DAVA::KeyedArchive* archive, DAVA::uint32 parts);
    void LoadMaterialPreset(DAVA::KeyedArchive* archive, DAVA::NMaterial* material, const DAVA::SerializationContext& context, DAVA::uint32 parts);
    static void StoreMaterialPresetImpl(DAVA::KeyedArchive* archive, DAVA::NMaterial* material, const DAVA::SerializationContext& context, bool storeForUndo);

    void PrepareSerializationContext(DAVA::SerializationContext& context);

private:
    DAVA::ScopedPtr<DAVA::KeyedArchive> redoInfo;
    DAVA::ScopedPtr<DAVA::KeyedArchive> undoInfo;
    DAVA::RefPtr<DAVA::NMaterial> material;
    DAVA::Scene* scene;
    DAVA::uint32 materialParts = 0;
};
