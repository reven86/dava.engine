#ifndef __FRAMEWORKLOOP_H__
#define __FRAMEWORKLOOP_H__


#include <QPointer>

#include "DavaLoop.h"
#include "Platform/Qt5/QtLayer.h"


class DavaGLWidgetV2;

class FrameworkLoop
    : public LoopItem
    , public DAVA::QtLayerDelegate
{
    Q_OBJECT

public:
    FrameworkLoop();
    ~FrameworkLoop();

    void SetGLWidget( QWidget *w );

    // LoopItem
    void ProcessFrame() override;

    // QtLayerDelegate
    void Quit() override;
    void ShowAssertMessage( const char* message ) override;

private:
    QPointer< DavaGLWidgetV2 > glWidget;
};



#endif // __FRAMEWORKLOOP_H__
