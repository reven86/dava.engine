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


#ifndef __SCENE_SAVER_H__
#define __SCENE_SAVER_H__

#include "CommandLine/SceneUtils/SceneUtils.h"

using namespace DAVA;

class SceneSaver : RenderObjectsFlusher
{
public:
	SceneSaver();
	virtual ~SceneSaver();
    
    void SetInFolder(const FilePath &folderPathname);
    void SetOutFolder(const FilePath &folderPathname);
    
    void SaveFile(const String &fileName, Set<String> &errorLog);
	void ResaveFile(const String &fileName, Set<String> &errorLog);
    void SaveScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog);
    
    void EnableCopyConverted(bool enabled);

    void ResaveYamlFilesRecursive(const DAVA::FilePath & folder, DAVA::Set<DAVA::String> &errorLog) const;

    
protected:
    void ReleaseTextures();

    void CopyTextures(Scene *scene);
    void CopyTexture(const FilePath &texturePathname);

	void CopyReferencedObject(Entity *node);
	void CopyEffects(Entity *node);
	void CopyEmitter(ParticleEmitter *emitter);

	void CopyCustomColorTexture(Scene *scene, const FilePath & sceneFolder, Set<String> &errorLog);

protected:
    SceneUtils sceneUtils;
    TexturesMap texturesForSave;
    DAVA::Set<DAVA::FilePath> effectFolders;
    bool copyConverted = false;
};



#endif // __SCENE_SAVER_H__