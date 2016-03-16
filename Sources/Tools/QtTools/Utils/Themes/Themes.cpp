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

namespace Themes_namespace
{
const QString themeSettingsGroup = "QtTools/Themes";
const QString themeSettingsKey = "ThemeName";
}

namespace Themes
{
QPalette defaultPalette;
QString defaultStyleSheet;
eTheme currentTheme;
bool themesInitialized = false;
QStringList themesNames = { "classic", "dark" };
QColor textColor(192, 192, 192);
QColor disabledTextColor(100, 100, 100);
QColor windowColor(53, 53, 53);

void SetupClassicTheme();
void SetupDarkTheme();

void InitFromQApplication()
{
    themesInitialized = true;
    defaultStyleSheet = qApp->styleSheet();
    defaultPalette = QGuiApplication::palette();
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
        currentTheme = Classic;
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
    case Classic:
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
#ifdef Q_OS_MAC
    QString styleName = "macintosh";
#else
    QString styleName = "windowsVista";
#endif //Q_OS_MAC
    qApp->setStyle(QStyleFactory::create(styleName));
    qApp->setPalette(defaultPalette);
    qApp->setStyleSheet(defaultStyleSheet);
}

void SetupDarkTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, windowColor);
    darkPalette.setColor(QPalette::WindowText, textColor);

    darkPalette.setColor(QPalette::Base, windowColor);
    darkPalette.setColor(QPalette::AlternateBase, windowColor);
    darkPalette.setColor(QPalette::ToolTipBase, textColor);
    darkPalette.setColor(QPalette::ToolTipText, textColor);

    darkPalette.setColor(QPalette::Text, textColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledTextColor);

    darkPalette.setColor(QPalette::Button, windowColor);
    darkPalette.setColor(QPalette::ButtonText, textColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledTextColor);

    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, windowColor);

    darkPalette.setColor(QPalette::Highlight, QColor(0x37, 0x63, 0xAD));
    darkPalette.setColor(QPalette::Inactive, QPalette::Highlight, disabledTextColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, disabledTextColor);

    darkPalette.setColor(QPalette::HighlightedText, QColor(Qt::white));
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0xc0, 0xc0, 0xc0));

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #e0e0e0; background-color: #373737;  }");
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
    return currentTheme == Themes::Classic ? QColor(Qt::lightGray) : QColor(0x3f, 0x3f, 0x46);
}

QColor GetChangedPropertyColor()
{
    return currentTheme == Themes::Classic ? QColor(Qt::black) : QColor(225, 225, 225);
}

QColor GetPrototypeColor()
{
    return currentTheme == Themes::Classic ? QColor(Qt::blue) : QColor("CadetBlue");
}

QColor GetStyleSheetNodeColor()
{
    return currentTheme == Themes::Classic ? QColor(Qt::darkGreen) : QColor("light green");
}

QColor GetRulerWidgetBackgroungColor()
{
    return currentTheme == Themes::Classic ? QColor(Qt::white) : windowColor;
}
};
