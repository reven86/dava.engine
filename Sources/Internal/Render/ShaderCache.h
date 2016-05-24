#ifndef __DAVAENGINE_SHADER_CACHE_H__
#define __DAVAENGINE_SHADER_CACHE_H__

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

void RelaoadShaders();

ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines);

void BuildFlagsKey(const FastName& name, const HashMap<FastName, int32>& defines, Vector<int32>& key);
};
};

#endif // __DAVAENGINE_SHADER_CACHE_H__
