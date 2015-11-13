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


#include "Render/Highlevel/SpriteObject.h"
#include "Render/RenderCallbacks.h"
#include "Render/Material/NMaterialNames.h"

namespace DAVA 
{

SpriteObject::SpriteObject()
{
    ScopedPtr<Texture> pink(Texture::CreatePink());
    ScopedPtr<Sprite> localSprite(Sprite::CreateFromTexture(pink, 0, 0, static_cast<float32>(pink->GetWidth()), static_cast<float32>(pink->GetHeight())));
    Init(localSprite, 0, Vector2(1.0f, 1.0f), Vector2(0.0f, 0.0f));
    RegisterRestoreCallback();
}

SpriteObject::SpriteObject(const FilePath& pathToSprite, int32 _frame, const Vector2& reqScale, const Vector2& pivotPoint)
{
    ScopedPtr<Sprite> localSprite(Sprite::Create(pathToSprite));
    Init(localSprite, _frame, reqScale, pivotPoint);
    RegisterRestoreCallback();
}

SpriteObject::SpriteObject(Sprite* spr, int32 _frame, const Vector2& reqScale, const Vector2& pivotPoint)
{
	Init(spr, _frame, reqScale, pivotPoint);
    RegisterRestoreCallback();
}

SpriteObject::~SpriteObject()
{
    Clear();
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &SpriteObject::Restore));
}

void SpriteObject::RegisterRestoreCallback()
{
    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &SpriteObject::Restore));
}

void SpriteObject::ClearRenderBatches()
{
    while (GetRenderBatchCount() > 0)
    {
        RenderBatch* batch = GetRenderBatch(0U);
        if (batch->vertexBuffer.IsValid())
            rhi::DeleteVertexBuffer(batch->vertexBuffer);
        if (batch->indexBuffer.IsValid())
            rhi::DeleteIndexBuffer(batch->indexBuffer);
        RemoveRenderBatch(0U);
    }
}

void SpriteObject::Clear()
{
    ClearRenderBatches();
    bbox.Empty();
    SafeRelease(sprite);
}

void SpriteObject::Init( Sprite *spr, int32 _frame, const Vector2 &reqScale, const Vector2 &pivotPoint )
{
    Clear();

	type = TYPE_SPRITE;
	spriteType = SPRITE_OBJECT;

	sprScale = reqScale;
	sprPivot = pivotPoint;
	sprite = SafeRetain(spr);
	frame = _frame;

	SetupRenderBatch();
}

