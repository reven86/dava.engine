#include "DavaGLWidgetV2.h"

#include <QWindow>
#include <QOpenGLContext>
#include <QDebug>
#include <QtPlatformheaders/QWGLNativeContext>

#include "Platform/Qt5/Win32/QtLayerWin32.h"
#include "Debug/DVAssert.h"


DavaGLWidgetV2::DavaGLWidgetV2( QWidget* parent )
    : QWidget( parent )
    , context( nullptr )
{
    setAttribute( Qt::WA_NativeWindow );
    setAttribute( Qt::WA_OpaquePaintEvent );
    //setAttribute( Qt::WA_ );
}

DavaGLWidgetV2::~DavaGLWidgetV2()
{
}

void DavaGLWidgetV2::Init()
{
    context = new QOpenGLContext();

    QSurfaceFormat format;
    format.setVersion( 2, 0 );
    format.setProfile( QSurfaceFormat::CompatibilityProfile );
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
    
    context->setFormat( format );
    const bool result = context->create();
    DVASSERT( result );

    context->makeCurrent( this->windowHandle() );

    BeginFrame();

    const int w = width();
    const int h = height();
    
    DAVA::QtLayerWin32 *qtLayer = static_cast< DAVA::QtLayerWin32 * >( DAVA::QtLayer::Instance() );

    qtLayer->SetupWidget( GetContextId(), w, h );
    qtLayer->OnResume();

    EndFrame();
}

quint64 DavaGLWidgetV2::GetContextId() const
{
    QWGLNativeContext nativeContext = context->nativeHandle().value< QWGLNativeContext >();
    const quint64 contextId = reinterpret_cast<quint64>( nativeContext.context() );

    return contextId;
}

void DavaGLWidgetV2::BeginFrame()
{
    //context->makeCurrent( this->windowHandle() );
}

void DavaGLWidgetV2::EndFrame()
{
    //context->doneCurrent();
}

void DavaGLWidgetV2::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );
}

QPaintEngine* DavaGLWidgetV2::paintEngine() const
{
    return nullptr;
}