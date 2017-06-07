#include "DeveloperTools.h"
#include "Main/mainwindow.h"
#include "Commands2/EntityAddCommand.h"

#include "Qt/ImageSplitterDialog/ImageSplitterDialogNormal.h"

#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/LandscapeSystem.h"

#include "Render/Highlevel/SkinnedMesh.h"

#include "Classes/Selection/Selection.h"

#include <QInputDialog>

using namespace DAVA;

DeveloperTools::DeveloperTools(QWidget* parent)
    : QObject(parent)
{
}

void DeveloperTools::OnDebugFunctionsGridCopy()
{
    SceneEditor2* currentScene = sceneHolder.GetScene();
    float32 z = 0;
    const float32 xshift = 10.0;
    const float32 yshift = 10.0;
    const float32 zshift = 0.0;

    FastName inGlossinessName("inGlossiness");
    FastName inSpecularityName("inSpecularity");

    const SelectableGroup& selection = Selection::GetSelection();
    if (selection.GetSize() == 1)
    {
        Entity* entity = selection.GetContent().front().AsEntity();

        const Matrix4& matrix = entity->GetLocalTransform();

        for (uint32 x = 0; x < 10; ++x)
        {
            for (uint32 y = 0; y < 10; ++y)
            {
                Matrix4 translation;
                translation.CreateTranslation(Vector3(x * xshift, y * yshift, z * zshift));

                Matrix4 newMatrix = matrix * translation;
                Entity* clonedEntity = entity->Clone();
                clonedEntity->SetLocalTransform(newMatrix);

                RenderObject* renderObject = GetRenderObject(clonedEntity);
                NMaterial* material = renderObject->GetRenderBatch(0)->GetMaterial();
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

                StaticOcclusionSystem* sosystem = currentScene->staticOcclusionSystem;
                DVASSERT(sosystem);
                sosystem->InvalidateOcclusionIndicesRecursively(clonedEntity);

                currentScene->AddNode(clonedEntity);
            }
        }
    }
}

