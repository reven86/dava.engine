#include "Themes.h"
#include "Debug/DVassert.h"
#include "Base/GlobalEnum.h"

#include "Preferences/PreferencesStorage.h"
#include "Preferences/PreferencesRegistrator.h"

#include <QtGlobal>
#include <QStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QFont>

ENUM_DECLARE(Themes::eTheme)
{
    ENUM_ADD_DESCR(Themes::Light, "Classic");
    ENUM_ADD_DESCR(Themes::Dark, "Dark");
};

namespace Themes_local
{
const DAVA::FastName themeSettingsKey("ThemeName");
GlobalValuesRegistrator registrator(themeSettingsKey, DAVA::VariantType(static_cast<DAVA::int64>(Themes::Dark)));
}

namespace Themes
{
QColor lightTextColor(Qt::black);
QColor lightDisabledTextColor(0x0, 0x0, 0x0, 0x50);
QColor lightWindowColor(0xF0, 0xF0, 0xF0);

QColor darkTextColor(0xF2, 0xF2, 0xF2);
QColor darkDisabledTextColor(0x75, 0x75, 0x75);
QColor darkWindowColor(0x32, 0x32, 0x32);

bool themesInitialized = false;

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
    PreferencesStorage::Instance()->SetValue(Themes_local::themeSettingsKey, DAVA::VariantType(static_cast<DAVA::int64>(theme)));
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
    return GetCurrentTheme() == Light ? Qt::lightGray : QColor(0x3F, 0x3F, 0x46);
}

QColor GetChangedPropertyColor()
{
    return GetCurrentTheme() == Light ? Qt::black : QColor(0xE3, 0xE1, 0x8A);
}

QColor GetPrototypeColor()
{
    return GetCurrentTheme() == Light ? QColor(Qt::blue) : QColor("CadetBlue");
}

QColor GetStyleSheetNodeColor()
{
    return GetCurrentTheme() == Light ? QColor(Qt::darkGreen) : QColor("light green");
}

QColor GetRulerWidgetBackgroungColor()
{
    return GetCurrentTheme() == Light ? lightWindowColor : darkWindowColor;
}
};
