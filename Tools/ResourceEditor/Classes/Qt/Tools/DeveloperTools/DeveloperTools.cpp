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


#include "DeveloperTools.h"
#include "Main/mainwindow.h"
#include "Scene/System/SelectionSystem.h"
#include "Commands2/EntityAddCommand.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Render/Highlevel/SkinnedMesh.h"

#include "QtTools/SpyWidget/SpySearch/SpySearch.h"
#include "Qt/ImageSplitterDialog/ImageSplitterDialogNormal.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"

#include <QInputDialog>

#include "DAVAEngine.h"
using namespace DAVA;

DeveloperTools::DeveloperTools(QObject *parent)
    : QObject(parent)
{
}

void DeveloperTools::OnDebugFunctionsGridCopy()
{
    SceneEditor2 * currentScene = QtMainWindow::Instance()->GetCurrentScene();
    float32 z = 0;
    const float32 xshift = 10.0;
    const float32 yshift = 10.0;
    const float32 zshift = 0.0;

    FastName inGlossinessName("inGlossiness");
    FastName inSpecularityName("inSpecularity");

    if (currentScene->selectionSystem->GetSelectionCount() == 1)
    {
        Entity * entity = currentScene->selectionSystem->GetSelectionEntity(0);

        const Matrix4 & matrix = entity->GetLocalTransform();

        for (uint32 x = 0; x < 10; ++x)
        {
            for (uint32 y = 0; y < 10; ++y)
            {
                Matrix4 translation;
                translation.CreateTranslation(Vector3(x * xshift, y * yshift, z * zshift));

                Matrix4 newMatrix = matrix * translation;
                Entity * clonedEntity = entity->Clone();
                clonedEntity->SetLocalTransform(newMatrix);

                RenderObject * renderObject = GetRenderObject(clonedEntity);
                NMaterial * material = renderObject->GetRenderBatch(0)->GetMaterial();
                float32 inGlossiness = (float32)x / 9.0;
                float32 inSpecularity = (float32)y / 9.0;

                if (material->HasLocalProperty(inGlossinessName))
                    material->SetPropertyValue(inGlossinessName, &inGlossiness);
                else
                    material->AddProperty(inGlossinessName, &inGlossiness, rhi::ShaderProp::TYPE_FLOAT1);

                if (material->HasLocalProperty(inSpecularityName))
                    material->SetPropertyValue(inSpecularityName, &inSpecularity);
                else
                    material->AddProperty(inSpecularityName, &inSpecularity, rhi::ShaderProp::TYPE_FLOAT1);

                StaticOcclusionSystem *sosystem = currentScene->staticOcclusionSystem;
                DVASSERT(sosystem);
                sosystem->InvalidateOcclusionIndicesRecursively(clonedEntity);

                currentScene->AddNode(clonedEntity);
            }
        }
    }
}


