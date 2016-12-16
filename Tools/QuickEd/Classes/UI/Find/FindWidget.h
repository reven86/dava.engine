#pragma once

#include <QDockWidget>
#include <QPointer>
#include <QStandardItemModel>
#include "ui_FindWidget.h"

#include "UI/Find/FindItem.h"
#include "Base/BaseTypes.h"

class Document;
class Project;

class FindWidget : public QDockWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget* parent = nullptr);
    ~FindWidget() = default;

    void ShowResults(const DAVA::Vector<FindItem>& items);

public slots:
    void OnDocumentChanged(Document* document);

private slots:
    void OnActivated(const QModelIndex& index);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    enum
    {
        PACKAGE_DATA = Qt::UserRole + 1,
        CONTROL_DATA = Qt::UserRole
    };

    Ui::FindWidget ui;
    DAVA::Vector<FindItem> items;
    QStandardItemModel* model = nullptr;

    Project* project = nullptr;
};
