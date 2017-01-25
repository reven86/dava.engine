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

QStringList EditorLocalizationSystem::GetAvailableLocales() const
{
    return availableLocales;
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
                availableLocales << localeStr;
            }
        }
    }
}

void EditorLocalizationSystem::Cleanup()
{
    availableLocales.clear();
    LocalizationSystem::Instance()->Cleanup();
}

QString EditorLocalizationSystem::GetCurrentLocale() const
{
    return currentLocale;
}

void EditorLocalizationSystem::SetCurrentLocale(const QString& locale)
{
    DVASSERT(!locale.isEmpty());
    DVASSERT(availableLocales.contains(locale));

    currentLocale = locale;

    LocalizationSystem::Instance()->SetCurrentLocale(currentLocale.toStdString());
    LocalizationSystem::Instance()->Init();

    emit CurrentLocaleChanged(currentLocale);
}
