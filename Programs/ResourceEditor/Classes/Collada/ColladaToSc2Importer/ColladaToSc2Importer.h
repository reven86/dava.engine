#ifndef __COLLADA_TO_SC2_IMPORTER_H__
#define __COLLADA_TO_SC2_IMPORTER_H__

#include "Collada/ColladaToSc2Importer/ImportLibrary.h"
#include "Collada/ColladaErrorCodes.h"

namespace DAVA
{
class Entity;
class ColladaSceneNode;
class ImportLibrary;

class ColladaToSc2Importer
{
public:
    ColladaToSc2Importer();
    eColladaErrorCodes SaveSC2(ColladaScene* colladaScene, const FilePath& scenePath);

private:
    void ImportAnimation(ColladaSceneNode* colladaNode, Entity* nodeEntity);
    void LoadMaterialParents(ColladaScene* colladaScene);
    void LoadAnimations(ColladaScene* colladaScene);
    bool VerifyColladaMesh(ColladaMeshInstance* mesh, const FastName& nodeName);
    eColladaErrorCodes VerifyDavaMesh(RenderObject* mesh, const FastName name);
    eColladaErrorCodes ImportMeshes(const Vector<ColladaMeshInstance*>& meshInstances, Entity* node);
    eColladaErrorCodes BuildSceneAsCollada(Entity* root, ColladaSceneNode* colladaNode);
    Mesh* GetMeshFromCollada(ColladaMeshInstance* mesh, const bool isShadow);

    ImportLibrary library;
};
};

#endif