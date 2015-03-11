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
class MemProfPlot;
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
    void OnViewDumpButton();
    
public:
    explicit MemProfWidget(QWidget *parent = NULL);
    ~MemProfWidget();

    void AppendText(const QString& text);
    void ChangeStatus(const char* status, const char* reason);
    
    void ClearStat();
    void SetStatConfig(const DAVA::MMStatConfig* config);
    void UpdateStat(const DAVA::MMStat* stat);

    void ShowDump(const DAVA::Vector<DAVA::uint8>& v);

    void UpdateProgress(size_t total, size_t recv);
    
private:
    void UpdateLabels(const DAVA::MMStat* stat, DAVA::uint32 alloc, DAVA::uint32 total);
    void CreateUI();
    void CreateLabels(const DAVA::MMStatConfig* config);
    void Deletelabels();
    
private:
    QScopedPointer<Ui::MemProfWidget> ui;
    MemProfPlot* plot;

    DAVA::uint32 tagCount;
    DAVA::uint32 allocPoolCount;

    QToolBar* toolbar;
    QFrame* frame;
    struct LabelPack
    {
        LabelPack() : allocInternal(nullptr), internalBlockCount(nullptr), ghostBlockCount(nullptr), ghostSize(nullptr), realSize(nullptr) {}
        ~LabelPack();
        QLabel* alloc;
        QLabel* total;
        QLabel* allocInternal;
        QLabel* internalBlockCount;
        QLabel* ghostBlockCount;
        QLabel* ghostSize;
        QLabel* realSize;
    };
    LabelPack* labels;
    
    MemProfInfoModel * model;
    QTableView * tableView;

};

#endif // __DEVICELOGWIDGET_H__
