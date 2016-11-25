#include "Render/ShaderCache.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"
#include "Render/RHI/rhi_ShaderSource.h"
#include "Render/RHI/Common/PreProcess.h"

#define RHI_TRACE_CACHE_USAGE 0

namespace DAVA
{
namespace ShaderDescriptorCache
{
struct ShaderSourceCode
{
    Vector<char> vertexProgText;
    Vector<char> fragmentProgText;

    FilePath vertexProgSourcePath;
    FilePath fragmentProgSourcePath;

    uint32 vSrcHash = 0;
    uint32 fSrcHash = 0;
};

namespace
{
Map<Vector<int32>, ShaderDescriptor*> shaderDescriptors;
Map<FastName, ShaderSourceCode> shaderSourceCodes;
Mutex shaderCacheMutex;
bool loadingNotifyEnabled = false;
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
    DVASSERT(initialized);
    LockGuard<Mutex> guard(shaderCacheMutex);
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

Vector<int32> BuildFlagsKey(const FastName& name, const HashMap<FastName, int32>& defines)
{
    Vector<int32> key;
    key.reserve(defines.size() * 2 + 1);
    for (const auto& define : defines)
    {
        key.emplace_back(define.first.Index());
        key.emplace_back(define.second);
    }

    // reinterpret cast to pairs and sort them
    using Int32Pair = std::pair<int32, int32>;
    Int32Pair* begin = reinterpret_cast<Int32Pair*>(key.data());
    std::sort(begin, begin + key.size() / 2, [](const Int32Pair& l, const Int32Pair& r) {
        return l.first < r.first;
    });

    key.push_back(name.Index());
    return key;
}

void LoadFromSource(const String& source, ShaderSourceCode& sourceCode)
{
    sourceCode.vertexProgSourcePath = FilePath(source + "-vp.cg");
    sourceCode.fragmentProgSourcePath = FilePath(source + "-fp.cg");

    //later move it into FileSystem

    //vertex
    bool loaded = true;
    File* fp = File::Create(sourceCode.vertexProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = static_cast<uint32>(fp->GetSize());
        sourceCode.vertexProgText.resize(fileSize + 1);
        sourceCode.vertexProgText[fileSize] = 0;
        uint32 dataRead = fp->Read(sourceCode.vertexProgText.data(), fileSize);
        if (dataRead != fileSize)
        {
            loaded = false;
            sourceCode.vertexProgText.resize(1);
            sourceCode.vertexProgText.shrink_to_fit();
            Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        loaded = false;
        Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    if (!loaded)
    {
        sourceCode.vertexProgText.resize(1);
        sourceCode.vertexProgText[0] = 0;
    }

    //fragment
    loaded = true;
    fp = File::Create(sourceCode.fragmentProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = static_cast<uint32>(fp->GetSize());
        sourceCode.fragmentProgText.resize(fileSize + 1);
        sourceCode.fragmentProgText[fileSize] = 0;
        uint32 dataRead = fp->Read(sourceCode.fragmentProgText.data(), fileSize);
        if (dataRead != fileSize)
        {
            loaded = false;
            sourceCode.fragmentProgText.resize(1);
            sourceCode.fragmentProgText.shrink_to_fit();
            Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        loaded = false;
        Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    if (!loaded)
    {
        sourceCode.fragmentProgText.resize(1);
        sourceCode.fragmentProgText[0] = 0;
    }

    sourceCode.vSrcHash = HashValue_N(sourceCode.vertexProgText.data(), static_cast<uint32>(strlen(sourceCode.vertexProgText.data())));
    sourceCode.fSrcHash = HashValue_N(sourceCode.fragmentProgText.data(), static_cast<uint32>(strlen(sourceCode.fragmentProgText.data())));
}

const ShaderSourceCode& GetSourceCode(const FastName& name)
{
    auto sourceIt = shaderSourceCodes.find(name);
    if (sourceIt != shaderSourceCodes.end()) //source found
        return sourceIt->second;

    LoadFromSource(name.c_str(), shaderSourceCodes[name]);
    return shaderSourceCodes.at(name);
}

void SetLoadingNotifyEnabled(bool enable)
{
    loadingNotifyEnabled = enable;
}

ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines)
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    Vector<int32> key = BuildFlagsKey(name, defines);

    auto descriptorIt = shaderDescriptors.find(key);
    if (descriptorIt != shaderDescriptors.end())
        return descriptorIt->second;

    if (loadingNotifyEnabled)
    {
        Vector<String> sortedKeys;
        sortedKeys.reserve(defines.size());
        for (const auto& define : defines)
        {
            sortedKeys.emplace_back(DAVA::Format("%s=%d,", define.first.c_str(), define.second));
        }
        std::sort(sortedKeys.begin(), sortedKeys.end());

        String flags;
        flags.reserve(16 * defines.size());
        for (const auto& define : sortedKeys)
        {
            flags += define;
        }
        Logger::Error("Forbidden call to GetShaderDescriptor(%s, { %s })", name.c_str(), flags.c_str());
    }

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
                progDefines.insert(progDefines.begin() + i + 1, Format("%d", it.second));
                doAdd = false;
                break;
            }
        }

        if (doAdd)
        {
            progDefines.push_back(String(it.first.c_str()));
            progDefines.push_back(Format("%d", it.second));
        }
    }

