#pragma once

#include <QFrame>
#include <QPointer>
#include <QStandardItemModel>

#include "Base/BaseTypes.h"

class Document;

namespace Ui
{
class SearchCriteriasWidget;
}

class SearchCriteriasWidget : public QFrame
{
    Q_OBJECT
public:
    SearchCriteriasWidget(QWidget* parent = nullptr);
    ~SearchCriteriasWidget() override;

public slots:
    void OnDocumentChanged(Document* arg)
    {
        doc = arg;
    };

private slots:
    void OnAddCriteriaClicked();
    void OnRemoveCriteriaClicked();

private:
    QStandardItemModel* model = nullptr;
    std::unique_ptr<Ui::SearchCriteriasWidget> ui;

    Document* doc = nullptr;
};
