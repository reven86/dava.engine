#pragma once

#include <QDialog>
#include <memory>

#include <memory>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class UI;
}
}

class QWidget;
class RunUIViewerDialog : public QDialog
{
public:
    RunUIViewerDialog(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, QWidget* parent = nullptr);
    ~RunUIViewerDialog();

private:
    void OnOk();

    struct RunData;
    std::unique_ptr<RunData> runData;
};
