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


#ifndef __OPTION_NAME_H__
#define __OPTION_NAME_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

//command line constants for unification of command line
class OptionName
{
public:
    static const DAVA::String deprecated_forceClose;
    static const DAVA::String deprecated_Export;

    static const DAVA::String Output;
    static const DAVA::String OutFile;
    static const DAVA::String OutDir;

    static const DAVA::String File;
    static const DAVA::String ProcessFile;

    static const DAVA::String ProcessFileList;

    static const DAVA::String Folder;
    static const DAVA::String InDir;
    static const DAVA::String ProcessDir;

    static const DAVA::String QualityConfig;

    static const DAVA::String Split;
    static const DAVA::String Merge;
    static const DAVA::String Save;
    static const DAVA::String Resave;
    static const DAVA::String Build;
    static const DAVA::String Convert;
    static const DAVA::String Create;

    static const DAVA::String Links;
    static const DAVA::String Scene;
    static const DAVA::String Texture;
    static const DAVA::String Yaml;

    static const DAVA::String GPU;
    static const DAVA::String Quality;
    static const DAVA::String Force;
    static const DAVA::String Mipmaps;

    static const DAVA::String SaveNormals;
    static const DAVA::String CopyConverted;
    static const DAVA::String SetCompression;
    static const DAVA::String SetPreset;
    static const DAVA::String SavePreset;
    static const DAVA::String PresetOpt;
    static const DAVA::String PresetsList;

    static const DAVA::String UseAssetCache;
    static const DAVA::String AssetCacheIP;
    static const DAVA::String AssetCachePort;
    static const DAVA::String AssetCacheTimeout;

    static const DAVA::String MakeNameForGPU(DAVA::eGPUFamily gpuFamily);
};

#endif // __OPTION_NAME_H__
