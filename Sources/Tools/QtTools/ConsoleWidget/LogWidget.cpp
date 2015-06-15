#include "LogWidget.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QClipboard>
#include <QApplication>
#include <QTextStream>
#include <QEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTime>

#include "LogModel.h"
#include "LogFilterModel.h"
#include "LogDelegate.h"


namespace
{
    const int scrollDelay = 50;
}


LogWidget::LogWidget(QWidget* parent)
    : QWidget(parent)
    , eventSkipper(new QTimer(this))
    , doAutoScroll(true)
    , scrollStateDetected(false)
{
    setupUi(this);
    time.start();
    eventSkipper->setInterval(scrollDelay);
    eventSkipper->setSingleShot(true);

    LogDelegate* delegate = new LogDelegate(log, this);
    logModel = new LogModel(this);
    logFilterModel = new LogFilterModel(this);

    logFilterModel->setSourceModel(logModel);
    log->setModel(logFilterModel);
    connect(delegate, &LogDelegate::copyRequest, this, &LogWidget::OnCopy);
    connect(delegate, &LogDelegate::clearRequest, this, &LogWidget::OnClear);
    log->installEventFilter(this);

    FillFiltersCombo();

    connect(filter, &CheckableComboBox::done, this, &LogWidget::OnFilterChanged);
    connect(filter, &CheckableComboBox::selectedUserDataChanged, this, &LogWidget::OnFilterChanged);
    connect(search, &LineEditEx::textUpdated, this, &LogWidget::OnTextFilterChanged);

    // Auto scroll feature
    connect(log->model(), &QAbstractItemModel::rowsAboutToBeInserted, this, &LogWidget::DetectAutoScroll);
    connect(log->model(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &LogWidget::DetectAutoScroll);
    connect(log->model(), &QAbstractItemModel::modelAboutToBeReset, this, &LogWidget::DetectAutoScroll);

    connect(log->model(), &QAbstractItemModel::rowsInserted, eventSkipper.data(), static_cast<void(QTimer::*)(void)>(&QTimer::start));
    connect(log->model(), &QAbstractItemModel::modelReset, eventSkipper.data(), static_cast<void(QTimer::*)(void)>(&QTimer::start));
    connect(eventSkipper.data(), &QTimer::timeout, this, &LogWidget::DoAutoScroll);
    OnFilterChanged();
}

LogModel* LogWidget::Model()
{
    return logModel;
}

void LogWidget::OnFilterChanged()
{
    const auto selection = filter->selectedUserData();
    LogFilterModel::LogLevels flags;
    for (auto sel : selection)
    {
        if (sel.canConvert<int>())
        {
            flags |= static_cast<DAVA::Logger::eLogLevel>(sel.toInt());
        }
    }
    logFilterModel->SetFilters(flags);
}

void LogWidget::OnTextFilterChanged(const QString& text)
{
    logFilterModel->SetFilterString(text);
}

void LogWidget::FillFiltersCombo()
{
    filter->addItem("LEVEL_FRAMEWORK", DAVA::Logger::LEVEL_FRAMEWORK);
    filter->addItem("LEVEL_DEBUG", DAVA::Logger::LEVEL_DEBUG);
    filter->addItem("LEVEL_INFO", DAVA::Logger::LEVEL_INFO);
    filter->addItem("LEVEL_WARNING", DAVA::Logger::LEVEL_WARNING);
    filter->addItem("LEVEL_ERROR", DAVA::Logger::LEVEL_ERROR);

    QAbstractItemModel* m = filter->model();
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QModelIndex index = m->index(i, 0, QModelIndex());
        const int ll = index.data(LogModel::LEVEL_ROLE).toInt();
        const QPixmap& pix = logModel->GetIcon(ll);
        m->setData(index, pix, Qt::DecorationRole);
        m->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}


bool LogWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == log)
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
            {
                QKeyEvent* ke = static_cast<QKeyEvent *>(event);
                if (ke->matches(QKeySequence::Copy))
                {
                    OnCopy();
                    return true;
                }
            }
            break;

        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void LogWidget::OnCopy()
{
    const QModelIndexList& selection = log->selectionModel()->selectedIndexes();
    const int n = selection.size();
    if (n == 0)
        return ;

    QMap< int, QModelIndex > sortedSelection;
    for (int i = 0; i < n; i++)
    {
        const QModelIndex& index = selection[i];
        const int realIdx = index.row();
        sortedSelection[realIdx] = index;
    }

    QString text;
    QTextStream ss(&text);
    for (auto it = sortedSelection.constBegin(); it != sortedSelection.constEnd(); ++it)
    {
        ss << it.value().data(Qt::DisplayRole).toString() << "\n";
    }
    ss.flush();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void LogWidget::OnClear()
{
    logModel->clear();
}

void LogWidget::DetectAutoScroll()
{
    static int sum;
    auto el = time.elapsed();
    if (el > 40) //we use 40ms as minimum screen reload rate. There is no sense to do ti faster
    {
        time.restart();
        QApplication::processEvents();
        sum += time.elapsed();
        time.restart();
    }
    if (scrollStateDetected)
        return;

    scrollStateDetected = true;
    QScrollBar *scroll = log->verticalScrollBar();
    doAutoScroll = (scroll->value() == scroll->maximum());
}

void LogWidget::DoAutoScroll()
{
    if (!doAutoScroll)
        return ;

    log->scrollToBottom();
    scrollStateDetected = false;
}
