#pragma once

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QToolButton>

#include "Base/BaseTypes.h"

class Document;

namespace Ui
{
class SearchCriteriaWidget;
}

class SearchCriteriaWidget : public QWidget
{
    Q_OBJECT
public:
    SearchCriteriaWidget(QWidget* parent = nullptr);
    ~SearchCriteriaWidget() override;

signals:
    void AddAnotherCriteria();
    void RemoveCriteria();

private slots:
    void OnAddAnotherCriteriaPressed();
    void OnRemoveCriteriaPressed();
    void OnCriteriaSelected(int index);

private:
    QHBoxLayout* layout = nullptr;
    QToolButton* addCriteriaButton = nullptr;
    QToolButton* removeCriteriaButton = nullptr;
    QComboBox* criteria = nullptr;

    QHBoxLayout* innerLayout = nullptr;

    QWidget* editor = nullptr;
};
