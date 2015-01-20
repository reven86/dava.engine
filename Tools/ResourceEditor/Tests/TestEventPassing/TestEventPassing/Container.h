#ifndef CONTAINER_H
#define CONTAINER_H

#include <QWidget>
#include <QPointer>


class Container
    : public QWidget
{
    Q_OBJECT

public:
    Container(QWidget *parent = nullptr);
    ~Container();

private:
    bool event( QEvent *e ) override;

    void keyPressEvent( QKeyEvent * e ) override;
    void keyReleaseEvent( QKeyEvent * e ) override;
    void resizeEvent( QResizeEvent * e ) override;
    void moveEvent( QMoveEvent * e ) override;

    QPointer< QWidget > m_backend;
};

#endif // CONTAINER_H
