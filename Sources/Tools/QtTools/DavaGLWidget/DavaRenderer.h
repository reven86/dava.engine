#pragma once

#if !defined(__DAVAENGINE_COREV2__)

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
POP_QT_WARNING_SUPRESSOR

class QSurface;
class QOpenGLContext;

class DavaRenderer : public QObject
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
public:
    DavaRenderer(QSurface* surface, QOpenGLContext* context);
    ~DavaRenderer() override;
public slots:
    void paint();
};
class RenderContextGuard
{
public:
    RenderContextGuard();
    ~RenderContextGuard();
};

#endif // __DAVAENGINE_COREV2__