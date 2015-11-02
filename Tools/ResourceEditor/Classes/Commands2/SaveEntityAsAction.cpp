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


#include "SaveEntityAsAction.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material/NMaterial.h"

#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Classes/StringConstants.h"


using namespace DAVA;

SaveEntityAsAction::SaveEntityAsAction(const EntityGroup *_entities, const FilePath &_path)
	: CommandAction(CMDID_ENTITY_SAVE_AS, "Save Entities As")
	, entities(_entities)
	, sc2Path(_path)
{ }

SaveEntityAsAction::~SaveEntityAsAction()
{ }

void SaveEntityAsAction::Redo()
{
	uint32 count = static_cast<uint32>(entities->Size());
    if (!sc2Path.IsEmpty() && sc2Path.IsEqualToExtension(".sc2") && (nullptr != entities) && (count > 0))
    {
        const auto RemoveReferenceToOwner = [](Entity* entity) {
			KeyedArchive *props = GetCustomPropertiesArchieve(entity);
			if(nullptr != props)
			{
				props->DeleteKey(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
			}
        };

        ScopedPtr<Scene> scene(new Scene());
        ScopedPtr<Entity> container(nullptr);

        if (count == 1) // saving of single object
        {
            container.reset(entities->GetEntity(0)->Clone());
            RemoveReferenceToOwner(container);
            container->SetLocalTransform(Matrix4::IDENTITY);
        }
		else // saving of group of objects
		{
            container.reset(new Entity());

            const Vector3 oldZero = entities->GetCommonZeroPos();
            for (uint32 i = 0; i < count; ++i)
            {
				ScopedPtr<Entity> clone(entities->GetEntity(i)->Clone());

				const Vector3 offset = clone->GetLocalTransform().GetTranslationVector() - oldZero;
				Matrix4 newLocalTransform = clone->GetLocalTransform();
				newLocalTransform.SetTranslationVector(offset);
				clone->SetLocalTransform(newLocalTransform);

				container->AddNode(clone);
				RemoveReferenceToOwner(clone);
			}

			container->SetName(sc2Path.GetFilename().c_str());
		}
        DVASSERT(container);

        //Remove global material from parent materials
        Scene* sourceScene = entities->GetEntity(0)->GetScene();
        Set<NMaterial*> parentsMaterials;
        NMaterial* sourceGlobalMaterial = (sourceScene != nullptr) ? sourceScene->GetGlobalMaterial() : nullptr;
        if (sourceGlobalMaterial)
        {
            List<NMaterial*> newMaterials;
            container->GetDataNodes(newMaterials);

            for (auto& mat : newMaterials)
            {
                if (mat->GetParent() == sourceGlobalMaterial)
                {
                    //reset global material inheritance
                    mat->SetParent(nullptr);
                    parentsMaterials.insert(mat);
                }
            }
        }

        scene->AddNode(container); //1. Added new items in zero position with identity matrix
        scene->staticOcclusionSystem->InvalidateOcclusion(); //2. invalidate static occlusion indeces
        RemoveLightmapsRecursive(container); //3. Reset lightmaps

        scene->SaveScene(sc2Path);

        //restore global material inheritance
        for (auto& mat : parentsMaterials)
        {
            mat->SetParent(sourceGlobalMaterial);
        }
    }
}

void SaveEntityAsAction::RemoveLightmapsRecursive(Entity *entity) const
{
	RenderObject * renderObject = GetRenderObject(entity);
	if (nullptr != renderObject)
	{
		const uint32 batchCount = renderObject->GetRenderBatchCount();
		for (uint32 b = 0; b < batchCount; ++b)
		{
            NMaterial* material = renderObject->GetRenderBatch(b)->GetMaterial();
            if ((nullptr != material) && material->HasLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP))
            {
                material->RemoveTexture(NMaterialTextureName::TEXTURE_LIGHTMAP);
            }
        }
    }

	const int32 count = entity->GetChildrenCount();
	for (int32 ch = 0; ch < count; ++ch)
	{
		RemoveLightmapsRecursive(entity->GetChild(ch));
	}
}

