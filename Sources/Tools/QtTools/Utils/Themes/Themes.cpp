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
#include "Base/GlobalEnum.h"

#include "QtTools/EditorPreferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/PreferencesRegistrator.h"

#include <QtGlobal>
#include <QStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>

ENUM_DECLARE(Themes::eTheme)
{
    ENUM_ADD_DESCR(Themes::Classic, "Classic");
    ENUM_ADD_DESCR(Themes::Dark, "Dark");
};

namespace Themes_local
{
const DAVA::FastName themeSettingsKey("ThemeName");
GlobalValuesRegistrator registrator(themeSettingsKey, DAVA::VariantType(static_cast<DAVA::int64>(Themes::Classic)));
}

namespace Themes
{
QPalette defaultPalette;
QString defaultStyleSheet;
bool themesInitialized = false;
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

    SetCurrentTheme(GetCurrentTheme());
}

QStringList ThemesNames()
{
    QStringList names;
    const auto& themesMap = GlobalEnumMap<eTheme>::Instance();
    for (size_t i = 0; i < themesMap->GetCount(); ++i)
    {
        int value;
        bool ok = themesMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create Themes names");
            break;
        }
        names << QString::fromStdString(themesMap->ToString(value));
    }
    return names;
}

void SetCurrentTheme(const QString& theme)
{
    DAVA::String themeStr = theme.toStdString();
    const auto& themesMap = GlobalEnumMap<eTheme>::Instance();
    for (size_t i = 0; i < themesMap->GetCount(); ++i)
    {
        int value;
        bool ok = themesMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used");
            break;
        }
        if (themesMap->ToString(value) == themeStr)
        {
            eTheme newTheme = static_cast<eTheme>(value);
            SetCurrentTheme(newTheme);
            return;
        }
    }

    DVASSERT_MSG(false, "Invalid theme passed to SetTheme");
}

void SetCurrentTheme(eTheme theme)
{
    if (!themesInitialized)
    {
        qWarning("ThemesFactiry uninitialized");
        return;
    }
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
    PreferencesStorage::Instance()->SetValue(Themes_local::themeSettingsKey, DAVA::VariantType(static_cast<DAVA::int64>(theme)));
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

QString GetCurrentThemeStr()
{
    DAVA::String name = GlobalEnumMap<eTheme>::Instance()->ToString(GetCurrentTheme());
    return QString::fromStdString(name);
}

eTheme GetCurrentTheme()
{
    DAVA::VariantType value = PreferencesStorage::Instance()->GetValue(Themes_local::themeSettingsKey);
    return static_cast<eTheme>(value.AsInt64());
}

QColor GetViewLineAlternateColor()
{
    return GetCurrentTheme() == Themes::Classic ? QColor(Qt::lightGray) : QColor(0x3f, 0x3f, 0x46);
}

QColor GetChangedPropertyColor()
{
    return GetCurrentTheme() == Themes::Classic ? QColor(Qt::black) : QColor(225, 225, 225);
}

QColor GetPrototypeColor()
{
    return GetCurrentTheme() == Themes::Classic ? QColor(Qt::blue) : QColor("CadetBlue");
}

QColor GetStyleSheetNodeColor()
{
    return GetCurrentTheme() == Themes::Classic ? QColor(Qt::darkGreen) : QColor("light green");
}

QColor GetRulerWidgetBackgroungColor()
{
    return GetCurrentTheme() == Themes::Classic ? QColor(Qt::white) : windowColor;
}
};
