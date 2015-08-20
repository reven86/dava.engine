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


#ifndef __DEVICELOGWIDGET_H__
#define __DEVICELOGWIDGET_H__

#include "Base/BaseTypes.h"

#include <QWidget>
#include <QColor>
#include <QPointer>
#include <QScopedPointer>

namespace Ui {
    class MemProfWidget;
} // namespace Ui

class QLabel;
class QCustomPlot;
class QFrame;
class QToolBar;

class QCustomPlot;

namespace DAVA
{
    struct MMStatConfig;
    struct MMCurStat;
}   // namespace DAVA

class AllocPoolModel;
class TagModel;
class GeneralStatModel;
class SnapshotListModel;
class MemoryStatItem;
class ProfilingSession;

class MemProfWidget : public QWidget
{
    Q_OBJECT

signals:
    void OnSnapshotButton();
    
public:
    MemProfWidget(ProfilingSession* profSession, QWidget *parent = nullptr);
    virtual ~MemProfWidget();

public slots:
    void ConnectionEstablished(bool newConnection);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived(DAVA::uint32 itemCount);
    void SnapshotProgress(DAVA::uint32 totalSize, DAVA::uint32 recvSize);

    void RealtimeToggled(bool checked);
    void DiffClicked();
    void PlotClicked(QMouseEvent* ev);

    void SnapshotList_OnDoubleClicked(const QModelIndex& index);

private:
    void InitUI();
    void ReinitPlot();
    void UpdatePlot(const MemoryStatItem& stat);
    void SetPlotData();

private:
    QScopedPointer<Ui::MemProfWidget> ui;

    ProfilingSession* profileSession = nullptr;
    QPointer<AllocPoolModel> allocPoolModel;
    QPointer<TagModel> tagModel;
    QPointer<GeneralStatModel> generalStatModel;
    QPointer<SnapshotListModel> snapshotModel;

    DAVA::Vector<QColor> poolColors;
    bool realtimeMode;
};

#endif // __DEVICELOGWIDGET_H__
