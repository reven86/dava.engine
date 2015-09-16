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


#include "ArchiveExtraction.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{

String GetTempFileName()
{
    Array<char, 128> tempPath {};
    ::GetTempPathA(tempPath.size(), tempPath.data());

    Array<char, MAX_PATH> tempFilePath {};
    ::GetTempFileNameA(tempPath.data(), "DAVA", 0, tempFilePath.data());

    return tempFilePath.data();
}

//We don't have any API for working with ZIP, so we use Python for extracting manifest
//https://upload.wikimedia.org/wikipedia/ru/7/78/Trollface.svg
bool ExtractFileFromArchive(const String& zipFile, const String& file, const String& outFile)
{
    FileSystem* fs = FileSystem::Instance();
    FilePath scriptFilePath(fs->GetCurrentExecutableDirectory(), "extraction.py");

    String script = "import zipfile                         \n"
                    "zf = zipfile.ZipFile('" +
    zipFile + "')\n"
              "data = zf.read('" +
    file + "')         \n"
           "file = open('" +
    outFile + "', 'wb')   \n"
              "file.write(data)                       \n";

    RefPtr<File> scriptFile(File::PureCreate(scriptFilePath, File::CREATE | File::WRITE));
    if (!scriptFile)
        return false;

    scriptFile->WriteString(script);
    scriptFile.Reset();

    int res = ::system(("python.exe " + scriptFilePath.GetAbsolutePathname()).c_str());
    fs->DeleteFile(scriptFilePath);

    return res == 0;
}

}  // namespace DAVA