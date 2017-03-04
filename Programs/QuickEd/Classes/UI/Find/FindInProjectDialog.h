#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>

#include "Base/BaseTypes.h"
#include "Document/Document.h"
#include "UI/Find/FindItem.h"

class EditorSystemsManager;
class FindFilter;
class SearchCriteriasWidget;

namespace Ui
{
class FindInProjectDialog;
}

class FindInProjectDialog : public QDialog
{
    Q_OBJECT
public:
    FindInProjectDialog(QWidget* parent = nullptr);
    ~FindInProjectDialog() override;

    std::unique_ptr<FindFilter> BuildFindFilter() const;

private:
    std::unique_ptr<Ui::FindInProjectDialog> ui;
};
