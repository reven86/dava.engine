#include "DavaGLWidgetV2.h"

#include <QOpenGLContext>


#include "Platform/Qt5/Win32/QtLayerWin32.h"


DavaGLWidgetV2::DavaGLWidgetV2( QWidget* parent )
    : QOpenGLWidget( parent )
{
}

DavaGLWidgetV2::~DavaGLWidgetV2()
{
}

void DavaGLWidgetV2::Init()
{
    QOpenGLContext *c = context();
    const bool result = c->create();
    DAVA::QtLayerWin32 *qtLayer = static_cast< DAVA::QtLayerWin32 * >( DAVA::QtLayer::Instance() );
}