/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Render/ShaderCache.h"
#include "FileSystem/FileSystem.h"
#include "Base/Data.h"
#include "Render/Shader.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
    
    
ShaderAsset::ShaderAsset(const FastName & _name,
                         Data * _vertexShaderData,
                         Data * _fragmentShaderData)
{
    name = _name;
    vertexShaderData = SafeRetain(_vertexShaderData);
    fragmentShaderData = SafeRetain(_fragmentShaderData);
    
    vertexShaderDataStart = 0;
    vertexShaderDataSize = 0;
    
    fragmentShaderDataStart = 0;
    fragmentShaderDataSize = 0;
}

ShaderAsset::~ShaderAsset()
{
	DVASSERT(Thread::IsMainThread());

    HashMap<FastNameSet, Shader *>::iterator end = compiledShaders.end();
    for (HashMap<FastNameSet, Shader *>::iterator it = compiledShaders.begin(); it != end; ++it)
    {
		Shader * shader = it->second;
        SafeRelease(shader);
    }
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
}

void ShaderAsset::SetShaderData(Data * _vertexShaderData, Data * _fragmentShaderData)
{
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
   
    vertexShaderData = SafeRetain(_vertexShaderData);
    fragmentShaderData = SafeRetain(_fragmentShaderData);
    
    vertexShaderDataStart = 0;
    vertexShaderDataSize = 0;
    
    fragmentShaderDataStart = 0;
    fragmentShaderDataSize = 0;
}


Shader * ShaderAsset::Compile(const FastNameSet & defines)
{
    Shader * shader = compiledShaders.at(defines);
    if (shader)return shader;
    
	compileShaderMutex.Lock();

	shader = compiledShaders.at(defines); //to check if shader was created while mutex was locked
	if (NULL == shader)
	{
        shader = Shader::CreateShader(name,
                                            vertexShaderData,
                                            fragmentShaderData,
                                            vertexShaderDataStart,
                                            vertexShaderDataSize,
                                            fragmentShaderDataStart,
                                            fragmentShaderDataSize,
                                            defines);
	
		Function<void()> fn = Bind(&ShaderAsset::CompileShaderInternal, this, shader, defines);
		uint32 jobId = JobManager::Instance()->CreateMainJob(fn);
		JobManager::Instance()->WaitMainJobID(jobId);
	}
    
	compileShaderMutex.Unlock();

    return shader;
}
	
void ShaderAsset::ReloadShaders()
{
    HashMap < FastNameSet, Shader *>::iterator it = compiledShaders.begin();
    HashMap < FastNameSet, Shader *>::iterator endIt = compiledShaders.end();
	for( ; it != endIt; ++it)
	{
		Shader *shader = it->second;
		shader->Reload(vertexShaderData, fragmentShaderData, vertexShaderDataStart, vertexShaderDataSize, fragmentShaderDataStart, fragmentShaderDataSize);

		Function<void()> fn = Bind(&ShaderAsset::ReloadShaderInternal, this, shader);
		uint32 jobId = JobManager::Instance()->CreateMainJob(fn);
		JobManager::Instance()->WaitMainJobID(jobId);
	}
}

void ShaderAsset::CompileShaderInternal(Shader *shader, FastNameSet defines)
{
	shader->Recompile();
	BindShaderDefaults(shader);
	compiledShaders.insert(defines, shader);
}

void ShaderAsset::ReloadShaderInternal(Shader *shader)
{
	shader->Recompile();
	BindShaderDefaults(shader);
}

