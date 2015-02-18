#ifndef __DEVICELOGWIDGET_H__
#define __DEVICELOGWIDGET_H__

#include <QWidget>
#include <QScopedPointer>
#include <QTableView.h>


#include "MemoryManager/MemoryManagerTypes.h"
#include "MemoryManager/MemoryManager.h"
namespace Ui {
    class MemProfWidget;
} // namespace Ui

class QLabel;
class QCustomPlot;
class MemProfPlot;
class MemProfInfoModel;
class MemoryProfDataChunk;
class MemProfWidget : public QWidget
{
    Q_OBJECT

signals:

public:
    explicit MemProfWidget(QWidget *parent = NULL);
    ~MemProfWidget();

    void AppendText(const QString& text);
    void ChangeStatus(const char* status, const char* reason);
    
    void ClearStat();
    void UpdateStat(const MemoryProfDataChunk* stat);
    
    void SetModel(MemProfInfoModel * model);

private:
    void UpdateLabels(const  MemoryProfDataChunk* stat, DAVA::uint32 alloc, DAVA::uint32 total);
    void CreateUI();

    
private:
    QScopedPointer<Ui::MemProfWidget> ui;
    MemProfPlot* plot;
    
    DAVA::uint32 offset;
    DAVA::uint32 prevOrder;
    
    struct label_pack
    {
        QLabel* title;
        QLabel* alloc;
        QLabel* total;
        QLabel* max_block_size;
        QLabel* nblocks;
    };
    MemProfInfoModel * model;
    QTableView * dataView;
};

#endif // __DEVICELOGWIDGET_H__
