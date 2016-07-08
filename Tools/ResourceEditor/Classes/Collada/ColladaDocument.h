#ifndef __COLLADALOADER_COLLADADOCUMENT_H__
#define __COLLADALOADER_COLLADADOCUMENT_H__

#include "ColladaIncludes.h"
#include "ColladaMesh.h"
#include "ColladaScene.h"
#include "ColladaErrorCodes.h"

#include "DAVAEngine.h"

namespace DAVA
{
class ColladaDocument
{
public:
    eColladaErrorCodes Open(const char* filename);
    bool ExportAnimations(const char* filename);
    bool ExportNodeAnimations(FCDocument* exportDoc, FCDSceneNode* exportNode);

    void Close();

    void Render();
    void LoadTextures();

    bool IsEmptyNode(ColladaSceneNode* node);

    eColladaErrorCodes SaveSC2(const FilePath& scenePath) const;
    String GetTextureName(const FilePath& scenePath, ColladaTexture* texture);

    void GetAnimationTimeInfo(FCDocument* document, float32& timeStart, float32& timeEnd);

    FILE* sceneFP;
    ColladaScene* colladaScene;

private:
    FCDocument* document;
};
};

#endif