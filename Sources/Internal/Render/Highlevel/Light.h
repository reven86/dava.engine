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



#ifndef __DAVAENGINE_LIGHT_NODE_H__
#define __DAVAENGINE_LIGHT_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneFile/SerializationContext.h"

//default direction (with identity matrix) is -y
namespace DAVA 
{
class SceneFileV2;
class Camera;
    
class Light : public BaseObject
{
public:
    enum eType
    {
        TYPE_DIRECTIONAL = 0,
        TYPE_SPOT,
        TYPE_POINT,
		TYPE_SKY,
        TYPE_AMBIENT,

		TYPE_COUNT
    };
    
    enum
    {
        IS_DYNAMIC = 1 << 0,
        CAST_SHADOW = 1 << 1,
    };
protected:
    virtual ~Light();
public:
    Light();
    
    virtual BaseObject * Clone(BaseObject * dstNode = NULL);

    void SetType(eType _type);
    void SetAmbientColor(const Color & _color);
    void SetDiffuseColor(const Color & _color);
    void SetIntensity(float32 intensity);
    
    eType GetType() const;
    const Color & GetAmbientColor() const;
    const Color & GetDiffuseColor() const;    
    float32 GetIntensity() const;
    
    const Vector3 & GetPosition() const;
    const Vector3 & GetDirection() const;
    
    void SetPosition(const Vector3 & position);
    void SetDirection(const Vector3 & direction);
    
    void SetPositionDirectionFromMatrix(const Matrix4 & worldTransform);

    const Vector4 & CalculatePositionDirectionBindVector(Camera * camera);

    //virtual void Update(float32 timeElapsed);
    //virtual void Draw();
    
	virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);

	const bool IsDynamic();
	void SetDynamic(const bool & isDynamic);
    void AddFlag(uint32 flag);
    void RemoveFlag(uint32 flag);
    uint32 GetFlags();

	//void SetRenderSystem
    
protected:
	uint32 flags;
    Camera * camera;
    uint32 lastUpdatedFrame;
    uint32 type;
    Vector3 position;
    Vector3 direction;
    Vector4 resultPositionDirection;
    
    Color ambientColor;
    Color diffuseColor;
    float32 intensity;
public:
    
    INTROSPECTION_EXTEND(Light, BaseObject,
        MEMBER(position, "Position", I_SAVE | I_VIEW)
        MEMBER(direction, "Direction", I_SAVE | I_VIEW)
                     
        MEMBER(type, InspDesc("Type", GlobalEnumMap<Light::eType>::Instance()), I_SAVE | I_VIEW | I_EDIT)

		PROPERTY("isDynamic", "isDynamic", IsDynamic, SetDynamic, I_VIEW | I_EDIT)
                         
        MEMBER(ambientColor, "Ambient Color", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(diffuseColor, "Color", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(intensity, "Intensity", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(flags, "Flags", I_SAVE | I_VIEW | I_EDIT)
    );
};

};

#endif //__DAVAENGINE_LIGHT_NODE_H__