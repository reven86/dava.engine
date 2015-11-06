#include "LogWidget.h"
#include <QDebug>
#include <QClipboard>
#include <QKeyEvent>
#include <QScrollBar>

#include "LogModel.h"
#include "LogFilterModel.h"
#include "LogDelegate.h"

#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"
#include "ui_LogWidget.h"


LogWidget::LogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LogWidget)
{
    ui->setupUi(this);
    ui->toolButton_clearFilter->setIcon(QIcon(":/QtTools/Icons/reset.png"));
    ui->toolButton_clearFilter->setToolTip(tr("Clear filter string"));

    ui->toolButton_clearConsole->setIcon(QIcon(":/QtTools/Icons/clear.png"));
    ui->toolButton_clearConsole->setToolTip(tr("Clear console window"));

    logModel = new LogModel(this);
    logFilterModel = new LogFilterModel(this);

    logFilterModel->setSourceModel(logModel);
    ui->log->setModel(logFilterModel);
    ui->log->installEventFilter(this);
    ui->log->setUniformItemSizes(true);

    LogDelegate *logDelegate = new LogDelegate(ui->log);
    FillFiltersCombo();
    connect(logDelegate, &LogDelegate::copyRequest, this, &LogWidget::OnCopy);
    connect(logDelegate, &LogDelegate::clearRequest, logModel, &LogModel::Clear);
    connect(ui->toolButton_clearFilter, &QToolButton::clicked, ui->search, &LineEditEx::clear);
    connect(ui->toolButton_clearConsole, &QToolButton::clicked, logModel, &LogModel::Clear);
    connect(ui->filter, &CheckableComboBox::selectedUserDataChanged, logFilterModel, &LogFilterModel::SetFilters);
    connect(ui->search, &LineEditEx::textUpdated, logFilterModel, &LogFilterModel::setFilterFixedString);
    connect(ui->log->model(), &QAbstractItemModel::rowsAboutToBeInserted, this, &LogWidget::OnBeforeAdded);
    connect(ui->log, &QListView::clicked, this, &LogWidget::OnItemClicked);
    scrollTimer = new QTimer(this);
    scrollTimer->setSingleShot(true);
    scrollTimer->setInterval(1);
    connect(scrollTimer, &QTimer::timeout, this, &LogWidget::UpdateScroll);
}

LogWidget::~LogWidget()
{
    delete ui;
}

void LogWidget::SetConvertFunction(LogModel::ConvertFunc func)
{
    logModel->SetConvertFunction(func);
}

QByteArray LogWidget::Serialize() const
{
    QByteArray retData;
    QDataStream stream(&retData, QIODevice::WriteOnly);
    stream << ui->search->text();
    stream << ui->filter->selectedUserData();
    return retData;
}

void LogWidget::Deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    QString filterString;
    stream >> filterString;
    if (stream.status() == QDataStream::ReadCorruptData || stream.status() == QDataStream::ReadPastEnd)
    {
        return;
    }
    QVariantList logLevels;
    stream >> logLevels;

    if (stream.status() == QDataStream::ReadCorruptData)
    {
        return;
    }
    ui->search->setText(filterString);
    ui->filter->selectUserData(logLevels);
}

void LogWidget::AddMessage(DAVA::Logger::eLogLevel ll, const char* msg)
{
    logModel->AddMessage(ll, msg);
}

void LogWidget::FillFiltersCombo()
{
    const auto &logMap = GlobalEnumMap<DAVA::Logger::eLogLevel>::Instance();
    for (size_t i = 0; i < logMap->GetCount(); ++i)
    {
        int value;
        bool ok = logMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create eLogLevel list");
            break;
        }
        ui->filter->addItem(logMap->ToString(value), value);
    }

    QAbstractItemModel* m = ui->filter->model();
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
    if (watched == ui->log)
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
    const QModelIndexList& selection = ui->log->selectionModel()->selectedIndexes();
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

    QStringList strList;
    for (auto it = sortedSelection.constBegin(); it != sortedSelection.constEnd(); ++it)
    {
        strList << it.value().data(Qt::DisplayRole).toString();
    }
    QString text = strList.join('\n');
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void LogWidget::OnBeforeAdded()
{
    bool onBottom = ui->log->verticalScrollBar()->value() == ui->log->verticalScrollBar()->maximum();
    if (onBottom)
    {
        scrollTimer->start();
    }
}

void LogWidget::UpdateScroll()
{
    ui->log->scrollToBottom();
}

void LogWidget::OnItemClicked(const QModelIndex &index)
{
   emit ItemClicked(logFilterModel->data(index, LogModel::INTERNAL_DATA_ROLE).toString());
};