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


#include "FrameworkLoop.h"

#include "Platform/Qt5/QtLayer.h"
#include "Core/Core.h"

#include "Render/RenderBase.h"


#include <QWindow>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#if defined( Q_OS_WIN )
#include <QtPlatformheaders/QWGLNativeContext>
#elif defined( Q_OS_MAC )
#endif

#include "QtTools/DavaGLWidget/davaglwidget.h"


FrameworkLoop::FrameworkLoop()
    : LoopItem()
{
    SetMaxFps( 60 );

    DAVA::QtLayer::Instance()->SetDelegate( this );
}

FrameworkLoop::~FrameworkLoop()
{
    DAVA::QtLayer::Instance()->SetDelegate( nullptr );
}

void FrameworkLoop::SetOpenGLWindow( DavaGLWidget* w )
{
    DVASSERT( w != nullptr );
    glWidget = w;

    connect( w, &QObject::destroyed, this, &FrameworkLoop::OnWindowDestroyed );
    connect( w, &DavaGLWidget::Initialized, this, &FrameworkLoop::OnWindowInitialized );
}

QOpenGLContext* FrameworkLoop::Context()
{
	if (context.isNull())
    {
        context = new QOpenGLContext( glWidget );

        QSurfaceFormat fmt;
        if ( glWidget != nullptr )
        {
            fmt = glWidget->GetGLWindow()->requestedFormat();

            QObject::connect(context, &QOpenGLContext::aboutToBeDestroyed, this, &FrameworkLoop::ContextWillBeDestroyed);
        }

        fmt.setOption( fmt.options() | QSurfaceFormat::DebugContext );

        fmt.setRenderableType( QSurfaceFormat::OpenGL );
        fmt.setVersion( 2, 0 );
        fmt.setDepthBufferSize( 24 );
        fmt.setStencilBufferSize( 8 );
        fmt.setSwapInterval( 1 );
        fmt.setSwapBehavior( QSurfaceFormat::DoubleBuffer );

        context->setFormat( fmt );
        context->create();

        if ( glWidget != nullptr )
        {
            context->makeCurrent( glWidget->GetGLWindow() );
        }
        
        openGlFunctions.reset( new QOpenGLFunctions( context ) );
        openGlFunctions->initializeOpenGLFunctions();
    #ifdef Q_OS_WIN
        glewInit();
    #endif
    }
    else if ( glWidget != nullptr )
    {
        context->makeCurrent( glWidget->GetGLWindow() );
    }

    return context.data();
}

void FrameworkLoop::DoneContext()
{
    if (!context.isNull())
    {
        context->doneCurrent();
    }
}


void FrameworkLoop::ProcessFrameInternal()
{
    if (nullptr != context && nullptr != glWidget)
    {
        context->makeCurrent(glWidget->GetGLWindow());
    }
    DAVA::QtLayer::Instance()->ProcessFrame();

    if (glWidget != nullptr)
    {
        QEvent updateEvent(QEvent::UpdateRequest);
        QApplication::sendEvent(glWidget->GetGLWindow(), &updateEvent);
    }
}

void FrameworkLoop::Quit()
{
}

void FrameworkLoop::OnWindowDestroyed()
{
    context->makeCurrent( nullptr );
}

void MakeCurrentGL()
{
    FrameworkLoop::Instance()->Context();
}

void DoneGLContext()
{
    FrameworkLoop::Instance()->DoneContext();
}

void FrameworkLoop::OnWindowInitialized()
{
    DAVA::Core::Instance()->rendererParams.acquireContextFunc = &MakeCurrentGL;
    DAVA::Core::Instance()->rendererParams.releaseContextFunc = &DoneGLContext;

    DAVA::QtLayer::Instance()->AppStarted();

    DAVA::QtLayer::Instance()->OnResume();
}

void FrameworkLoop::ContextWillBeDestroyed()
{
    DAVA::Logger::FrameworkDebug("[FrameworkLoop::%s]", __FUNCTION__);
}
