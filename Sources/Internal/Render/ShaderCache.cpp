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
    HashMap<FastNameSet, Shader *>::iterator end = compiledShaders.end();
    for (HashMap<FastNameSet, Shader *>::iterator it = compiledShaders.begin(); it != end; ++it)
    {
		Shader * shader = it->second;
        SafeRelease(shader);
    }
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
}

Shader * ShaderAsset::Compile(const FastNameSet & defines)
{
    Shader * checkShader = compiledShaders.at(defines);
    if (checkShader)return checkShader;
    
    Shader * shader = Shader::CompileShader(name,
                                            vertexShaderData,
                                            fragmentShaderData,
                                            vertexShaderDataStart,
                                            vertexShaderDataSize,
                                            fragmentShaderDataStart,
                                            fragmentShaderDataSize,
                                            defines);
	
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN,
														   Message(this, &ShaderAsset::BindShaderDefaultsInternal, shader));
	JobInstanceWaiter waiter(job);
	waiter.Wait();

    compiledShaders.insert(defines, shader);
    return shader;
}
	
void ShaderAsset::BindShaderDefaultsInternal(BaseObject * caller, void * param, void *callerData)
{
	BindShaderDefaults((Shader*)param);
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
            shader->SetUniformValueByIndex(ui, value.int32Value);
            //Logger::FrameworkDebug("Assign: %s = %d", uniform->name.c_str(), value.int32Value);
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
    FastNameMap<ShaderAsset*>::iterator end = shaderAssetMap.end();
    for (FastNameMap<ShaderAsset*>::iterator it = shaderAssetMap.begin(); it != end; ++it)
    {
		ShaderAsset * asset = it->second;
        SafeDelete(asset);
    }
}

void ShaderCache::ClearAllLastBindedCaches()
{
    FastNameMap<ShaderAsset*>::iterator end = shaderAssetMap.end();
    for (FastNameMap<ShaderAsset*>::iterator it = shaderAssetMap.begin(); it != end; ++it)
        it->second->ClearAllLastBindedCaches();
}
    
ShaderAsset * ShaderCache::ParseShader(const FastName & name, Data * vertexShaderData, Data * fragmentShaderData)
{
    ShaderAsset * asset = new ShaderAsset(name, vertexShaderData, fragmentShaderData);
    
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
            Vector<String> tokens;
            Split(line, " \t", tokens);
            
            if ((tokens.size() == 3) && (tokens[1] == "=") && (tokens[2].size() > 0))
            {
                ShaderAsset::DefaultValue value;
                if ((tokens[2].find(".") != String::npos) || (tokens[2].find("-") != String::npos))
                    value.float32Value = (float32)atof(tokens[2].c_str());
                else
                    value.int32Value = atoi(tokens[2].c_str());
                FastName fastName = FastName(tokens[0]);
                asset->defaultValues.insert(fastName, value);
                
                Logger::Debug("Shader Default: %s = %d", fastName.c_str(), value.int32Value);
            }
        }
        //Logger::Debug("%s", line.c_str());
        lineBegin = lineEnd + lineEnding;
    }
    
    asset->vertexShaderDataStart = vertexShaderStartPosition;
    asset->vertexShaderDataSize = vertexShaderData->GetSize() - (vertexShaderStartPosition - vertexShaderData->GetPtr());

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
            // GetToken();
            Vector<String> tokens;
            Split(line, " \t", tokens);
            
            if ((tokens.size() == 3) && (tokens[1] == "=") && (tokens[2].size() > 0))
            {
                ShaderAsset::DefaultValue value;
                if ((tokens[2].find(".") != String::npos) || (tokens[2].find("-") != String::npos))
                    value.float32Value = (float32)atof(tokens[2].c_str());
                else
                    value.int32Value = atoi(tokens[2].c_str());
                FastName fastName = FastName(tokens[0]);
                asset->defaultValues.insert(fastName, value);
                
                Logger::Debug("Shader Default: %s = %d", fastName.c_str(), value.int32Value);
            }
        }
        
        //Logger::Debug("%s", line.c_str());

        lineBegin = lineEnd + lineEnding;
    }
    asset->fragmentShaderDataStart = fragmentShaderStartPosition;
    asset->fragmentShaderDataSize = fragmentShaderData->GetSize() - (fragmentShaderStartPosition - fragmentShaderData->GetPtr());
    
    
    //        curData = strtok((char*)fragmentShaderData, "\n");
    //        while(curData != NULL)
    //        {
    //            if (strcmp(curData, TOKEN_FRAGMENT_SHADER) == 0)
    //            {
    //                fragmentShaderStartPosition = (uint8*)curData + sizeof(TOKEN_FRAGMENT_SHADER);
    //            }
    //            curData = strtok(NULL, "\n");
    //        }
    return asset;
}

    
ShaderAsset * ShaderCache::Load(const FastName & shaderFastName)
{
    ShaderAsset * checkAsset = shaderAssetMap.at(shaderFastName);
    DVASSERT(checkAsset == 0);

    String shader = shaderFastName.c_str();
    String vertexShaderPath = shader + ".vsh";
    String fragmentShaderPath = shader + ".fsh";
    
    uint32 vertexShaderSize = 0, fragmentShaderSize = 0;
    
    uint8 * vertexShaderBytes = FileSystem::Instance()->ReadFileContents(vertexShaderPath, vertexShaderSize);
    Data * vertexShaderData = new Data(vertexShaderBytes, vertexShaderSize);
    
    uint8 * fragmentShaderBytes = FileSystem::Instance()->ReadFileContents(fragmentShaderPath, fragmentShaderSize);
    Data * fragmentShaderData = new Data(fragmentShaderBytes, fragmentShaderSize);
    
    ShaderAsset * asset = ParseShader(shaderFastName, vertexShaderData, fragmentShaderData);
    //new ShaderAsset(vertexShaderData, fragmentShaderData);

    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);

    shaderAssetMap.Insert(shaderFastName, asset);
    return asset;
};


ShaderAsset * ShaderCache::Get(const FastName & shader)
{
    return shaderAssetMap.at(shader);
}
    
Shader * ShaderCache::Get(const FastName & shaderName, const FastNameSet & definesSet)
{
    ShaderAsset * asset = shaderAssetMap.at(shaderName);
    if (!asset)
    {
        asset = Load(shaderName);
    }
    Shader * shader = asset->Get(definesSet);
    //Logger::FrameworkDebug(Format("shader: %s %d", shaderName.c_str(), shader->GetRetainCount()).c_str());
    return shader;
}

    

    
    
};