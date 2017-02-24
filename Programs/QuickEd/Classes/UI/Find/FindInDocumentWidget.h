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

class FindFilter;
class SearchCriteriasWidget;

class FindInDocumentWidget : public QWidget
{
    Q_OBJECT
public:
    FindInDocumentWidget(QWidget* parent = nullptr);
    ~FindInDocumentWidget() override;

    std::shared_ptr<FindFilter> BuildFindFilter() const;

public slots:
    void OnDocumentChanged(Document* document);

private slots:
    void OnFindClicked();

    void OnItemFound(FindItem item);
    void OnProgressChanged(int filesProcessed, int totalFiles);
    void OnFindFinished();

private:
    Document* document = nullptr;
    QHBoxLayout* layout = nullptr;
    SearchCriteriasWidget* findFiltersWidget = nullptr;
    QToolButton* findButton = nullptr;
    QMenu* findButtonsMenu = nullptr;
};
