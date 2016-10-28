#include "ApplyMaterialPresetCommand.h"

#include "Render/Material/NMaterial.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FilePath.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace ApplyMaterialPresetDetail
{
const DAVA::String contentNodeName("content");

void StoreMaterialTextures(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                           DAVA::KeyedArchive* content, const DAVA::SerializationContext& context)
{
    DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);

    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata);
    for (const auto& texName : membersList)
    {
        if (material->HasLocalTexture(texName))
        {
            auto texturePath = material->GetLocalTexture(texName)->GetPathname();
            if (!texturePath.IsEmpty())
            {
                DAVA::String textureRelativePath = texturePath.GetRelativePathname(context.GetScenePath());
                if (textureRelativePath.size() > 0)
                {
                    content->SetString(texName.c_str(), textureRelativePath);
                }
            }
        }
    }
}

void StoreMaterialFlags(DAVA::NMaterial* material, const DAVA::InspMember* materialMember, DAVA::KeyedArchive* content)
{
    DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);
    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata);
    for (const auto& flagName : membersList)
    {
        if (material->HasLocalFlag(flagName))
        {
            content->SetInt32(flagName.c_str(), material->GetLocalFlagValue(flagName));
        }
    }
}

void StoreMaterialProperties(DAVA::NMaterial* material, const DAVA::InspMember* materialMember, DAVA::KeyedArchive* content)
{
    DAVA::InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    DAVA::InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);
    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(ddata);
    for (const auto& propertyName : membersList)
    {
        if (material->HasLocalProperty(propertyName))
        {
            rhi::ShaderProp::Type propertyType = material->GetLocalPropType(propertyName);
            const DAVA::float32* propertyValue = material->GetLocalPropValue(propertyName);
            DAVA::uint32 arraySize = material->GetLocalPropArraySize(propertyName);
            DAVA::uint32 dataSize = static_cast<DAVA::uint32>(sizeof(DAVA::float32) * DAVA::ShaderDescriptor::CalculateDataSize(propertyType, 1));

            DAVA::ScopedPtr<DAVA::KeyedArchive> prop(new DAVA::KeyedArchive());
            prop->SetUInt32("type", static_cast<DAVA::uint32>(propertyType));
            prop->SetUInt32("size", arraySize);
            prop->SetByteArray("data", reinterpret_cast<const DAVA::uint8*>(propertyValue), dataSize);
            content->SetArchive(propertyName.c_str(), prop);
        }
    }
}

void UpdateMaterialPropertiesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* content)
{
    const DAVA::KeyedArchive::UnderlyingMap& properties = content->GetArchieveData();
    for (const auto& pm : properties)
    {
        DVASSERT(DAVA::VariantType::TYPE_KEYED_ARCHIVE == pm.second->type);

        DAVA::FastName propName(pm.first);
        DAVA::KeyedArchive* propertyArchive = pm.second->AsKeyedArchive();

        /*
        * Here we are checking if propData if valid, because yaml parser can
        * completely delete (skip) byte array node if it contains invalid data
        */
        const DAVA::float32* propData = reinterpret_cast<const DAVA::float32*>(propertyArchive->GetByteArray("data"));
        if (nullptr != propData)
        {
            rhi::ShaderProp::Type propType = static_cast<rhi::ShaderProp::Type>(propertyArchive->GetUInt32("type"));
            DAVA::uint32 propSize = propertyArchive->GetUInt32("size");

            if (material->HasLocalProperty(propName))
            {
                auto existingType = material->GetLocalPropType(propName);
                auto existingSize = material->GetLocalPropArraySize(propName);
                if ((existingType == propType) && (existingSize == propSize))
                {
                    material->SetPropertyValue(propName, propData);
                }
            }
            else
            {
                material->AddProperty(propName, propData, propType, propSize);
            }
        }
    }
}

void UpdateMaterialFlagsFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* content)
{
    const auto flags = content->GetArchieveData();
    for (const auto& fm : flags)
    {
        if (material->HasLocalFlag(DAVA::FastName(fm.first)))
            material->SetFlag(DAVA::FastName(fm.first), fm.second->AsInt32());
        else
            material->AddFlag(DAVA::FastName(fm.first), fm.second->AsInt32());
    }
}

void UpdateMaterialTexturesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* content, const DAVA::FilePath& scenePath)
{
    const auto& texturesMap = content->GetArchieveData();
    for (const auto& tm : texturesMap)
    {
        auto texture = DAVA::Texture::CreateFromFile(scenePath + tm.second->AsString());

        DAVA::FastName textureName(tm.first);
        if (material->HasLocalTexture(textureName))
        {
            material->SetTexture(textureName, texture);
        }
        else
        {
            material->AddTexture(textureName, texture);
        }
    }
}
}

ApplyMaterialPresetCommand::ApplyMaterialPresetCommand(const DAVA::FilePath& presetPath, DAVA::NMaterial* material_, DAVA::Scene* scene_)
    : RECommand(CMDID_MATERIAL_APPLY_PRESET, "Apply material preset")
    , redoInfo(new DAVA::KeyedArchive())
    , undoInfo(new DAVA::KeyedArchive())
    , material(DAVA::RefPtr<DAVA::NMaterial>::ConstructWithRetain(material_))
    , scene(scene_)
{
    redoInfo->LoadFromYamlFile(presetPath);
}

