#pragma once

#include <QString>
#include <QStringList>
#include <QPalette>

class QApplication;

namespace Themes
{
enum eTheme : int
{
    Light,
    Dark
};
void InitFromQApplication();
QStringList ThemesNames();
void SetCurrentTheme(const QString& theme);
void SetCurrentTheme(eTheme theme);
QString GetCurrentThemeStr();
eTheme GetCurrentTheme();

QColor GetViewLineAlternateColor();
QColor GetChangedPropertyColor();
QColor GetPrototypeColor();
QColor GetStyleSheetNodeColor();
QColor GetRulerWidgetBackgroungColor();
QColor GetHighligtedItemTextColor();
QColor GetErrorColor();
};
