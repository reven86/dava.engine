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


#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>

class FileManager
{
public:
    static FileManager * Instance();

    const QString & GetDocumentsDirectory();
    const QString & GetBaseAppsDirectory();
    const QString & GetTempDirectory();
    const QString & GetLauncherDirectory();
    const QString & GetSelfUpdateTempDirectory();

    const QString & GetTempDownloadFilepath();

    bool DeleteDirectory(const QString & path);
    void ClearTempDirectory();

    void MakeDirectory(const QString & path);

    void MoveFilesOnlyToDirectory(const QString & dirFrom, const QString & dirTo);
    //directories path must be with '/' at the end

    QString GetApplicationFolder(const QString & branchID, const QString & appID);
    QString GetBranchFolder(const QString & branchID);

private:
    FileManager();

    static FileManager * instance;

    QString docDir;
    QString launcherDir;
    QString baseAppDir;
    QString tempDir;
    QString tempSelfUpdateDir;
    QString tempFile;
};

#endif // FILEMANAGER_H
