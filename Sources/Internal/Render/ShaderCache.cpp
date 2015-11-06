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
#include "Render/RHI/rhi_ShaderCache.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{
namespace ShaderDescriptorCache
{
struct ShaderSourceCode
{
    char8* vertexProgText;
    char8* fragmentProgText;

    FilePath vertexProgSourcePath;
    FilePath fragmentProgSourcePath;
};

namespace
{
Map<Vector<int32>, ShaderDescriptor*> shaderDescriptors;
Map<FastName, ShaderSourceCode> shaderSourceCodes;
Mutex shaderCacheMutex;
bool initialized = false;
}

void Initialize()
{
    DVASSERT(!initialized);
    initialized = true;
}

void Uninitialize()
{
    DVASSERT(initialized);
    Clear();
    initialized = false;
}

void Clear()
{
    //RHI_COMPLETE - clear shader descriptors here too?
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    for (auto& it : shaderSourceCodes)
    {
        SafeDelete(it.second.vertexProgText);
        SafeDelete(it.second.fragmentProgText);
    }
    shaderSourceCodes.clear();
}

void ClearDynamicBindigs()
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    for (auto& it : shaderDescriptors)
    {
        it.second->ClearDynamicBindings();
    }
}

void BuildFlagsKey(const FastName& name, const HashMap<FastName, int32>& defines, Vector<int32>& key)
{
    key.clear();
    key.reserve(defines.size() * 2 + 1);
    for (auto& define : defines)
    {
        key.push_back(define.first.Index());
        key.push_back(define.second);
    }
    key.push_back(name.Index());
}

ShaderSourceCode LoadFromSource(const String& source)
{
    ShaderSourceCode sourceCode;
    sourceCode.vertexProgSourcePath = FilePath(source + "-vp.cg");
    sourceCode.fragmentProgSourcePath = FilePath(source + "-fp.cg");

    //later move it into FileSystem

    //vertex
    File* fp = File::Create(sourceCode.vertexProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = fp->GetSize();
        sourceCode.vertexProgText = new char8[fileSize + 1];
        sourceCode.vertexProgText[fileSize] = 0;
        uint32 dataRead = fp->Read((uint8*)sourceCode.vertexProgText, fileSize);
        if (dataRead != fileSize)
        {
            Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    //fragment
    fp = File::Create(sourceCode.fragmentProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = fp->GetSize();
        sourceCode.fragmentProgText = new char8[fileSize + 1];
        sourceCode.fragmentProgText[fileSize] = 0;
        uint32 dataRead = fp->Read((uint8*)sourceCode.fragmentProgText, fileSize);
        if (dataRead != fileSize)
        {
            Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    return sourceCode;
}

ShaderSourceCode GetSourceCode(const FastName& name)
{
    auto sourceIt = shaderSourceCodes.find(name);
    if (sourceIt != shaderSourceCodes.end()) //source found
    {
        return sourceIt->second;
    }
    else
    {
        ShaderSourceCode sourceCode = LoadFromSource(name.c_str());
        shaderSourceCodes[name] = sourceCode;
        return sourceCode;
    }
}

ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines)
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    /*key*/
    Vector<int32> key;
    BuildFlagsKey(name, defines, key);

    auto descriptorIt = shaderDescriptors.find(key);
    if (descriptorIt != shaderDescriptors.end())
        return descriptorIt->second;

    //not found - create new shader
    Vector<String> progDefines;
    progDefines.reserve(defines.size() * 2);
    String resName(name.c_str());
    resName += "  defines: ";
    for (auto& it : defines)
    {
        bool doAdd = true;

        for (size_t i = 0; i != progDefines.size(); i += 2)
        {
            if (strcmp(it.first.c_str(), progDefines[i].c_str()) < 0)
            {
                progDefines.insert(progDefines.begin() + i, String(it.first.c_str()));
                progDefines.insert(progDefines.begin() + i + 1, DAVA::Format("%d", it.second));
                doAdd = false;
                break;
            }
        }

        if (doAdd)
        {
            progDefines.push_back(String(it.first.c_str()));
            progDefines.push_back(DAVA::Format("%d", it.second));
        }
    }

    for (size_t i = 0; i != progDefines.size(); i += 2)
        resName += Format("%s = %s, ", progDefines[i + 0].c_str(), progDefines[i + 1].c_str());

    FastName vProgUid, fProgUid;
    vProgUid = FastName(String("vSource: ") + resName);
    fProgUid = FastName(String("fSource: ") + resName);

    ShaderSourceCode sourceCode = GetSourceCode(name);
    const uint32 vSrcHash = HashValue_N(sourceCode.vertexProgText, static_cast<uint32>(strlen(sourceCode.vertexProgText)));
    const uint32 fSrcHash = HashValue_N(sourceCode.fragmentProgText, static_cast<uint32>(strlen(sourceCode.fragmentProgText)));
    const rhi::ShaderSource* vSource = rhi::ShaderSourceCache::Get(vProgUid, vSrcHash);
    const rhi::ShaderSource* fSource = rhi::ShaderSourceCache::Get(fProgUid, fSrcHash);
    rhi::ShaderSource vSource2(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str());
    rhi::ShaderSource fSource2(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str());

    if (vSource)
    {
        Logger::Info("using cached \"%s\"", vProgUid.c_str());
    }
    else
    {
        Logger::Info("building \"%s\"", vProgUid.c_str());
        vSource2.Construct(rhi::PROG_VERTEX, sourceCode.vertexProgText, progDefines);
        rhi::ShaderSourceCache::Update(vProgUid, vSrcHash, vSource2);
        vSource = &vSource2;
    }

    if (fSource)
    {
        Logger::Info("using cached \"%s\"", fProgUid.c_str());
    }
    else
    {
        Logger::Info("building \"%s\"", fProgUid.c_str());
        fSource2.Construct(rhi::PROG_FRAGMENT, sourceCode.fragmentProgText, progDefines);
        rhi::ShaderSourceCache::Update(fProgUid, fSrcHash, fSource2);
        fSource = &fSource2;
    }

    rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vSource->SourceCode());
    rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fSource->SourceCode());

    //ShaderDescr
    rhi::PipelineState::Descriptor psDesc;
    psDesc.vprogUid = vProgUid;
    psDesc.fprogUid = fProgUid;
    psDesc.vertexLayout = vSource->ShaderVertexLayout();
    psDesc.blending = fSource->Blending();
    rhi::HPipelineState piplineState = rhi::AcquireRenderPipelineState(psDesc);
    ShaderDescriptor* res = new ShaderDescriptor(piplineState, vProgUid, fProgUid);
    res->sourceName = name;
    res->defines = defines;
    res->valid = piplineState.IsValid(); //later add another conditions
    if (res->valid)
    {
        res->UpdateConfigFromSource(const_cast<rhi::ShaderSource*>(vSource), const_cast<rhi::ShaderSource*>(fSource));
        res->requiredVertexFormat = GetVertexLayoutRequiredFormat(psDesc.vertexLayout);
    }

    shaderDescriptors[key] = res;
    return res;
}

