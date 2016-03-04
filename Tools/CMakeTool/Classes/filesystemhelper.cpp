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


#include "filesystemhelper.h"
#include <QDir>
#include <QRegularExpression>

FileSystemHelper::FileSystemHelper(QObject* parent)
    : QObject(parent)
{
}

QString FileSystemHelper::ResolveUrl(const QString& url)
{
    QRegularExpression regExp;
#ifdef Q_OS_MAC
    regExp.setPattern("^(file:/{2})"); //on unix systems path started with '/'
#elif defined Q_OS_WIN
    regExp.setPattern("^(file:/{3})");
#endif //Q_OS_MAC Q_OS_WIN;
    QString resolvedUrl(url);
    resolvedUrl.replace(regExp, "");
    return resolvedUrl;
}

bool FileSystemHelper::IsDirExists(const QString& dirPath)
{
    if (dirPath.isEmpty())
    {
        return false;
    }
    QDir dir(dirPath);
    return dir.exists();
}

bool FileSystemHelper::IsFileExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.isFile() && fileInfo.exists();
}

QString FileSystemHelper::FindCMakeBin(const QString& path)
{
    QString davaFolder = "dava.framework";
    int index = path.indexOf(davaFolder);
    if (index == -1)
    {
        return "";
    }
    QString davaPath = path.left(path.indexOf(index + davaFolder.length()));
    QString cmakePath = davaPath + "/Tools/Bin" +
#ifdef Q_OS_MAC
    "/CMake.app/Contents/bin/cmake";
#elif defined Q_OS_WIN
    "/cmake/bin/cmake.exe";
#endif //Q_OS_MAC Q_OS_WIN
    if (!QFile::exists(cmakePath))
    {
        return "";
    }
    return QDir::toNativeSeparators(cmakePath);
}

bool FileSystemHelper::ClearFolderContent(const QString& folderPath)
{
    QDir dir(folderPath);
    if (folderPath.isEmpty() || !dir.exists())
    {
        return false;
    }
    if (dir.removeRecursively())
    {
        return dir.mkpath(folderPath);
    }
    return false;
}
