#include "Classes/Collada/ImportParams.h"

#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/CustomPropertiesComponent.h>
#include <Scene3D/Lod/LodComponent.h>
#include <Utils/StringFormat.h>
#include <Render/Shader.h>
#include <Render/Material/NMaterialNames.h>

namespace DAEConverter
{
namespace ImportParamsDetail
{
void SaveConfigToArchive(const DAVA::FilePath& sceneDirPath, const DAVA::MaterialConfig& config, DAVA::RefPtr<DAVA::KeyedArchive> archive)
{
    using namespace DAVA;

    if (config.fxName.IsValid())
    {
        archive->SetString(NMaterialSerializationKey::FXName, config.fxName.c_str());
    }

    if (config.name.IsValid())
    {
        archive->SetString(NMaterialSerializationKey::ConfigName, config.name.c_str());
    }

    ScopedPtr<KeyedArchive> propertiesArchive(new KeyedArchive());
    for (HashMap<FastName, NMaterialProperty *>::iterator it = config.localProperties.begin(), itEnd = config.localProperties.end(); it != itEnd; ++it)
    {
        NMaterialProperty* property = it->second;

        uint32 dataSize = ShaderDescriptor::CalculateDataSize(property->type, property->arraySize) * sizeof(float32);
        uint32 storageSize = sizeof(uint8) + sizeof(uint32) + dataSize;
        uint8* propertyStorage = new uint8[storageSize];

        memcpy(propertyStorage, &property->type, sizeof(uint8));
        memcpy(propertyStorage + sizeof(uint8), &property->arraySize, sizeof(uint32));
        memcpy(propertyStorage + sizeof(uint8) + sizeof(uint32), property->data.get(), dataSize);

        propertiesArchive->SetByteArray(it->first.c_str(), propertyStorage, storageSize);

        SafeDeleteArray(propertyStorage);
    }
    archive->SetArchive("properties", propertiesArchive);

    ScopedPtr<KeyedArchive> texturesArchive(new KeyedArchive());
    for (auto it = config.localTextures.begin(), itEnd = config.localTextures.end(); it != itEnd; ++it)
    {
        if (!NMaterialTextureName::IsRuntimeTexture(it->first) && !it->second->path.IsEmpty())
        {
            String textureRelativePath = it->second->path.GetRelativePathname(sceneDirPath);
            if (textureRelativePath.size() > 0)
            {
                texturesArchive->SetString(it->first.c_str(), textureRelativePath);
            }
        }
    }
    archive->SetArchive("textures", texturesArchive);

    ScopedPtr<KeyedArchive> flagsArchive(new KeyedArchive());
    for (HashMap<FastName, int32>::iterator it = config.localFlags.begin(), itEnd = config.localFlags.end(); it != itEnd; ++it)
    {
        if (!NMaterialFlagName::IsRuntimeFlag(it->first))
            flagsArchive->SetInt32(it->first.c_str(), it->second);
    }
    archive->SetArchive("flags", flagsArchive);
}

void SaveMaterial(const DAVA::FilePath& sceneDirPath, DAVA::NMaterial* material, DAVA::RefPtr<DAVA::KeyedArchive> archive)
{
    using namespace DAVA;

    const FastName& qualityGroup = material->GetQualityGroup();
    if (qualityGroup.IsValid())
        archive->SetString(NMaterialSerializationKey::QualityGroup, qualityGroup.c_str());

    uint32 configsCount = material->GetConfigCount();
    archive->SetUInt32(NMaterialSerializationKey::ConfigCount, configsCount);
    for (uint32 i = 0; i < configsCount; ++i)
    {
        DAVA::RefPtr<KeyedArchive> configArchive(new KeyedArchive());
        const DAVA::MaterialConfig& config = material->GetConfig(i);
        SaveConfigToArchive(sceneDirPath, config, configArchive);
        archive->SetArchive(Format(NMaterialSerializationKey::ConfigArchive.c_str(), i), configArchive.Get());
    }
}

void LoadConfigFromArchive(const DAVA::FilePath& sceneDirPath, DAVA::MaterialConfig& config, DAVA::RefPtr<DAVA::KeyedArchive> archive)
{
    using namespace DAVA;
    if (archive == nullptr)
    {
        DVASSERT(false);
        return;
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::FXName))
    {
        config.fxName = FastName(archive->GetString(NMaterialSerializationKey::FXName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::ConfigName))
    {
        config.name = FastName(archive->GetString(NMaterialSerializationKey::ConfigName));
    }

    if (archive->IsKeyExists("properties"))
    {
        const KeyedArchive::UnderlyingMap& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint8) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();
            FastName propName = FastName(it->first);
            uint8 propType = *ptr;
            ptr += sizeof(uint8);
            uint32 propSize = *(reinterpret_cast<const uint32*>(ptr));
            ptr += sizeof(uint32);
            const float32* data = reinterpret_cast<const float32*>(ptr);

            NMaterialProperty* newProp = new NMaterialProperty();
            newProp->name = propName;
            newProp->type = rhi::ShaderProp::Type(propType);
            newProp->arraySize = propSize;
            newProp->data.reset(new float32[ShaderDescriptor::CalculateDataSize(newProp->type, newProp->arraySize)]);
            newProp->SetPropertyValue(data);
            config.localProperties[newProp->name] = newProp;
        }
    }

