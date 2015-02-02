#ifndef __DAVAGLWIDGETV2_H__
#define __DAVAGLWIDGETV2_H__


#include <QOpenGLWidget>
#include <QOpenGLFunctions>


class DavaGLWidgetV2
    : public QOpenGLWidget
    //, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit DavaGLWidgetV2( QWidget *parent = nullptr );
    ~DavaGLWidgetV2();

    void Init();
    quint64 GetContextId() const;

    void BeginFrame();
    void EndFrame();

private:
    void initializeGL() override;
};




#endif // __DAVAGLWIDGETV2_H__
