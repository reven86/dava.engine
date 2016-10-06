#include "ApplyMaterialPresetCommand.h"

#include "Commands2/RECommandIDs.h"
#include "Classes/Qt/Project/ProjectManager.h"

#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Render/Material/NMaterial.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FilePath.h"

namespace ApplyMaterialPresetDetail
{
using namespace DAVA;
const String contentNodeName("content");

template <typename T>
void ClearContent(const Function<const HashMap<FastName, T>&()>& getContentFn, const Function<void(const FastName&)>& removeFn)
{
    // We have to clear local content before applying undo snapshot, because applying will not delete fields
    // Snapshot contains only info about "exists fields", but not containfo about "not exists fields"
    HashMap<FastName, T> info = getContentFn();
    for (const auto& item : info)
    {
        removeFn(item.first);
    }
}

template <typename T>
void AccumulateLocalInfo(const HashMap<FastName, T>& info, Vector<FastName>& members)
{
    // When we create snapshot for undo, we should store all local info, to restore it exactly to same state
    members.reserve(info.size());
    for (const auto& item : info)
    {
        members.push_back(item.first);
    }
}

void AccumulateInspInfo(NMaterial* material, const InspMember* materialMember, Vector<FastName>& members)
{
    if (materialMember == nullptr || materialMember->Dynamic() == nullptr)
        return;

    // When we store preset on disk, we need only valid fields for current material template
    InspInfoDynamic* dynamicInfo = materialMember->Dynamic()->GetDynamicInfo();
    InspInfoDynamic::DynamicData ddata = dynamicInfo->Prepare(material, false);
    members = dynamicInfo->MembersList(ddata);
}

void StoreMaterialTextures(NMaterial* material, KeyedArchive* content, const SerializationContext& context, bool storeForUndo)
{
    Vector<FastName> membersList;
    if (storeForUndo)
    {
        AccumulateLocalInfo(material->GetLocalTextures(), membersList);
    }
    else
    {
        AccumulateInspInfo(material, material->GetTypeInfo()->Member(DAVA::FastName("localTextures")), membersList);
    }

    for (const auto& texName : membersList)
    {
        if (material->HasLocalTexture(texName))
        {
            const FilePath& texturePath = material->GetLocalTexture(texName)->GetPathname();
            if (!texturePath.IsEmpty())
            {
                String textureRelativePath = texturePath.GetRelativePathname(context.GetScenePath());
                if (textureRelativePath.size() > 0)
                {
                    content->SetString(texName.c_str(), textureRelativePath);
                }
            }
        }
    }
}

void StoreMaterialFlags(NMaterial* material, KeyedArchive* content, bool storeForUndo)
{
    Vector<FastName> membersList;
    if (storeForUndo)
    {
        AccumulateLocalInfo(material->GetLocalFlags(), membersList);
    }
    else
    {
        AccumulateInspInfo(material, material->GetTypeInfo()->Member(FastName("localFlags")), membersList);
    }

    for (const auto& flagName : membersList)
    {
        if (material->HasLocalFlag(flagName))
        {
            content->SetInt32(flagName.c_str(), material->GetLocalFlagValue(flagName));
        }
    }
}

void StoreMaterialProperties(NMaterial* material, KeyedArchive* content, bool storeForUndo)
{
    Vector<FastName> membersList;
    if (storeForUndo)
    {
        AccumulateLocalInfo(material->GetLocalProperties(), membersList);
    }
    else
    {
        AccumulateInspInfo(material, material->GetTypeInfo()->Member(FastName("localProperties")), membersList);
    }

    for (const auto& propertyName : membersList)
    {
        if (material->HasLocalProperty(propertyName))
        {
            rhi::ShaderProp::Type propertyType = material->GetLocalPropType(propertyName);
            const float32* propertyValue = material->GetLocalPropValue(propertyName);
            uint32 arraySize = material->GetLocalPropArraySize(propertyName);
            uint32 dataSize = static_cast<uint32>(sizeof(float32) * ShaderDescriptor::CalculateDataSize(propertyType, 1));

            ScopedPtr<KeyedArchive> prop(new KeyedArchive());
            prop->SetUInt32("type", static_cast<uint32>(propertyType));
            prop->SetUInt32("size", arraySize);
            prop->SetByteArray("data", reinterpret_cast<const uint8*>(propertyValue), dataSize);
            content->SetArchive(propertyName.c_str(), prop);
        }
    }
}

void UpdateMaterialPropertiesFromPreset(NMaterial* material, KeyedArchive* content, bool updateForUndo)
{
    if (updateForUndo)
    {
        ApplyMaterialPresetDetail::ClearContent(MakeFunction(material, &NMaterial::GetLocalProperties), MakeFunction(material, &NMaterial::RemoveProperty));
    }

    const KeyedArchive::UnderlyingMap& properties = content->GetArchieveData();
    for (const auto& pm : properties)
    {
        DVASSERT(VariantType::TYPE_KEYED_ARCHIVE == pm.second->type);

        FastName propName(pm.first);
        KeyedArchive* propertyArchive = pm.second->AsKeyedArchive();

        /*
        * Here we are checking if propData if valid, because yaml parser can
        * completely delete (skip) byte array node if it contains invalid data
        */
        const float32* propData = reinterpret_cast<const float32*>(propertyArchive->GetByteArray("data"));
        if (nullptr != propData)
        {
            rhi::ShaderProp::Type propType = static_cast<rhi::ShaderProp::Type>(propertyArchive->GetUInt32("type"));
            uint32 propSize = propertyArchive->GetUInt32("size");

            if (material->HasLocalProperty(propName))
            {
                rhi::ShaderProp::Type existingType = material->GetLocalPropType(propName);
                DAVA::uint32 existingSize = material->GetLocalPropArraySize(propName);
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

void UpdateMaterialFlagsFromPreset(NMaterial* material, KeyedArchive* content, bool updateForUndo)
{
    if (updateForUndo)
    {
        ApplyMaterialPresetDetail::ClearContent(MakeFunction(material, &NMaterial::GetLocalFlags), MakeFunction(material, &NMaterial::RemoveFlag));
    }

    const auto flags = content->GetArchieveData();
    for (const auto& fm : flags)
    {
        if (material->HasLocalFlag(FastName(fm.first)))
            material->SetFlag(FastName(fm.first), fm.second->AsInt32());
        else
            material->AddFlag(FastName(fm.first), fm.second->AsInt32());
    }
}

void UpdateMaterialTexturesFromPreset(NMaterial* material, KeyedArchive* content, const FilePath& scenePath, bool updateForUndo)
{
    if (updateForUndo)
    {
        ApplyMaterialPresetDetail::ClearContent(MakeFunction(material, &NMaterial::GetLocalTextures), MakeFunction(material, &NMaterial::RemoveTexture));
    }

    const auto& texturesMap = content->GetArchieveData();
    for (const auto& tm : texturesMap)
    {
        ScopedPtr<Texture> texture(Texture::CreateFromFile(scenePath + tm.second->AsString()));

        FastName textureName(tm.first);
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
    StoreMaterialPresetImpl(undoInfo.get(), material.Get(), context, true);

    DVASSERT(undoInfo->IsKeyExists(ApplyMaterialPresetDetail::contentNodeName));
}

void ApplyMaterialPresetCommand::Undo()
{
    LoadMaterialPreset(undoInfo.get(), ALL, true);
}

bool ApplyMaterialPresetCommand::IsClean() const
{
    return false;
}

void ApplyMaterialPresetCommand::Redo()
{
    LoadMaterialPreset(redoInfo.get(), materialParts, false);
}

void ApplyMaterialPresetCommand::StoreMaterialPreset(DAVA::KeyedArchive* preset, DAVA::NMaterial* material, const DAVA::SerializationContext& context)
{
    StoreMaterialPresetImpl(preset, material, context, false);
}

void ApplyMaterialPresetCommand::LoadMaterialPreset(DAVA::KeyedArchive* archive, DAVA::uint32 parts, bool loadForUndo)
{
    DAVA::SerializationContext context;
    PrepareSerializationContext(context);
    LoadMaterialPreset(archive, material.Get(), context, parts, loadForUndo);
    context.ResolveMaterialBindings();
}

void ApplyMaterialPresetCommand::LoadMaterialPreset(DAVA::KeyedArchive* archive, DAVA::NMaterial* material, const DAVA::SerializationContext& context, DAVA::uint32 parts, bool loadForUndo)
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
        UpdateMaterialFlagsFromPreset(material, preset->GetArchive("flags"), loadForUndo);
    }

    if ((parts == ALL || (materialParts & PROPERTIES)) && preset->IsKeyExists("properties"))
    {
        UpdateMaterialPropertiesFromPreset(material, preset->GetArchive("properties"), loadForUndo);
    }

    if ((parts == ALL || (materialParts & TEXTURES)) && preset->IsKeyExists("textures"))
    {
        UpdateMaterialTexturesFromPreset(material, preset->GetArchive("textures"), context.GetScenePath(), loadForUndo);
    }
}

void ApplyMaterialPresetCommand::StoreMaterialPresetImpl(DAVA::KeyedArchive* archive, DAVA::NMaterial* material, const DAVA::SerializationContext& context, bool storeForUndo)
{
    using namespace ApplyMaterialPresetDetail;

    DAVA::ScopedPtr<DAVA::KeyedArchive> content(new DAVA::KeyedArchive());
    const DAVA::InspInfo* info = material->GetTypeInfo();

    DAVA::ScopedPtr<DAVA::KeyedArchive> texturesArchive(new DAVA::KeyedArchive());
    DAVA::ScopedPtr<DAVA::KeyedArchive> flagsArchive(new DAVA::KeyedArchive());
    DAVA::ScopedPtr<DAVA::KeyedArchive> propertiesArchive(new DAVA::KeyedArchive());

    StoreMaterialTextures(material, texturesArchive, context, storeForUndo);
    StoreMaterialFlags(material, flagsArchive, storeForUndo);
    StoreMaterialProperties(material, propertiesArchive, storeForUndo);

    content->SetArchive("flags", flagsArchive);
    content->SetArchive("textures", texturesArchive);
    content->SetArchive("properties", propertiesArchive);

    const FastName& fxName = material->GetLocalFXName();
    if (fxName.IsValid())
        content->SetFastName("fxname", fxName);

    const FastName& qualityGroup = material->GetQualityGroup();
    if (qualityGroup.IsValid())
        content->SetFastName("group", qualityGroup);

    archive->SetUInt32("serializationContextVersion", context.GetVersion());
    archive->SetArchive("content", content);
}

void ApplyMaterialPresetCommand::PrepareSerializationContext(DAVA::SerializationContext& context)
{
    context.SetScene(scene);
    context.SetScenePath(ProjectManager::Instance()->GetProjectPath());
    context.SetVersion(DAVA::VersionInfo::Instance()->GetCurrentVersion().version);
}
