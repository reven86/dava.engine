#include "IssueNavigatorWidget.h"

#include "ui_IssueNavigatorWidget.h"

#include <TArc/Core/FieldBinder.h>

#include <QKeyEvent>

IssueNavigatorWidget::IssueNavigatorWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IssueNavigatorWidget())
{
    ui->setupUi(this);

    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels(QStringList() << "Error"
                                                   << "Path To Control"
                                                   << "Package Path"
                                                   << "Property Name");

    ui->treeView->setModel(model);
    connect(ui->treeView, &QTreeView::activated, this, &IssueNavigatorWidget::OnActivated);
    ui->treeView->installEventFilter(this);
}

IssueNavigatorWidget::~IssueNavigatorWidget()
{
}

void IssueNavigatorWidget::AddIssue(const Issue& issue)
{
    QList<QStandardItem*> items;

    items << new QStandardItem(QString::fromStdString(issue.message));
    items << new QStandardItem(QString::fromStdString(issue.pathToControl));
    items << new QStandardItem(QString::fromStdString(issue.packagePath));
    items << new QStandardItem(QString::fromStdString(issue.propertyName));

    for (QStandardItem* item : items)
    {
        item->setEditable(false);
        item->setData(issue.sectionId, ISSUE_SECTION_DATA);
        item->setData(issue.issueId, ISSUE_ID_DATA);
        item->setData(QString::fromStdString(issue.packagePath), PACKAGE_DATA);
        item->setData(QString::fromStdString(issue.pathToControl), CONTROL_DATA);
    }

    model->appendRow(items);
}

void IssueNavigatorWidget::ChangeMessage(DAVA::int32 sectionId_, DAVA::int32 issueId_, const DAVA::String& message)
{
    for (int row = 0; row < model->rowCount(); row++)
    {
        QStandardItem* item = model->item(row);
        DAVA::int32 sectionId = item->data(ISSUE_SECTION_DATA).toInt();
        DAVA::int32 issueId = item->data(ISSUE_ID_DATA).toInt();
        if (issueId == issueId_ && sectionId == sectionId_)
        {
            item->setText(QString::fromStdString(message));
            break;
        }
    }
}

void IssueNavigatorWidget::RemoveIssue(DAVA::int32 sectionId_, DAVA::int32 issueId_)
{
    for (int row = 0; row < model->rowCount();)
    {
        QStandardItem* item = model->item(row);
        DAVA::int32 sectionId = item->data(ISSUE_SECTION_DATA).toInt();
        DAVA::int32 issueId = item->data(ISSUE_ID_DATA).toInt();
        if (issueId == issueId_ && sectionId == sectionId_)
        {
            model->removeRows(row, 1);
        }
        else
        {
            row++;
        }
    }
}

void IssueNavigatorWidget::OnActivated(const QModelIndex& index)
{
    QString path = index.data(PACKAGE_DATA).toString();
    if (index.data(CONTROL_DATA).isValid())
    {
        QString control = index.data(CONTROL_DATA).toString();
        emit JumpToControl(DAVA::FilePath(path.toStdString()), control.toStdString());
    }
    else
    {
        emit JumpToPackage(DAVA::FilePath(path.toStdString()));
    }
}

bool IssueNavigatorWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if (ui->treeView->selectionModel()->currentIndex().isValid())
            {
                OnActivated(ui->treeView->selectionModel()->currentIndex());
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}
