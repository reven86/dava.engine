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

using namespace DAVA;

class SceneExporter : RenderObjectsFlusher
{
public:

	SceneExporter();
	virtual ~SceneExporter();
    
    void SetGPUForExporting(const eGPUFamily newGPU);
    
	void SetCompressionQuality(TextureConverter::eConvertQuality quality);

    void SetInFolder(const FilePath &folderPathname);
    void SetOutFolder(const FilePath &folderPathname);
    
	void EnableOptimizations( bool enable );

    void ExportSceneFile(const String &fileName, Set<String> &errorLog);
    void ExportTextureFile(const String &fileName, Set<String> &errorLog);
    
    void ExportSceneFolder(const String &folderName, Set<String> &errorLog);
    void ExportTextureFolder(const String &folderName, Set<String> &errorLog);
    
    void ExportScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog);
    
protected:
    
    void RemoveEditorNodes(Entity *rootNode);
    void RemoveEditorCustomProperties(Entity *rootNode);
    
    bool ExportDescriptors(DAVA::Scene *scene, Set<String> &errorLog);
    bool ExportTextureDescriptor(const FilePath &pathname, Set<String> &errorLog);
    bool ExportTexture(const TextureDescriptor * descriptor, Set<String> &errorLog);
    void CompressTextureIfNeed(const TextureDescriptor * descriptor, Set<String> &errorLog);

    bool ExportLandscape(Scene *scene, Set<String> &errorLog);

protected:
    SceneUtils sceneUtils;
    eGPUFamily exportForGPU = eGPUFamily::GPU_ORIGIN;
	TextureConverter::eConvertQuality quality = TextureConverter::eConvertQuality::ECQ_DEFAULT;
	bool optimizeOnExport = false;
};



#endif // __SCENE_EXPORTER_H__