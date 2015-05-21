#include "DynamicBindings.h"

#include "Math/AABBox3.h"
#include "Core/Core.h"
#include "Platform/SystemTimer.h"
#include "Render/RenderBase.h"
#include "Render/RenderBase.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA
{
namespace
{
const FastName DYNAMIC_PARAM_NAMES[DynamicBindings::DYNAMIC_PARAMETERS_COUNT] =
{
    FastName("unknownSemantic"),
    FastName("worldMatrix"),//PARAM_WORLD,
    FastName("invWorldMatrix"), //PARAM_INV_WORLD,
    FastName("worldInvTransposeMatrix"), //PARAM_WORLD_INV_TRANSPOSE,

    FastName("viewMatrix"), //PARAM_VIEW,
    FastName("invViewMatrix"), //PARAM_INV_VIEW,
    FastName("projMatrix"), //PARAM_PROJ,
    FastName("invProjMatrix"), //PARAM_INV_PROJ,

    FastName("worldViewMatrix"), //PARAM_WORLD_VIEW,
    FastName("invWorldViewMatrix"), //PARAM_INV_WORLD_VIEW,
    FastName("worldViewInvTransposeMatrix"), //PARAM_NORMAL, // NORMAL MATRIX

    FastName("viewProjMatrix"), //PARAM_VIEW_PROJ,
    FastName("invViewProjMatrix"), //PARAM_INV_VIEW_PROJ,

    FastName("worldViewProjMatrix"), //PARAM_WORLD_VIEW_PROJ,
    FastName("invWorldViewProjMatrix"), //PARAM_INV_WORLD_VIEW_PROJ,

    FastName("flatColor"),
    FastName("globalTime"),
    FastName("worldScale"),

    FastName("cameraPosition"), // PARAM_CAMERA_POS,
    FastName("cameraDirection"), // PARAM_CAMERA_DIR,
    FastName("cameraUp"), // PARAM_CAMERA_UP,

    FastName("lightPosition0"),
    FastName("lightColor0"),
    FastName("lightAmbientColor0"),

    FastName("localBoundingBox"),
    FastName("worldViewObjectCenter"),
    FastName("boundingBoxSize"),

    FastName("trunkOscillationParams"),
    FastName("leafOscillationParams"),
    FastName("speedTreeLightSmoothing"),

    FastName("sphericalHarmonics[0]"),

    FastName("jointPositions[0]"),
    FastName("jointQuaternions[0]"),
    FastName("jointsCount"),

    FastName("viewportSize"),
    FastName("rcpViewportSize"),
    FastName("viewportOffset"),

    FastName("shadowColor")
};

const FastName DYNAMIC_TEXTURE_NAMES[DynamicBindings::DYNAMIC_TEXTURES_COUNT] =
{
    FastName("unknownTexture"),
    FastName("dynamicReflection"),
    FastName("dynamicRefraction")
};

}

DynamicBindings::eUniformSemantic DynamicBindings::GetUniformSemanticByName(const FastName& name)
{
    for (int32 k = 0; k < DYNAMIC_PARAMETERS_COUNT; ++k)
        if (name == DYNAMIC_PARAM_NAMES[k])return (eUniformSemantic)k;
    return UNKNOWN_SEMANTIC;
}

DynamicBindings::eTextureSemantic DynamicBindings::GetTextureSemanticByName(const FastName& name)
{
    for (int32 k = 0; k < DYNAMIC_TEXTURES_COUNT; ++k)
        if (name == DYNAMIC_TEXTURE_NAMES[k])return (eTextureSemantic)k;
    return TEXTURE_STATIC;
}

void DynamicBindings::SetDynamicParam(DynamicBindings::eUniformSemantic shaderSemantic, const void * value, pointer_size _updateSemantic)
{
    //AutobindVariableData * var = &dynamicParameters[shaderSemantic];
    //if (var->updateSemantic
    if (_updateSemantic == UPDATE_SEMANTIC_ALWAYS || dynamicParameters[shaderSemantic].updateSemantic != _updateSemantic)
    {

        if (_updateSemantic == UPDATE_SEMANTIC_ALWAYS)
            dynamicParameters[shaderSemantic].updateSemantic++;
        else
            dynamicParameters[shaderSemantic].updateSemantic = _updateSemantic;

        dynamicParameters[shaderSemantic].value = value;
        dynamicParamersRequireUpdate &= ~(1 << shaderSemantic);
       
        switch (shaderSemantic)
        {
        case PARAM_WORLD:
            dynamicParamersRequireUpdate |= ((1 << PARAM_INV_WORLD) | (1 << PARAM_WORLD_VIEW) | (1 << PARAM_INV_WORLD_VIEW) | (1 << PARAM_WORLD_VIEW_OBJECT_CENTER)
                | (1 << PARAM_WORLD_VIEW_PROJ) | (1 << PARAM_INV_WORLD_VIEW_PROJ) | (1 << PARAM_WORLD_VIEW_INV_TRANSPOSE) | (1 << PARAM_WORLD_INV_TRANSPOSE) | (1 << PARAM_WORLD_SCALE));
            break;
        case PARAM_VIEW:
            dynamicParamersRequireUpdate |= ((1 << PARAM_INV_VIEW)
                | (1 << PARAM_WORLD_VIEW)
                | (1 << PARAM_INV_WORLD_VIEW)
                | (1 << PARAM_WORLD_VIEW_PROJ)
                | (1 << PARAM_INV_WORLD_VIEW_PROJ)
                | (1 << PARAM_VIEW_PROJ)
                | (1 << PARAM_INV_VIEW_PROJ)
                | (1 << PARAM_WORLD_VIEW_INV_TRANSPOSE)
                | (1 << PARAM_WORLD_VIEW_OBJECT_CENTER));
            break;
        case PARAM_PROJ:
            dynamicParamersRequireUpdate |= ((1 << PARAM_INV_PROJ) | (1 << PARAM_VIEW_PROJ) | (1 << PARAM_INV_VIEW_PROJ) |
                (1 << PARAM_WORLD_VIEW_PROJ) | (1 << PARAM_INV_WORLD_VIEW_PROJ));
            break;
        case PARAM_LOCAL_BOUNDING_BOX:
            dynamicParamersRequireUpdate |= (1 << PARAM_BOUNDING_BOX_SIZE) | (1 << PARAM_WORLD_VIEW_OBJECT_CENTER);
        default:
            break;
        }

    }
}

void DynamicBindings::ComputeWorldViewMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW))
    {
        worldViewMatrix = GetDynamicParamMatrix(PARAM_WORLD) * GetDynamicParamMatrix(PARAM_VIEW);
        SetDynamicParam(PARAM_WORLD_VIEW, &worldViewMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

void DynamicBindings::ComputeWorldViewObjectCenterIfRequired()
{
    ComputeWorldViewMatrixIfRequired();
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW_OBJECT_CENTER))
    {
        AABBox3 * objectBox = (AABBox3*)GetDynamicParam(PARAM_LOCAL_BOUNDING_BOX);
        Matrix4 * worldView = (Matrix4 *)GetDynamicParam(PARAM_WORLD_VIEW);
        worldViewObjectCenter = objectBox->GetCenter() * (*worldView);
        SetDynamicParam(PARAM_WORLD_VIEW_OBJECT_CENTER, &worldViewObjectCenter, UPDATE_SEMANTIC_ALWAYS);
    }
}

