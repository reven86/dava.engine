#include "DockLODEditor/DistanceSlider.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Functional/Function.h"
#include "Scene3D/Entity.h"
#include "Utils/StringFormat.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QSplitter>

namespace DistanceSliderDetails
{
const DAVA::Vector<QColor> backgroundColors =
{
  Qt::green,
  Qt::red,
  Qt::yellow,
  Qt::blue
};

const int MIN_WIDGET_WIDTH = 1;

DAVA::int32 RoundFloat32(DAVA::float32 value)
{
    return static_cast<DAVA::int32>(DAVA::Round(value));
}
}

DistanceSlider::DistanceSlider(QWidget* parent /*= 0*/)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setObjectName(QString::fromUtf8("layout"));

    splitter = new QSplitter(this);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setGeometry(geometry());
    splitter->setOrientation(Qt::Horizontal);
    splitter->setMinimumHeight(20);
    splitter->setChildrenCollapsible(false);
    splitter->installEventFilter(this);

    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    connect(splitter, &QSplitter::splitterMoved, this, &DistanceSlider::SplitterMoved);
}

void DistanceSlider::SetFramesCount(DAVA::uint32 count)
{
    DVASSERT(framesCount == 0);
    DVASSERT(count <= DistanceSliderDetails::backgroundColors.size());

    framesCount = count;

    frames.reserve(framesCount);
    realDistances.resize(framesCount, 0.0f);

    for (DAVA::uint32 i = 0; i < framesCount; ++i)
    {
        QFrame* frame = new QFrame(splitter);

        frame->setObjectName(QString::fromUtf8(DAVA::Format("frame_%d", i).c_str()));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);

        QPalette pallete;
        pallete.setColor(QPalette::Background, DistanceSliderDetails::backgroundColors[i]);

        frame->setPalette(pallete);
        frame->setAutoFillBackground(true);
        frame->setMinimumWidth(DistanceSliderDetails::MIN_WIDGET_WIDTH);

        splitter->addWidget(frame);

        frames.push_back(frame);
    }

    if (framesCount > 0)
    {
        //install event filter for handlers
        DAVA::uint32 handlesCount = static_cast<DAVA::uint32>(splitterHandles.size());
        for (DAVA::uint32 i = handlesCount; i < framesCount; i++)
        {
            QObject* obj = splitter->handle(static_cast<int>(i));
            obj->installEventFilter(this);
            splitterHandles.push_back(obj);
        }
    }
}

void DistanceSlider::SetLayersCount(DAVA::uint32 count)
{
    layersCount = count;
    for (DAVA::uint32 i = 0; i < framesCount; ++i)
    {
        frames[i]->setVisible(i < layersCount);
    }
}

const DAVA::Vector<DAVA::float32>& DistanceSlider::GetDistances() const
{
    return realDistances;
}

void DistanceSlider::SetDistances(const DAVA::Vector<DAVA::float32>& distances_)
{
    DVASSERT(distances_.size() == DAVA::LodComponent::MAX_LOD_LAYERS);

    if (distances_ == realDistances || layersCount == 0)
    {
        return;
    }

    DAVA::uint32 distancesCount = static_cast<DAVA::uint32>(DAVA::Min(realDistances.size(), distances_.size()));
    for (DAVA::uint32 layer = 0; layer < distancesCount; ++layer)
    {
        realDistances[layer] = DAVA::Round(distances_[layer]);
    }

    BuildUIFromDistances();
}

void DistanceSlider::BuildUIFromDistances()
{
    const int splitterWidth = splitter->geometry().width() - splitter->handleWidth() * (layersCount - 1);

    const DAVA::float32 scaleSize = GetScaleSize();
    const DAVA::float32 widthCoef = splitterWidth / scaleSize;

    QList<int> sizes;
    DAVA::int32 lastLayerSize = 0;
    for (DAVA::uint32 i = 1; i < layersCount; ++i)
    {
        DAVA::int32 prevSize = lastLayerSize;
        lastLayerSize = DistanceSliderDetails::RoundFloat32(realDistances[i] * widthCoef);
        sizes.push_back(lastLayerSize - prevSize);
    }
    sizes.push_back(splitterWidth - lastLayerSize);

    for (DAVA::uint32 i = layersCount; i < framesCount; ++i)
    {
        sizes.push_back(DistanceSliderDetails::MIN_WIDGET_WIDTH);
    }

    QSignalBlocker guard(splitter);
    splitter->setSizes(sizes);
}

void DistanceSlider::SplitterMoved(int pos, int index)
{
    DVASSERT(layersCount > 0)

    const int splitterWidth = splitter->geometry().width() - splitter->handleWidth() * (layersCount - 1);
    const DAVA::float32 scaleSize = GetScaleSize();
    const DAVA::float32 widthCoef = scaleSize / splitterWidth;

    QList<int> sizes = splitter->sizes();
    int sz = 0;
    for (int i = 0; (i < int(sizes.size())) && (i + 1 < int(layersCount)); ++i)
    {
        sz += sizes.at(i);
        realDistances[i + 1] = DistanceSliderDetails::RoundFloat32(sz * widthCoef);
    }

    emit DistanceHandleMoved();
}

DAVA::float32 DistanceSlider::GetScaleSize() const
{
    return DAVA::LodComponent::MAX_LOD_DISTANCE;
}

bool DistanceSlider::eventFilter(QObject* obj, QEvent* e)
{
    bool retValue = QWidget::eventFilter(obj, e);
    if (e->type() == QEvent::MouseButtonRelease)
    {
        if (std::find(splitterHandles.begin(), splitterHandles.end(), obj) != splitterHandles.end())
        {
            emit DistanceHandleReleased();
        }
    }
    if (e->type() == QEvent::Resize && obj == splitter)
    {
        BuildUIFromDistances();
    }

    return retValue;
}