bool ApplyMaterialPresetCommand::IsValidPreset() const
{
    return redoInfo->IsKeyExists(ApplyMaterialPresetDetail::contentNodeName);
}

void ApplyMaterialPresetCommand::Init(DAVA::uint32 materialParts_)
{
    materialParts = materialParts_;
    DAVA::SerializationContext context;
    PrepareSerializationContext(context);
    StoreMaterialPreset(undoInfo.get(), material.Get(), context);

    DVASSERT(undoInfo->IsKeyExists(ApplyMaterialPresetDetail::contentNodeName));
}

void ApplyMaterialPresetCommand::Undo()
{
    HashMap<DAVA::FastName, DAVA::MaterialTextureInfo*> textures = material->GetLocalTextures();
    for (const auto& info : textures)
    {
        material->RemoveTexture(info.first);
    }
    LoadMaterialPreset(undoInfo.get(), ALL);
}

bool ApplyMaterialPresetCommand::IsClean() const
{
    return false;
}

void ApplyMaterialPresetCommand::Redo()
{
    LoadMaterialPreset(redoInfo.get(), materialParts);
}

void ApplyMaterialPresetCommand::StoreMaterialPreset(DAVA::KeyedArchive* preset, DAVA::NMaterial* material, const DAVA::SerializationContext& context)
{
    using namespace ApplyMaterialPresetDetail;

    DAVA::ScopedPtr<DAVA::KeyedArchive> content(new DAVA::KeyedArchive());
    const DAVA::InspInfo* info = material->GetTypeInfo();

    DAVA::ScopedPtr<DAVA::KeyedArchive> texturesArchive(new DAVA::KeyedArchive());
    DAVA::ScopedPtr<DAVA::KeyedArchive> flagsArchive(new DAVA::KeyedArchive());
    DAVA::ScopedPtr<DAVA::KeyedArchive> propertiesArchive(new DAVA::KeyedArchive());

    const DAVA::InspMember* materialMember = info->Member(DAVA::FastName("localTextures"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        StoreMaterialTextures(material, materialMember, texturesArchive, context);

    materialMember = info->Member(DAVA::FastName("localFlags"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        StoreMaterialFlags(material, materialMember, flagsArchive);

    materialMember = info->Member(DAVA::FastName("localProperties"));
    if ((nullptr != materialMember) && (nullptr != materialMember->Dynamic()))
        StoreMaterialProperties(material, materialMember, propertiesArchive);

    content->SetArchive("flags", flagsArchive);
    content->SetArchive("textures", texturesArchive);
    content->SetArchive("properties", propertiesArchive);

    auto fxName = material->GetLocalFXName();
    if (fxName.IsValid())
        content->SetFastName("fxname", fxName);

    auto qualityGroup = material->GetQualityGroup();
    if (qualityGroup.IsValid())
        content->SetFastName("group", qualityGroup);

    preset->SetUInt32("serializationContextVersion", context.GetVersion());
    preset->SetArchive("content", content);
}

void ApplyMaterialPresetCommand::LoadMaterialPreset(DAVA::KeyedArchive* archive, DAVA::uint32 parts)
{
    DAVA::SerializationContext context;
    PrepareSerializationContext(context);
    LoadMaterialPreset(archive, material.Get(), context, parts);
    context.ResolveMaterialBindings();
}

void ApplyMaterialPresetCommand::LoadMaterialPreset(DAVA::KeyedArchive* archive, DAVA::NMaterial* material, const DAVA::SerializationContext& context, DAVA::uint32 parts)
{
    using namespace ApplyMaterialPresetDetail;

    DAVA::KeyedArchive* preset = archive->GetArchive(ApplyMaterialPresetDetail::contentNodeName);

    if ((parts == ALL || (materialParts & GROUP)) && preset->IsKeyExists("group"))
    {
        material->SetQualityGroup(preset->GetFastName("group"));
    }

    if ((parts == ALL || (materialParts & TEMPLATE)) && preset->IsKeyExists("fxname"))
    {
        material->SetFXName(preset->GetFastName("fxname"));
    }

    if ((parts == ALL || (materialParts & PROPERTIES)) && preset->IsKeyExists("flags"))
    {
        UpdateMaterialFlagsFromPreset(material, preset->GetArchive("flags"));
    }

    if ((parts == ALL || (materialParts & PROPERTIES)) && preset->IsKeyExists("properties"))
    {
        UpdateMaterialPropertiesFromPreset(material, preset->GetArchive("properties"));
    }

    if ((parts == ALL || (materialParts & TEXTURES)) && preset->IsKeyExists("textures"))
    {
        UpdateMaterialTexturesFromPreset(material, preset->GetArchive("textures"), context.GetScenePath());
    }
}

void ApplyMaterialPresetCommand::PrepareSerializationContext(DAVA::SerializationContext& context)
{
    context.SetScene(scene);
    context.SetScenePath(ProjectManager::Instance()->GetProjectPath());
    context.SetVersion(DAVA::VersionInfo::Instance()->GetCurrentVersion().version);
}
