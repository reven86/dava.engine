#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/FilterAndSortBar.h"

#include "QtTools/ComboBox/CheckableComboBox.h"

#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>

using namespace DAVA;

FilterAndSortBar::FilterAndSortBar(const ProfilingSession* session_, int32 flags, QWidget* parent)
    : QWidget(parent)
    , session(session_)
{
    DVASSERT((flags & FLAG_ENABLE_ALL) != 0);
    Init(flags);
}

FilterAndSortBar::~FilterAndSortBar() = default;

void FilterAndSortBar::Init(int32 flags)
{
    QHBoxLayout* layout = new QHBoxLayout;
    if (flags & FLAG_ENABLE_GROUPING)
    {
        layout->addWidget(CreateGroupCombo());
    }
    if (flags & FLAG_ENABLE_SORTING)
    {
        layout->addWidget(CreateSortCombo());
    }
    if (flags & FLAG_ENABLE_FILTER_BY_POOL)
    {
        layout->addWidget(CreateFilterPoolCombo());
    }
    if (flags & FLAG_ENABLE_FILTER_BY_TAG)
    {
        layout->addWidget(CreateFilterTagCombo());
    }
    if (flags & FLAG_ENABLE_HIDE_SAME)
    {
        layout->addWidget(CreateHideTheSameCheck());
    }
    if (flags & FLAG_ENABLE_HIDE_DIFFERENT)
    {
        layout->addWidget(CreateHideDifferentCheck());
    }
    if (flags & FLAG_ENABLE_BLOCK_ORDER)
    {
        layout->addWidget(CreateBlockOrderWidgets());
    }
    setLayout(layout);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}

QComboBox* FilterAndSortBar::CreateGroupCombo()
{
    std::pair<QString, int> items[] = {
        { "Group by backtrace", GROUP_BY_BACKTRACE },
        { "Group by size", GROUP_BY_SIZE }
    };

    int nrows = static_cast<int>(COUNT_OF(items));
    groupComboModel.reset(new QStandardItemModel(nrows, 1));
    for (int i = 0; i < nrows; ++i)
    {
        QStandardItem* item = new QStandardItem(QString(items[i].first));
        item->setData(items[i].second, Qt::UserRole + 1);

        groupComboModel->setItem(i, 0, item);
    }

    QComboBox* widget = new QComboBox;
    connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(GroupOrderCombo_CurrentIndexChanged(int)));

    widget->setModel(groupComboModel.get());
    widget->setCurrentIndex(0);
    return widget;
}

QComboBox* FilterAndSortBar::CreateSortCombo()
{
    std::pair<QString, int> items[] = {
        { "Sort by order", SORT_BY_ORDER },
        { "Sort by size", SORT_BY_SIZE },
        { "Sort by pool", SORT_BY_POOL },
        { "Sort by backtrace", SORT_BY_BACKTRACE }
    };

    int nrows = static_cast<int>(COUNT_OF(items));
    sortComboModel.reset(new QStandardItemModel(nrows, 1));
    for (int i = 0; i < nrows; ++i)
    {
        QStandardItem* item = new QStandardItem(QString(items[i].first));
        item->setData(items[i].second, Qt::UserRole + 1);

        sortComboModel->setItem(i, 0, item);
    }

    QComboBox* widget = new QComboBox;
    connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(SortOrderCombo_CurrentIndexChanged(int)));

    widget->setModel(sortComboModel.get());
    widget->setCurrentIndex(0);
    return widget;
}

CheckableComboBox* FilterAndSortBar::CreateFilterPoolCombo()
{
    CheckableComboBox* widget = new CheckableComboBox;

    int nrows = static_cast<int>(session->AllocPoolCount());
    for (int i = 0; i < nrows; ++i)
    {
        const String& name = session->AllocPoolName(i);
        widget->addItem(name.c_str(), 1 << i);
    }

    QAbstractItemModel* model = widget->model();
    for (int i = 0; i < nrows; ++i)
    {
        QModelIndex index = model->index(i, 0);
        model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }

    connect(widget, &CheckableComboBox::selectedUserDataChanged, this, &FilterAndSortBar::FilterPoolCombo_DataChanged);
    return widget;
}