    if (archive->IsKeyExists("textures"))
    {
        const KeyedArchive::UnderlyingMap& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = texturesMap.begin(); it != texturesMap.end(); ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo* texInfo = new MaterialTextureInfo();
            texInfo->path = sceneDirPath + relativePathname;
            config.localTextures[FastName(it->first)] = texInfo;
        }
    }

    if (archive->IsKeyExists("flags"))
    {
        const KeyedArchive::UnderlyingMap& flagsMap = archive->GetArchive("flags")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
            config.localFlags[FastName(it->first)] = it->second->AsInt32();
        }
    }
}

void LoadMaterial(const DAVA::FilePath& sceneDirPath, DAVA::NMaterial* material, DAVA::RefPtr<DAVA::KeyedArchive> archive)
{
    using namespace DAVA;
    while (material->GetConfigCount() > 1)
    {
        material->RemoveConfig(material->GetConfigCount() - 1);
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::QualityGroup))
    {
        material->SetQualityGroup(FastName(archive->GetString(NMaterialSerializationKey::QualityGroup).c_str()));
    }

    uint32 configCount = archive->GetUInt32(NMaterialSerializationKey::ConfigCount, 1);
    Vector<MaterialConfig> configs(configCount, MaterialConfig());

    for (uint32 i = 0; i < configCount; ++i)
    {
        RefPtr<KeyedArchive> configArchive = RefPtr<KeyedArchive>::ConstructWithRetain(archive->GetArchive(Format(NMaterialSerializationKey::ConfigArchive.c_str(), i)));
        LoadConfigFromArchive(sceneDirPath, configs[i], configArchive);
    }

    for (size_t i = 0; i < configs.size(); ++i)
    {
        material->InsertConfig(material->GetConfigCount(), configs[i]);
    }
    material->RemoveConfig(0);
}

void SaveComponent(DAVA::RefPtr<DAVA::Entity> entity, DAVA::Map<DAVA::FastName, DAVA::Component*>& componentMap, const DAVA::Function<DAVA::Component*(DAVA::Entity*)>& getComponentFn)
{
    DAVA::Component* component = getComponentFn(entity.Get());
    if (component != nullptr)
    {
        componentMap.emplace(entity->GetName(), component->Clone(nullptr));
    }
}

void RestoreComponent(DAVA::RefPtr<DAVA::Entity> entity, DAVA::Map<DAVA::FastName, DAVA::Component*>& componentMap, const DAVA::Function<DAVA::Component*(DAVA::Entity*)>& getComponentFn)
{
    const DAVA::FastName& entityName = entity->GetName();
    auto iter = componentMap.find(entityName);
    if (iter != componentMap.end())
    {
        DAVA::Component* component = getComponentFn(entity.Get());
        if (component != nullptr)
        {
            entity->RemoveComponent(component);
        }

        entity->AddComponent(iter->second);
        componentMap.erase(iter);
    }
}

