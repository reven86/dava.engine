#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QToolButton>

#include <Base/BaseTypes.h>

class FindFilter;
class SearchCriteriasWidget;

class FindInDocumentWidget : public QWidget
{
    Q_OBJECT
public:
    FindInDocumentWidget(QWidget* parent = nullptr);
    ~FindInDocumentWidget() override;

    std::shared_ptr<FindFilter> BuildFindFilter() const;

    void Reset();

signals:
    void OnFindFilterReady(std::shared_ptr<FindFilter> filter);
    void OnFindNext();
    void OnFindAll();
    void OnCancelFind();

private slots:
    void OnFindClicked();
    void OnFindAllClicked();
    void OnCriteriasChanged();

protected:
    bool event(QEvent* event) override;

private:
    void EmitFilterChanges();

    Document* document = nullptr;
    QHBoxLayout* layout = nullptr;
    SearchCriteriasWidget* findFiltersWidget = nullptr;
    QToolButton* findButton = nullptr;
    QToolButton* findAllButton = nullptr;

    bool hasChanges = true;
};
