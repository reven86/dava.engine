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


#include "FileListTest.h"

using namespace DAVA;


FileListTest::FileListTest()
    :   TestTemplate<FileListTest>("FileListTest")
{
    FileSystem::Instance()->DeleteDirectory("~doc:/TestData/FileListTest/", true);
    RecursiveCopy("~res:/TestData/FileListTest/", "~doc:/TestData/FileListTest/");

	RegisterFunction(this, &FileListTest::ResTestFunction, String("ResTestFunction"), NULL);
	RegisterFunction(this, &FileListTest::DocTestFunction, String("DocTestFunction"), NULL);
    RegisterFunction(this, &FileListTest::HiddenFileTest, String("HiddenAttrTest"), NULL);
    RegisterFunction(this, &FileListTest::HiddenDirTest, String("HiddenDirTest"), NULL);
}

void FileListTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void FileListTest::UnloadResources()
{
	RemoveAllControls();
}

void FileListTest::ResTestFunction(PerfFuncData * data)
{
    Logger::Debug("[FileListTest::ResTestFunction]");

    ScopedPtr<FileList> fileList( new FileList("~res:/TestData/FileListTest/") );

    TEST_VERIFY(fileList->GetDirectoryCount() == 4);
    TEST_VERIFY(fileList->GetFileCount() == 0);

    for(int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
    {
        if(fileList->IsNavigationDirectory(ifo)) continue;

        String filename = fileList->GetFilename(ifo);
        FilePath pathname = fileList->GetPathname(ifo);
        ScopedPtr<FileList> files( new FileList(pathname) );
        TEST_VERIFY(files->GetDirectoryCount() == 0);
        
        if(filename == "Folder1")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/");
            TEST_VERIFY(files->GetFileCount() == 4);

            for(int32 ifi = 0; ifi < files->GetCount(); ++ifi)
            {
                if(files->IsNavigationDirectory(ifi)) continue;

                String filename = files->GetFilename(ifi);
                FilePath pathname = files->GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file1");
                }
                else if (filename == ".file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/.file1");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file2.txt");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if (filename == ".Folder1")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/.Folder1/");
            TEST_VERIFY(files->GetFileCount() == 1);
        }
        else if(filename == "Folder2")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/");
            TEST_VERIFY(files->GetFileCount() == 6);
            for(int32 ifi = 0; ifi < files->GetCount(); ++ifi)
            {
                if(files->IsNavigationDirectory(ifi)) continue;

                String filename = files->GetFilename(ifi);
                FilePath pathname = files->GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file1");
                }
                else if(filename == "file1.txt")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file1.txt");
                }
                else if(filename == "file2")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file2");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file2.txt");
                }
                else if(filename == "file3")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file3");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if(filename == "Folder3")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/");
            TEST_VERIFY(files->GetFileCount() == 2);
            for(int32 ifi = 0; ifi < files->GetCount(); ++ifi)
            {
                if(files->IsNavigationDirectory(ifi)) continue;
                
                String filename = files->GetFilename(ifi);
                FilePath pathname = files->GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/file1");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else
        {
            TEST_VERIFY(false);
        }
    }
}

void FileListTest::DocTestFunction(PerfFuncData * data)
{
    Logger::Debug("[FileListTest::DocTestFunction]");
    
    ScopedPtr<FileList> fileList( new FileList("~doc:/TestData/FileListTest/") );
    
    TEST_VERIFY(fileList->GetDirectoryCount() == 4);
    TEST_VERIFY(fileList->GetFileCount() == 0);
    
    for(int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
    {
        if(fileList->IsNavigationDirectory(ifo)) continue;
        
        String filename = fileList->GetFilename(ifo);
        FilePath pathname = fileList->GetPathname(ifo);
        ScopedPtr<FileList> files( new FileList(pathname) );
        TEST_VERIFY(files->GetDirectoryCount() == 0);
        
        if(filename == "Folder1")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/");
            TEST_VERIFY(files->GetFileCount() == 4);
            
            for(int32 ifi = 0; ifi < files->GetCount(); ++ifi)
            {
                if(files->IsNavigationDirectory(ifi)) continue;
                
                String filename = files->GetFilename(ifi);
                FilePath pathname = files->GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file1");
                }
                else if(filename == ".file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/.file1");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file2.txt");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if (filename == ".Folder1")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/.Folder1/");
            TEST_VERIFY(files->GetFileCount() == 1);
        }
        else if(filename == "Folder2")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/");
            TEST_VERIFY(files->GetFileCount() == 6);
            for(int32 ifi = 0; ifi < files->GetCount(); ++ifi)
            {
                if(files->IsNavigationDirectory(ifi)) continue;
                
                String filename = files->GetFilename(ifi);
                FilePath pathname = files->GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file1");
                }
                else if(filename == "file1.txt")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file1.txt");
                }
                else if(filename == "file2")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file2");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file2.txt");
                }
                else if(filename == "file3")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file3");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if(filename == "Folder3")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/");
            TEST_VERIFY(files->GetFileCount() == 2);
            for(int32 ifi = 0; ifi < files->GetCount(); ++ifi)
            {
                if(files->IsNavigationDirectory(ifi)) continue;
                
                String filename = files->GetFilename(ifi);
                FilePath pathname = files->GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/file1");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else
        {
            TEST_VERIFY(false);
        }
    }
}

