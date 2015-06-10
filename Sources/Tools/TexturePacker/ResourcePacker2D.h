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


#ifndef __DAVAENGINE_RESOURCEPACKER2D_H__
#define __DAVAENGINE_RESOURCEPACKER2D_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class DefinitionFile;
class YamlNode;

class ResourcePacker2D
{
public:
    using FILESMAP = std::map<String, String>;
	ResourcePacker2D();

	// Packing of resources section
	void InitFolders(const FilePath & inputPath,const FilePath & outputPath);
	void PackResources(eGPUFamily forGPU);
    void RecalculateMD5ForOutputDir();
    
	void RecursiveTreeWalk(const FilePath & inputPath,const FilePath & outputPath, const Vector<String> & flags = Vector<String>());
	bool IsModifyDateChagedDir(const FilePath & processDirectoryPath, const FilePath & pathname);
	bool IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName, bool isRecursive);
	bool IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName);
	
    DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName, bool twoSideMargin, uint32 texturesMargin);
	Vector<String> FetchFlags(const FilePath & flagsPathname);

	static String GetProcessFolderName();
	bool SaveFileListToYaml(const FilePath & yamlFilePath);
	bool CheckSpriteFilesDates(YamlNode *rootNode);
	void FillSpriteFilesMap(const FilePath & inputPathName);
public:
    
	FilePath inputGfxDirectory;
	FilePath outputGfxDirectory;
	FilePath excludeDirectory;
	String gfxDirName;
    
	bool isGfxModified;
    
	bool isLightmapsPacking;
	bool clearProcessDirectory;
    eGPUFamily requestedGPUFamily;
 	FILESMAP spriteFiles;

	const Set<String>& GetErrors() const;
	
protected:
	bool isRecursiveFlagSet(const Vector<String> & flags);
	Set<String> errors;

	void AddError(const String& errorMsg);
};
};


#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
