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


#include "uwp_runner.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

using namespace DAVA;

bool ExtractManifestFromPackage(const String& zipFile, const String& outFile);

String ReadManifest(const FilePath& package)
{
    FileSystem* fs = FileSystem::Instance();
    FilePath manifestFilePath(fs->GetCurrentExecutableDirectory(), "manifest");
    SCOPE_EXIT { fs->DeleteFile(manifestFilePath); };

    if (!ExtractManifestFromPackage(package.GetAbsolutePathname(), 
                                    manifestFilePath.GetAbsolutePathname()))
    {
        return "";
    }

    RefPtr<File> manifestFile(File::PureCreate(manifestFilePath, File::OPEN | File::READ));
    String content;
    manifestFile->ReadString(content);
    
    return content;
}


//We don't have any API for working with ZIP, so we use Python for extracting manifest
//https://upload.wikimedia.org/wikipedia/ru/7/78/Trollface.svg
bool ExtractManifestFromPackage(const String& zipFile, const String& outFile)
{
    FileSystem* fs = FileSystem::Instance();
    FilePath scriptFilePath(fs->GetCurrentExecutableDirectory(), "extraction.py");

    String script = "import zipfile                         \n"
                    "zf = zipfile.ZipFile('" + zipFile + "')\n"
                    "data = zf.read('AppxManifest.xml')     \n"
                    "file = open('" + outFile + "', 'w')    \n"
                    "file.write(data)                       \n";

    RefPtr<File> scriptFile(File::PureCreate(scriptFilePath, File::CREATE | File::WRITE));
    if (!scriptFile)
        return false;

    scriptFile->WriteString(script);
    scriptFile.Reset();

    ::system(("python.exe " + scriptFilePath.GetAbsolutePathname()).c_str());
    fs->DeleteFile(scriptFilePath);
    
    return fs->IsFile(outFile);
}