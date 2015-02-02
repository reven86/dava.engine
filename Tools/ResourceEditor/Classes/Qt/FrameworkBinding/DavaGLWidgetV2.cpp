#include "DavaGLWidgetV2.h"

#include <QOpenGLContext>
#include <QDebug>
#include <QtPlatformheaders/QWGLNativeContext>

#include "Platform/Qt5/Win32/QtLayerWin32.h"
#include "Debug/DVAssert.h"


DavaGLWidgetV2::DavaGLWidgetV2( QWidget* parent )
    : QOpenGLWidget( parent )
{
    QSurfaceFormat format;
    format.setVersion( 2, 0 );
    format.setProfile( QSurfaceFormat::CompatibilityProfile );
    format.setDepthBufferSize( 24 );
    format.setStencilBufferSize( 8 );
    format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
    
    //setUpdateBehavior( QOpenGLWidget::PartialUpdate );

    setFormat( format );
}

DavaGLWidgetV2::~DavaGLWidgetV2()
{
}

void DavaGLWidgetV2::initializeGL()
{
    //initializeOpenGLFunctions();
    //QOpenGLWidget::initializeGL();

    QOpenGLContext *c = context();
    const bool result = c->create();
    DVASSERT( result );
}

void DavaGLWidgetV2::Init()
{
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
    QOpenGLContext *c = context();
    QWGLNativeContext nativeContext = c->nativeHandle().value< QWGLNativeContext >();
    const quint64 contextId = reinterpret_cast<quint64>( nativeContext.context() );

    return contextId;
}

void DavaGLWidgetV2::BeginFrame()
{
    makeCurrent();
}

void DavaGLWidgetV2::EndFrame()
{
    doneCurrent();
}