#ifndef __DAVAGLWIDGETV2_H__
#define __DAVAGLWIDGETV2_H__


#include <QWidget>
#include <QOpenGLContext>


class DavaGLWidgetV2
    : public QWidget
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
    void paintEvent( QPaintEvent *e ) override;
    QPaintEngine* paintEngine() const override;

    QOpenGLContext *context;
};




#endif // __DAVAGLWIDGETV2_H__
