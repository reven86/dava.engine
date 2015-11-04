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


#include "CreatePlaneLODCommand.h"
#include "Qt/Scene/SceneHelper.h"
#include "Qt/Settings/SettingsManager.h"
#include "Classes/CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "Scene/SceneHelper.h"

#include "Render/Material/NMaterialNames.h"

using namespace DAVA;

CreatePlaneLODCommand::CreatePlaneLODCommand(const CreatePlaneLODCommandHelper::RequestPointer& request_)
    : Command2(CMDID_LOD_CREATE_PLANE, "Create Plane LOD")
    , request(request_)
{
    DVASSERT(GetRenderObject(GetEntity()));
}

void CreatePlaneLODCommand::Redo()
{
    CreateTextureFiles();
    request->planeBatch->GetMaterial()->GetEffectiveTexture(NMaterialTextureName::TEXTURE_ALBEDO)->Reload();

    auto entity = GetEntity();
    auto renderObject = DAVA::GetRenderObject(entity);
    float lodDistance = 2.0f * request->lodComponent->GetLodLayerDistance(request->newLodIndex - 1);
    renderObject->AddRenderBatch(request->planeBatch, request->newLodIndex, -1);
    request->lodComponent->SetLodLayerDistance(request->newLodIndex, lodDistance);
}

void CreatePlaneLODCommand::Undo()
{
    DAVA::RenderObject* ro = DAVA::GetRenderObject(GetEntity());

    //restore batches
    ro->RemoveRenderBatch(request->planeBatch);

    //restore distances
    request->lodComponent->lodLayersArray = request->savedDistances;

    // fix visibility settings
    DAVA::int32 maxLodIndex = ro->GetMaxLodIndex();
    if (request->lodComponent->forceLodLayer > maxLodIndex)
        request->lodComponent->forceLodLayer = maxLodIndex;

    request->lodComponent->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
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
    ImageSystem::Instance()->Save(request->texturePath, request->planeImage);
    TextureDescriptorUtils::CreateDescriptorIfNeed(request->texturePath);
}

void CreatePlaneLODCommand::DeleteTextureFiles()
{
    FileSystem::Instance()->DeleteFile(request->texturePath);
    FileSystem::Instance()->DeleteFile(TextureDescriptor::GetDescriptorPathname(request->texturePath));
}

DAVA::RenderBatch * CreatePlaneLODCommand::GetRenderBatch() const
{
    return request->planeBatch;
}
