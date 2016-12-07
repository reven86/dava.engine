#pragma once

#include <QDockWidget>
#include <QPointer>
#include "ui_FindWidget.h"

#include "Base/BaseTypes.h"

class Document;
class LibraryModel;
class Project;

class FindWidget : public QDockWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget* parent = nullptr);
    ~FindWidget() = default;

public slots:
    void OnDocumentChanged(Document* document);

private:
    Ui::FindWidget ui;
};
