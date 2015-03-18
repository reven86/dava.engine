#ifndef __DAVALOOP_H__
#define __DAVALOOP_H__


#include <QObject>
#include <QPointer>
#include <QElapsedTimer>
#include <QMap>

#include "Base/Singleton.h"


class QTimer;


class LoopItem
    : public QObject
{
    Q_OBJECT

public:
    explicit LoopItem( QObject *parent = nullptr );
    ~LoopItem();

    void SetMaxFps( int maxFps );
    int MaxFps() const;
    int Fps() const;

    virtual void ProcessFrame();

private:
    int maxFps;
    int fps;
};


class DavaLoop
    : public QObject
    , public DAVA::Singleton< DavaLoop >
{
    Q_OBJECT

private:
    using TimerMap = QMap< QTimer *, QPointer< LoopItem > >;

public:
    DavaLoop();
    ~DavaLoop();

    void StartLoop( LoopItem *item );
    void StopLoop( LoopItem *item );

private slots:
    void ProcessItem();
    void OnItemDestroyed();

private:
    TimerMap loops;
};



#endif // __DAVALOOP_H__
