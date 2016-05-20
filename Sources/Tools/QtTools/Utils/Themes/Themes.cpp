/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Themes.h"
#include "Debug/DVassert.h"
#include <QtGlobal>
#include <QStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#include <QFont>

namespace Themes_namespace
{
const QString themeSettingsGroup = "QtTools/Themes";
const QString themeSettingsKey = "ThemeName";
}

namespace Themes
{
eTheme currentTheme;
bool themesInitialized = false;
QStringList themesNames = { "light", "dark" };

QColor lightTextColor(Qt::black);
QColor lightDisabledTextColor(0x0, 0x0, 0x0, 0x50);
QColor lightWindowColor(0xF0, 0xF0, 0xF0);

QColor darkTextColor(0xF2, 0xF2, 0xF2);
QColor darkDisabledTextColor(0x75, 0x75, 0x75);
QColor darkWindowColor(0x32, 0x32, 0x32);

void SetupClassicTheme();
void SetupDarkTheme();

void InitFromQApplication()
{
#if defined(Q_OS_MAC)
    //this is default font on MAC OS X
    qApp->setFont(QFont(".SF NS Text", 13));
#elif defined(Q_OS_WIN)
    //this is default font on Windows
    qApp->setFont(QFont("MS Shell Dlg 2", 10));
#else
#error "unsupported OS"
#endif //platform
    themesInitialized = true;
    qAddPostRoutine([]() {
        QSettings settings(QApplication::organizationName(), QApplication::applicationName());
        settings.beginGroup(Themes_namespace::themeSettingsGroup);
        settings.setValue(Themes_namespace::themeSettingsKey, static_cast<int>(currentTheme));
        settings.endGroup();
        settings.sync();
    });
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup(Themes_namespace::themeSettingsGroup);
    auto value = settings.value(Themes_namespace::themeSettingsKey);
    settings.endGroup();
    if (value.canConvert<int>())
    {
        currentTheme = static_cast<eTheme>(value.value<int>());
    }
    else
    {
        currentTheme = Light;
    }
    SetCurrentTheme(currentTheme);
}

QStringList ThemesNames()
{
    return themesNames;
}

void SetCurrentTheme(const QString& theme)
{
    if (!themesNames.contains(theme))
    {
        qWarning("Invalid theme passed to SetTheme");
        return;
    }
    int index = themesNames.indexOf(theme);
    SetCurrentTheme(static_cast<eTheme>(index));
}

void SetCurrentTheme(eTheme theme)
{
    if (!themesInitialized)
    {
        qWarning("ThemesFactiry uninitialized");
        return;
    }
    currentTheme = theme;
    switch (theme)
    {
    case Light:
        SetupClassicTheme();
        break;
    case Dark:
        SetupDarkTheme();
        break;
    default:
        DVASSERT(false && "unhandled theme passed to SetCurrentTheme");
        break;
    }
}

void SetupClassicTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, lightWindowColor);
    lightPalette.setColor(QPalette::WindowText, QColor(0x25, 0x25, 0x25));
    lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, lightDisabledTextColor);

    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::Disabled, QPalette::Base, lightWindowColor);

    lightPalette.setColor(QPalette::AlternateBase, lightWindowColor);
    lightPalette.setColor(QPalette::ToolTipBase, lightWindowColor);
    lightPalette.setColor(QPalette::ToolTipText, lightTextColor);

    lightPalette.setColor(QPalette::Text, lightTextColor);
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, lightDisabledTextColor);

    lightPalette.setColor(QPalette::Button, lightWindowColor);
    lightPalette.setColor(QPalette::ButtonText, lightTextColor.lighter(130));
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, lightDisabledTextColor);

    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, Qt::blue);
    lightPalette.setColor(QPalette::Disabled, QPalette::Light, lightWindowColor);

    lightPalette.setColor(QPalette::Highlight, QColor(0x43, 0x8B, 0xBF));
    lightPalette.setColor(QPalette::Inactive, QPalette::Highlight, lightWindowColor);
    lightPalette.setColor(QPalette::Disabled, QPalette::Highlight, lightWindowColor);

    lightPalette.setColor(QPalette::HighlightedText, lightTextColor);
    lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, lightDisabledTextColor);

    qApp->setPalette(lightPalette);

    qApp->setStyleSheet("QDockWidget::title { background: #d5d5d5; }"
                        //workaround for expanded combobox interval
                        "QComboBox { font: 11px;  }");
}

void SetupDarkTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, darkWindowColor);
    darkPalette.setColor(QPalette::WindowText, darkTextColor);

    darkPalette.setColor(QPalette::Base, darkWindowColor.darker(130));
    darkPalette.setColor(QPalette::AlternateBase, darkWindowColor);
    darkPalette.setColor(QPalette::ToolTipBase, darkWindowColor);
    darkPalette.setColor(QPalette::ToolTipText, darkTextColor);

    darkPalette.setColor(QPalette::Text, darkTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, darkDisabledTextColor);

    darkPalette.setColor(QPalette::Button, darkWindowColor);
    darkPalette.setColor(QPalette::ButtonText, darkTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, darkDisabledTextColor);

    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(0x2A, 0x82, 0xDA));
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkWindowColor);

    darkPalette.setColor(QPalette::Highlight, QColor(0x37, 0x63, 0xAD));
    darkPalette.setColor(QPalette::Inactive, QPalette::Highlight, darkDisabledTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, darkDisabledTextColor);

    darkPalette.setColor(QPalette::HighlightedText, QColor(Qt::white));
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0xC0, 0xC0, 0xC0));

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #e0e0e0; background-color: #373737;  }"
                        "QTabBar::close-button { image: url(:/Icons/close.png); }"
                        "QDockWidget::title { background: #454545; }"
                        //workaround for expanded combobox interval
                        "QComboBox{ font: 11px; }");
}

const QString& GetCurrentThemeStr()
{
    return themesNames.at(currentTheme);
}

eTheme GetCurrentTheme()
{
    return currentTheme;
}

QColor GetViewLineAlternateColor()
{
    return currentTheme == Light ? Qt::lightGray : QColor(0x3F, 0x3F, 0x46);
}

QColor GetChangedPropertyColor()
{
    return currentTheme == Light ? Qt::black : QColor(0xE3, 0xE1, 0x8A);
}

QColor GetPrototypeColor()
{
    return currentTheme == Light ? QColor(Qt::blue) : QColor("CadetBlue");
}

QColor GetStyleSheetNodeColor()
{
    return currentTheme == Light ? QColor(Qt::darkGreen) : QColor("light green");
}

QColor GetRulerWidgetBackgroungColor()
{
    return currentTheme == Light ? lightWindowColor : darkWindowColor;
}
};
