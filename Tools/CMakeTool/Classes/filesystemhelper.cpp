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

QVariant FileSystemHelper::resolveUrl(QVariant url)
{
    if (!url.canConvert<QString>())
    {
        return "";
    }
    QString str = url.toString();
    QRegularExpression regExp;
#ifdef Q_OS_MAC
    regExp.setPattern("^(file:/{2})"); //on unix systems path started with '/'
#elif defined Q_OS_WIN
    regExp.setPattern("^(file:/{3})");
#endif //Q_OS_MAC Q_OS_WIN;
    str.replace(regExp, "");
    return str;
}

QVariant FileSystemHelper::isDirExists(QVariant dirPath)
{
    if (!dirPath.canConvert<QString>() || dirPath.toString().isEmpty())
    {
        return false;
    }
    QDir dir(dirPath.toString());
    return dir.exists();
}

QVariant FileSystemHelper::FindCMakeBin(QVariant pathToDavaFramework)
{
    if (!pathToDavaFramework.canConvert<QString>())
    {
        return "";
    }
    QString davaFolder = "dava.framework";
    QString davaPath = pathToDavaFramework.toString();
    int index = davaPath.indexOf(davaFolder);
    if (index == -1)
    {
        return "";
    }
    davaPath = davaPath.left(davaPath.indexOf(index + davaFolder.length()));
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
    return cmakePath;
}

QVariant FileSystemHelper::ClearBuildFolder(QVariant buildFolder)
{
    if (!buildFolder.canConvert<QString>())
    {
        return false;
    }
    QString folderPath = buildFolder.toString();
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
