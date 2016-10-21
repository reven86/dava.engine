#pragma once
#include <QDialog>
#include <memory>

namespace Ui
{
class DialogAddPreset;
}

class EditorFontSystem;

class DialogAddPreset : public QDialog
{
    Q_OBJECT
public:
    explicit DialogAddPreset(EditorFontSystem* aEditorFontSystem, const QString& originalPresetName, QWidget* parent = nullptr);
    ~DialogAddPreset();

private slots:
    void OnNewPresetNameChanged();
    void OnAccept();

private:
    std::unique_ptr<Ui::DialogAddPreset> ui;
    EditorFontSystem* editorFontSystem = nullptr;
};
