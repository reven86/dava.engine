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


#include "VisibilityToolProxy.h"

VisibilityToolProxy::VisibilityToolProxy(int32 size)
    : size(size)
    , visibilityPoint(Vector2(-1.f, -1.f))
    , isVisibilityPointSet(false)
{
    visibilityToolTexture = Texture::CreateFBO((uint32)size, (uint32)size, FORMAT_RGBA8888);

    rhi::Viewport viewport;
    viewport.x = viewport.y = 0U;
    viewport.width = (uint32)size;
    viewport.height = (uint32)size;
    RenderHelper::CreateClearPass(visibilityToolTexture->handle, PRIORITY_CLEAR, Color(0.f, 0.f, 0.f, 0.f), viewport);
}

VisibilityToolProxy::~VisibilityToolProxy()
{
	SafeRelease(visibilityToolTexture);
}

int32 VisibilityToolProxy::GetSize()
{
	return size;
}

Texture* VisibilityToolProxy::GetTexture()
{
	return visibilityToolTexture;
}

void VisibilityToolProxy::SetVisibilityPoint(const Vector2& visibilityPoint)
{
	this->visibilityPoint = visibilityPoint;
}

Vector2 VisibilityToolProxy::GetVisibilityPoint()
{
	return visibilityPoint;
}

bool VisibilityToolProxy::IsVisibilityPointSet()
{
	return isVisibilityPointSet;
}

void VisibilityToolProxy::UpdateVisibilityPointSet(bool visibilityPointSet)
{
	isVisibilityPointSet = visibilityPointSet;
}
