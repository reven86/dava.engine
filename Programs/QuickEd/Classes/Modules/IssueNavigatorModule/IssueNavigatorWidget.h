#pragma once

#include <QWidget>

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

class IssueNavigatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IssueNavigatorWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent = nullptr);
    ~IssueNavigatorWidget() override;

    std::unique_ptr<Ui::IssueNavigatorWidget> ui;
};
