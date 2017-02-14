#include "TArc/Controls/QtWrapLayout.h"

#include <Debug/DVAssert.h>

#include <private/qlayout_p.h>
#include <private/qlayoutengine_p.h>

#include <QSize>
#include <QtGlobal>
#include <QWidget>

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
    QtWrapLayoutItem(QLayoutItem* i)
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
    size_t rowIndex = 0;
    size_t columnIndex = 0;

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
    void CalcRanges(Vector<std::pair<size_t, size_t>>& ranges, const std::pair<size_t, size_t>& testedRange, int32 spacing, int32 margin);

    int32 layoutWidth; // width last layout calculation was made for
    int32 layoutHeight;

    QSize minSize; // calculated based on minimum size to layout into one row
    QSize preferedSize; // calculated based on size hint to layout into one row
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

void QtWrapLayoutPrivate::CalcRanges(Vector<std::pair<size_t, size_t>>& ranges, const std::pair<size_t, size_t>& testedRange, int32 spacing, int32 margin)
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
    fullRowMinWidth += margin;

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
            CalcRanges(leftSubRanges, leftSubRange, spacing, margin);
            CalcRanges(rightSubRanges, rightSubRange, spacing, margin);
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

    DVASSERT(items.empty() == false);

    // Early out if we have no changes that would cause a change in vertical layout
    if (width == layoutWidth && flags[Dirty] == false && flags[SizeDirty] == false)
    {
        return;
    }

    layoutWidth = width;
    layoutHeight = 0;
    layoutColumnWidths.clear();
    layoutRowHeights.clear();

    int32 userVSpacing = q->GetVerticalSpacing();
    int32 userHSpacing = q->GetHorizontalSpacing();
    DVASSERT(userVSpacing >= 0);
    DVASSERT(userHSpacing >= 0);

    int leftMargin, topMargin, rightMargin, bottomMargin;
    q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    int32 horizontalMargin = leftMargin + rightMargin;
    int32 availableHorizontalSpace = layoutWidth - horizontalMargin;

    // make sure our sizes are up to date
    UpdateSizes();
    Vector<std::pair<size_t, size_t>> ranges;
    CalcRanges(ranges, std::make_pair(0, items.size()), userHSpacing, horizontalMargin);

    DVASSERT(ranges.empty() == false);

    std::pair<size_t, size_t> firstRange = ranges.front();
    // CalcRanges should place items by one in column if we can't place items in columns evenly
    // so range.second - range - first should be equal for all ranges
    size_t columnCount = firstRange.second - firstRange.first;
    size_t rowCount = ranges.size();
    layoutColumnWidths.resize(columnCount);
    layoutRowHeights.resize(rowCount);

    QVector<QLayoutStruct> verticalLayout(static_cast<int>(rowCount));
    for (size_t rangeIndex = 0; rangeIndex < ranges.size(); ++rangeIndex)
    {
        int32 minRowWidth = 0;
        int32 sizeHintRowWidth = 0;
        bool verticalExpand = false;
        QVector<QLayoutStruct> horizontalLayout;
        horizontalLayout.reserve(static_cast<int>(columnCount));
        const std::pair<size_t, size_t>& range = ranges[rangeIndex];
        for (size_t itemIndex = range.first; itemIndex < range.second; ++itemIndex)
        {
            QtWrapLayoutItem* item = items[itemIndex];
            item->rowIndex = rangeIndex;
            item->columnIndex = itemIndex - range.first;

            horizontalLayout.push_back(QLayoutStruct());
            QLayoutStruct& layoutStruct = horizontalLayout.back();
            layoutStruct.init(0, item->minSize.width());
            layoutStruct.sizeHint = item->sizeHint.width();
            layoutStruct.maximumSize = item->maxSize.width();
            layoutStruct.expansive = (item->ExpandingDirections() & Qt::Horizontal);
            layoutStruct.empty = false;

            minRowWidth += item->minSize.width();
            sizeHintRowWidth += item->sizeHint.width();
        }

        qGeomCalc(horizontalLayout, 0, horizontalLayout.size(), 0, availableHorizontalSpace, userHSpacing);

        int32 maxRowMinHeight = 0; // maximum value of minimumHeight of minimumHeightForWidth of items in current row
        int32 maxRowSizeHintHeight = 0; // maximum value of sizeHintHeight of items in current row
        int32 maxRowMaxHeight = 0; // maximum value of maxHegiht of items in current row
        for (size_t itemIndex = range.first; itemIndex < range.second; ++itemIndex)
        {
            DVASSERT(layoutColumnWidths.size() == range.second - range.first);
            size_t layoutStructIndex = itemIndex - range.first;
            const QLayoutStruct& layoutStruct = horizontalLayout[static_cast<int>(layoutStructIndex)];
            QtWrapLayoutItem* item = items[itemIndex];

            if (item->HasHeightForWidth())
            {
                maxRowMinHeight = Max(maxRowMinHeight, item->MinimumHeightForWidth(layoutStruct.size));
                maxRowSizeHintHeight = Max(maxRowSizeHintHeight, item->HeightForWidth(layoutStruct.size));
                maxRowMaxHeight = Max(maxRowMaxHeight, maxRowSizeHintHeight);
            }
            else
            {
                maxRowMinHeight = Max(maxRowMinHeight, item->minSize.height());
                maxRowSizeHintHeight = Max(maxRowSizeHintHeight, item->sizeHint.height());
                maxRowMaxHeight = Max(maxRowMaxHeight, item->maxSize.height());
            }

            layoutColumnWidths[layoutStructIndex] = Max(layoutColumnWidths[layoutStructIndex], layoutStruct.size);
        }

        QLayoutStruct& verticalLayoutStruct = verticalLayout[static_cast<int>(rangeIndex)];
        verticalLayoutStruct.init(0 /*stretch factor*/, maxRowMinHeight);
        verticalLayoutStruct.sizeHint = maxRowSizeHintHeight;
        verticalLayoutStruct.maximumSize = maxRowMaxHeight;
        verticalLayoutStruct.expansive = verticalExpand;
        verticalLayoutStruct.empty = false;
        layoutRowHeights[rangeIndex] = maxRowSizeHintHeight;
    }

    qGeomCalc(verticalLayout, 0, verticalLayout.size(), 0, QLAYOUTSIZE_MAX, userVSpacing);

    int32 fullHeight = 0;
    for (size_t rowIndex = 0; rowIndex < layoutRowHeights.size(); ++rowIndex)
    {
        layoutRowHeights[rowIndex] = Min(verticalLayout[static_cast<int>(rowIndex)].size, layoutRowHeights[rowIndex]);
        fullHeight += layoutRowHeights[rowIndex];
    }

    int32 rowSpacingCount = layoutRowHeights.empty() == false ? static_cast<int32>(layoutRowHeights.size()) - 1 : 0;
    int32 vAdditionalSpace = rowSpacingCount * userVSpacing + topMargin + bottomMargin;
    layoutHeight = fullHeight + vAdditionalSpace;
    flags[Dirty] = false;
}

