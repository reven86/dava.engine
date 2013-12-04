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

namespace DAVA
{

/**
    
 */
class SceneFileV2;
class DataNode : public BaseObject
{
protected:
	virtual ~DataNode();
public:	
	DataNode();
    virtual int32 Release();

    
    /**
     */
    void SetScene(Scene * _scene);
    inline Scene * GetScene() { return scene; };
    
    /**
        \brief Set name of this particular node.
        \param[in] new name for this node
     */
    void SetName(const String & name);

    /**
        \brief Get name of this particular node.
        \returns name of this node
     */
    const String & GetName() const;
    
    DataNode *	FindByName(const String & searchName);
    virtual void	AddNode(DataNode * node);
	virtual void	RemoveNode(DataNode * node);
	virtual DataNode * GetChild(int32 index);
	virtual int32   GetChildrenCount();
	virtual void	RemoveAllChildren();

    //DataNode * FindByAddress();
    int32  GetNodeIndex();
    uint64 GetPreviousPointer(); 
    
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
protected:
    uint64 pointer;
    Scene * scene;
    String name;
    Vector<DataNode*> children;
    int32 index;
    
public:
    
    INTROSPECTION_EXTEND(DataNode, BaseObject,
        MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(index, "Index", I_SAVE)
        MEMBER(pointer, "Pointer", I_SAVE)
        COLLECTION(children, "Children", I_SAVE | I_VIEW | I_EDIT)
    );
};
    
/*class DataNodeArray : public BaseObject
{
public:
    DataNodeArray(Scene * _scene);
    
    virtual void	AddNode(DataNode * node);
	virtual void	RemoveNode(DataNode * node);
	virtual DataNode * GetChild(int32 index);
	virtual int32   GetChildrenCount();
	virtual void	RemoveAllChildren();

protected:
    
    Scene * scene;
};*/
    

};

#endif // __DAVAENGINE_SCENENODE_H__





