#ifndef __FRAMEWORKLOOP_H__
#define __FRAMEWORKLOOP_H__


#include <QPointer>
#include <QScopedPointer>

#include "DavaLoop.h"
#include "Platform/Qt5/QtLayer.h"


class QWindow;
class QOpenGLFunctions;
class QOpenGLContext;


class FrameworkLoop
    : public LoopItem
    , public DAVA::QtLayerDelegate
    , public DAVA::Singleton< FrameworkLoop >
{
    Q_OBJECT

public:
    FrameworkLoop();
    ~FrameworkLoop();

    void SetOpenGLWindow( QWindow *w );
    QOpenGLContext * Context();
    quint64 GetRenderContextId() const;

    // LoopItem
    void ProcessFrame() override;

    // QtLayerDelegate
    void Quit() override;
    void ShowAssertMessage( const char* message ) override;

private slots:
    void onWindowDestroyed();

private:
    QPointer< QOpenGLContext > context;
    QScopedPointer< QOpenGLFunctions > openGlFunctions;
    QPointer< QWindow > openGlWindow;
};



#endif // __FRAMEWORKLOOP_H__
