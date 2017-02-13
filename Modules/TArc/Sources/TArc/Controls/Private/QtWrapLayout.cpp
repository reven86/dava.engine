#include "TArc/Controls/QtWrapLayout.h"

#include <Debug/DVAssert.h>

#include <private/qlayout_p.h>
#include <private/qlayoutengine_p.h>

#include <QSize>
#include <QtGlobal>

namespace DAVA
{
namespace TArc
{
namespace QtWrapLayoutDetails
{
template <size_t count>
void ResetBitSet(Bitset<count>& bits, std::initializer_list<bool> values)
{
    DVASSERT(values.size() == count);
    size_t index = 0;
    for (bool v : values)
    {
        bits[index] = v;
        ++index;
    }
}
}

struct QtWrapLayoutItem
{
    QtWrapLayoutItem(QLayoutItem* i, int32 stretch)
        : item(i)
    {
    }

    ~QtWrapLayoutItem()
    {
        delete item;
    }

    bool HasHeightForWidth() const
    {
        return item->hasHeightForWidth();
    }

    int32 HeightForWidth(int32 width) const
    {
        return item->heightForWidth(width);
    }

    int32 MinimumHeightForWidth(int32 width) const
    {
        return item->minimumHeightForWidth(width);
    }

    Qt::Orientations ExpandingDirections() const
    {
        return item->expandingDirections();
    }

    int32 GetStretch() const
    {
        QWidget* w = item->widget();
        if (stretch == 0 && w != nullptr)
        {
            return w->sizePolicy().verticalStretch();
        }

        return stretch;
    }

    void SetGeometry(const QRect& r)
    {
        item->setGeometry(r);
    }

    QRect GetGeometry() const
    {
        return item->geometry();
    }

    void Update()
    {
        minSize = item->minimumSize();
        sizeHint = item->sizeHint();
        maxSize = item->maximumSize();
        if ((ExpandingDirections() & Qt::Horizontal) == 0)
        {
            maxSize.setWidth(sizeHint.width());
        }
    }

    QLayoutItem* item;
    int32 rowIndex = 0;
    int32 columnIndex = 0;
    int32 stretch = 0;

    QSize minSize;
    QSize sizeHint;
    QSize maxSize;
};

class QtWrapLayoutPrivate : public QLayoutPrivate
{
    Q_DECLARE_PUBLIC(QtWrapLayout)
public:
    Vector<QtWrapLayoutItem*> items;
    Vector<int32> layoutColumnWidths;
    Vector<int32> layoutRowHeights;

    void Layout();
    void Layout(int32 width);
    void UpdateSizes();
    void CalcRanges(Vector<std::pair<size_t, size_t>>& ranges, const std::pair<size_t, size_t>& testedRange, int32 spacing);

    int32 layoutWidth; // width last layout calculation was made for

    QSize minSize; // calculated based on minimum size
    QSize preferedSize; // calculated based on size hint
    int32 hSpacing = 6;
    int32 vSpacing = 6;