auto GetIndex = [](const FileList* files, DAVA::String filename)
{
    auto i = 0;
    for (; i < files->GetCount(); ++i)
    {
        if (files->GetFilename(i) == filename)
            break;
    }
    return i;
};

void FileListTest::HiddenFileTest(PerfFuncData* data)
{
    Logger::Debug(__FUNCTION__);

    //You can't set the Hidden attribute to file or folders on Windows Store platform
#if defined(__DAVAENGINE_WIN32__)
    
    FilePath file1 = FilePath("~res:/TestData/FileListTest/Folder1/file1");
    auto file1str = file1.GetAbsolutePathname();
    auto attrs = GetFileAttributesA(file1str.c_str());
    
    if (attrs & FILE_ATTRIBUTE_HIDDEN)
    {
        SetFileAttributesA(file1str.c_str(), attrs ^ FILE_ATTRIBUTE_HIDDEN );
    }

    ScopedPtr<FileList> files(new FileList("~res:/TestData/FileListTest/Folder1/"));
    TEST_VERIFY(files->GetFileCount() == 4);
    auto i = GetIndex(files, "file1");
    TEST_VERIFY(i < files->GetCount());
    TEST_VERIFY(files->IsHidden(i) == false);

    SetFileAttributesA(file1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);

    files = new FileList("~res:/TestData/FileListTest/Folder1/");
    TEST_VERIFY(files->GetFileCount() == 4);
    i = GetIndex(files, "file1");
    TEST_VERIFY(i < files->GetCount());
    TEST_VERIFY(files->IsHidden(i) == true);

    SetFileAttributesA(file1str.c_str(), attrs);
    
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    
    ScopedPtr<FileList> files(new FileList("~res:/TestData/FileListTest/Folder1/"));
    TEST_VERIFY(files->GetFileCount() == 4);
    for (auto i = 0; i < files->GetCount(); ++i)
    {
        if (!files->IsDirectory(i))
        {
            bool startsWithDot = (files->GetFilename(i)[0] == '.');
            TEST_VERIFY(files->IsHidden(i) == startsWithDot);
        }
    }
    
#endif //PLATFORMS
}

void FileListTest::HiddenDirTest(PerfFuncData* data)
{
    Logger::Debug(__FUNCTION__);
 
    //You can't set the Hidden attribute to file or folders on Windows Store platform
#if defined(__DAVAENGINE_WIN32__)
    
    FilePath dir1 = FilePath("~res:/TestData/FileListTest/Folder1/");
    auto dir1str = dir1.GetAbsolutePathname();
    auto attrs = GetFileAttributesA(dir1str.c_str());
    
    if (attrs & FILE_ATTRIBUTE_HIDDEN)
    {
        SetFileAttributesA(dir1str.c_str(), attrs ^ FILE_ATTRIBUTE_HIDDEN );
    }
    
    ScopedPtr<FileList> files(new FileList("~res:/TestData/FileListTest/"));
    TEST_VERIFY(files->GetDirectoryCount() == 4);
    auto i = GetIndex(files, "Folder1");
    TEST_VERIFY(i < files->GetCount());
    TEST_VERIFY(files->IsHidden(i) == false);
    
    SetFileAttributesA(dir1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
    
    files = new FileList("~res:/TestData/FileListTest/");
    TEST_VERIFY(files->GetDirectoryCount() == 4);
    i = GetIndex(files, "Folder1");
    TEST_VERIFY(i < files->GetCount());
    TEST_VERIFY(files->IsHidden(i) == true);
    
    SetFileAttributesA(dir1str.c_str(), attrs);
    
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    
    ScopedPtr<FileList> files(new FileList("~res:/TestData/FileListTest/"));
    TEST_VERIFY(files->GetDirectoryCount() == 4);
    for (auto i = 0; i < files->GetCount(); ++i)
    {
        if (files->IsDirectory(i))
        {
            bool startsWithDot = (files->GetFilename(i)[0] == '.');
            TEST_VERIFY(files->IsHidden(i) == startsWithDot);
        }
    }
    
#endif //PLATFORMS
}

void FileListTest::RecursiveCopy(const DAVA::FilePath &src, const DAVA::FilePath &dst)
{
    DVASSERT(src.IsDirectoryPathname() && dst.IsDirectoryPathname());
    
    FileSystem::Instance()->CreateDirectory(dst, true);
    FileSystem::Instance()->CopyDirectory(src, dst);

    ScopedPtr<FileList> fileList( new FileList(src) );
    for(int32 i = 0; i < fileList->GetCount(); ++i)
    {
        if(fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
        {
            RecursiveCopy(fileList->GetPathname(i), dst + (fileList->GetFilename(i) + "/"));
        }
    }
}
