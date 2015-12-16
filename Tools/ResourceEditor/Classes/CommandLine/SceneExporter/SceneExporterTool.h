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


#ifndef __SCENE_EXPORTER_TOOL_H__
#define __SCENE_EXPORTER_TOOL_H__

#include "CommandLine/CommandLineTool.h"
#include "TextureCompression/TextureConverter.h"

class SceneExporter;
class SceneExporterTool: public CommandLineTool
{
    enum eAction : DAVA::int8
    {
        ACTION_NONE = -1,

        ACTION_EXPORT_FILE,
        ACTION_EXPORT_FOLDER,
        ACTION_EXPORT_FILELIST
    };

    enum eObject : DAVA::int32
    {
        OBJECT_NONE = -1,

        OBJECT_SCENE = 0,
        OBJECT_TEXTURE
    };

public:
    SceneExporterTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;

    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    void ExportFolder(SceneExporter& exporter);
    void ExportFile(SceneExporter& exporter);
    void ExportFileList(SceneExporter& exporter);

    eAction commandAction = ACTION_NONE;
    eObject commandObject = OBJECT_NONE;

    DAVA::String filename;
    DAVA::String foldername;
    DAVA::FilePath fileListPath;

    DAVA::FilePath inFolder;
    DAVA::FilePath outFolder;
    DAVA::FilePath qualityConfigPath;

    DAVA::eGPUFamily requestedGPU = DAVA::GPU_ORIGIN;
    bool optimizeOnExport = true;

    DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::ECQ_DEFAULT;
};


#endif // __SCENE_EXPORTER_TOOL_H__