void ShaderAsset::BindShaderDefaults(Shader * shader)
{
    shader->Bind();
    uint32 count = shader->GetUniformCount(); // TODO: Fix shader get uniform count type to uint32
    //uint32 defaultCount = (uint32)defaultValues.size();
    for (uint32 ui = 0; ui < count; ++ui)
    {
        Shader::Uniform * uniform = shader->GetUniform(ui);
      
        if (defaultValues.count(uniform->name) > 0)
        {
            const DefaultValue & value = defaultValues.at(uniform->name);
            
            switch (value.type)
            {
                case Shader::UT_FLOAT_MAT4:
                    shader->SetUniformValueByIndex(ui, Matrix4(value.matrix4Value[0], value.matrix4Value[1], value.matrix4Value[2], value.matrix4Value[3],
                                                               value.matrix4Value[4], value.matrix4Value[5], value.matrix4Value[6], value.matrix4Value[7],
                                                               value.matrix4Value[8], value.matrix4Value[9], value.matrix4Value[10], value.matrix4Value[11],
                                                               value.matrix4Value[12], value.matrix4Value[13], value.matrix4Value[14], value.matrix4Value[15]));
                break;
                case Shader::UT_FLOAT_MAT3:
                    shader->SetUniformValueByIndex(ui, Matrix3(value.matrix3Value[0], value.matrix3Value[1], value.matrix3Value[2],
                                                               value.matrix3Value[3], value.matrix3Value[4], value.matrix3Value[5],
                                                               value.matrix3Value[6], value.matrix3Value[7], value.matrix3Value[8]));
                break;
                case Shader::UT_FLOAT_VEC3:
                    shader->SetUniformValueByIndex(ui, Vector3(value.vector3Value[0], value.vector3Value[1], value.vector3Value[2]));
                break;
                case Shader::UT_FLOAT_VEC2:
                    shader->SetUniformValueByIndex(ui, Vector2(value.vector2Value[0], value.vector2Value[1]));
                break;
                case Shader::UT_FLOAT:
                    shader->SetUniformValueByIndex(ui, value.float32Value);
                break;
                case Shader::UT_INT:
                    shader->SetUniformValueByIndex(ui, value.int32Value);
                break;

                default:
                break;
            }
            
        }
    }
    shader->Unbind();
}

    
void ShaderAsset::Remove(const FastNameSet & defines)
{
    
}
    
Shader * ShaderAsset::Get(const FastNameSet & defines)
{
	/*String str;
	defines.ToString(str);
	
	if(dbgMap.find(str) == dbgMap.end())
	{
		dbgMap[str] = 0;
	}
	else
	{
		DVASSERT(compiledShaders.GetValue(defines));
	}*/
	
//    String definesToDraw;
//    for (FastNameSet::iterator it = defines.begin(), end = defines.end(); it != end; ++it)
//    {
//        
//    }
    
    Shader * shader = compiledShaders.at(defines);
    
    if (!shader)
    {
        shader = Compile(defines);
    }
    
    return shader;
}

void ShaderAsset::ClearAllLastBindedCaches()
{
    HashMap<FastNameSet, Shader *>::iterator end = compiledShaders.end();
    for (HashMap<FastNameSet, Shader *>::iterator it = compiledShaders.begin(); it != end; ++it)
        it->second->ClearLastBindedCaches();
}

ShaderCache::ShaderCache()
{
    
}
    
ShaderCache::~ShaderCache()
{
	shaderAssetMapMutex.Lock();

    FastNameMap<ShaderAsset*>::iterator end = shaderAssetMap.end();
    for (FastNameMap<ShaderAsset*>::iterator it = shaderAssetMap.begin(); it != end; ++it)
    {
		ShaderAsset * asset = it->second;
        SafeRelease(asset);
    }
	shaderAssetMap.clear();
	
	shaderAssetMapMutex.Unlock();
}

void ShaderCache::ClearAllLastBindedCaches()
{
	shaderAssetMapMutex.Lock();

    FastNameMap<ShaderAsset*>::iterator end = shaderAssetMap.end();
    for (FastNameMap<ShaderAsset*>::iterator it = shaderAssetMap.begin(); it != end; ++it)
        it->second->ClearAllLastBindedCaches();

	shaderAssetMapMutex.Unlock();
}
    