void DeveloperTools::OnDebugCreateTestSkinnedObject()
{
    SceneEditor2 * currentScene = QtMainWindow::Instance()->GetCurrentScene();
    if(!currentScene) return;
    ScopedPtr<Entity> entity(new Entity());
    entity->SetName(FastName("SkeletonTestComponent"));
    
    int boxesCount = 4;
    Vector3 boxes[]={Vector3(0,0,0), Vector3(0,0,10), Vector3(2,0,15), Vector3(-2,0,15)};

    AABBox3 jointBox(Vector3(-1, -1, -1), Vector3(1,1,1));
    SkeletonComponent* component = new SkeletonComponent();        

    Vector<SkeletonComponent::JointConfig> configJoints;    
    configJoints.push_back(SkeletonComponent::JointConfig(SkeletonComponent::INVALID_JOINT_INDEX, 0, FastName("root0"), Vector3(0,0,0), Quaternion(0,0,0,1), 1.0, AABBox3(jointBox.min+boxes[0], jointBox.max+boxes[0])));    
    configJoints.push_back(SkeletonComponent::JointConfig(0, 1, FastName("root0.bone0"), Vector3(0,0,10), Quaternion(0,0,0,1), 1.0, AABBox3(jointBox.min+boxes[1], jointBox.max+boxes[1])));
    configJoints.push_back(SkeletonComponent::JointConfig(1, 2, FastName("root0.bone0.bone0"), Vector3(2,0,5), Quaternion(0,0,0,1), 1.0, AABBox3(jointBox.min+boxes[2], jointBox.max+boxes[2])));
    configJoints.push_back(SkeletonComponent::JointConfig(1, 3, FastName("root0.bone0.bone1"), Vector3(-2,0,5), Quaternion(0,0,0,1), 1.0, AABBox3(jointBox.min+boxes[3], jointBox.max+boxes[3])));    
    component->SetConfigJoints(configJoints);
    entity->AddComponent(component);        
    
    ScopedPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->AllocateData(EVF_VERTEX | EVF_JOINTINDEX | EVF_JOINTWEIGHT, boxesCount*8, boxesCount*24);
    for (int32 i=0; i<boxesCount; i++)
    {
        polygonGroup->SetCoord(i*8+0, boxes[i]+Vector3(jointBox.min.x, jointBox.min.y, jointBox.min.z));
        polygonGroup->SetCoord(i*8+1, boxes[i]+Vector3(jointBox.min.x, jointBox.max.y, jointBox.min.z));
        polygonGroup->SetCoord(i*8+2, boxes[i]+Vector3(jointBox.max.x, jointBox.max.y, jointBox.min.z));
        polygonGroup->SetCoord(i*8+3, boxes[i]+Vector3(jointBox.max.x, jointBox.min.y, jointBox.min.z));
        polygonGroup->SetCoord(i*8+4, boxes[i]+Vector3(jointBox.min.x, jointBox.min.y, jointBox.max.z));
        polygonGroup->SetCoord(i*8+5, boxes[i]+Vector3(jointBox.min.x, jointBox.max.y, jointBox.max.z));
        polygonGroup->SetCoord(i*8+6, boxes[i]+Vector3(jointBox.max.x, jointBox.max.y, jointBox.max.z));
        polygonGroup->SetCoord(i*8+7, boxes[i]+Vector3(jointBox.max.x, jointBox.min.y, jointBox.max.z));
        for (int32 v=0; v<8; v++)
        {
            polygonGroup->SetJointIndex(i*8+v, 0, i);
            polygonGroup->SetJointWeight(i*8+v, 0, 1.0f);
        }

        polygonGroup->SetIndex(i*24+0, i*8+0); polygonGroup->SetIndex(i*24+1, i*8+1);
        polygonGroup->SetIndex(i*24+2, i*8+1); polygonGroup->SetIndex(i*24+3, i*8+2);
        polygonGroup->SetIndex(i*24+4, i*8+2); polygonGroup->SetIndex(i*24+5, i*8+3);
        polygonGroup->SetIndex(i*24+6, i*8+3); polygonGroup->SetIndex(i*24+7, i*8+0);

        polygonGroup->SetIndex(i*24+8, i*8+0); polygonGroup->SetIndex(i*24+9, i*8+4);
        polygonGroup->SetIndex(i*24+10, i*8+1); polygonGroup->SetIndex(i*24+11, i*8+5);
        polygonGroup->SetIndex(i*24+12, i*8+2); polygonGroup->SetIndex(i*24+13, i*8+6);
        polygonGroup->SetIndex(i*24+14, i*8+3); polygonGroup->SetIndex(i*24+15, i*8+7);
        polygonGroup->SetIndex(i*24+16, i*8+4); polygonGroup->SetIndex(i*24+17, i*8+5);
        polygonGroup->SetIndex(i*24+18, i*8+5); polygonGroup->SetIndex(i*24+19, i*8+6);
        polygonGroup->SetIndex(i*24+20, i*8+6); polygonGroup->SetIndex(i*24+21, i*8+7);
        polygonGroup->SetIndex(i*24+22, i*8+7); polygonGroup->SetIndex(i*24+23, i*8+4);
    }

    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_LINELIST);

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetMaterialName(FastName("DebugSkeleton"));
    material->SetFXName(NMaterialName::DECAL_OPAQUE);
    material->SetFlag(NMaterialFlagName::FLAG_SKINNING, 1);

    ScopedPtr<RenderBatch> renderBatch(new RenderBatch());
    renderBatch->SetMaterial(material);
    renderBatch->SetPolygonGroup(polygonGroup);

    ScopedPtr<SkinnedMesh> skinnedMesh(new SkinnedMesh());
    skinnedMesh->AddRenderBatch(renderBatch);

    RenderComponent* renderComponent = new RenderComponent();
    renderComponent->SetRenderObject(skinnedMesh);
    entity->AddComponent(renderComponent);

    currentScene->Exec(new EntityAddCommand(entity, currentScene));
}

void DeveloperTools::OnImageSplitterNormals()
{
    ImageSplitterDialogNormal dlg(QtMainWindow::Instance());
    dlg.exec();
}

void DeveloperTools::OnSpyWidget()
{
    auto spySearch = new SpySearch(this);
    spySearch->show();
}

void DeveloperTools::OnReplaceTextureMipmap()
{
    QStringList items = QStringList()
    << QString(NMaterialTextureName::TEXTURE_ALBEDO.c_str())
    << QString(NMaterialTextureName::TEXTURE_LIGHTMAP.c_str())
    << QString(NMaterialTextureName::TEXTURE_DETAIL.c_str())
    << QString(NMaterialTextureName::TEXTURE_NORMAL.c_str());

    bool isOk;
    QString item = QInputDialog::getItem(QtMainWindow::Instance(), "Replace mipmaps", "Textures:", items, 0, true, &isOk);
    if (isOk)
    {
        MipMapReplacer::ReplaceMipMaps(QtMainWindow::Instance()->GetCurrentScene(), FastName(item.toStdString().c_str()));
    }
}