void QtWrapLayoutPrivate::Layout()
{
    Layout(QLAYOUTSIZE_MAX);
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
    minSize = QSize(0, 0);
    preferedSize = QSize(0, 0);

    for (QtWrapLayoutItem* item : items)
    {
        item->Update();
        minSize = minSize.expandedTo(item->minSize);
        preferedSize.rwidth() += item->sizeHint.width();
        preferedSize.rheight() = Max(preferedSize.height(), item->sizeHint.height());
        Qt::Orientations expandOrientation = item->ExpandingDirections();
        expandH |= (expandOrientation & Qt::Horizontal) != 0;
        expandV |= (expandOrientation & Qt::Vertical) != 0;
    }

    int leftMargin, topMargin, rightMargin, bottomMargin;
    q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    minSize.rwidth() += (leftMargin + rightMargin);
    preferedSize.rwidth() += (items.size() > 1) ? (static_cast<int32>(items.size()) - 1) * q->GetHorizontalSpacing() : 0;

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
    if (d->vSpacing >= 0)
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

void QtWrapLayout::AddLayout(QLayout* layout)
{
    Q_D(QtWrapLayout);
    if (layout && !d->checkLayout(layout))
    {
        return;
    }

    if (adoptLayout(layout))
    {
        d->items.push_back(new QtWrapLayoutItem(layout));
    }
    invalidate();
}

void QtWrapLayout::addItem(QLayoutItem* item)
{
    Q_D(QtWrapLayout);
    d->items.push_back(new QtWrapLayoutItem(item));
    invalidate();
}

QLayoutItem* QtWrapLayout::itemAt(int index) const
{
    Q_D(const QtWrapLayout);
    if (static_cast<int>(d->items.size()) <= index)
        return nullptr;

    return d->items[index]->item;
}

QLayoutItem* QtWrapLayout::takeAt(int index)
{
    Q_D(QtWrapLayout);
    if (static_cast<int>(d->items.size()) <= index)
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

        d->Layout(rect.width());
        Vector<int32>& columnWidths = d->layoutColumnWidths;
        Vector<int32>& rowHeights = d->layoutRowHeights;
        Vector<int32> columnOffsets = columnWidths;
        Vector<int32> rowOffsets = rowHeights;

        auto accumulateOffets = [](Vector<int32>& v, int32 spacing, int32 margin)
        {
            DVASSERT(spacing >= 0);
            size_t index = 0;
            int32 accumulatedSize = margin;
            while (index < v.size())
            {
                int32 currentSize = v[index] + spacing;
                v[index] = accumulatedSize;
                accumulatedSize += currentSize;
                ++index;
            }
        };

        // after accumulateOffets we have calculated offsets of each row and column inside target geometry (QRect cr)
        accumulateOffets(columnOffsets, GetHorizontalSpacing(), cr.left());
        accumulateOffets(rowOffsets, GetVerticalSpacing(), cr.top());

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
        e->UpdateSizes();
    }
    return d->minSize;
}

QSize QtWrapLayout::sizeHint() const
{
    Q_D(const QtWrapLayout);
    if (!d->preferedSize.isValid())
    {
        QtWrapLayoutPrivate* e = GetNonConstPrivate();
        e->UpdateSizes();
    }
    return d->preferedSize;
}

void QtWrapLayout::invalidate()
{
    Q_D(QtWrapLayout);
    QtWrapLayoutDetails::ResetBitSet<QtWrapLayoutPrivate::Count>(d->flags, { true, true, false, false });
    d->minSize = QSize();
    d->preferedSize = QSize();
    d->layoutWidth = 0;
    d->layoutHeight = 0;
    QLayout::invalidate();
}

bool QtWrapLayout::hasHeightForWidth() const
{
    return true;
}

int QtWrapLayout::heightForWidth(int width) const
{
    QtWrapLayoutPrivate* d = GetNonConstPrivate();
    d->Layout(width);

    return d->layoutHeight;
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
    return static_cast<int>(d->items.size());
}

QtWrapLayoutPrivate* QtWrapLayout::GetNonConstPrivate() const
{
    Q_D(const QtWrapLayout);
    return const_cast<QtWrapLayoutPrivate*>(d);
}

} // namespace TArc
} // namespace DAVA