#ifndef __FRAMEWORKLOOP_H__
#define __FRAMEWORKLOOP_H__


#include <QPointer>
#include <QScopedPointer>

#include "DavaLoop.h"
#include "Platform/Qt5/QtLayer.h"


class QWindow;
class QOpenGLFunctions;
class QOpenGLContext;
class DavaGLWidget;


class FrameworkLoop
    : public LoopItem
    , public DAVA::QtLayerDelegate
    , public DAVA::Singleton< FrameworkLoop >
{
    Q_OBJECT

public:
    FrameworkLoop();
    ~FrameworkLoop();

    void SetOpenGLWindow( DavaGLWidget *w );
    QOpenGLContext * Context();
    quint64 GetRenderContextId() const;


    // QtLayerDelegate
    void Quit() override;

protected:
    // LoopItem
    void ProcessFrameInternal() override;

    
private slots:
    void OnWindowDestroyed();
    void OnWindowInitialized();
    
    void ContextWillBeDestroyed();

private:
    QPointer< QOpenGLContext > context;
    QScopedPointer< QOpenGLFunctions > openGlFunctions;
    QPointer< DavaGLWidget > glWidget;
};



#endif // __FRAMEWORKLOOP_H__
