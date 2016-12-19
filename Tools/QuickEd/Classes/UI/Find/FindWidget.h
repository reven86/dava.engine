#pragma once

#include <QDockWidget>
#include <QPointer>
#include <QStandardItemModel>
#include "ui_FindWidget.h"

#include "UI/Find/FindItem.h"
#include "UI/Find/FindFilter.h"
#include "Base/BaseTypes.h"

class Document;
class Project;
class QtThread;

class FindWidget : public QDockWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget* parent = nullptr);
    ~FindWidget() override;

    void Find(std::unique_ptr<FindFilter>&& filter);

signals:
    void StopAll();

public slots:
    void OnProjectChanged(Project* project);

private slots:
    void OnItemFound(FindItem item);
    void OnProgressChanged(int filesProcessed, int totalFiles);
    void OnFindFinished();
    void OnActivated(const QModelIndex& index);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    enum
    {
        PACKAGE_DATA = Qt::UserRole + 1,
        CONTROL_DATA
    };

    Ui::FindWidget ui;
    std::unique_ptr<FindFilter> filter;
    QStandardItemModel* model = nullptr;

    Project* project = nullptr;
    QtThread* thread = nullptr;
};
