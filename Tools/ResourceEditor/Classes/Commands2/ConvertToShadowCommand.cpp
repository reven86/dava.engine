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


#include "ConvertToShadowCommand.h"

ConvertToShadowCommand::ConvertToShadowCommand(DAVA::RenderBatch *batch)
    : Command2(CMDID_CONVERT_TO_SHADOW, "Convert To Shadow")
	, oldBatch(batch)
	, newBatch(NULL)
{
	DVASSERT(oldBatch);

	renderObject = DAVA::SafeRetain(oldBatch->GetRenderObject());
	DVASSERT(renderObject);

	oldBatch->Retain();

    newBatch = new DAVA::RenderBatch();
    DAVA::PolygonGroup * shadowPg = DAVA::MeshUtils::CreateShadowPolygonGroup(oldBatch->GetPolygonGroup());
    newBatch->SetPolygonGroup(shadowPg);
    shadowPg->Release();

    DAVA::NMaterial * shadowMaterial = new DAVA::NMaterial();
    shadowMaterial->SetMaterialName(DAVA::FastName("Shadow_Material"));
    shadowMaterial->SetFXName(DAVA::NMaterialName::SHADOW_VOLUME);

    newBatch->SetMaterial(shadowMaterial);

    shadowMaterial->Release();
}

ConvertToShadowCommand::~ConvertToShadowCommand()
{
	DAVA::SafeRelease(oldBatch);
	DAVA::SafeRelease(newBatch);
    DAVA::SafeRelease(renderObject);
}

void ConvertToShadowCommand::Redo()
{
    renderObject->ReplaceRenderBatch(oldBatch, newBatch);
}

void ConvertToShadowCommand::Undo()
{
    renderObject->ReplaceRenderBatch(newBatch, oldBatch);
}

DAVA::Entity* ConvertToShadowCommand::GetEntity() const
{
    return NULL;
}

bool ConvertToShadowCommand::CanConvertBatchToShadow(DAVA::RenderBatch *renderBatch)
{
    if(renderBatch && renderBatch->GetMaterial())
    {
        return renderBatch->GetMaterial()->GetEffectiveFXName() != DAVA::NMaterialName::SHADOW_VOLUME;
    }

	return false;
}
