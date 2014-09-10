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


#include "FileManagerWrapper.h"

using namespace DAVA;

const Vector<String> FileManagerWrapper::GetFileListByExtension(const String& path, const String& ext, int32 maxLevel)
{
    ScopedPtr<FileList> fileList( new FileList(path) );

    Vector<String> list;
    for(int32 i = 0; i < fileList->GetCount(); ++i)
    {
        if(fileList->IsDirectory(i))
		{
			if (maxLevel > 0)
			{
				String subDirName = fileList->GetFilename(i) + "/";
				if (subDirName == "./" || subDirName == "../")
				{
					continue;
				}

				Vector<String> subDirList = GetFileListByExtension(path + subDirName, ext, maxLevel - 1);
				for (Vector<String>::iterator it = subDirList.begin(); it != subDirList.end(); ++it)
				{
					list.push_back(subDirName + *it);
				}
			}
			else
			{
				continue;
			}
		}

        String curFileName = fileList->GetFilename(i);
        int32 dotIndex = curFileName.rfind('.');
        if(dotIndex != String::npos)
        {
            String curFileExt = curFileName.substr(dotIndex);

            if(curFileExt == ext)
                list.push_back(curFileName);
        }
    }

    return list;
}