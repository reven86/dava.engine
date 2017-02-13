#pragma once

#include <QDockWidget>
#include <QPointer>
#include <QStandardItemModel>
#include "ui_FindResultsWidget.h"

#include "UI/Find/FindItem.h"
#include "UI/Find/FindFilter.h"
#include "Base/BaseTypes.h"

class Document;
class Project;
class Finder;

class FindResultsWidget : public QDockWidget
{
    Q_OBJECT
public:
    FindResultsWidget(QWidget* parent = nullptr);
    ~FindResultsWidget() override;

    void Find(std::unique_ptr<FindFilter>&& filter);

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

    Ui::FindResultsWidget ui;
    std::unique_ptr<FindFilter> filter;
    QStandardItemModel* model = nullptr;

    Project* project = nullptr;
    Finder* finder = nullptr;
};
