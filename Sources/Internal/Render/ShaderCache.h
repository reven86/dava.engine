/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_SHADER_CACHE_H__
#define __DAVAENGINE_SHADER_CACHE_H__

#include "Base/Singleton.h"
#include "Base/FastName.h"
#include "Base/FastNameMap.h"
#include "Base/BaseMath.h"
#include "Render/Shader.h"

namespace DAVA
{
    
class Data;
    
// TODO: LRU cache for shaders or other type of resources
class ShaderAsset : public BaseObject
{
public:
    struct DefaultValue
    {
        Shader::eUniformType type;
        union
        {
            int32 int32Value;
            float32 float32Value;
            float32 vector2Value[2];
            float32 vector3Value[3];
            float32 vector4Value[4];
            float32 matrix2Value[2 * 2];
            float32 matrix3Value[3 * 3];
            float32 matrix4Value[4 * 4];
        };
    };
    
    ShaderAsset(const FastName & name,
                Data * _vertexShaderData,
                Data * _fragmentShaderData);
    
    ~ShaderAsset();
    
    void Remove(const FastNameSet & defines);
    Shader * Get(const FastNameSet & defines);
    void BindShaderDefaults(Shader * shader);
    const DefaultValue & GetDefaultValue(const FastName & name) { return defaultValues[name]; };
	
private:

	Shader * Compile(const FastNameSet & defines);

	void SetShaderData(Data * _vertexShaderData, Data * _fragmentShaderData);
    void ReloadShaders();
	
	void CompileShaderInternal(Shader *shader, FastNameSet defines);
	void ReloadShaderInternal(Shader *shader);

    void ClearAllLastBindedCaches();
    
protected:
    FastName name;
    Data * vertexShaderData;
    Data * fragmentShaderData;
    uint8 * vertexShaderDataStart;
    uint32 vertexShaderDataSize;
    uint8 * fragmentShaderDataStart;
    uint32 fragmentShaderDataSize;

	Mutex compileShaderMutex;

    HashMap<FastName, DefaultValue> defaultValues;
    
    HashMap < FastNameSet, Shader *> compiledShaders;
    friend class ShaderCache;
};
    
class ShaderCache : public Singleton<ShaderCache>
{
public:
    ShaderCache();
    ~ShaderCache();
    
    ShaderAsset * Load(const FastName & shader);
    ShaderAsset * Get(const FastName & shader);
    Shader * Get(const FastName & shader, const FastNameSet & definesSet);

    void ClearAllLastBindedCaches();

    void Reload();
    
private:

    void LoadAsset(ShaderAsset *asset);
    void ParseShader(ShaderAsset * asset);
    void ParseDefaultVariable(ShaderAsset * asset, const String & inputLine);

	Mutex shaderAssetMapMutex;
    FastNameMap<ShaderAsset*> shaderAssetMap;
};
};

#endif // __DAVAENGINE_SHADER_CACHE_H__