void SpriteObject::Restore()
{
    RenderBatch* batch = GetRenderBatch(0U);
    if (batch == nullptr)
        return;

    rhi::HVertexBuffer vBuffer = batch->vertexBuffer;
    rhi::HIndexBuffer iBuffer = batch->indexBuffer;

    if (rhi::NeedRestoreVertexBuffer(vBuffer) || rhi::NeedRestoreIndexBuffer(iBuffer))
        UpdateBufferData(vBuffer, iBuffer);
}
void SpriteObject::UpdateBufferData(rhi::HVertexBuffer vBuffer, rhi::HIndexBuffer iBuffer)
{
    uint32 framesCount = sprite->GetFrameCount();
    uint32 vxCount = framesCount * 4;
    uint32 indCount = framesCount * 6;

    Vector<float32> verticies(vxCount * (3 + 2));
    Vector<uint16> indices(indCount);

    float32* verticesPtr = verticies.data();
    uint16* indicesPtr = indices.data();
    for (uint32 i = 0; i < framesCount; ++i)
    {
        float32 x0 = sprite->GetRectOffsetValueForFrame(i, Sprite::X_OFFSET_TO_ACTIVE) - sprPivot.x;
        float32 y0 = sprite->GetRectOffsetValueForFrame(i, Sprite::Y_OFFSET_TO_ACTIVE) - sprPivot.y;
        float32 x1 = x0 + sprite->GetRectOffsetValueForFrame(i, Sprite::ACTIVE_WIDTH);
        float32 y1 = y0 + sprite->GetRectOffsetValueForFrame(i, Sprite::ACTIVE_HEIGHT);
        x0 *= sprScale.x;
        x1 *= sprScale.y;
        y0 *= sprScale.x;
        y1 *= sprScale.y;

        float32* pT = sprite->GetTextureVerts(i);

        *((Vector3*)verticesPtr) = Vector3(x0, y0, 0);
        bbox.AddPoint(*((Vector3*)verticesPtr));
        verticesPtr += 3;
        *((Vector2*)verticesPtr) = *((Vector2*)(pT + 0));
        verticesPtr += 2;
        *((Vector3*)verticesPtr) = Vector3(x1, y0, 0);
        bbox.AddPoint(*((Vector3*)verticesPtr));
        verticesPtr += 3;
        *((Vector2*)verticesPtr) = *((Vector2*)(pT + 2));
        verticesPtr += 2;
        *((Vector3*)verticesPtr) = Vector3(x0, y1, 0);
        bbox.AddPoint(*((Vector3*)verticesPtr));
        verticesPtr += 3;
        *((Vector2*)verticesPtr) = *((Vector2*)(pT + 4));
        verticesPtr += 2;
        *((Vector3*)verticesPtr) = Vector3(x1, y1, 0);
        bbox.AddPoint(*((Vector3*)verticesPtr));
        verticesPtr += 3;
        *((Vector2*)verticesPtr) = *((Vector2*)(pT + 6));
        verticesPtr += 2;

        *indicesPtr = i * 4 + 0;
        ++indicesPtr;
        *indicesPtr = i * 4 + 1;
        ++indicesPtr;
        *indicesPtr = i * 4 + 2;
        ++indicesPtr;

        *indicesPtr = i * 4 + 2;
        ++indicesPtr;
        *indicesPtr = i * 4 + 1;
        ++indicesPtr;
        *indicesPtr = i * 4 + 3;
        ++indicesPtr;
    }
    rhi::UpdateVertexBuffer(vBuffer, verticies.data(), 0, vxCount * (3 + 2) * sizeof(float32));
    rhi::UpdateIndexBuffer(iBuffer, indices.data(), 0, indCount * sizeof(uint16));
}

void SpriteObject::SetupRenderBatch()
{
    if (sprite == nullptr)
        return;

    uint32 vxCount = sprite->GetFrameCount() * 4;
    uint32 indCount = sprite->GetFrameCount() * 6;

    NMaterial* material = new NMaterial();
    material->SetMaterialName(FastName("SpriteObject_material"));
    material->SetFXName(NMaterialName::TEXTURED_ALPHABLEND);
    material->SetRuntime(true);
    material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, sprite->GetTexture(frame));

    RenderBatch* batch = new RenderBatch();
    batch->SetMaterial(material);

    batch->vertexBuffer = rhi::CreateVertexBuffer(vxCount * (3 + 2) * sizeof(float32));
    batch->indexBuffer = rhi::CreateIndexBuffer(indCount * sizeof(uint16));
    UpdateBufferData(batch->vertexBuffer, batch->indexBuffer);

    batch->vertexBase = 0;
    batch->vertexCount = vxCount;
    batch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    batch->indexCount = 6;
    batch->startIndex = frame * 6;

    rhi::VertexLayout vxLayout;
    vxLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vxLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    batch->vertexLayoutId = rhi::VertexLayout::UniqueId(vxLayout);

    AddRenderBatch(batch);

    SafeRelease(material);
    SafeRelease(batch);
}


RenderObject * SpriteObject::Clone(RenderObject *newObject)
{
    if (newObject == nullptr)
    {
        DVASSERT_MSG(IsPointerToExactClass<SpriteObject>(this), "Can clone only SpriteObject");
        newObject = new SpriteObject(sprite, frame, sprScale, sprPivot);
    }

    SpriteObject* spriteObject = static_cast<SpriteObject*>(newObject);

    spriteObject->flags = flags;
	spriteObject->RemoveFlag(RenderObject::MARKED_FOR_UPDATE);
	spriteObject->debugFlags = debugFlags;
	spriteObject->ownerDebugInfo = ownerDebugInfo;

	return spriteObject;
}


