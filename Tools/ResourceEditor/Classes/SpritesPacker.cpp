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


#include "SpritesPacker.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "CommandLine/CommandLineParser.h"
#include "Qt/Settings/SettingsManager.h"

SpritesPacker::~SpritesPacker()
{

}

void SpritesPacker::SetInputDir(const FilePath & _inputDir)
{
    DVASSERT(_inputDir.IsDirectoryPathname());
	inputDir = _inputDir;
}

void SpritesPacker::SetOutputDir(const FilePath & _outputDir)
{
    DVASSERT(_outputDir.IsDirectoryPathname());
	outputDir = _outputDir;
}

void SpritesPacker::PackLightmaps(DAVA::eGPUFamily gpu)
{
	return PerformPack(true, gpu);
}

void SpritesPacker::PackTextures(DAVA::eGPUFamily gpu)
{
	return PerformPack(false, gpu);
}

void SpritesPacker::PerformPack(bool isLightmapPacking, DAVA::eGPUFamily gpu)
{
	FileSystem::Instance()->CreateDirectory(outputDir, true);

    ResourcePacker2D resourcePacker;

    resourcePacker.forceRepack = true;
    resourcePacker.InitFolders(inputDir, outputDir);
    resourcePacker.isLightmapsPacking = isLightmapPacking;

    if (SettingsManager::GetValue(Settings::General_AssetCache_UseCache).AsBool())
    {
        auto ip = SettingsManager::GetValue(Settings::General_AssetCache_Ip).AsString();
        auto port = SettingsManager::GetValue(Settings::General_AssetCache_Port).AsString();
        auto timeout = SettingsManager::GetValue(Settings::General_AssetCache_Timeout).AsString();
        resourcePacker.SetCacheClientTool("~res:/AssetCacheClient", ip, port, timeout);
    }

    resourcePacker.PackResources(gpu);
}
