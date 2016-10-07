#pragma once

#include <QInputDialog>

class MultilineTextInputDialog final : protected QInputDialog
{
public:
    static QString GetMultiLineText(QWidget* parent, const QString& title, const QString& label, const QString& text = QString(), bool* ok = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags(), Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

private:
    MultilineTextInputDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    bool eventFilter(QObject* obj, QEvent* event) override;
};