void ShaderCache::ParseDefaultVariable(ShaderAsset * asset, const String & inputLine)
{
    if (inputLine.find("uniform") == String::npos) return;
    
    Vector<String> tokens;
    Split(inputLine, " (,\t;", tokens);
    
    
    /*
        Line format: 
        uniform [highp] vec3 var1 = vec3(1.0, 1.0, 1.0);
        uniform mat3 var2 = mat3(1.0, 1.0, 1.0, 5.0, 1.0, 0.4, 8.5, 0.6, 0.8);
        uniform float float = 1.0;
     */
    
    // Remove precision qualifiers
    String qualifier;
    for (size_t k = 0; k < tokens.size(); ++k)
    {
        if ((tokens[k] == "highp") || (tokens[k] == "mediump") || (tokens[k] == "lowp"))
        {
            qualifier = tokens[k];
            tokens.erase (tokens.begin() + k);
            k--;
        }
    }
    
    if (tokens[0] == "uniform")
    {
        //
        const String & type = tokens[1];
        const String & name = tokens[2];
        const String & equals = tokens[3];
        
        DVASSERT(equals == "=");
        
        uint32 valuesCount = 0;
        ShaderAsset::DefaultValue value;
        if ((type == "sampler2D") || (type == "samplerCube"))
        {
            value.type = Shader::UT_INT;
            value.int32Value = atoi(tokens[4].c_str());
        }
        else if (type == "float")
        {
            value.type = Shader::UT_FLOAT;
            value.float32Value = (float32)atof(tokens[4].c_str());
        }
        else if (type == "vec2")
        {
            value.type = Shader::UT_FLOAT_VEC2;
            valuesCount = 2;
        }
        else if (type == "vec3")
        {
            value.type = Shader::UT_FLOAT_VEC3;
            valuesCount = 3;
        }
        else if (type == "vec4")
        {
            value.type = Shader::UT_FLOAT_VEC4;
            valuesCount = 4;
        }
        else if (type == "mat2")
        {
            value.type = Shader::UT_FLOAT_MAT2;
            valuesCount = 2 * 2;
        }
        else if (type == "mat3")
        {
            value.type = Shader::UT_FLOAT_MAT3;
            valuesCount = 3 * 3;
        }
        else if (type == "mat4")
        {
            value.type = Shader::UT_FLOAT_MAT4;
            valuesCount = 4 * 4;
        }
        
        if (valuesCount > 1)
            for (uint32 k = 0; k < valuesCount; ++k)
            {
                value.matrix4Value[k] = (float32)atof(tokens[5 + k].c_str());
            };
        
        FastName fastName = FastName(name);
        asset->defaultValues.insert(fastName, value);
        
        //return tokens[0] + String(" ") + type + String(" ") + qualifier + String(" ") + name + ";";
    }
    
    /*if ((tokens.size() == 3) && (tokens[1] == "=") && (tokens[2].size() > 0))
    {
        ShaderAsset::DefaultValue value;
        if ((tokens[2].find(".") != String::npos) || (tokens[2].find("-") != String::npos))
        value.float32Value = (float32)atof(tokens[2].c_str());
        else
        value.int32Value = atoi(tokens[2].c_str());
        FastName fastName = FastName(tokens[0]);
        asset->defaultValues.insert(fastName, value);
        
        Logger::Debug("Shader Default: %s = %d", fastName.c_str(), value.int32Value);
    }*/
}
    