void DeveloperTools::OnDebugCreateTestSkinnedObject()
{
    SceneEditor2* currentScene = sceneHolder.GetScene();
    if (!currentScene)
        return;
    ScopedPtr<Entity> entity(new Entity());
    entity->SetName(FastName("SkeletonTestComponent"));

    int boxesCount = 4;
    Vector3 boxes[] = { Vector3(0, 0, 0), Vector3(0, 0, 10), Vector3(2, 0, 15), Vector3(-2, 0, 15) };

    AABBox3 jointBox(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    SkeletonComponent* component = new SkeletonComponent();

    Vector<SkeletonComponent::JointConfig> configJoints;
    configJoints.push_back(SkeletonComponent::JointConfig(SkeletonComponent::INVALID_JOINT_INDEX, 0, FastName("root0"), Vector3(0, 0, 0), Quaternion(0, 0, 0, 1), 1.0, AABBox3(jointBox.min + boxes[0], jointBox.max + boxes[0])));
    configJoints.push_back(SkeletonComponent::JointConfig(0, 1, FastName("root0.bone0"), Vector3(0, 0, 10), Quaternion(0, 0, 0, 1), 1.0, AABBox3(jointBox.min + boxes[1], jointBox.max + boxes[1])));
    configJoints.push_back(SkeletonComponent::JointConfig(1, 2, FastName("root0.bone0.bone0"), Vector3(2, 0, 5), Quaternion(0, 0, 0, 1), 1.0, AABBox3(jointBox.min + boxes[2], jointBox.max + boxes[2])));
    configJoints.push_back(SkeletonComponent::JointConfig(1, 3, FastName("root0.bone0.bone1"), Vector3(-2, 0, 5), Quaternion(0, 0, 0, 1), 1.0, AABBox3(jointBox.min + boxes[3], jointBox.max + boxes[3])));
    component->SetConfigJoints(configJoints);
    entity->AddComponent(component);

    ScopedPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_LINELIST);
    polygonGroup->AllocateData(EVF_VERTEX | EVF_JOINTINDEX | EVF_JOINTWEIGHT, boxesCount * 8, boxesCount * 24);
    for (int32 i = 0; i < boxesCount; i++)
    {
        polygonGroup->SetCoord(i * 8 + 0, boxes[i] + Vector3(jointBox.min.x, jointBox.min.y, jointBox.min.z));
        polygonGroup->SetCoord(i * 8 + 1, boxes[i] + Vector3(jointBox.min.x, jointBox.max.y, jointBox.min.z));
        polygonGroup->SetCoord(i * 8 + 2, boxes[i] + Vector3(jointBox.max.x, jointBox.max.y, jointBox.min.z));
        polygonGroup->SetCoord(i * 8 + 3, boxes[i] + Vector3(jointBox.max.x, jointBox.min.y, jointBox.min.z));
        polygonGroup->SetCoord(i * 8 + 4, boxes[i] + Vector3(jointBox.min.x, jointBox.min.y, jointBox.max.z));
        polygonGroup->SetCoord(i * 8 + 5, boxes[i] + Vector3(jointBox.min.x, jointBox.max.y, jointBox.max.z));
        polygonGroup->SetCoord(i * 8 + 6, boxes[i] + Vector3(jointBox.max.x, jointBox.max.y, jointBox.max.z));
        polygonGroup->SetCoord(i * 8 + 7, boxes[i] + Vector3(jointBox.max.x, jointBox.min.y, jointBox.max.z));
        for (int32 v = 0; v < 8; v++)
        {
            polygonGroup->SetJointIndex(i * 8 + v, 0, i);
            polygonGroup->SetJointWeight(i * 8 + v, 0, 1.0f);
        }

        polygonGroup->SetIndex(i * 24 + 0, i * 8 + 0);
        polygonGroup->SetIndex(i * 24 + 1, i * 8 + 1);
        polygonGroup->SetIndex(i * 24 + 2, i * 8 + 1);
        polygonGroup->SetIndex(i * 24 + 3, i * 8 + 2);
        polygonGroup->SetIndex(i * 24 + 4, i * 8 + 2);
        polygonGroup->SetIndex(i * 24 + 5, i * 8 + 3);
        polygonGroup->SetIndex(i * 24 + 6, i * 8 + 3);
        polygonGroup->SetIndex(i * 24 + 7, i * 8 + 0);

        polygonGroup->SetIndex(i * 24 + 8, i * 8 + 0);
        polygonGroup->SetIndex(i * 24 + 9, i * 8 + 4);
        polygonGroup->SetIndex(i * 24 + 10, i * 8 + 1);
        polygonGroup->SetIndex(i * 24 + 11, i * 8 + 5);
        polygonGroup->SetIndex(i * 24 + 12, i * 8 + 2);
        polygonGroup->SetIndex(i * 24 + 13, i * 8 + 6);
        polygonGroup->SetIndex(i * 24 + 14, i * 8 + 3);
        polygonGroup->SetIndex(i * 24 + 15, i * 8 + 7);
        polygonGroup->SetIndex(i * 24 + 16, i * 8 + 4);
        polygonGroup->SetIndex(i * 24 + 17, i * 8 + 5);
        polygonGroup->SetIndex(i * 24 + 18, i * 8 + 5);
        polygonGroup->SetIndex(i * 24 + 19, i * 8 + 6);
        polygonGroup->SetIndex(i * 24 + 20, i * 8 + 6);
        polygonGroup->SetIndex(i * 24 + 21, i * 8 + 7);
        polygonGroup->SetIndex(i * 24 + 22, i * 8 + 7);
        polygonGroup->SetIndex(i * 24 + 23, i * 8 + 4);
    }

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

    currentScene->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(entity, currentScene)));
}

void DeveloperTools::OnImageSplitterNormals()
{
    ImageSplitterDialogNormal dlg(GetParentWidget());
    dlg.exec();
}

void DeveloperTools::OnReplaceTextureMipmap()
{
    QStringList items = QStringList()
    << QString(NMaterialTextureName::TEXTURE_ALBEDO.c_str())
    << QString(NMaterialTextureName::TEXTURE_LIGHTMAP.c_str())
    << QString(NMaterialTextureName::TEXTURE_DETAIL.c_str())
    << QString(NMaterialTextureName::TEXTURE_NORMAL.c_str());

    bool isOk;
    QString item = QInputDialog::getItem(GetParentWidget(), "Replace mipmaps", "Textures:", items, 0, true, &isOk);
    if (isOk)
    {
        MipMapReplacer::ReplaceMipMaps(sceneHolder.GetScene(), FastName(item.toStdString().c_str()));
    }
}

void DeveloperTools::OnToggleLandscapeInstancing()
{
    SceneEditor2* currentScene = sceneHolder.GetScene();

    for (Landscape* l : currentScene->landscapeSystem->GetLandscapeObjects())
    {
        l->SetUseInstancing(!l->IsUseInstancing());
        Logger::FrameworkDebug("Landscape uses instancing: %s", l->IsUseInstancing() ? "true" : "false");
    }
}

QWidget* DeveloperTools::GetParentWidget()
{
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    DVASSERT(parentWidget);
    return parentWidget;
}
