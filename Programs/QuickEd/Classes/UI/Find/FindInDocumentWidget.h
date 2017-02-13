#pragma once

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QToolButton>

#include "Base/BaseTypes.h"

class FindFilter;
class SearchCriteriasWidget;

class FindInDocumentWidget : public QWidget
{
    Q_OBJECT
public:
    FindInDocumentWidget(QWidget* parent = nullptr);
    ~FindInDocumentWidget() override;

    std::shared_ptr<FindFilter> BuildFindFilter() const;

private:
    QHBoxLayout* layout = nullptr;
    SearchCriteriasWidget* findFiltersWidget = nullptr;
    QToolButton* findButton = nullptr;
    QMenu* findButtonsMenu = nullptr;
};
