#include "DavaRenderer.h"
#include "Render/RenderBase.h"

#include "Core/Core.h"
#include "Platform/Qt5/QtLayer.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

#include <QOpenGLContext>
#include <QApplication>

namespace
{
class OGLContextBinder : public DAVA::Singleton<OGLContextBinder>
{
public:
    OGLContextBinder(QSurface* surface, QOpenGLContext* context)
        : davaContext(surface, context)
    {
    }

    void AcquireContext()
    {
        QSurface* prevSurface = nullptr;
        QOpenGLContext* prevContext = QOpenGLContext::currentContext();
        if (prevContext != nullptr)
            prevSurface = prevContext->surface();

        contextStack.emplace(prevSurface, prevContext);

        if (prevContext != davaContext.context)
        {
            davaContext.context->makeCurrent(davaContext.surface);
        }
    }

    void ReleaseContext()
    {
        DVASSERT(!contextStack.empty());
        QOpenGLContext* currentContext = QOpenGLContext::currentContext();

        ContextNode topNode = contextStack.top();
        contextStack.pop();

        if (topNode.context == currentContext)
        {
            return;
        }
        else if (currentContext != nullptr)
        {
            currentContext->doneCurrent();
        }

        if (topNode.context != nullptr && topNode.surface != nullptr)
        {
            topNode.context->makeCurrent(topNode.surface);
        }
    }

private:
    struct ContextNode
    {
        ContextNode(QSurface* surface_ = nullptr, QOpenGLContext* context_ = nullptr)
            : surface(surface_)
            , context(context_)
        {
        }

        QSurface* surface = nullptr;
        QOpenGLContext* context = nullptr;
    };

    ContextNode davaContext;
    DAVA::Stack<ContextNode> contextStack;
};

void AcqureContext()
{
    if (OGLContextBinder::Instance())
    {
        OGLContextBinder::Instance()->AcquireContext();
    }
}

void ReleaseContext()
{
    if (OGLContextBinder::Instance())
    {
        OGLContextBinder::Instance()->ReleaseContext();
    }
}
}

DavaRenderer::DavaRenderer(QSurface* surface, QOpenGLContext* context)
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
    OGLContextBinder::Instance()->Release();
}

void DavaRenderer::paint()
{
    DAVA::QtLayer::Instance()->ProcessFrame();
}

RenderContextGuard::RenderContextGuard()
{
    AcqureContext();
}

RenderContextGuard::~RenderContextGuard()
{
    ReleaseContext();
}