void RelaoadShaders()
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    //clear cached source files
    for (auto& it : shaderSourceCodes)
    {
        SafeDelete(it.second.vertexProgText);
        SafeDelete(it.second.fragmentProgText);
    }
    shaderSourceCodes.clear();

    //reload shaders
    for (auto& shaderDescr : shaderDescriptors)
    {
        ShaderDescriptor* shader = shaderDescr.second;

        /*Sources*/
        ShaderSourceCode sourceCode = GetSourceCode(shader->sourceName);
        rhi::ShaderSource vSource(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str());
        rhi::ShaderSource fSource(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str());
        Vector<String> progDefines;
        progDefines.reserve(shader->defines.size() * 2);
        for (auto& it : shader->defines)
        {
            progDefines.push_back(String(it.first.c_str()));
            progDefines.push_back(DAVA::Format("%d", it.second));
        }
        vSource.Construct(rhi::PROG_VERTEX, sourceCode.vertexProgText, progDefines);
        fSource.Construct(rhi::PROG_FRAGMENT, sourceCode.fragmentProgText, progDefines);

        rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_VERTEX, shader->vProgUid, vSource.SourceCode());
        rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_FRAGMENT, shader->fProgUid, fSource.SourceCode());

        //ShaderDescr
        rhi::PipelineState::Descriptor psDesc;
        psDesc.vprogUid = shader->vProgUid;
        psDesc.fprogUid = shader->fProgUid;
        psDesc.vertexLayout = vSource.ShaderVertexLayout();
        psDesc.blending = fSource.Blending();
        rhi::ReleaseRenderPipelineState(shader->piplineState);
        shader->piplineState = rhi::AcquireRenderPipelineState(psDesc);
        shader->valid = shader->piplineState.IsValid(); //later add another conditions
        if (shader->valid)
        {
            shader->UpdateConfigFromSource(&vSource, &fSource);
            shader->requiredVertexFormat = GetVertexLayoutRequiredFormat(psDesc.vertexLayout);
        }
        else
        {
            shader->requiredVertexFormat = 0;
        }
    }
}
}
};