void ShaderCache::ParseShader(ShaderAsset * asset)
{
    Data * vertexShaderData = asset->vertexShaderData;
    Data * fragmentShaderData = asset->fragmentShaderData;
    
    static const char * TOKEN_CONFIG = "<CONFIG>";
    static const char * TOKEN_VERTEX_SHADER = "<VERTEX_SHADER>";
    static const char * TOKEN_FRAGMENT_SHADER = "<FRAGMENT_SHADER>";
    
    uint8 * vertexShaderStartPosition = vertexShaderData->GetPtr();
    
    String sourceFile((char8*)vertexShaderData->GetPtr(), vertexShaderData->GetSize());
    //size_t size = sourceFile.size();
    
    size_t lineBegin	= 0;
    size_t lineComment	= 0;
    size_t lineEnd		= 0;
    size_t lineEnding   = 0;
    
    bool lastLine = false;
    
    //Vector<String> includesList;		// used to prevent double or recursive includes
    //includesList.push_back(vertexShaderPath.GetFilename());
    
    bool configStarted = false;
    
    while(1)
    {
        if(lastLine)
        {
            break;
        }
        
        // get next line
        lineEnding = 0;
        lineEnd = sourceFile.find("\r\n", lineBegin);
        if (String::npos != lineEnd)
        {
            lineEnding = 2;
        }else
        {
            lineEnd = sourceFile.find("\n", lineBegin);
            lineEnding = 1;
        }
        if(String::npos == lineEnd)
        {
            lastLine = true;
            lineEnd = sourceFile.size();
        }
        
        // skip comment
        lineComment = sourceFile.find("//", lineBegin);
        size_t lineLen = 0;
        if(String::npos == lineComment || lineComment > lineEnd)
        {
            lineLen = lineEnd - lineBegin;
        }else
        {
            lineLen = lineComment - lineBegin;
        }
        
        String line = sourceFile.substr(lineBegin, lineLen);
        if (line == TOKEN_VERTEX_SHADER)
        {
            vertexShaderStartPosition = (uint8*)vertexShaderData->GetPtr() + lineBegin + lineLen + lineEnding;
            configStarted = false;
        }
        else if (line == TOKEN_CONFIG)
        {
            configStarted = true;
        }
        else if (configStarted)
        {
            // GetToken();
            ParseDefaultVariable(asset, line);
        }
        //Logger::Debug("%s", line.c_str());
        lineBegin = lineEnd + lineEnding;
    }
    
    asset->vertexShaderDataStart = vertexShaderStartPosition;
    asset->vertexShaderDataSize = static_cast<uint32>(vertexShaderData->GetSize() - (vertexShaderStartPosition - vertexShaderData->GetPtr()));
    
    //    includesList.clear();
    //    includesList.push_back(fragmentShaderPath.GetFilename());
    uint8 * fragmentShaderStartPosition = fragmentShaderData->GetPtr();
    sourceFile = String((char8*)fragmentShaderData->GetPtr(), fragmentShaderData->GetSize());
    configStarted = false;
    
    lineBegin	= 0;
    lineComment	= 0;
    lineEnd		= 0;
    
    lastLine = false;
    
    while(1)
    {
        if(lastLine)
        {
            break;
        }
        /*
         lineEnding = 0;
         // get next line
         lineEnd = sourceFile.find("\r\n", lineBegin);
         if(String::npos != lineEnd)
         {
         lineEnding = 2;
         }else
         {
         lineEnd = sourceFile.find("\n", lineBegin);
         if(String::npos != lineEnd)
         {
         lineEnding = 1;
         }
         }
         */
        // get next line
        lineEnding = 0;
        lineEnd = sourceFile.find("\r\n", lineBegin);
        if (String::npos != lineEnd)
        {
            lineEnding = 2;
        }else
        {
            lineEnd = sourceFile.find("\n", lineBegin);
            lineEnding = 1;
        }
        if(String::npos == lineEnd)
        {
            lastLine = true;
            lineEnd = sourceFile.size();
        }
        
        // skip comment
        lineComment = sourceFile.find("//", lineBegin);
        size_t lineLen = 0;
        if(String::npos == lineComment || lineComment > lineEnd)
        {
            lineLen = lineEnd - lineBegin;
        }else
        {
            lineLen = lineComment - lineBegin;
        }
        
        String line = sourceFile.substr(lineBegin, lineLen);
        if (line == TOKEN_FRAGMENT_SHADER)
        {
            fragmentShaderStartPosition = (uint8*)fragmentShaderData->GetPtr() + lineBegin + lineLen + lineEnding;
            configStarted = false;
        }else if (line == TOKEN_CONFIG)
        {
            configStarted = true;
        }
        else if (configStarted)
        {
            ParseDefaultVariable(asset, line);
        }
        lineBegin = lineEnd + lineEnding;
    }
    asset->fragmentShaderDataStart = fragmentShaderStartPosition;
    asset->fragmentShaderDataSize = static_cast<uint32>(fragmentShaderData->GetSize() - (fragmentShaderStartPosition - fragmentShaderData->GetPtr()));
    
    
    //        curData = strtok((char*)fragmentShaderData, "\n");
    //        while(curData != NULL)
    //        {
    //            if (strcmp(curData, TOKEN_FRAGMENT_SHADER) == 0)
    //            {
    //                fragmentShaderStartPosition = (uint8*)curData + sizeof(TOKEN_FRAGMENT_SHADER);
    //            }
    //            curData = strtok(NULL, "\n");
    //        }
}

