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


#include "Render/Highlevel/LandscapeChunk.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA
{

LandscapeChunk::LandscapeChunk(Landscape * _landscape)
    : landscape(_landscape)
{
    //SetOwnerLayerName(LAYER_OPAQUE);
}
    
LandscapeChunk::~LandscapeChunk()
{
    
}
    
void LandscapeChunk::Draw(const FastName & ownerPassName, Camera * camera)
{
	if(NULL != landscape)
	{
        landscape->BindDynamicParameters(camera);
		landscape->Draw(camera);
	}
}

void LandscapeChunk::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    RenderBatch::Save(archive, serializationContext);
}
 
void LandscapeChunk::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    RenderBatch::Load(archive, serializationContext);
}

};
