#pragma once

#include <Base/BaseTypes.h>

#include <QWidget>

class ProfilingSession;

class QComboBox;
class QCheckBox;
class QLineEdit;
class CheckableComboBox;
class QStandardItemModel;

class FilterAndSortBar : public QWidget
{
    Q_OBJECT

public:
    enum
    {
        FLAG_ENABLE_SORTING = 0x01,
        FLAG_ENABLE_FILTER_BY_POOL = 0x02,
        FLAG_ENABLE_FILTER_BY_TAG = 0x04,
        FLAG_ENABLE_HIDE_SAME = 0x08,
        FLAG_ENABLE_HIDE_DIFFERENT = 0x10,
        FLAG_ENABLE_BLOCK_ORDER = 0x20,
        FLAG_ENABLE_GROUPING = 0x40,

        FLAG_ENABLE_ALL = FLAG_ENABLE_SORTING | FLAG_ENABLE_FILTER_BY_POOL | FLAG_ENABLE_FILTER_BY_TAG | FLAG_ENABLE_HIDE_SAME |
        FLAG_ENABLE_HIDE_DIFFERENT |
        FLAG_ENABLE_BLOCK_ORDER |
        FLAG_ENABLE_GROUPING,
        FLAG_ENABLE_ALL_FOR_DIFF = FLAG_ENABLE_ALL,
        FLAG_ENABLE_ALL_FOR_SINGLE = FLAG_ENABLE_ALL & ~(FLAG_ENABLE_HIDE_SAME | FLAG_ENABLE_HIDE_DIFFERENT)
    };

    enum eSortOrder
    {
        SORT_BY_ORDER = 0,
        SORT_BY_SIZE,
        SORT_BY_POOL,
        SORT_BY_BACKTRACE,
    };

    enum eGroupOrder
    {
        GROUP_BY_BACKTRACE = 0,
        GROUP_BY_SIZE
    };

public:
    FilterAndSortBar(const ProfilingSession* session, DAVA::int32 flags, QWidget* parent = nullptr);
    virtual ~FilterAndSortBar();

signals:
    void GroupOrderChanged(int order);
    void SortingOrderChanged(int order);
    void FilterChanged(DAVA::uint32 poolMask, DAVA::uint32 tagMask);
    void HideTheSameChanged(bool hide);
    void HideDifferentChanged(bool hide);
    void BlockOrderChanged(DAVA::uint32 minBlockOrder);

private slots:
    void GroupOrderCombo_CurrentIndexChanged(int index);
    void SortOrderCombo_CurrentIndexChanged(int index);
    void FilterPoolCombo_DataChanged(const QVariantList& data);
    void FilterTagCombo_DataChanged(const QVariantList& data);
    void HideTheSameCheck_StateChanged(int state);
    void HideDifferentCheck_StateChanged(int state);
    void BlockOrderButton_Clicked();

private:
    void Init(DAVA::int32 flags);
    QComboBox* CreateGroupCombo();
    QComboBox* CreateSortCombo();
    CheckableComboBox* CreateFilterPoolCombo();
    CheckableComboBox* CreateFilterTagCombo();
    QCheckBox* CreateHideTheSameCheck();
    QCheckBox* CreateHideDifferentCheck();
    QWidget* CreateBlockOrderWidgets();

private:
    const ProfilingSession* session = nullptr;

    DAVA::int32 groupOrder = GROUP_BY_BACKTRACE;
    DAVA::int32 sortOrder = SORT_BY_ORDER;
    DAVA::uint32 filterPoolMask = 0;
    DAVA::uint32 filterTagMask = 0;
    bool hideTheSame = false;
    bool hideDifferent = false;

    std::unique_ptr<QStandardItemModel> groupComboModel;
    std::unique_ptr<QStandardItemModel> sortComboModel;
    QLineEdit* minBlockOrderWidget = nullptr;
};
