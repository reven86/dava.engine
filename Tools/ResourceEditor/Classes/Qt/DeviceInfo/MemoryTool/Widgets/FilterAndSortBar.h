/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __MEMORYTOOL_FILTERANDSORTBAR_H__
#define __MEMORYTOOL_FILTERANDSORTBAR_H__

#include "Base/BaseTypes.h"

#include <QWidget>

class ProfilingSession;

class QComboBox;
class QCheckBox;
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

        FLAG_ENABLE_ALL = FLAG_ENABLE_SORTING | FLAG_ENABLE_FILTER_BY_POOL | FLAG_ENABLE_FILTER_BY_TAG | FLAG_ENABLE_HIDE_SAME,
        FLAG_ENABLE_ALL_FOR_DIFF = FLAG_ENABLE_ALL,
        FLAG_ENABLE_ALL_FOR_SINGLE = FLAG_ENABLE_ALL & ~FLAG_ENABLE_HIDE_SAME
    };

    enum eSortOrder
    {
        SORT_BY_ORDER = 0,
        SORT_BY_SIZE,
        SORT_BY_POOL,
        SORT_BY_BACKTRACE,
    };

public:
    FilterAndSortBar(const ProfilingSession* session, DAVA::int32 flags, QWidget* parent = nullptr);
    virtual ~FilterAndSortBar();

signals:
    void SortingOrderChanged(int order);
    void FilterChanged(DAVA::uint32 poolMask, DAVA::uint32 tagMask);
    void HideTheSameChanged(bool hide);

private slots:
    void SortOrderCombo_CurrentIndexChanged(int index);
    void FilterPoolCombo_DataChanged(const QVariantList& data);
    void FilterTagCombo_DataChanged(const QVariantList& data);
    void HideTheSameCheck_StateChanges(int state);

private:
    void Init(DAVA::int32 flags);
    QComboBox* CreateSortCombo();
    CheckableComboBox* CreateFilterPoolCombo();
    CheckableComboBox* CreateFilterTagCombo();
    QCheckBox* CreateHideTheSameCheck();

private:
    const ProfilingSession* session = nullptr;

    DAVA::int32 sortOrder = SORT_BY_ORDER;
    DAVA::uint32 filterPoolMask = 0;
    DAVA::uint32 filterTagMask = 0;
    bool hideTheSame = false;

    std::unique_ptr<QStandardItemModel> sortComboModel;
};

#endif  // __MEMORYTOOL_FILTERANDSORTBAR_H__
