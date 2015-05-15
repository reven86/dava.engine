#include "FrameworkLoop.h"

#include "Platform/Qt5/QtLayer.h"

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
    if ( context.isNull() )
    {
        context = new QOpenGLContext( glWidget );

        QSurfaceFormat fmt;
        if ( glWidget != nullptr )
        {
            fmt = glWidget->GetGLWindow()->requestedFormat();
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
#if RHI_COMPLETE_EDITOR        
    #ifdef Q_OS_WIN
        glewInit();
    #endif
#endif // RHI_COMPLETE_EDITOR
    }
    else if ( glWidget != nullptr )
    {
        context->makeCurrent( glWidget->GetGLWindow() );
    }

    return context.data();
}

quint64 FrameworkLoop::GetRenderContextId() const
{
    if ( context.isNull() )
        return 0;

    quint64 id = 0;

#if defined( Q_OS_WIN )
    QWGLNativeContext nativeContext = context->nativeHandle().value< QWGLNativeContext >();
    id = reinterpret_cast<quint64>( nativeContext.context() );
#elif defined( Q_OS_MAC )
    // TODO: fix includes / compilation
    //QCocoaNativeContext nativeContext = context->nativeHandle().value< QCocoaNativeContext >();
    //id = reinterpret_cast<quint64>( nativeContext.context()->CGLContextObj() );
    id = reinterpret_cast<quint64>( CGLGetCurrentContext() );
#endif

    return id;
}

void FrameworkLoop::ProcessFrame()
{
    // We need to call makeCurrent, because there is a crash in native OS X open file dialog
#if RHI_COMPLETE_EDITOR
    if ( glWidget != nullptr && DAVA::RenderManager::Instance()->GetRenderContextId() != GetRenderContextId() )
    {
        context->makeCurrent(glWidget->GetGLWindow());
    }
#endif // RHI_COMPLETE_EDITOR
    
    DAVA::QtLayer::Instance()->ProcessFrame();
    if ( glWidget != nullptr )
    {
        QEvent updateEvent( QEvent::UpdateRequest );
        QApplication::sendEvent( glWidget->GetGLWindow(), &updateEvent );
    }
}

void FrameworkLoop::Quit()
{
#if RHI_COMPLETE_EDITOR
    DAVA::RenderManager::Instance()->SetRenderContextId( 0 );
#endif // RHI_COMPLETE_EDITOR
}

void FrameworkLoop::OnWindowDestroyed()
{
    context->makeCurrent( nullptr );
}

void FrameworkLoop::OnWindowInitialized()
{
    DAVA::QtLayer::Instance()->InitializeGlWindow( GetRenderContextId() );
    DAVA::QtLayer::Instance()->OnResume();
}