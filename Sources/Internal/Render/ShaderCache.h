#pragma once

#include "Base/FastNameMap.h"
#include "Base/Singleton.h"
#include "Base/FastName.h"
#include "Render/Shader.h"

namespace DAVA
{
namespace ShaderDescriptorCache
{
void Initialize();
void Uninitialize();
void Clear();
void ClearDynamicBindigs();

void ReloadShaders();

void SetLoadingNotifyEnabled(bool enable);
ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines);

Vector<int32> BuildFlagsKey(const FastName& name, const HashMap<FastName, int32>& defines);
};
};
