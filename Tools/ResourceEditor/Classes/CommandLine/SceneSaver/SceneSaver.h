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

class SceneSaver
{
public:
    SceneSaver();
    virtual ~SceneSaver();

    void SetInFolder(const DAVA::FilePath& folderPathname);
    void SetOutFolder(const DAVA::FilePath& folderPathname);

    void SaveFile(const DAVA::String& fileName, DAVA::Set<DAVA::String>& errorLog);
    void ResaveFile(const DAVA::String& fileName, DAVA::Set<DAVA::String>& errorLog);
    void SaveScene(DAVA::Scene* scene, const DAVA::FilePath& fileName, DAVA::Set<DAVA::String>& errorLog);

    void EnableCopyConverted(bool enabled);

    void ResaveYamlFilesRecursive(const DAVA::FilePath& folder, DAVA::Set<DAVA::String>& errorLog) const;

private:
    void ReleaseTextures();

    void CopyTextures(DAVA::Scene* scene);
    void CopyTexture(const DAVA::FilePath& texturePathname);

    void CopyReferencedObject(DAVA::Entity* node);
    void CopyEffects(DAVA::Entity* node);
    void CopyAllParticlesEmitters(const DAVA::ParticleEmitterData& emitterData);
    void CopyEmitterByPath(const DAVA::FilePath& emitterConfigPath);
    void CopyEmitter(DAVA::ParticleEmitter* emitter);
    DAVA::Set<DAVA::FilePath> EnumAlternativeEmittersFilepaths(const DAVA::FilePath& originalFilepath) const;

    void CopyCustomColorTexture(DAVA::Scene* scene, const DAVA::FilePath& sceneFolder, DAVA::Set<DAVA::String>& errorLog);

    SceneUtils sceneUtils;
    DAVA::TexturesMap texturesForSave;
    DAVA::Set<DAVA::FilePath> effectFolders;
    bool copyConverted = false;
};



#endif // __SCENE_SAVER_H__