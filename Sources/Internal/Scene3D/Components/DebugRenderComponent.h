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


#ifndef __DAVAENGINE_SCENE3D_DEBUG_RENDER_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_DEBUG_RENDER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{

class DebugRenderComponent : public Component
{
public:
    enum
	{
		DEBUG_DRAW_NONE			= 0x0,
		DEBUG_DRAW_AABBOX		= 0x1,
		DEBUG_DRAW_LOCAL_AXIS	= 0x2,
		DEBUG_DRAW_AABOX_CORNERS= 0x4,
		DEBUG_DRAW_LIGHT_NODE	= 0x8,
        DEBUG_DRAW_NORMALS		= 0x10,
        DEBUG_DRAW_GRID			= 0x20,
		DEBUG_DRAW_USERNODE		= 0x40,
		DEBUG_DRAW_RED_AABBOX	= 0x80,
		DEBUG_DRAW_CAMERA		= 0x100,

		DEBUG_AUTOCREATED		= 0x80000000,
        DEBUG_DRAW_ALL			= 0xFFFFFFFF,
	};
protected:
    virtual ~DebugRenderComponent();
public:
    DebugRenderComponent();
    
    IMPLEMENT_COMPONENT_TYPE(DEBUG_RENDER_COMPONENT);

    void SetDebugFlags(uint32 debugFlags);
    uint32 GetDebugFlags() const;

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

private:
    uint32 curDebugFlags;
    
public:
    INTROSPECTION_EXTEND(DebugRenderComponent, Component,
        PROPERTY("curDebugFlags", "Debug Flags ", GetDebugFlags, SetDebugFlags, I_SAVE | I_VIEW | I_EDIT)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