    for (size_t i = 0; i != progDefines.size(); i += 2)
        resName += Format("%s = %s, ", progDefines[i + 0].c_str(), progDefines[i + 1].c_str());

    FastName vProgUid, fProgUid;
    vProgUid = FastName(String("vSource: ") + resName);
    fProgUid = FastName(String("fSource: ") + resName);

    ShaderPreprocessScope preprocessScope;

    const ShaderSourceCode& sourceCode = GetSourceCode(name);

    const rhi::ShaderSource* vSource = rhi::ShaderSourceCache::Get(vProgUid, sourceCode.vSrcHash);
    rhi::ShaderSource vSource2(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str());
    if (vSource == nullptr)
    {
        char localBuffer[RHI_SHADER_SOURCE_BUFFER_SIZE] = {};
        Memcpy(localBuffer, sourceCode.vertexProgText.data(), strlen(sourceCode.vertexProgText.data()));
        vSource2.Construct(rhi::PROG_VERTEX, localBuffer, progDefines);
        rhi::ShaderSourceCache::Update(vProgUid, sourceCode.vSrcHash, vSource2);
        vSource = &vSource2;
    }
#if (RHI_TRACE_CACHE_USAGE)
    else
    {
        Logger::Info("Using cached vertex shader: %s", vProgUid.c_str());
    }
#endif

    const rhi::ShaderSource* fSource = rhi::ShaderSourceCache::Get(fProgUid, sourceCode.fSrcHash);
    rhi::ShaderSource fSource2(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str());
    if (fSource == nullptr)
    {
        char localBuffer[RHI_SHADER_SOURCE_BUFFER_SIZE] = {};
        memcpy(localBuffer, sourceCode.fragmentProgText.data(), strlen(sourceCode.fragmentProgText.data()));
        fSource2.Construct(rhi::PROG_FRAGMENT, localBuffer, progDefines);
        rhi::ShaderSourceCache::Update(fProgUid, sourceCode.fSrcHash, fSource2);
        fSource = &fSource2;
    }
#if (RHI_TRACE_CACHE_USAGE)
    else
    {
        Logger::Info("Using cached: %s", fProgUid.c_str());
    }
#endif

    {
        rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vSource->SourceCode());
        rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fSource->SourceCode());
    }

    ShaderDescriptor* res = nullptr;
    {
        rhi::PipelineState::Descriptor psDesc;
        psDesc.vprogUid = vProgUid;
        psDesc.fprogUid = fProgUid;
        psDesc.vertexLayout = vSource->ShaderVertexLayout();
        psDesc.blending = fSource->Blending();
        rhi::HPipelineState piplineState = rhi::AcquireRenderPipelineState(psDesc);
        res = new ShaderDescriptor(piplineState, vProgUid, fProgUid);
        res->sourceName = name;
        res->defines = defines;
        res->valid = piplineState.IsValid(); //later add another conditions
        if (res->valid)
        {
            res->UpdateConfigFromSource(const_cast<rhi::ShaderSource*>(vSource), const_cast<rhi::ShaderSource*>(fSource));
            res->requiredVertexFormat = GetVertexLayoutRequiredFormat(psDesc.vertexLayout);
        }
    }
    shaderDescriptors[key] = res;

    return res;
}

void ReloadShaders()
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);
    shaderSourceCodes.clear();

    ShaderPreprocessScope preprocessScope;

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
            progDefines.push_back(Format("%d", it.second));
        }
        vSource.Construct(rhi::PROG_VERTEX, sourceCode.vertexProgText.data(), progDefines);
        fSource.Construct(rhi::PROG_FRAGMENT, sourceCode.fragmentProgText.data(), progDefines);

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
