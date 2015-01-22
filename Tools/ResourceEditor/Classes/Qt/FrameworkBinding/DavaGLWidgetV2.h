#ifndef __DAVAGLWIDGETV2_H__
#define __DAVAGLWIDGETV2_H__


#include <QOpenGLWidget>


class DavaGLWidgetV2
    : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit DavaGLWidgetV2( QWidget *parent = nullptr );
    ~DavaGLWidgetV2();

    void Init();

private:
};




#endif // __DAVAGLWIDGETV2_H__