CheckableComboBox* FilterAndSortBar::CreateFilterTagCombo()
{
    CheckableComboBox* widget = new CheckableComboBox;

    int nrows = static_cast<int>(session->TagCount());
    for (int i = 0; i < nrows; ++i)
    {
        const String& name = session->TagName(i);
        widget->addItem(name.c_str(), 1 << i);
    }

    QAbstractItemModel* model = widget->model();
    for (int i = 0; i < nrows; ++i)
    {
        QModelIndex index = model->index(i, 0);
        model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }

    connect(widget, &CheckableComboBox::selectedUserDataChanged, this, &FilterAndSortBar::FilterTagCombo_DataChanged);
    return widget;
}

QCheckBox* FilterAndSortBar::CreateHideTheSameCheck()
{
    QCheckBox* widget = new QCheckBox("Hide same blocks");
    widget->setTristate(false);

    connect(widget, &QCheckBox::stateChanged, this, &FilterAndSortBar::HideTheSameCheck_StateChanged);

    return widget;
}

QCheckBox* FilterAndSortBar::CreateHideDifferentCheck()
{
    QCheckBox* widget = new QCheckBox("Hide different blocks");
    widget->setTristate(false);

    connect(widget, &QCheckBox::stateChanged, this, &FilterAndSortBar::HideDifferentCheck_StateChanged);

    return widget;
}

QWidget* FilterAndSortBar::CreateBlockOrderWidgets()
{
    QPushButton* apply = new QPushButton("Apply");
    connect(apply, &QPushButton::clicked, this, &FilterAndSortBar::BlockOrderButton_Clicked);

    minBlockOrderWidget = new QLineEdit;

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(apply);
    layout->addWidget(minBlockOrderWidget);

    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

void FilterAndSortBar::GroupOrderCombo_CurrentIndexChanged(int index)
{
    QModelIndex modelIndex = groupComboModel->index(index, 0);
    if (modelIndex.isValid())
    {
        int v = groupComboModel->data(modelIndex, Qt::UserRole + 1).toInt();
        emit GroupOrderChanged(v);
    }
}

void FilterAndSortBar::SortOrderCombo_CurrentIndexChanged(int index)
{
    QModelIndex modelIndex = sortComboModel->index(index, 0);
    if (modelIndex.isValid())
    {
        int v = sortComboModel->data(modelIndex, Qt::UserRole + 1).toInt();
        emit SortingOrderChanged(v);
    }
}

void FilterAndSortBar::FilterPoolCombo_DataChanged(const QVariantList& data)
{
    filterPoolMask = 0;
    for (const QVariant& v : data)
    {
        filterPoolMask |= v.toUInt();
    }
    emit FilterChanged(filterPoolMask, filterTagMask);
}

void FilterAndSortBar::FilterTagCombo_DataChanged(const QVariantList& data)
{
    filterTagMask = 0;
    for (const QVariant& v : data)
    {
        filterTagMask |= v.toUInt();
    }
    emit FilterChanged(filterPoolMask, filterTagMask);
}

void FilterAndSortBar::HideTheSameCheck_StateChanged(int state)
{
    hideTheSame = state == Qt::Checked;
    emit HideTheSameChanged(hideTheSame);
}

void FilterAndSortBar::HideDifferentCheck_StateChanged(int state)
{
    hideDifferent = state == Qt::Checked;
    emit HideDifferentChanged(hideDifferent);
}

void FilterAndSortBar::BlockOrderButton_Clicked()
{
    uint32 value = 0;
    QString text = minBlockOrderWidget->text();
    if (!text.isEmpty())
    {
        bool valueIsValid = false;
        value = text.toUInt(&valueIsValid, 10);
        if (!valueIsValid)
            value = 0;
    }
    emit BlockOrderChanged(value);
}
