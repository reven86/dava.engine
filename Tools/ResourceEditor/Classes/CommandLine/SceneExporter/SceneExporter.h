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


#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TextureCompression/TextureConverter.h"

class SceneExporter
{
public:
    void SetGPUForExporting(const DAVA::eGPUFamily newGPU);

    void SetCompressionQuality(DAVA::TextureConverter::eConvertQuality quality);

    void SetInFolder(const DAVA::FilePath& folderPathname);
    void SetOutFolder(const DAVA::FilePath& folderPathname);

    void EnableOptimizations(bool enable);

    void ExportSceneFile(const DAVA::String& fileName, DAVA::Set<DAVA::String>& errorLog);
    void ExportTextureFile(const DAVA::String& fileName, DAVA::Set<DAVA::String>& errorLog);

    void ExportSceneFolder(const DAVA::String& folderName, DAVA::Set<DAVA::String>& errorLog);
    void ExportTextureFolder(const DAVA::String& folderName, DAVA::Set<DAVA::String>& errorLog);

    void ExportScene(DAVA::Scene* scene, const DAVA::FilePath& fileName, DAVA::Set<DAVA::String>& errorLog);

private:
    void RemoveEditorNodes(DAVA::Entity* rootNode);
    void RemoveEditorCustomProperties(DAVA::Entity* rootNode);

    bool ExportDescriptors(DAVA::Scene* scene, DAVA::Set<DAVA::String>& errorLog);
    bool ExportTextureDescriptor(const DAVA::FilePath& pathname, DAVA::Set<DAVA::String>& errorLog);
    bool ExportTexture(const DAVA::TextureDescriptor* descriptor, DAVA::Set<DAVA::String>& errorLog);
    void CompressTexture(const DAVA::TextureDescriptor* descriptor);
    bool CopyCompressedTexture(const DAVA::TextureDescriptor* descriptor, DAVA::Set<DAVA::String>& errorLog);

    bool ExportLandscape(DAVA::Scene* scene, DAVA::Set<DAVA::String>& errorLog);

    SceneUtils sceneUtils;
    DAVA::eGPUFamily exportForGPU = DAVA::eGPUFamily::GPU_ORIGIN;
    DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::eConvertQuality::ECQ_DEFAULT;
    bool optimizeOnExport = true;
    bool exportForAllGPUs = false;
};



#endif // __SCENE_EXPORTER_H__