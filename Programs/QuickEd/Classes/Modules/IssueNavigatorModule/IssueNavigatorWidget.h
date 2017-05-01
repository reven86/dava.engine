#pragma once

#include <Base/BaseTypes.h>
#include <QWidget>
#include <QStandardItemModel>

namespace DAVA
{
class Any;
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

namespace Ui
{
class IssueNavigatorWidget;
}

struct Issue
{
    DAVA::int32 sectionId;
    DAVA::int32 issueId;
    DAVA::String message;
    DAVA::String packagePath;
    DAVA::String pathToControl;
    DAVA::String propertyName;
};

class IssueNavigatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IssueNavigatorWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent = nullptr);
    ~IssueNavigatorWidget() override;

    void AddIssue(const Issue& issue);
    void ChangeMessage(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& message);
    void RemoveIssue(DAVA::int32 sectionId, DAVA::int32 issueId);

private slots:
    void OnActivated(const QModelIndex& index);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    std::unique_ptr<Ui::IssueNavigatorWidget> ui;
    QStandardItemModel* model = nullptr;

    enum
    {
        PACKAGE_DATA = Qt::UserRole + 1,
        CONTROL_DATA,
        ISSUE_SECTION_DATA,
        ISSUE_ID_DATA
    };
};
