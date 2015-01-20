#ifndef BACKEND_H
#define BACKEND_H

#include <QWidget>

class Backend
    : public QWidget
{
    Q_OBJECT

public:
    Backend( QWidget *parent = nullptr );
    ~Backend();

private:
    bool event( QEvent *e ) override;
    void paintEvent( QPaintEvent *e ) override;

    void keyPressEvent( QKeyEvent * e ) override;
    void keyReleaseEvent( QKeyEvent * e ) override;
};

#endif // BACKEND_H
