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


#ifndef __DAVAENGINE_DATANODE_H__
#define __DAVAENGINE_DATANODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{

/**
    
 */
class DataNode : public BaseObject
{
protected:
	virtual ~DataNode();
	
public:
	
	static const uint16 NodeRuntimeFlag;
	
public:	
	DataNode();
    virtual int32 Release();

    
    /**
     */
    void SetScene(Scene * _scene);
    inline Scene * GetScene() { return scene; };
    
    DataNode *	FindByName(const String & searchName);
	virtual void	AddNode(DataNode * node);

    int32  GetNodeIndex();
    uint64 GetPreviousPointer();
	
	inline uint16 GetNodeGlags() const
	{
		return nodeFlags;
	}
	
	inline void AddNodeFlags(uint16 flags)
	{
		nodeFlags = nodeFlags | flags;
	}
	
	inline void RemoveNodeFlags(uint16 flags)
	{
		nodeFlags = nodeFlags & ~flags;
	}
    
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
    
    virtual void UpdateUniqueKey(uint64 newKeyValue) {};
    
    inline void SetDataIndex(int32 idx);
    
protected:
    uint64 pointer;
    Scene * scene;

    int32 index;
	uint16 nodeFlags;
    
public:
    
    INTROSPECTION_EXTEND(DataNode, BaseObject,
        MEMBER(index, "Index", I_SAVE)
        MEMBER(pointer, "Pointer", I_SAVE)
    );
};

inline void DataNode::SetDataIndex(int32 idx)
{
    index = idx;
}

    
};

#endif // __DAVAENGINE_DATANODE_H__





