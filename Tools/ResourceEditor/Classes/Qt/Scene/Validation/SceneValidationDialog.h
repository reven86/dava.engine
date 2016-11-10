#pragma once

#include "Base/Introspection.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"

PUSH_QT_WARNING_SUPRESSOR
#include <QDialog>
POP_QT_WARNING_SUPRESSOR

namespace Ui
{
class SceneValidationDialog;
}

class SceneValidationDialog : public QDialog
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
public:
    explicit SceneValidationDialog(SceneEditor2* scene, QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnSelectAllClicked(bool);
    void OnOptionToggled(int, bool);
    void Validate();
    void ShowConsole(bool checked);

private:
    void LoadOptions();
    void SaveOptions();

    bool AreAllOptionsSetTo(bool value);

    void DoMatrices();

private:
    SceneEditor2* scene;
    Ui::SceneValidationDialog* ui;
};
