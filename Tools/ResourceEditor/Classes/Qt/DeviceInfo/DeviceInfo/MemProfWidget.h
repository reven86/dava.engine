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

#include <QWidget>
#include <QScopedPointer>
#include <qtableview.h>
#include "Base/BaseTypes.h"

namespace Ui {
    class MemProfWidget;
} // namespace Ui

class QLabel;
class QCustomPlot;
class QFrame;
class QToolBar;

class MemProfInfoModel;
namespace DAVA
{
struct MMStatConfig;
struct MMStat;
}

class MemProfWidget : public QWidget
{
    Q_OBJECT

signals:
    void OnDumpButton();
    
public:
    explicit MemProfWidget(QWidget *parent = NULL);
    ~MemProfWidget();

    void AppendText(const QString& text);
    void ChangeStatus(const char* status, const char* reason);
    
    void ClearStat();
    void SetStatConfig(const DAVA::MMStatConfig* config);
    void UpdateStat(const DAVA::MMStat* stat);

    void UpdateProgress(size_t total, size_t recv);
    
private:
    void UpdateLabels(const DAVA::MMStat* stat, DAVA::uint32 alloc, DAVA::uint32 total);
    void CreateUI();
    void CreateLabels(const DAVA::MMStatConfig* config);
    void Deletelabels();
    
private:
    QScopedPointer<Ui::MemProfWidget> ui;
    QCustomPlot* plot;

    DAVA::uint32 tagCount;
    DAVA::uint32 allocPoolCount;

    QToolBar* toolbar;
    QFrame* frame;
    struct label_pack
    {
        label_pack() : title(nullptr), alloc(nullptr), total(nullptr), max_block_size(nullptr), nblocks(nullptr) {}
        ~label_pack();
        QLabel* title;
        QLabel* alloc;
        QLabel* total;
        QLabel* max_block_size;
        QLabel* nblocks;
    };
    label_pack* labels;
    
    MemProfInfoModel * model;
    QTableView * tableView;

};

#endif // __DEVICELOGWIDGET_H__
