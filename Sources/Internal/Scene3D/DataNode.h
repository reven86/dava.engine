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
public:
    
    static const int32 INVALID_ID = 0;
    
public:
    DataNode();

    void SetScene(Scene * _scene);
    Scene* GetScene() const;
    
    DataNode* FindByName(const String & searchName);

    uint64 GetNodeID() const;
    void SetNodeID(uint64 id);

    void SetRuntime(bool isRuntime);
    bool IsRuntime() const;
    
    virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
    
protected:
    virtual ~DataNode();

    uint64 id;
    bool isRuntime;
    Scene * scene;

public:
    INTROSPECTION_EXTEND(DataNode, BaseObject,
        MEMBER(id, "Id", I_SAVE)
    );
};

};

#endif // __DAVAENGINE_DATANODE_H__