ShaderAsset * ShaderCache::Load(const FastName & shaderFastName)
{
    ShaderAsset * asset = new ShaderAsset(shaderFastName, NULL, NULL);
    LoadAsset(asset);

	shaderAssetMapMutex.Lock();
	ShaderAsset * checkAsset = shaderAssetMap.at(shaderFastName);
	DVASSERT(checkAsset == 0);
	
	shaderAssetMap.Insert(shaderFastName, asset);
	shaderAssetMapMutex.Unlock();

    return asset;
};


ShaderAsset * ShaderCache::Get(const FastName & shader)
{
	shaderAssetMapMutex.Lock();
	ShaderAsset *asset = shaderAssetMap.at(shader);
	shaderAssetMapMutex.Unlock();
    return asset;
}
    
Shader * ShaderCache::Get(const FastName & shaderName, const FastNameSet & definesSet)
{
	shaderAssetMapMutex.Lock();
    ShaderAsset * asset = shaderAssetMap.at(shaderName);
	shaderAssetMapMutex.Unlock();

    if (!asset)
    {
        asset = Load(shaderName);
    }
    Shader * shader = asset->Get(definesSet);
    //Logger::FrameworkDebug(Format("shader: %s %d", shaderName.c_str(), shader->GetRetainCount()).c_str());
    return shader;
}
    
void ShaderCache::Reload()
{
	shaderAssetMapMutex.Lock();

    FastNameMap<ShaderAsset*>::iterator it = shaderAssetMap.begin();
    FastNameMap<ShaderAsset*>::iterator endIt = shaderAssetMap.end();
    for( ; it != endIt; ++it)
    {
        ShaderAsset *asset = it->second;

        LoadAsset(asset);
        asset->ReloadShaders();
    }

	shaderAssetMapMutex.Unlock();
}

  
void ShaderCache::LoadAsset(ShaderAsset *asset)
{
    const FastName & shaderFastName = asset->name;

    String shader = shaderFastName.c_str();
    String vertexShaderPath = shader + ".vsh";
    String fragmentShaderPath = shader + ".fsh";

    uint32 vertexShaderSize = 0, fragmentShaderSize = 0;

    uint8 * vertexShaderBytes = FileSystem::Instance()->ReadFileContents(vertexShaderPath, vertexShaderSize);
    Data * vertexShaderData = new Data(vertexShaderBytes, vertexShaderSize);

    uint8 * fragmentShaderBytes = FileSystem::Instance()->ReadFileContents(fragmentShaderPath, fragmentShaderSize);
    Data * fragmentShaderData = new Data(fragmentShaderBytes, fragmentShaderSize);

    asset->SetShaderData(vertexShaderData, fragmentShaderData);
    ParseShader(asset);
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
}
    
};