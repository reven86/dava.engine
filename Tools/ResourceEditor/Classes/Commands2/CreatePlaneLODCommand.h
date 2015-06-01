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


#ifndef __CREATE_PLANE_LOD_COOMAND_H__
#define __CREATE_PLANE_LOD_COOMAND_H__

#include "Commands2/Command2.h"
#include "DAVAEngine.h"

class CreatePlaneLODCommand : public Command2
{
public:
	CreatePlaneLODCommand(DAVA::LodComponent * lodComponent, DAVA::int32 fromLodLayer, DAVA::uint32 textureSize, const DAVA::FilePath & texturePath);
    ~CreatePlaneLODCommand();

    virtual void Undo();
	virtual void Redo();
	virtual DAVA::Entity* GetEntity() const;
    
    static void DrawToTexture(DAVA::Entity * entity, DAVA::Camera * camera, DAVA::Texture * toTexture, DAVA::int32 fromLodLayer = -1, const DAVA::Rect & viewport = DAVA::Rect(0, 0, -1, -1), bool clearTarget = true);

    DAVA::RenderBatch * GetRenderBatch() const;
    
protected:

    void CreatePlaneImage();
    void CreatePlaneBatch();

    void CreateTextureFiles();
    void DeleteTextureFiles();

    static bool IsHorisontalMesh(const DAVA::AABBox3 & bbox);

    DAVA::LodComponent * lodComponent;
    DAVA::Vector<DAVA::LodComponent::LodDistance> savedDistances;
    
    DAVA::RenderBatch * planeBatch;
    DAVA::Image *planeImage;
    
    DAVA::int32 newLodIndex;
    DAVA::int32 newSwitchIndex;

    DAVA::int32 fromLodLayer;
    DAVA::uint32 textureSize;
    DAVA::FilePath textureSavePath;
};


#endif // #ifndef __CREATE_PLANE_LOD_COOMAND_H__