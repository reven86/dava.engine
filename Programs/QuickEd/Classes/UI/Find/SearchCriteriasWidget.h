#pragma once

#include <QFrame>
#include <QPointer>

#include "Base/BaseTypes.h"

class Document;

namespace Ui
{
class SearchCriteriasWidget;
}

class SearchCriteriaWidget;

class SearchCriteriasWidget : public QFrame
{
    Q_OBJECT
public:
    SearchCriteriasWidget(QWidget* parent = nullptr);
    ~SearchCriteriasWidget() override;

    std::unique_ptr<FindFilter> BuildFindFilter() const;

private slots:
    void OnAddCriteriaClicked();
    void OnRemoveCriteriaClicked();

private:
    DAVA::Set<SearchCriteriaWidget*> filterWidgets;

    std::unique_ptr<Ui::SearchCriteriasWidget> ui;
};
