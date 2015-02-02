#include "FrameworkLoop.h"

#include "Platform/Qt5/QtLayer.h"
#include "Render/RenderManager.h"

#include "DavaGLWidgetV2.h"


FrameworkLoop::FrameworkLoop()
    : LoopItem()
{
    SetMaxFps( 60 );
}

FrameworkLoop::~FrameworkLoop()
{
}

void FrameworkLoop::ProcessFrame()
{
    const bool isWidgetPresent = !glWidget.isNull();

    if ( isWidgetPresent )
    {
        glWidget->BeginFrame();
        //DAVA::RenderManager::Instance()->SetRenderContextId( glWidget->GetContextId() );
    }

    DAVA::QtLayer::Instance()->ProcessFrame();

    if ( isWidgetPresent )
    {
        glWidget->EndFrame();
    }
}

void FrameworkLoop::Quit()
{
}

void FrameworkLoop::ShowAssertMessage( const char* message )
{
}

void FrameworkLoop::SetGLWidget( QWidget *w )
{
    glWidget = qobject_cast<DavaGLWidgetV2 *>( w );
}
