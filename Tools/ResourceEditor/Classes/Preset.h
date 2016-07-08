#ifndef __PRESET_MANAGER_H__
#define __PRESET_MANAGER_H__

namespace DAVA
{
class TextureDescriptor;
class NMaterial;
class KeyedArchive;
class FilePath;
}

namespace Preset
{
bool SaveArchive(const DAVA::KeyedArchive* presetArchive, const DAVA::FilePath& path);
DAVA::KeyedArchive* LoadArchive(const DAVA::FilePath& path);

bool ApplyTexturePreset(DAVA::TextureDescriptor* descriptor, const DAVA::KeyedArchive* preset);

bool DialogSavePresetForTexture(const DAVA::TextureDescriptor* descriptor);
bool DialogLoadPresetForTexture(DAVA::TextureDescriptor* descriptor);

bool DialogSavePresetForMaterial(DAVA::NMaterial* material);
bool DialogLoadPresetForMaterial(DAVA::NMaterial* material);
}
#endif // __PRESET_MANAGER_H__
