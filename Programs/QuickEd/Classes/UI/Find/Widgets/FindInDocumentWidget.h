#pragma once

#include <Base/BaseTypes.h>
#include <QAction>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QWidget>

class FindFilter;
class CompositeFindFilterWidget;

class FindInDocumentWidget : public QWidget
{
    Q_OBJECT
public:
    FindInDocumentWidget(QWidget* parent = nullptr);

    std::shared_ptr<FindFilter> BuildFindFilter() const;

    void Reset();

signals:
    void OnFindFilterReady(std::shared_ptr<FindFilter> filter);
    void OnFindNext();
    void OnFindPrevious();
    void OnFindAll();
    void OnStopFind();

private slots:
    void OnFindNextClicked();
    void OnFindPreviousClicked();
    void OnFindAllClicked();
    void OnFiltersChanged();

protected:
    bool event(QEvent* event) override;

private:
    void EmitFilterChanges();

    QHBoxLayout* layout = nullptr;
    CompositeFindFilterWidget* findFiltersWidget = nullptr;
    QToolButton* findButton = nullptr;
    QMenu* menu = nullptr;
    QAction* findAction = nullptr;
    QAction* findAllAction = nullptr;

    bool hasChanges = true;
};
