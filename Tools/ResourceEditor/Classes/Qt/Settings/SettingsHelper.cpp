#include "SettingsHelper.h"
#include "SettingsManager.h"

#include "Render/GPUFamilyDescriptor.h"

namespace settings
{
DAVA::eGPUFamily GetGPUFormat()
{
    return static_cast<DAVA::eGPUFamily>(DAVA::GPUFamilyDescriptor::ConvertValueToGPU(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}
}