void DynamicBindings::ComputeWorldScaleIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_SCALE))
    {
        worldScale = GetDynamicParamMatrix(PARAM_WORLD).GetScaleVector();

        SetDynamicParam(PARAM_WORLD_SCALE, &worldScale, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::UpdateGlobalTimeIfRequired()
{
    uint32 globalFrameIndex = Core::Instance()->GetGlobalFrameIndex();
    if (dynamicParameters[PARAM_GLOBAL_TIME].updateSemantic != globalFrameIndex)
    {
        frameGlobalTime = SystemTimer::Instance()->GetGlobalTime();
        SetDynamicParam(PARAM_GLOBAL_TIME, &frameGlobalTime, globalFrameIndex);
    }
}

inline void DynamicBindings::ComputeLocalBoundingBoxSizeIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_BOUNDING_BOX_SIZE))
    {
        AABBox3 * objectBox = (AABBox3*)DynamicBindings::GetDynamicParam(PARAM_LOCAL_BOUNDING_BOX);
        boundingBoxSize = objectBox->GetSize();

        SetDynamicParam(PARAM_BOUNDING_BOX_SIZE, &boundingBoxSize, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeViewProjMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_VIEW_PROJ))
    {
        viewProjMatrix = GetDynamicParamMatrix(PARAM_VIEW) * GetDynamicParamMatrix(PARAM_PROJ);
        SetDynamicParam(PARAM_VIEW_PROJ, &viewProjMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeWorldViewProjMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW_PROJ))
    {
        ComputeViewProjMatrixIfRequired();
        worldViewProjMatrix = GetDynamicParamMatrix(PARAM_WORLD) * GetDynamicParamMatrix(PARAM_VIEW_PROJ);
        SetDynamicParam(PARAM_WORLD_VIEW_PROJ, &worldViewProjMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeInvWorldViewMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_INV_WORLD_VIEW))
    {
        ComputeWorldViewMatrixIfRequired();
        worldViewMatrix.GetInverse(invWorldViewMatrix);
        SetDynamicParam(PARAM_INV_WORLD_VIEW, &invWorldViewMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeWorldViewInvTransposeMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW_INV_TRANSPOSE))
    {
        ComputeInvWorldViewMatrixIfRequired();
        normalMatrix = invWorldViewMatrix;
        normalMatrix.Transpose();
        SetDynamicParam(PARAM_WORLD_VIEW_INV_TRANSPOSE, &normalMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}


inline void DynamicBindings::ComputeInvWorldMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_INV_WORLD))
    {
        const Matrix4 & worldMatrix = GetDynamicParamMatrix(PARAM_WORLD);
        worldMatrix.GetInverse(invWorldMatrix);
        SetDynamicParam(PARAM_INV_WORLD, &invWorldMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeWorldInvTransposeMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_INV_TRANSPOSE))
    {
        ComputeInvWorldMatrixIfRequired();
        worldInvTransposeMatrix = invWorldMatrix;
        worldInvTransposeMatrix.Transpose();
        SetDynamicParam(PARAM_WORLD_INV_TRANSPOSE, &worldInvTransposeMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}


int32 DynamicBindings::GetDynamicParamArraySize(DynamicBindings::eUniformSemantic shaderSemantic, int32 defaultValue)
{
    if ((shaderSemantic == PARAM_JOINT_POSITIONS) || (shaderSemantic == PARAM_JOINT_QUATERNIONS))
        return *((int32*)GetDynamicParam(PARAM_JOINTS_COUNT));
    else
        return defaultValue;
}
const void * DynamicBindings::GetDynamicParam(eUniformSemantic shaderSemantic)
{
    switch (shaderSemantic)
    {
    case PARAM_WORLD_VIEW_PROJ:
        ComputeWorldViewProjMatrixIfRequired();
        break;
    case PARAM_WORLD_VIEW:
        ComputeWorldViewMatrixIfRequired();
        break;
    case PARAM_WORLD_SCALE:
        ComputeWorldScaleIfRequired();
        break;
    case PARAM_INV_WORLD_VIEW:
        ComputeInvWorldViewMatrixIfRequired();
        break;
    case PARAM_WORLD_VIEW_INV_TRANSPOSE:
        ComputeWorldViewInvTransposeMatrixIfRequired();
        break;
    case PARAM_WORLD_INV_TRANSPOSE:
        ComputeWorldInvTransposeMatrixIfRequired();
        break;
    case PARAM_WORLD_VIEW_OBJECT_CENTER:
        ComputeWorldViewObjectCenterIfRequired();
        break;
    case PARAM_BOUNDING_BOX_SIZE:
        ComputeLocalBoundingBoxSizeIfRequired();
        break;
    case PARAM_GLOBAL_TIME:
        UpdateGlobalTimeIfRequired();
        break;
    default:
        break;
    }
    DVASSERT(dynamicParameters[shaderSemantic].value != 0);
    return dynamicParameters[shaderSemantic].value;
}

pointer_size DynamicBindings::GetDynamicParamUpdateSemantic(eUniformSemantic shaderSemantic)
{
    return dynamicParameters[shaderSemantic].updateSemantic;
}


rhi::HTexture DynamicBindings::GetDynamicTexture(eTextureSemantic semantic)
{
    DVASSERT(semantic != TEXTURE_STATIC);
    DVASSERT(semantic < DYNAMIC_TEXTURES_COUNT);
    if (!dynamicTextures[semantic].IsValid())
        InitDynamicTexture(semantic);

    return dynamicTextures[semantic];
}
void DynamicBindings::ClearDynamicTextures()
{
    for (int32 i = 0; i < DYNAMIC_TEXTURES_COUNT; i++)
    {
        if (dynamicTextures[i].IsValid())
        {
            rhi::DeleteTexture(dynamicTextures[i]);
            dynamicTextures[i] = rhi::HTexture();
        }
    }
}

void DynamicBindings::InitDynamicTexture(eTextureSemantic semantic)
{
    DVASSERT(!dynamicTextures[semantic].IsValid());
    rhi::Texture::Descriptor descriptor;
    switch (semantic)
    {    
    case DAVA::DynamicBindings::TEXTURE_DYNAMIC_REFLECTION:        
        descriptor.width = REFLECTION_TEX_SIZE;
        descriptor.height = REFLECTION_TEX_SIZE;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_R5G6B5;                
        dynamicTextures[semantic] = rhi::CreateTexture(descriptor);
        break;
    case DAVA::DynamicBindings::TEXTURE_DYNAMIC_REFRACTION:        
        descriptor.width = REFRACTION_TEX_SIZE;
        descriptor.height = REFRACTION_TEX_SIZE;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_R5G6B5;
        dynamicTextures[semantic] = rhi::CreateTexture(descriptor);
        break;
    
    default:
        DVASSERT_MSG(false, "Trying to init unknown texture as dynamic");
        break;
    }
}

rhi::SamplerState::Descriptor::Sampler DynamicBindings::GetDynamicTextureSamplerState(eTextureSemantic semantic)
{
    rhi::SamplerState::Descriptor::Sampler sampler;
    sampler.addrU = rhi::TEXADDR_MIRROR;
    sampler.addrV = rhi::TEXADDR_MIRROR;
    sampler.addrW = rhi::TEXADDR_MIRROR;
    sampler.magFilter = rhi::TEXFILTER_LINEAR;
    sampler.minFilter = rhi::TEXFILTER_LINEAR;
    sampler.mipFilter = rhi::TEXMIPFILTER_NONE;
    return sampler;
}

}