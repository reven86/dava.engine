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


#ifndef __DAVAENGINE_SERIALIZATIONCONTEXT_H__
#define __DAVAENGINE_SERIALIZATIONCONTEXT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Render/RHI/rhi_Type.h"

namespace DAVA
{

	class Scene;
	class DataNode;
	class NMaterial;
	class Texture;
	class NMaterial;
    class PolygonGroup;

	class SerializationContext
	{
    public:
        struct PolygonGroupLoadInfo
        {            
            uint32 filePos = 0;
            int32 requestedFormat = 0;
            bool onScene = false;
        };
	
	public:        
        SerializationContext();
		~SerializationContext();
				
		inline void SetVersion(uint32 curVersion)
		{
			version = curVersion;
		}
		
		inline uint32 GetVersion() const
		{
			return version;
		}
		
		inline void SetDebugLogEnabled(bool state)
		{
			debugLogEnabled = state;
		}
		
		inline bool IsDebugLogEnabled() const
		{
			return debugLogEnabled;
		}
		
		inline void SetScene(Scene* target)
		{
			scene = target;
		}
		
		inline Scene* GetScene()
		{
			return scene;
		}
		
		inline void SetScenePath(const FilePath& path)
		{
			scenePath = path;
		}
		
		inline const FilePath& GetScenePath() const
		{
			return scenePath;
		}
		
		inline void SetRootNodePath(const FilePath& path)
		{
			rootNodePathName = path;
		}
		
		inline const FilePath& GetRootNodePath() const
		{
			return rootNodePathName;
		}
		
		inline void SetDataBlock(uint64 blockId, DataNode* data)
		{            
            DVASSERT(dataBlocks.find(blockId) == dataBlocks.end());
            
			dataBlocks[blockId] = data;
		}
		
		inline DataNode* GetDataBlock(uint64 blockId)
		{
			Map<uint64, DataNode*>::iterator it = dataBlocks.find(blockId);
			return (it != dataBlocks.end()) ? it->second : NULL;
		}
		
		inline void AddBinding(uint64 parentKey, NMaterial* material)
		{
			MaterialBinding binding;
			binding.childMaterial = material;
			binding.parentKey = parentKey;
			
			materialBindings.push_back(binding);
		}
		
        inline void SetGlobalMaterialKey(uint64 materialKey)
        {
            globalMaterialKey = materialKey;
        }

        inline uint64 GetGlobalMaterialKey()
        {
            return globalMaterialKey;
        }

		inline void SetLastError(uint32 error)
		{
			lastError = error;
		}
		
		inline uint32 GetLastError()
		{
			return lastError;
		}
		
		inline void SetDefaultMaterialQuality(const FastName& quality)
		{
			defaultMaterialQuality = quality;
		}
		
		inline const FastName& GetDefaultMaterialQuality() const
		{
			return defaultMaterialQuality;
		}
		
		void ResolveMaterialBindings();

        void AddLoadedPolygonGroup(PolygonGroup *group, uint32 dataFilePos);
        void AddRequestedPolygonGroupFormat(PolygonGroup *group, int32 format);
        void LoadPolygonGroupData(File *file);

	private:
		struct MaterialBinding
		{
			uint64 parentKey = 0;
			NMaterial* childMaterial = nullptr;
		};

		Map<uint64, DataNode*> dataBlocks;
		Map<uint64, NMaterial*> importedMaterials;
		Vector<MaterialBinding> materialBindings;
        Map<PolygonGroup*, PolygonGroupLoadInfo> loadedPolygonGroups;

		Scene* scene = nullptr;
		FilePath rootNodePathName;
		FilePath scenePath;
		FastName defaultMaterialQuality;
        uint64 globalMaterialKey = 0;
		uint32 lastError = 0;
		uint32 version = 0;

		bool debugLogEnabled = false;
	};
};

#endif
