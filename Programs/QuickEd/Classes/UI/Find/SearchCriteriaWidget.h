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

class CriteriaEditor
: public QWidget
{
    Q_OBJECT
public:
    CriteriaEditor(QWidget* parent)
        : QWidget(parent)
    {
    }

    virtual std::unique_ptr<FindFilter> BuildFindFilter() = 0;

signals:
    void CriteriaChanged();
};

class SearchCriteriaWidget : public QWidget
{
    Q_OBJECT
public:
    SearchCriteriaWidget(QWidget* parent = nullptr);
    ~SearchCriteriaWidget() override;

    std::shared_ptr<FindFilter> BuildFindFilter() const;

signals:
    void AddAnotherCriteria();
    void RemoveCriteria();
    void CriteriaChanged();

private slots:
    void OnCriteriaSelected(int index);

private:
    QHBoxLayout* layout = nullptr;
    QToolButton* addCriteriaButton = nullptr;
    QToolButton* removeCriteriaButton = nullptr;
    QComboBox* criteria = nullptr;

    QHBoxLayout* innerLayout = nullptr;

    CriteriaEditor* editor = nullptr;
};
