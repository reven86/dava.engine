#include "CreatePlaneLODCommand.h"

#include "Render/Material/NMaterialNames.h"
#include "Scene3D/Lod/LodComponent.h"

#include "Commands2/RECommandIDs.h"
#include "Scene/SceneHelper.h"
#include "Settings/SettingsManager.h"
#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"

using namespace DAVA;

CreatePlaneLODCommand::CreatePlaneLODCommand(const CreatePlaneLODCommandHelper::RequestPointer& request_)
    : RECommand(CMDID_LOD_CREATE_PLANE, "Create Plane LOD")
    , request(request_)
{
    DVASSERT(GetRenderObject(GetEntity()));
}

void CreatePlaneLODCommand::Redo()
{
    CreateTextureFiles();

    ScopedPtr<Texture> fileTexture(Texture::CreateFromFile(request->texturePath));
    NMaterial* material = request->planeBatch->GetMaterial();
    if (material != nullptr)
    {
        if (material->HasLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO))
        {
            material->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, fileTexture);
        }
        else
        {
            material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, fileTexture);
        }
        fileTexture->Reload();
    }

    auto entity = GetEntity();
    auto renderObject = DAVA::GetRenderObject(entity);
    renderObject->AddRenderBatch(request->planeBatch, request->newLodIndex, -1);
}

void CreatePlaneLODCommand::Undo()
{
    DAVA::RenderObject* ro = DAVA::GetRenderObject(GetEntity());

    //restore batches
    ro->RemoveRenderBatch(request->planeBatch);

    DeleteTextureFiles();
}

DAVA::Entity* CreatePlaneLODCommand::GetEntity() const
{
    return request->lodComponent->GetEntity();
}

void CreatePlaneLODCommand::CreateTextureFiles()
{
    DVASSERT(request->planeImage);

    FilePath folder = request->texturePath.GetDirectory();
    FileSystem::Instance()->CreateDirectory(folder, true);
    ImageSystem::Save(request->texturePath, request->planeImage);
    TextureDescriptorUtils::CreateOrUpdateDescriptor(request->texturePath);
}

void CreatePlaneLODCommand::DeleteTextureFiles()
{
    FileSystem::Instance()->DeleteFile(request->texturePath);
    FileSystem::Instance()->DeleteFile(TextureDescriptor::GetDescriptorPathname(request->texturePath));
}

DAVA::RenderBatch* CreatePlaneLODCommand::GetRenderBatch() const
{
    return request->planeBatch;
}
