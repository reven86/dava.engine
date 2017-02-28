#pragma once

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QToolButton>

#include "Base/BaseTypes.h"
#include "Document/Document.h"
#include "UI/Find/FindItem.h"

class EditorSystemsManager;
class FindFilter;
class SearchCriteriasWidget;

class FindInDocumentWidget : public QWidget
{
    Q_OBJECT
public:
    FindInDocumentWidget(QWidget* parent = nullptr);
    ~FindInDocumentWidget() override;

    std::shared_ptr<FindFilter> BuildFindFilter() const;

signals:
    void OnFindFilterReady(std::shared_ptr<FindFilter> filter);

private slots:
    void OnFindClicked();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Document* document = nullptr;
    QHBoxLayout* layout = nullptr;
    SearchCriteriasWidget* findFiltersWidget = nullptr;
    QToolButton* findButton = nullptr;
    QMenu* findButtonsMenu = nullptr;
    EditorSystemsManager* systemsManager = nullptr;
};