void SpriteObject::SetFrame(int32 newFrame)
{
	frame = Clamp(newFrame, 0, sprite->GetFrameCount() - 1);

    if (GetRenderBatchCount() > 0)
    {
        GetRenderBatch(0)->GetMaterial()->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, sprite->GetTexture(frame));
        GetRenderBatch(0)->startIndex = frame * 6;
    }
}

int32 SpriteObject::GetFrame() const
{
	return frame;
}

Sprite * SpriteObject::GetSprite() const
{
	return sprite;
}

void SpriteObject::SetSpriteType(eSpriteType _type)
{
	spriteType = _type;
}

SpriteObject::eSpriteType SpriteObject::GetSpriteType() const
{
	return spriteType;
}

const Vector2 & SpriteObject::GetScale() const
{
	return sprScale;
}

const Vector2 & SpriteObject::GetPivot() const
{
	return sprPivot;
}

void SpriteObject::BindDynamicParameters(Camera* camera)
{
    const Matrix4& cameraMatrix = camera->GetMatrix();
    switch (spriteType)
    {
    case SpriteObject::SPRITE_OBJECT:
    {
        worldMatrix = (*worldTransform);
        break;
    };
    case SpriteObject::SPRITE_BILLBOARD:
    {
        Matrix4 inverse(Matrix4::IDENTITY);

        inverse._00 = cameraMatrix._00;
        inverse._01 = cameraMatrix._10;
        inverse._02 = cameraMatrix._20;

        inverse._10 = cameraMatrix._01;
        inverse._11 = cameraMatrix._11;
        inverse._12 = cameraMatrix._21;

        inverse._20 = cameraMatrix._02;
        inverse._21 = cameraMatrix._12;
        inverse._22 = cameraMatrix._22;

        worldMatrix = inverse * (*worldTransform);
        break;
    };
    case SpriteObject::SPRITE_BILLBOARD_TO_CAMERA:
    {
        Vector3 look = camera->GetPosition() - Vector3(0.0f, 0.0f, 0.0f) * (*worldTransform);
        look.Normalize();
        Vector3 right = CrossProduct(camera->GetUp(), look);
        Vector3 up = CrossProduct(look, right);

        Matrix4 matrix = Matrix4::IDENTITY;
        matrix._00 = right.x;
        matrix._01 = right.y;
        matrix._02 = right.z;

        matrix._10 = up.x;
        matrix._11 = up.y;
        matrix._12 = up.z;

        matrix._20 = look.x;
        matrix._21 = look.y;
        matrix._22 = look.z;

        worldMatrix = matrix * (*worldTransform);
        break;
    };
    }

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &worldMatrix, (pointer_size)&worldMatrix);
}

void SpriteObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    // we dont need to save render batche(s)
    // because sprite creating it on loading
    Vector<IndexedRenderBatch> currentRenderBatches;
    renderBatchArray.swap(currentRenderBatches);
    RenderObject::Save(archive, serializationContext);
    renderBatchArray.swap(currentRenderBatches);

    if ((archive == nullptr) || (sprite == nullptr))
    {
        return;
    }

    FilePath filePath = sprite->GetRelativePathname();
    if (!filePath.IsEmpty())
    {
        archive->SetString("sprite.path", filePath.GetRelativePathname(serializationContext->GetScenePath()));
    }
}

void SpriteObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    RenderObject::Load(archive, serializationContext);

    if (archive == nullptr)
    {
        return;
    }

    String path = archive->GetString("sprite.path");
    if (!path.empty())
    {
        ScopedPtr<Sprite> localSprite(Sprite::Create(serializationContext->GetScenePath() + path));
        if (localSprite)
        {
            Init(localSprite, 0, Vector2(1, 1), Vector2(localSprite->GetWidth(), localSprite->GetHeight()) * 0.5f);
            AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
        }
	}
}
void SpriteObject::RecalcBoundingBox()
{
    // do nothing, box is being calculated during UpdateBufferData
}
};
