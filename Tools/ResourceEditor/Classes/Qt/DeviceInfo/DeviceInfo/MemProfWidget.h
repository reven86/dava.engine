#ifndef __DEVICELOGWIDGET_H__
#define __DEVICELOGWIDGET_H__

#include <QWidget>
#include <QScopedPointer>

namespace Ui {
    class MemProfWidget;
} // namespace Ui

class QLabel;
class QCustomPlot;

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
    //void UpdateStat(const net_mem_stat_t* stat);
    
private:
    //void UpdateLabels(const net_mem_stat_t* stat, DAVA::uint32 alloc, DAVA::uint32 total);
    void CreateUI();
    
private:
    QScopedPointer<Ui::MemProfWidget> ui;
    QCustomPlot* plot;

    struct label_pack
    {
        QLabel* title;
        QLabel* alloc;
        QLabel* total;
        QLabel* max_block_size;
        QLabel* nblocks;
    };
    label_pack labels[10 + 1];
};

#endif // __DEVICELOGWIDGET_H__