    enum Flags
    {
        SizeDirty,
        Dirty,
        ExpandHorizontal,
        ExpandVertical,
        Count
    };
    Bitset<Count> flags;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                               QtWrapLayoutPrivate implementation                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void QtWrapLayoutPrivate::CalcRanges(Vector<std::pair<size_t, size_t>>& ranges, const std::pair<size_t, size_t>& testedRange, int32 spacing)
{
    auto placeOnSeparateLine = [](Vector<std::pair<size_t, size_t>>& ranges, const std::pair<size_t, size_t>& testedRange)
    {
        ranges.reserve(testedRange.second - testedRange.first);
        for (size_t i = testedRange.first; i < testedRange.second; ++i)
        {
            ranges.push_back(std::make_pair(i, i + 1));
        }
    };

    int32 fullRowMinWidth = 0;
    for (size_t i = testedRange.first; i < testedRange.second; ++i)
    {
        fullRowMinWidth += (items[i]->minSize.width() + spacing);
    }
    fullRowMinWidth = Max(0, fullRowMinWidth - spacing);

    if (fullRowMinWidth <= layoutWidth)
    {
        ranges.push_back(testedRange);
    }
    else
    {
        size_t count = testedRange.second - testedRange.first;
        if ((count & 0x1) == 0)
        {
            size_t half = count >> 1;
            std::pair<size_t, size_t> leftSubRange(testedRange.first, testedRange.first + half);
            std::pair<size_t, size_t> rightSubRange(leftSubRange.second, testedRange.second);
            Vector<std::pair<size_t, size_t>> leftSubRanges;
            Vector<std::pair<size_t, size_t>> rightSubRanges;
            CalcRanges(leftSubRanges, leftSubRange, spacing);
            CalcRanges(rightSubRanges, rightSubRange, spacing);
            if (leftSubRanges.size() == rightSubRanges.size())
            {
                ranges.reserve(leftSubRanges.size() + rightSubRanges.size());
                ranges = leftSubRanges;
                ranges.insert(ranges.end(), rightSubRanges.begin(), rightSubRanges.end());
            }
            else
            {
                // sub ranges sizes not equal, it means that subdivision depth is different and we can't divide items
                // on 2 equal parts, so we will place each item on separate line
                placeOnSeparateLine(ranges, testedRange);
            }
        }
        else
        {
            // if we can't divide items on 2 equal parts, we should place items each on separate line
            placeOnSeparateLine(ranges, testedRange);
        }
    }
}

void QtWrapLayoutPrivate::Layout(int32 width)
{
    Q_Q(QtWrapLayout);

    // Early out if we have no changes that would cause a change in vertical layout
    if (width == layoutWidth && flags[Dirty] == false && flags[SizeDirty] == false)
    {
        return;
    }

    layoutWidth = width;
    layoutColumnWidths.clear();
    layoutRowHeights.clear();

    int32 userVSpacing = q->GetVerticalSpacing();
    int32 userHSpacing = q->GetHorizontalSpacing();
    DVASSERT(userVSpacing >= 0);
    DVASSERT(userHSpacing >= 0);

    // make sure our sizes are up to date
    UpdateSizes();
    Vector<std::pair<size_t, size_t>> ranges;
    CalcRanges(ranges, std::make_pair(0, items.size()), userHSpacing);

    if (shWidthWithSpacing < layoutWidth || minWidthWithSpacing < layoutWidth)
    {
        // layout into one line
        int32 maxMinHeight = 0; // maximum value of item->minimumHeightForWidth()
        int32 maxSizeHintHeight = 0; // maximum value of item->heightForWidth()

        size_t itemCount = items.size();
        minColumnWidths.resize(itemCount);
        sizeHintColumnWidths.resize(itemCount);

        // layout into one line
        QVector<QLayoutStruct> layoutStructs;
        layoutStructs.reserve(itemCount);
        for (QtWrapLayoutItem* item : items)
        {
            layoutStructs.push_back(QLayoutStruct());
            QLayoutStruct& layoutStruct = layoutStructs.back();
            layoutStruct.init(item->GetStretch(), item->minSize.width());
            layoutStruct.sizeHint = item->sizeHint.width();
            layoutStruct.maximumSize = item->maxSize.width();
            layoutStruct.expansive = (item->ExpandingDirections() & Qt::Horizontal);
            layoutStruct.empty = false;
        }

        qGeomCalc(layoutStructs, 0, layoutStructs.size(), 0, layoutWidth);
        for (size_t i = 0; i < itemCount; ++i)
        {
            QtWrapLayoutItem* item = items[i];
            if (item->HasHeightForWidth())
            {
                maxMinHeight = qMax(item->minSize.height(), maxMinHeight);
                maxSizeHintHeight = qMax(item->sizeHint.height(), maxSizeHintHeight);
            }
            else
            {
                maxMinHeight = qMax(item->minSize.height(), maxMinHeight);
                maxSizeHintHeight = qMax(item->sizeHint.height(), maxSizeHintHeight);
            }
        }

        minRowHeights.resize(1);
        sizeHintRowHeights.resize(1);
        minRowHeights[0] = maxMinHeight;
        sizeHintRowHeights[0] = maxSizeHintHeight;
    }
}

void QtWrapLayoutPrivate::Layout()
{
    Layout(QLAYOUTSIZE_MAX);
}

void QtWrapLayoutPrivate::CalcSizeHint()
{
    Q_Q(QtWrapLayout);

    int leftMargin, topMargin, rightMargin, bottomMargin;
    q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    Layout(QLAYOUTSIZE_MAX);

    int32 userHSpacing = q->GetHorizontalSpacing();
    int32 userVSpacing = q->GetVerticalSpacing();
    DVASSERT(userHSpacing >= 0);
    DVASSERT(userVSpacing >= 0);

    int32 minWidth = std::accumulate(minColumnWidths.begin(), minColumnWidths.end(), 0);
    int32 minHeight = std::accumulate(minRowHeights.begin(), minRowHeights.end(), 0);
    int32 sizeHintWidth = std::accumulate(sizeHintColumnWidths.begin(), sizeHintColumnWidths.end(), 0);
    int32 sizeHintHeight = std::accumulate(sizeHintRowHeights.begin(), sizeHintRowHeights.end(), 0);

    int32 hSpacingCount = minColumnWidths.empty() == true ? 0 : minRowHeights.size() - 1;
    int32 vSpacingCount = minRowHeights.empty() == true ? 0 : minRowHeights.size() - 1;
    int32 summyHSpacing = hSpacingCount * userHSpacing;
    int32 summyVSpacing = vSpacingCount * userVSpacing;

    int32 resultSizeHintHeight = sizeHintHeight + topMargin + bottomMargin + summyVSpacing;
    int32 resultMinHeight = minHeight + topMargin + bottomMargin + summyVSpacing;

    int32 resultSizeHintWidth = sizeHintWidth + leftMargin + rightMargin + summyHSpacing;
    int32 resultMinWidth = minWidth + leftMargin + rightMargin + summyHSpacing;

    minSize.rwidth() = qMin(resultMinWidth, QLAYOUTSIZE_MAX);
    minSize.rheight() = qMin(resultMinHeight, QLAYOUTSIZE_MAX);
    preferedSize.rwidth() = qMin(resultMinWidth, QLAYOUTSIZE_MAX);
    preferedSize.rheight() = qMin(resultSizeHintHeight, QLAYOUTSIZE_MAX);
}

void QtWrapLayoutPrivate::UpdateSizes()
{
    Q_Q(QtWrapLayout);
    if (flags[SizeDirty] == false)
    {
        return;
    }

    bool expandH = false;
    bool expandV = false;

    for (QtWrapLayoutItem* item : items)
    {
        item->Update();
        Qt::Orientations expandOrientation = item->ExpandingDirections();
        expandH |= (expandOrientation & Qt::Horizontal);
        expandV |= (expandOrientation & Qt::Vertical);
    }

    flags[SizeDirty] = false;
    flags[ExpandHorizontal] = expandH;
    flags[ExpandVertical] = expandV;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        QtWrapLayout                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

QtWrapLayout::QtWrapLayout(QWidget* parent /*= nullptr*/)
    : QLayout(*new QtWrapLayoutPrivate, 0, parent)
{
}

int32 QtWrapLayout::GetHorizontalSpacing() const
{
    Q_D(const QtWrapLayout);
    if (d->hSpacing >= 0)
    {
        return d->hSpacing;
    }
    else
    {
        return qSmartSpacing(this, QStyle::PM_LayoutHorizontalSpacing);
    }
}

void QtWrapLayout::SetHorizontalSpacing(int32 spacing)
{
    Q_D(QtWrapLayout);
    if (d->hSpacing != spacing)
    {
        d->hSpacing = spacing;
        invalidate();
    }
}

int32 QtWrapLayout::GetVerticalSpacing() const
{
    Q_D(const QtWrapLayout);
    if (d->hSpacing >= 0)
    {
        return d->vSpacing;
    }
    else
    {
        return qSmartSpacing(this, QStyle::PM_LayoutVerticalSpacing);
    }
}

void QtWrapLayout::SetVerticalSpacing(int32 spacing)
{
    Q_D(QtWrapLayout);
    if (d->vSpacing != spacing)
    {
        d->vSpacing = spacing;
        invalidate();
    }
}

void QtWrapLayout::AddLayout(QLayout* layout, int32 stretch /*= 0*/)
{
    Q_D(QtWrapLayout);
    if (layout && !d->checkLayout(layout))
    {
        return;
    }

    if (adoptLayout(layout))
    {
        d->items.push_back(new QtWrapLayoutItem(layout, stretch));
    }
    invalidate();
}

void QtWrapLayout::addItem(QLayoutItem* item)
{
    Q_D(QtWrapLayout);
    d->items.push_back(new QtWrapLayoutItem(item, 0));
    invalidate();
}

QLayoutItem* QtWrapLayout::itemAt(int index) const
{
    Q_D(const QtWrapLayout);
    if (index < static_cast<int>(d->items.size()))
        return nullptr;

    return d->items[index]->item;
}

QLayoutItem* QtWrapLayout::takeAt(int index)
{
    Q_D(QtWrapLayout);
    if (index < static_cast<int>(d->items.size()))
        return nullptr;

    QtWrapLayoutItem* wrapItem = d->items[index];
    QLayoutItem* item = wrapItem->item;
    wrapItem->item = 0;
    delete wrapItem;
    invalidate();

    return item;
}

void QtWrapLayout::setGeometry(const QRect& rect)
{
    Q_D(QtWrapLayout);
    if (d->flags[QtWrapLayoutPrivate::Dirty] || rect != geometry())
    {
        QRect cr = rect;
        int leftMargin, topMargin, rightMargin, bottomMargin;
        getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
        cr.adjust(+leftMargin, +topMargin, -rightMargin, -bottomMargin);

        d->Layout(cr.width());
        Vector<int32>& columnWidths = d->layoutColumnWidths;
        Vector<int32>& rowHeights = d->layoutRowHeights;
        Vector<int32> columnOffsets = columnWidths;
        Vector<int32> rowOffsets = rowHeights;

        auto accumulateOffets = [](Vector<int32>& v, int32 spacing)
        {
            DVASSERT(spacing >= 0);
            if (v.size() < 2)
            {
                return;
            }

            size_t index = 1;
            while (v.size() < index)
            {
                v[index] += (v[index - 1] + spacing);
            }
        };

        // after accumulateOffets we have calculated offsets of each row and column inside target geometry (QRect cr)
        accumulateOffets(columnOffsets, GetHorizontalSpacing());
        accumulateOffets(rowOffsets, GetVerticalSpacing());

        QPoint topLeft = cr.topLeft();
        for (QtWrapLayoutItem* item : d->items)
        {
            QPoint pos(columnOffsets[item->columnIndex], rowOffsets[item->rowIndex]);
            QSize maxSize(columnWidths[item->columnIndex], rowHeights[item->rowIndex]);

            QSize resultSize(maxSize.boundedTo(item->maxSize));
            Qt::Orientations expandDirection = item->ExpandingDirections();
            if (!expandDirection.testFlag(Qt::Horizontal))
            {
                resultSize.rwidth() = item->sizeHint.width();
            }

            if (!expandDirection.testFlag(Qt::Vertical))
            {
                resultSize.rheight() = item->sizeHint.height();
            }

            DVASSERT(resultSize.width() <= maxSize.width());
            DVASSERT(resultSize.height() <= maxSize.height());

            if (resultSize != maxSize)
            {
                Qt::Alignment alignment = item->item->alignment();
                if (alignment.testFlag(Qt::AlignVCenter))
                {
                    pos.ry() += ((maxSize.height() - resultSize.height()) >> 1);
                }
                else if (alignment.testFlag(Qt::AlignBottom))
                {
                    pos.ry() += (maxSize.height() - resultSize.height());
                }

                if (alignment.testFlag(Qt::AlignHCenter))
                {
                    pos.rx() += ((maxSize.width() - resultSize.width()) >> 1);
                }
                else if (alignment.testFlag(Qt::AlignRight))
                {
                    pos.rx() += (maxSize.width() - resultSize.width());
                }
            }

            item->SetGeometry(QRect(pos, resultSize));
        }
        QLayout::setGeometry(rect);
    }
}

QSize QtWrapLayout::minimumSize() const
{
    Q_D(const QtWrapLayout);
    if (!d->minSize.isValid())
    {
        QtWrapLayoutPrivate* e = GetNonConstPrivate();
        e->Layout();
    }
    return d->minSize;
}

QSize QtWrapLayout::sizeHint() const
{
    Q_D(const QtWrapLayout);
    if (!d->preferedSize.isValid())
    {
        QtWrapLayoutPrivate* e = GetNonConstPrivate();
        e->Layout();
    }
    return d->preferedSize;
}

void QtWrapLayout::invalidate()
{
    Q_D(QtWrapLayout);
    QtWrapLayoutDetails::ResetBitSet<QtWrapLayoutPrivate::Count>(d->flags, { true, true, false, false });
    d->minSize = QSize();
    d->preferedSize = QSize();
    QLayout::invalidate();
}

bool QtWrapLayout::hasHeightForWidth() const
{
    return true;
}

int QtWrapLayout::heightForWidth(int width) const
{
    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    QtWrapLayoutPrivate* d = GetNonConstPrivate();
    d->Layout(width);

    int targetWidth = width - leftMargin - rightMargin;
    if (targetWidth == d->fullRowSizeHintWidth)
        return d->preferedSize.height();
    else
        return d->minSize.height();
}

Qt::Orientations QtWrapLayout::expandingDirections() const
{
    QtWrapLayoutPrivate* d = GetNonConstPrivate();
    d->UpdateSizes();

    return static_cast<Qt::Orientations>(d->flags[QtWrapLayoutPrivate::ExpandHorizontal] * Qt::Horizontal +
                                         d->flags[QtWrapLayoutPrivate::ExpandVertical] * Qt::Vertical);
}

int QtWrapLayout::count() const
{
    Q_D(const QtWrapLayout);
    return d->items.size();
}

QtWrapLayoutPrivate* QtWrapLayout::GetNonConstPrivate() const
{
    Q_D(const QtWrapLayout);
    return const_cast<QtWrapLayoutPrivate*>(d);
}

} // namespace TArc
} // namespace DAVA