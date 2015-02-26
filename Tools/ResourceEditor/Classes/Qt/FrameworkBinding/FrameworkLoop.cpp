#include "FrameworkLoop.h"

#include "Platform/Qt5/QtLayer.h"
#include "Render/RenderManager.h"

#include <QWindow>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#if defined( Q_OS_WIN )
#include <QtPlatformheaders/QWGLNativeContext>
#elif defined( Q_OS_MAC )
#endif

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

void FrameworkLoop::SetOpenGLWindow( QWindow* w )
{
    DVASSERT( w != nullptr );
    openGlWindow = w;
    DAVA::QtLayer::Instance()->InitializeGlWindow( GetRenderContextId() );
    connect( w, &QObject::destroyed, this, &FrameworkLoop::onWindowDestroyed );
}

QOpenGLContext* FrameworkLoop::Context()
{
    if ( context.isNull() )
    {
        context = new QOpenGLContext( openGlWindow );

        QSurfaceFormat fmt;
        if ( openGlWindow != nullptr )
        {
            fmt = openGlWindow->requestedFormat();
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

        if ( openGlWindow != nullptr )
        {
            context->makeCurrent( openGlWindow );
        }
        
        openGlFunctions.reset( new QOpenGLFunctions() );
        openGlFunctions->initializeOpenGLFunctions();
    #ifdef Q_OS_WIN
        glewInit();
    #endif
    }
    else if ( openGlWindow != nullptr )
    {
        context->makeCurrent( openGlWindow );
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
    DAVA::QtLayer::Instance()->ProcessFrame();
    if ( openGlWindow != nullptr )
    {
        QEvent updateEvent( QEvent::UpdateRequest );
        QApplication::sendEvent( openGlWindow, &updateEvent );
    }
}

void FrameworkLoop::Quit()
{
    DAVA::RenderManager::Instance()->SetRenderContextId( 0 );
}

void FrameworkLoop::ShowAssertMessage( const char* message )
{
    Q_UNUSED( message );
}

void FrameworkLoop::onWindowDestroyed()
{
    context->makeCurrent( nullptr );
    //auto tt = GetRenderContextId();
    //tt = tt;
    //auto rm = DAVA::RenderManager::Instance();
    //if ( rm != nullptr )
    //    rm->SetRenderContextId( GetRenderContextId() );
}
