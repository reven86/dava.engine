#include "Project/EditorLocalizationSystem.h"
#include "FileSystem/LocalizationSystem.h"

#include <QLocale>
#include <QDirIterator>
#include <QDir>

using namespace DAVA;

EditorLocalizationSystem::EditorLocalizationSystem(QObject* parent)
    : QObject(parent)
{
}

QStringList EditorLocalizationSystem::GetAvailableLocaleNames() const
{
    return availableLocales.keys();
}

QStringList EditorLocalizationSystem::GetAvailableLocaleValues() const
{
    return availableLocales.values();
}

void EditorLocalizationSystem::SetDirectory(const QDir& directoryPath)
{
    Cleanup();

    LocalizationSystem* localizationSystem = LocalizationSystem::Instance();
    DVASSERT(nullptr != localizationSystem);

    FilePath directoryFilePath(directoryPath.absolutePath().toStdString() + "/"); //absolutePath doesn't contains with '/' symbol at end
    localizationSystem->SetDirectory(directoryFilePath);
    if (!directoryFilePath.IsEmpty())
    {
        QDirIterator dirIterator(directoryPath, QDirIterator::NoIteratorFlags);
        while (dirIterator.hasNext())
        {
            dirIterator.next();
            QFileInfo fileInfo = dirIterator.fileInfo();
            if (!fileInfo.isDir())
            {
                QString localeStr = fileInfo.baseName();
                QString localeName = GetLocaleNameFromStr(localeStr);
                availableLocales.insert(localeName, localeStr);
            }
        }
    }
}

void EditorLocalizationSystem::SetCurrentLocaleValue(const QString& localeStr)
{
    QStringList keys = availableLocales.keys(localeStr);
    DVASSERT(keys.size() == 1);
    SetCurrentLocale(keys.front());
}

void EditorLocalizationSystem::Cleanup()
{
    availableLocales.clear();
    LocalizationSystem::Instance()->Cleanup();
}

QString EditorLocalizationSystem::GetLocaleNameFromStr(const QString& localeStr)
{
    QLocale locale(localeStr);
    switch (locale.script())
    {
    default:
        return QLocale::languageToString(locale.language());

    case QLocale::SimplifiedChineseScript:
        return "Chinese simpl.";

    case QLocale::TraditionalChineseScript:
        return "Chinese trad.";
    }
}

QString EditorLocalizationSystem::GetCurrentLocale() const
{
    return currentLocale;
}

void EditorLocalizationSystem::SetCurrentLocale(const QString& locale)
{
    DVASSERT(!locale.isEmpty());
    currentLocale = locale;
    DVASSERT(availableLocales.contains(locale));
    LocalizationSystem::Instance()->SetCurrentLocale(availableLocales[locale].toStdString());
    LocalizationSystem::Instance()->Init();

    emit CurrentLocaleChanged(currentLocale);
}
