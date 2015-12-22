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
#include <QtGlobal>
#include <QStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>

namespace
{
const QString themeSettingsGroup = "QtTools/Themes";
const QString themeSettingsKey = "ThemeName";
}

QPalette ThemesFactory::defaultPalette;
QString ThemesFactory::defaultStyleSheet;
QString ThemesFactory::currentTheme;
bool ThemesFactory::themesInitialized = false;

void ThemesFactory::InitFromQApplication()
{
    themesInitialized = true;
    defaultStyleSheet = qApp->styleSheet();
    defaultPalette = qApp->palette();
    qAddPostRoutine([](){
        QSettings settings(QApplication::organizationName(), QApplication::applicationName());
        settings.beginGroup(themeSettingsGroup);
        settings.setValue(themeSettingsKey, currentTheme);
        settings.endGroup();
    });
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup(themeSettingsGroup);
    auto value = settings.value(themeSettingsKey);
    currentTheme = value.canConvert<QString>() ? value.toString() : "classic";
    settings.endGroup();
    SetCurrentTheme(currentTheme);
}

QStringList ThemesFactory::Themes()
{
    return QStringList()
        << "classic"
        << "dark";
}

void ThemesFactory::SetCurrentTheme(const QString& theme)
{
    if(!Themes().contains(theme))
    {
        qWarning("Invalid theme passed to SetTheme");
        return;
    }
    if (!themesInitialized)
    {
        qWarning("ThemesFactiry uninitialized");
        return;
    }
    if(theme == "classic")
    {
#ifdef Q_OS_MAC
        QString styleName = "macintosh";
#else
        QString styleName = "windows";
#endif //Q_OS_MAC
        qApp->setStyle(QStyleFactory::create(styleName));
        qApp->setPalette(defaultPalette);
        qApp->setStyleSheet(defaultStyleSheet);
    }
    else if(theme == "dark")
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53,53,53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25,25,25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53,53,53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        
        qApp->setPalette(darkPalette);
        
        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
    currentTheme = theme;
}

const QString& ThemesFactory::GetCurrentTheme()
{
    return currentTheme;
}