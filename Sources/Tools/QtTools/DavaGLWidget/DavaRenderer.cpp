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

#include "DAVAEngine.h"

#include "Platform/Qt5/QtLayer.h"
#include "DavaRenderer.h"

#include "Base/Singleton.h"

#include <QOpenGLContext>

namespace
{

class OGLContextBinder: public DAVA::Singleton<OGLContextBinder>
{
public:
    OGLContextBinder(QSurface * surface, QOpenGLContext * context)
        : renderSurface(surface)
        , renderContext(context)
    {
    }

    ~OGLContextBinder()
    {
    }

    void AcquireContex()
    {
        prevContext = QOpenGLContext::currentContext();
        if (prevContext != nullptr)
            prevSurface = prevContext->surface();
        else
            prevSurface = nullptr;

        renderContext->makeCurrent(renderSurface);
    }

    void ReleaseContext()
    {
        renderContext->doneCurrent();

        if (prevContext != nullptr && prevSurface != nullptr)
            prevContext->makeCurrent(prevSurface);
    }

private:
    QSurface * renderSurface = nullptr;
    QOpenGLContext * renderContext = nullptr;

    QSurface * prevSurface = nullptr;
    QOpenGLContext * prevContext = nullptr;
};

void AcqureContext()
{
    OGLContextBinder::Instance()->AcquireContex();
}

void ReleaseContext()
{
    OGLContextBinder::Instance()->ReleaseContext();
}

}

DavaRenderer::DavaRenderer(QSurface * surface, QOpenGLContext * context)
{
    DVASSERT(OGLContextBinder::Instance() == nullptr);
    new OGLContextBinder(surface, context);

    DAVA::Core::Instance()->rendererParams.acquireContextFunc = &AcqureContext;
    DAVA::Core::Instance()->rendererParams.releaseContextFunc = &ReleaseContext;

    DAVA::QtLayer::Instance()->AppStarted();
    DAVA::QtLayer::Instance()->OnResume();
}

DavaRenderer::~DavaRenderer()
{
    DAVA::QtLayer::Instance()->Release();}

void DavaRenderer::paint()
{
    DAVA::QtLayer::Instance()->ProcessFrame();
}