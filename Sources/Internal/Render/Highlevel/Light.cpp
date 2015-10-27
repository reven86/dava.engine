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


#include "Render/Highlevel/Light.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"


namespace DAVA 
{
    

Light::Light()
:	BaseObject(),
    flags(IS_DYNAMIC | CAST_SHADOW),
    camera(NULL),
    lastUpdatedFrame(0),
    type(TYPE_DIRECTIONAL),
    ambientColor(0.0f, 0.0f, 0.0f, 1.0f),
    diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    intensity(300.0f)
{
}
    
Light::~Light()
{
    
}
    
void Light::SetType(DAVA::Light::eType _type)
{
    type = _type;
}
    
void Light::SetAmbientColor(const Color & _color)
{
    ambientColor = _color;
}

void Light::SetDiffuseColor(const Color & _color)
{
    diffuseColor = _color;
}

void Light::SetIntensity(float32 _intensity)
{
    intensity = _intensity;
}

    
BaseObject * Light::Clone(BaseObject *dstNode)
{
    if(!dstNode)
    {
		DVASSERT_MSG(IsPointerToExactClass<Light>(this), "Can clone only LightNode");
        dstNode = new Light();
    }
    
    //BaseObject::Clone(dstNode);
    
    Light *lightNode = (Light *)dstNode;
    lightNode->type = type;
    lightNode->ambientColor = ambientColor;
    lightNode->diffuseColor = diffuseColor;
	lightNode->intensity = intensity;
	lightNode->flags = flags;
    
    return dstNode;
}

void Light::SetPositionDirectionFromMatrix(const Matrix4 & worldTransform)
{
    position = Vector3(0.0f, 0.0f, 0.0f) * worldTransform;
    direction = MultiplyVectorMat3x3(Vector3(0.0, -1.0f, 0.0f), worldTransform);
    direction.Normalize();
}

Light::eType Light::GetType() const
{
    return (eType)type;
}
const Vector3 & Light::GetPosition() const
{
    return position; 
}

const Vector3 & Light::GetDirection() const
{
    return direction;
}

void Light::SetPosition(const Vector3 & _position)
{
    position = _position;
}

void Light::SetDirection(const Vector3 & _direction)
{
    direction = _direction;
}

const Color & Light::GetAmbientColor() const
{
    return ambientColor;
}
    
const Color & Light::GetDiffuseColor() const
{
    return diffuseColor;
}
    
float32 Light::GetIntensity() const
{
    return intensity;
}

void Light::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
	BaseObject::SaveObject(archive);
	
	archive->SetInt32("type", type);
	archive->SetFloat("ambColor.r", ambientColor.r);
	archive->SetFloat("ambColor.g", ambientColor.g);
	archive->SetFloat("ambColor.b", ambientColor.b);
	archive->SetFloat("ambColor.a", ambientColor.a);

	archive->SetFloat("color.r", diffuseColor.r);
	archive->SetFloat("color.g", diffuseColor.g);
	archive->SetFloat("color.b", diffuseColor.b);
	archive->SetFloat("color.a", diffuseColor.a);
    
    archive->SetFloat("intensity", intensity);

	archive->SetUInt32("flags", flags);
}

void Light::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    BaseObject::LoadObject(archive);

    type = (eType)archive->GetInt32("type");
    
    ambientColor.r = archive->GetFloat("ambColor.r", ambientColor.r);
    ambientColor.g = archive->GetFloat("ambColor.g", ambientColor.g);
    ambientColor.b = archive->GetFloat("ambColor.b", ambientColor.b);
    ambientColor.a = archive->GetFloat("ambColor.a", ambientColor.a);
    
    diffuseColor.r = archive->GetFloat("color.r", diffuseColor.r);
    diffuseColor.g = archive->GetFloat("color.g", diffuseColor.g);
    diffuseColor.b = archive->GetFloat("color.b", diffuseColor.b);
    diffuseColor.a = archive->GetFloat("color.a", diffuseColor.a);
    
    intensity = archive->GetFloat("intensity", intensity);

	flags = archive->GetUInt32("flags", flags);
    
	//isDynamic = GetCustomProperties()->GetBool("editor.dynamiclight.enable", true);
}

// LIGHT
//void LightNode::Draw()
//{
//    SceneNode::Draw();
//}

const bool Light::IsDynamic()
{
	return (flags & IS_DYNAMIC) != 0;
}

void Light::SetDynamic(const bool & _isDynamic)
{
	if(_isDynamic)
	{
		AddFlag(IS_DYNAMIC);
	}
	else
	{
		RemoveFlag(IS_DYNAMIC);
	}
}

void Light::AddFlag(uint32 flag)
{
    flags |= flag;
}

void Light::RemoveFlag(uint32 flag)
{
    flags &= ~flag;
}
    
uint32 Light::GetFlags()
{
    return flags;
}

const Vector4 & Light::CalculatePositionDirectionBindVector(Camera * inCamera)
{
    uint32 globalFrameIndex = Core::Instance()->GetGlobalFrameIndex();
    if (inCamera != camera || lastUpdatedFrame != globalFrameIndex)
    {
        DVASSERT(inCamera);
        
        camera = inCamera;
        lastUpdatedFrame = globalFrameIndex;
        if (type == TYPE_DIRECTIONAL)
        {
            // Here we prepare direction according to shader direction usage. Shader use as ToLightDirection, so we invert it here
            resultPositionDirection = - (MultiplyVectorMat3x3(direction, camera->GetMatrix()));
            resultPositionDirection.w = 0.0f;
        }else
        {
            resultPositionDirection = position * camera->GetMatrix();
        }
    }
    return resultPositionDirection;
}

    
    
};
