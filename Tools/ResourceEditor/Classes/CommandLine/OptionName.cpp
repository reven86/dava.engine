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

#include "CommandLine/OptionName.h"
#include "Render/GPUFamilyDescriptor.h"

const DAVA::String OptionName::deprecated_forceClose("-forceclose");
const DAVA::String OptionName::deprecated_Export("-export");

const DAVA::String OptionName::Output("-output");
const DAVA::String OptionName::OutFile("-outfile");
const DAVA::String OptionName::OutDir("-outdir");

const DAVA::String OptionName::File("-file");
const DAVA::String OptionName::ProcessFile("-processfile");

const DAVA::String OptionName::ProcessFileList("-processfilelist");

const DAVA::String OptionName::Folder("-folder");
const DAVA::String OptionName::InDir("-indir");
const DAVA::String OptionName::ProcessDir("-processdir");

const DAVA::String OptionName::QualityConfig("-qualitycfgpath");

const DAVA::String OptionName::Split("-split");
const DAVA::String OptionName::Merge("-merge");
const DAVA::String OptionName::Save("-save");
const DAVA::String OptionName::Resave("-resave");
const DAVA::String OptionName::Build("-build");
const DAVA::String OptionName::Convert("-convert");
const DAVA::String OptionName::Create("-create");

const DAVA::String OptionName::Links("-links");
const DAVA::String OptionName::Scene("-scene");
const DAVA::String OptionName::Texture("-texture");
const DAVA::String OptionName::Yaml("-yaml");

const DAVA::String OptionName::GPU("-gpu");
const DAVA::String OptionName::Quality("-quality");
const DAVA::String OptionName::Force("-f");
const DAVA::String OptionName::Mipmaps("-m");

const DAVA::String OptionName::SaveNormals("-saveNormals");
const DAVA::String OptionName::CopyConverted("-copyconverted");
const DAVA::String OptionName::CopyCompression("-copycompression");
const DAVA::String OptionName::SetCompression("-setcompression");

const DAVA::String OptionName::UseAssetCache("-useCache");
const DAVA::String OptionName::AssetCacheIP("-ip");
const DAVA::String OptionName::AssetCachePort("-p");
const DAVA::String OptionName::AssetCacheTimeout("-t");

const DAVA::String OptionName::MakeNameForGPU(DAVA::eGPUFamily gpuFamily)
{
    return ("-" + DAVA::GPUFamilyDescriptor::GetGPUName(gpuFamily));
}
