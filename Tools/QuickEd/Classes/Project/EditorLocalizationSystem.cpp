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

void EditorLocalizationSystem::SetDirectory(const QDir &directoryPath)
{
    Cleanup();

    LocalizationSystem *localizationSystem = LocalizationSystem::Instance();
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

QString EditorLocalizationSystem::GetLocaleNameFromStr(QString localeStr)
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
    if (currentLocale != locale)
    {
        currentLocale = locale;
        DVASSERT(availableLocales.contains(locale));
        LocalizationSystem::Instance()->SetCurrentLocale(availableLocales[locale].toStdString());
        LocalizationSystem::Instance()->Init();

        emit CurrentLocaleChanged(currentLocale);
    }
}
