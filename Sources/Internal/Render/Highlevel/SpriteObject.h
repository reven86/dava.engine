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


#ifndef __DAVAENGINE_SPRITE_OBJECT_H__
#define __DAVAENGINE_SPRITE_OBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/2D/Sprite.h"

namespace DAVA 
{

class SpriteObject: public RenderObject
{
public:
    SpriteObject();
	SpriteObject(const FilePath &pathToSprite, int32 frame = 0
				, const Vector2 &reqScale = Vector2(1.0f, 1.0f)
				, const Vector2 &pivotPoint = Vector2(0.0f, 0.0f));
	SpriteObject(Sprite *spr, int32 frame = 0
				, const Vector2 &reqScale = Vector2(1.0f, 1.0f)
				, const Vector2 &pivotPoint = Vector2(0.0f, 0.0f));

	virtual ~SpriteObject();
	
	enum eSpriteType
    {
        SPRITE_OBJECT = 0,            //! draw sprite without any transformations. Set by default.
        SPRITE_BILLBOARD,             //! normal billboard when billboard is always parallel to the camera projection plane. It computed by multiplication of worldMatrix of node to [R]^-1 matrix of camera
        SPRITE_BILLBOARD_TO_CAMERA,   //! billboard is facing to camera point
    };
    
    /**
        \brief Set type of coordinates modification for the given sprite node
        \param[in] type type you want to set
     */
    void SetSpriteType(eSpriteType type);
    /**
        \brief Get type of coordinates modification for the given sprite node
        \returns type that was set to this sprite node
     */
    eSpriteType GetSpriteType() const;
    
	/**
        \brief Change sprite frame for this sprite node 
        \param[in] newFrame frame you want to set
     */
    void SetFrame(int32 newFrame);
    
    /**
        \brief Get frame for this sprite node
        \returns frame index that was set for this node last time
     */
    int32 GetFrame() const;

		
	Sprite * GetSprite() const;
	
	const Vector2& GetScale() const;
	const Vector2& GetPivot() const;

    RenderObject* Clone(RenderObject* newObject) override;

    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void RecalcBoundingBox() override;

    void BindDynamicParameters(Camera* camera) override;

private:
    void Clear();
    void ClearRenderBatches();
    void Restore();
    void UpdateBufferData(rhi::HVertexBuffer vBuffer, rhi::HIndexBuffer iBuffer);
	void Init(Sprite *spr, int32 _frame, const Vector2 &reqScale, const Vector2 &pivotPoint);
	void SetupRenderBatch();
    void RegisterRestoreCallback();

private:
    Sprite* sprite = nullptr;
    Matrix4 worldMatrix;
    Vector2 sprScale;
    Vector2 sprPivot;
    int32 frame = 0;
    eSpriteType spriteType = SPRITE_OBJECT;

public:
	INTROSPECTION_EXTEND(SpriteObject, RenderObject, 
		NULL
	);
};


};

#endif // __DAVAENGINE_SPRITE_OBJECT_H__