void AccumulateImportParamsImpl(const DAVA::RefPtr<DAVA::Entity>& entity, const DAVA::FilePath& sceneDirPath, ImportParams* params)
{
    using namespace DAVA;

    SaveComponent(entity, params->customPropertiesMap, [](DAVA::Entity* entity) -> DAVA::Component*
                  {
                      return GetCustomProperties(entity);
                  });

    SaveComponent(entity, params->lodComponentsMap, [](DAVA::Entity* entity) -> DAVA::Component*
                  {
                      return GetLodComponent(entity);
                  });

    RenderComponent* renderComponent = GetRenderComponent(entity.Get());
    if (renderComponent != nullptr)
    {
        RenderObject* renderObject = renderComponent->GetRenderObject();
        for (uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
        {
            RenderBatch* batch = renderObject->GetRenderBatch(i);
            NMaterial* material = batch->GetMaterial();
            while (material != nullptr)
            {
                auto materialIter = params->materialsMap.find(material->GetMaterialName());
                if (materialIter == params->materialsMap.end())
                {
                    RefPtr<KeyedArchive> parentMaterialArchive;
                    parentMaterialArchive.ConstructInplace();
                    SaveMaterial(sceneDirPath, material, parentMaterialArchive);
                    params->materialsMap.emplace(material->GetMaterialName(), parentMaterialArchive);
                }
                material = material->GetParent();
            }
        }
    }

    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        RefPtr<Entity> child = RefPtr<Entity>::ConstructWithRetain(entity->GetChild(i));
        AccumulateImportParamsImpl(child, sceneDirPath, params);
    }
}

void RestoreSceneParamsImpl(const DAVA::RefPtr<DAVA::Entity>& entity, const DAVA::FilePath& sceneDirPath, ImportParams* params, DAVA::Set<DAVA::FastName>& loadedMaterials)
{
    using namespace DAVA;

    const FastName& entityName = entity->GetName();

    // Restore Custom Properties
    RestoreComponent(entity, params->customPropertiesMap, [](DAVA::Entity* entity) -> DAVA::Component*
                     {
                         return GetCustomProperties(entity);
                     });

    RestoreComponent(entity, params->lodComponentsMap, [](DAVA::Entity* entity) -> DAVA::Component*
                     {
                         return GetLodComponent(entity);
                     });

    RenderComponent* renderComponent = GetRenderComponent(entity.Get());
    if (renderComponent != nullptr)
    {
        RenderObject* renderObject = renderComponent->GetRenderObject();
        for (uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
        {
            RenderBatch* batch = renderObject->GetRenderBatch(i);
            NMaterial* material = batch->GetMaterial();

            while (material != nullptr)
            {
                FastName materialName = material->GetMaterialName();
                if (loadedMaterials.count(materialName) == 0)
                {
                    auto materialIter = params->materialsMap.find(materialName);
                    if (materialIter != params->materialsMap.end())
                    {
                        LoadMaterial(sceneDirPath, material, materialIter->second);
                    }
                    loadedMaterials.insert(materialName);
                }
                material = material->GetParent();
            }
        }
    }

    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        RefPtr<Entity> child = RefPtr<Entity>::ConstructWithRetain(entity->GetChild(i));
        RestoreSceneParamsImpl(child, sceneDirPath, params, loadedMaterials);
    }
}
}

ImportParams::~ImportParams()
{
    for (auto iter : customPropertiesMap)
    {
        DAVA::SafeDelete(iter.second);
    }

    for (auto iter : lodComponentsMap)
    {
        DAVA::SafeDelete(iter.second);
    }

    materialsMap.clear();
}

void AccumulateImportParams(DAVA::RefPtr<DAVA::Scene> scene, const DAVA::FilePath& scenePath, ImportParams* params)
{
    ImportParamsDetail::AccumulateImportParamsImpl(scene, scenePath.GetDirectory(), params);
}

void RestoreSceneParams(DAVA::RefPtr<DAVA::Scene> scene, const DAVA::FilePath& scenePath, ImportParams* params)
{
    DAVA::Set<DAVA::FastName> loadedMaterials;
    ImportParamsDetail::RestoreSceneParamsImpl(scene, scenePath.GetDirectory(), params, loadedMaterials);
}

} // namespace DAEConverter