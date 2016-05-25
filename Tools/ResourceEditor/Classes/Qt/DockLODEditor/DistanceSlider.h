#ifndef __DISTANCE_SLIDER_H__
#define __DISTANCE_SLIDER_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Lod/LodComponent.h"

#include <QWidget>

class QSplitter;
class QFrame;

class LazyUpdater;

class DistanceSlider : public QWidget
{
    Q_OBJECT

public:
    DistanceSlider(QWidget* parent = 0);

    void SetFramesCount(DAVA::uint32 count);
    DAVA::uint32 GetFramesCount() const;

    void SetLayersCount(DAVA::uint32 count);
    DAVA::uint32 GetLayersCount() const;

    void SetDistances(const DAVA::Vector<DAVA::float32>& distances);
    const DAVA::Vector<DAVA::float32>& GetDistances() const;

signals:
    void DistanceHandleMoved();
    void DistanceHandleReleased();

protected slots:
    void SplitterMoved(int pos, int index);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    DAVA::float32 GetScaleSize() const;
    void BuildUIFromDistances();

private:
    QSplitter* splitter = nullptr;
    DAVA::Vector<QObject*> splitterHandles;

    DAVA::Vector<QFrame*> frames;
    DAVA::Vector<DAVA::float32> realDistances;

    DAVA::uint32 layersCount = 0;
    DAVA::uint32 framesCount = 0;
};

inline DAVA::uint32 DistanceSlider::GetLayersCount() const
{
    return layersCount;
}

inline DAVA::uint32 DistanceSlider::GetFramesCount() const
{
    return framesCount;
}

#endif // __DISTANCE_SLIDER_H__
