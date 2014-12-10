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


#include "FilePathTest.h"

using namespace DAVA;


FilePathTest::FilePathTest()
    :   TestTemplate<FilePathTest>("FilePathTest")
{
    RegisterFunction(this, &FilePathTest::FilePathOperationsTest, String("FilePathTest Operations"), NULL);
	RegisterFunction(this, &FilePathTest::MacTestFunction, String("FilePathTest Mac OS"), NULL);
	RegisterFunction(this, &FilePathTest::WinTestFunction, String("FilePathTest Win32"), NULL);
	RegisterFunction(this, &FilePathTest::WinStylePathTestFunction, String("FilePathTest Win32 Style"), NULL);
	RegisterFunction(this, &FilePathTest::BundleNameTest, String("FilePathTest BundleNameTest"), NULL);
	RegisterFunction(this, &FilePathTest::AddFunctionalityTest, String("FilePathTest AddFunctionalityTest"), NULL);
}

void FilePathTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void FilePathTest::UnloadResources()
{
	RemoveAllControls();
}

void FilePathTest::FilePathOperationsTest(PerfFuncData * data)
{
    Logger::Debug("[FilePathTest] FilePathOperationsTest");

    FilePath path0("~res:/Gfx/UI/texture.tex");
    FilePath path1("~res:/Gfx/UI/");
    
    FilePath path2 = path1 + "texture.tex";
    TEST_VERIFY(path2 == path0);
    TEST_VERIFY(path2.IsEqualToExtension(".tex"));
    TEST_VERIFY(path2.IsEqualToExtension(".tEx"));
    TEST_VERIFY(path2.IsEqualToExtension(".tEX"));
    TEST_VERIFY(path2.IsEqualToExtension(".teX"));

    FilePath path3 = FilePath::CreateWithNewExtension(path2, ".psd");
    TEST_VERIFY(path3 == "~res:/Gfx/UI/texture.psd");

    path3.TruncateExtension();
    TEST_VERIFY(path3 == "~res:/Gfx/UI/texture");
    TEST_VERIFY(!path3.IsEqualToExtension(".psd"));

    path3.ReplaceFilename("");
    TEST_VERIFY(path3.GetLastDirectoryName() == "UI");

    Logger::Debug("[FilePathTest] FilePathOperationsTest Done");
}

void FilePathTest::MacTestFunction(PerfFuncData * data)
{
    FilePath oldProjectPathname = FilePath::GetBundleName();
    
    Logger::Debug("[FilePathTest] MacOS");

    FilePath::SetBundleName("/TestProject");
    
    FilePath filepath0("~res:/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(!filepath0.IsEmpty());
    TEST_VERIFY(!filepath0.IsDirectoryPathname());
    
    TEST_VERIFY(filepath0.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath0.GetBasename() == "texture");
    TEST_VERIFY(filepath0.GetExtension() == ".tex");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");

    filepath0.ReplaceFilename("file.psd");
    TEST_VERIFY(filepath0.GetFilename() == "file.psd");
    TEST_VERIFY(filepath0.GetBasename() == "file");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");

    filepath0.ReplaceBasename("image");
    TEST_VERIFY(filepath0.GetFilename() == "image.psd");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");

    filepath0.ReplaceExtension(".doc");
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");

    TEST_VERIFY(filepath0.GetAbsolutePathname() == "/TestProject/Data/Gfx/UI/Screen/image.doc")

    
    filepath0.ReplaceDirectory(String("/Mac/Users"));
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == "/Mac/Users/");

    TEST_VERIFY(filepath0.GetAbsolutePathname() == "/Mac/Users/image.doc")

    
    FilePath filepath2(filepath0);
    TEST_VERIFY(filepath0 == filepath2);
    
    FilePath filepath3;
    TEST_VERIFY(filepath3.IsEmpty());
    TEST_VERIFY(!filepath3.IsDirectoryPathname());

    filepath3 = filepath0;
    TEST_VERIFY(filepath0 == filepath3);
    TEST_VERIFY(filepath2 == filepath3);

    
    FilePath filepath4("~res:/Gfx/UI/", "Screen/texture.tex");
    TEST_VERIFY(!filepath4.IsEmpty());
    TEST_VERIFY(!filepath4.IsDirectoryPathname());
    
    TEST_VERIFY(filepath4.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath4.GetBasename() == "texture");
    TEST_VERIFY(filepath4.GetExtension() == ".tex");
    TEST_VERIFY(filepath4.GetDirectory() == "~res:/Gfx/UI/Screen/");

    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/UI/Screen/") == "texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/UI/") == "Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/") == "UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/") == "Gfx/UI/Screen/texture.tex");

    
    FilePath filepath5("~res:/Gfx/UI/", "../Screen/texture.tex");
    TEST_VERIFY(!filepath5.IsEmpty());
    TEST_VERIFY(!filepath5.IsDirectoryPathname());
    
    TEST_VERIFY(filepath5.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath5.GetBasename() == "texture");
    TEST_VERIFY(filepath5.GetExtension() == ".tex");
    TEST_VERIFY(filepath5.GetDirectory() == "~res:/Gfx/Screen/");
    
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/Screen/") == "texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/") == "Gfx/Screen/texture.tex");
    
    FilePath filepath6("~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5 == filepath6);

    FilePath filepath7("~res:/Gfx/Screen/");
    TEST_VERIFY(!filepath7.IsEmpty());
    TEST_VERIFY(filepath7.IsDirectoryPathname());

    FilePath filepath8 = filepath7 + "texture.tex";
    TEST_VERIFY(filepath8 == filepath6);

    
    FilePath::SetBundleName("/TestProject/Data");
    FilePath filepath9("~res:/Gfx/UI/Screen/texture.tex");
    FilePath filepath10("~res:/Gfx/", "../../Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath9 == filepath10);
    TEST_VERIFY(filepath8 != filepath10);

    FilePath filepath11("texture.tex");
    TEST_VERIFY(!filepath11.IsEmpty());
    TEST_VERIFY(!filepath11.IsDirectoryPathname());
    
    TEST_VERIFY(filepath11.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath11.GetBasename() == "texture");
    TEST_VERIFY(filepath11.GetExtension() == ".tex");
    TEST_VERIFY(filepath11.GetDirectory() == FilePath("testFile").GetDirectory());

    filepath11.ReplaceBasename("image");
    TEST_VERIFY(filepath11.GetFilename() == "image.tex");

    filepath11.ReplaceExtension(".psd");
    TEST_VERIFY(filepath11.GetFilename() == "image.psd");

    filepath11.ReplaceFilename("music.mp3");
    TEST_VERIFY(filepath11.GetFilename() == "music.mp3");

    filepath11.ReplaceDirectory(String("/Users/Test"));
    TEST_VERIFY(filepath11.GetDirectory() == "/Users/Test/");

    
    FilePath filepath12("/Users/Test");
    filepath12.MakeDirectoryPathname();
    DVASSERT(filepath12.IsDirectoryPathname());
    
    TEST_VERIFY(filepath12.GetFilename() == "");
    TEST_VERIFY(filepath12.GetBasename() == "");
    TEST_VERIFY(filepath12.GetExtension() == "");
    TEST_VERIFY(filepath12.GetDirectory() == "/Users/Test/");

    
    FilePath filepath13("/Users/Test/file");
    TEST_VERIFY(filepath13.GetFilename() == "file");
    TEST_VERIFY(filepath13.GetBasename() == "file");
    TEST_VERIFY(filepath13.GetExtension() == "");
    TEST_VERIFY(filepath13.GetDirectory() == "/Users/Test/");

    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "/Mac/Users/image.doc");
    TEST_VERIFY(filepath2.GetAbsolutePathname() == "/Mac/Users/image.doc");
    TEST_VERIFY(filepath3.GetAbsolutePathname() == "/Mac/Users/image.doc");
    TEST_VERIFY(filepath4.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath5.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath6.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath7.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/Screen/");
    TEST_VERIFY(filepath8.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath9.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath10.GetAbsolutePathname() == "/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath11.GetAbsolutePathname() == "/Users/Test/music.mp3");
    TEST_VERIFY(filepath12.GetAbsolutePathname() == "/Users/Test/");
    TEST_VERIFY(filepath13.GetAbsolutePathname() == "/Users/Test/file");

    
    Logger::Debug("[FilePathTest] Mac OS Done");
    
    FilePath::SetBundleName(oldProjectPathname);
}

void FilePathTest::WinTestFunction(PerfFuncData * data)
{
	FilePath oldProjectPathname = FilePath::GetBundleName();
    
    Logger::Debug("[FilePathTest] Win32");
    
    FilePath::SetBundleName("c:/TestProject");
    
    FilePath filepath0("~res:/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(!filepath0.IsEmpty());
    TEST_VERIFY(!filepath0.IsDirectoryPathname());
    TEST_VERIFY(filepath0.GetType() == FilePath::PATH_IN_RESOURCES);
    
    TEST_VERIFY(filepath0.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath0.GetBasename() == "texture");
    TEST_VERIFY(filepath0.GetExtension() == ".tex");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");
    
    filepath0.ReplaceFilename("file.psd");
    TEST_VERIFY(filepath0.GetFilename() == "file.psd");
    TEST_VERIFY(filepath0.GetBasename() == "file");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");
    
    filepath0.ReplaceBasename("image");
    TEST_VERIFY(filepath0.GetFilename() == "image.psd");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");
    
    filepath0.ReplaceExtension(".doc");
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:/Gfx/UI/Screen/");
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/TestProject/Data/Gfx/UI/Screen/image.doc")
    
    
    filepath0.ReplaceDirectory(String("c:/Mac/Users"));
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == "c:/Mac/Users/");
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/Mac/Users/image.doc")
    TEST_VERIFY(filepath0.GetType() == FilePath::PATH_IN_FILESYSTEM);

    
    FilePath filepath2(filepath0);
    TEST_VERIFY(filepath0 == filepath2);
    TEST_VERIFY(filepath2.GetType() == FilePath::PATH_IN_FILESYSTEM);

    FilePath filepath3;
    TEST_VERIFY(filepath3.IsEmpty());
    TEST_VERIFY(!filepath3.IsDirectoryPathname());
    TEST_VERIFY(filepath3.GetType() == FilePath::PATH_EMPTY);

    
    filepath3 = filepath0;
    TEST_VERIFY(filepath0 == filepath3);
    TEST_VERIFY(filepath2 == filepath3);
    TEST_VERIFY(filepath3.GetType() == FilePath::PATH_IN_FILESYSTEM);

    
    FilePath filepath4("~res:/Gfx/UI/", "Screen/texture.tex");
    TEST_VERIFY(!filepath4.IsEmpty());
    TEST_VERIFY(!filepath4.IsDirectoryPathname());
    TEST_VERIFY(filepath4.GetType() == FilePath::PATH_IN_RESOURCES);

    
    TEST_VERIFY(filepath4.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath4.GetBasename() == "texture");
    TEST_VERIFY(filepath4.GetExtension() == ".tex");
    TEST_VERIFY(filepath4.GetDirectory() == "~res:/Gfx/UI/Screen/");
    
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/UI/Screen/") == "texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/UI/") == "Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/") == "UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/") == "Gfx/UI/Screen/texture.tex");
    
    
    FilePath filepath5("~res:/Gfx/UI/", "../Screen/texture.tex");
    TEST_VERIFY(!filepath5.IsEmpty());
    TEST_VERIFY(!filepath5.IsDirectoryPathname());
    TEST_VERIFY(filepath5.GetType() == FilePath::PATH_IN_RESOURCES);

    
    TEST_VERIFY(filepath5.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath5.GetBasename() == "texture");
    TEST_VERIFY(filepath5.GetExtension() == ".tex");
    TEST_VERIFY(filepath5.GetDirectory() == "~res:/Gfx/Screen/");
    
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/Screen/") == "texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/") == "Gfx/Screen/texture.tex");
    
    FilePath filepath6("~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5 == filepath6);
    TEST_VERIFY(filepath6.GetType() == FilePath::PATH_IN_RESOURCES);

    
    FilePath filepath7("~res:/Gfx/Screen/");
    TEST_VERIFY(!filepath7.IsEmpty());
    TEST_VERIFY(filepath7.IsDirectoryPathname());
    TEST_VERIFY(filepath7.GetType() == FilePath::PATH_IN_RESOURCES);

    FilePath filepath8 = filepath7 + "texture.tex";
    TEST_VERIFY(filepath8 == filepath6);
    TEST_VERIFY(filepath8.GetType() == FilePath::PATH_IN_RESOURCES);

    
    FilePath::SetBundleName("c:/TestProject/Data");
    FilePath filepath9("~res:/Gfx/UI/Screen/texture.tex");
    FilePath filepath10("~res:/Gfx/", "../../Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath9 == filepath10);
    TEST_VERIFY(filepath8 != filepath10);
    
    TEST_VERIFY(filepath9.GetType() == FilePath::PATH_IN_RESOURCES);
    TEST_VERIFY(filepath10.GetType() == FilePath::PATH_IN_RESOURCES);

    
    
    FilePath filepath11("texture.tex");
    TEST_VERIFY(!filepath11.IsEmpty());
    TEST_VERIFY(!filepath11.IsDirectoryPathname());
    TEST_VERIFY(filepath11.GetType() == FilePath::PATH_IN_FILESYSTEM);

    TEST_VERIFY(filepath11.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath11.GetBasename() == "texture");
    TEST_VERIFY(filepath11.GetExtension() == ".tex");
    TEST_VERIFY(filepath11.GetDirectory() == FilePath("testFile").GetDirectory());
    
    filepath11.ReplaceBasename("image");
    TEST_VERIFY(filepath11.GetFilename() == "image.tex");
    
    filepath11.ReplaceExtension(".psd");
    TEST_VERIFY(filepath11.GetFilename() == "image.psd");
    
    filepath11.ReplaceFilename("music.mp3");
    TEST_VERIFY(filepath11.GetFilename() == "music.mp3");
    
    filepath11.ReplaceDirectory(String("c:/Users/Test"));
    TEST_VERIFY(filepath11.GetDirectory() == "c:/Users/Test/");
    
    FilePath filepath12("c:/Users/Test");
    filepath12.MakeDirectoryPathname();
    DVASSERT(filepath12.IsDirectoryPathname());
    TEST_VERIFY(filepath12.GetType() == FilePath::PATH_IN_FILESYSTEM);

    
    TEST_VERIFY(filepath12.GetFilename() == "");
    TEST_VERIFY(filepath12.GetBasename() == "");
    TEST_VERIFY(filepath12.GetExtension() == "");
    TEST_VERIFY(filepath12.GetDirectory() == "c:/Users/Test/");

    
    FilePath filepath13("c:/Users/Test/file");
    TEST_VERIFY(filepath13.GetFilename() == "file");
    TEST_VERIFY(filepath13.GetBasename() == "file");
    TEST_VERIFY(filepath13.GetExtension() == "");
    TEST_VERIFY(filepath13.GetDirectory() == "c:/Users/Test/");
    TEST_VERIFY(filepath13.GetType() == FilePath::PATH_IN_FILESYSTEM);

    
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath2.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath3.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath4.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath5.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath6.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath7.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/");
    TEST_VERIFY(filepath8.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath9.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath10.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath11.GetAbsolutePathname() == "c:/Users/Test/music.mp3");
    TEST_VERIFY(filepath12.GetAbsolutePathname() == "c:/Users/Test/");
    TEST_VERIFY(filepath13.GetAbsolutePathname() == "c:/Users/Test/file");

    
    Logger::Debug("[FilePathTest] Win32 Done");
    
    FilePath::SetBundleName(oldProjectPathname);
}

void FilePathTest::WinStylePathTestFunction(PerfFuncData * data)
{
	FilePath oldProjectPathname = FilePath::GetBundleName();
    
    Logger::Debug("[FilePathTest] Win32 win style");
    
    FilePath::SetBundleName("c:\\TestProject");
    
    FilePath filepath0("~res:\\Gfx\\UI\\Screen\\texture.tex");
    
    TEST_VERIFY(!filepath0.IsEmpty());
    TEST_VERIFY(!filepath0.IsDirectoryPathname());
    
    TEST_VERIFY(filepath0.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath0.GetBasename() == "texture");
    TEST_VERIFY(filepath0.GetExtension() == ".tex");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:\\Gfx\\UI\\Screen\\");
    
    filepath0.ReplaceFilename("file.psd");
    TEST_VERIFY(filepath0.GetFilename() == "file.psd");
    TEST_VERIFY(filepath0.GetBasename() == "file");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:\\Gfx\\UI\\Screen\\");
    
    filepath0.ReplaceBasename("image");
    TEST_VERIFY(filepath0.GetFilename() == "image.psd");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:\\Gfx\\UI\\Screen\\");
    
    filepath0.ReplaceExtension(".doc");
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == "~res:\\Gfx\\UI\\Screen\\");
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/TestProject/Data/Gfx/UI/Screen/image.doc")
    
    
    filepath0.ReplaceDirectory(String("c:\\Mac\\Users"));
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == "c:\\Mac\\Users\\");
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/Mac/Users/image.doc")
    
    
    FilePath filepath2(filepath0);
    TEST_VERIFY(filepath0 == filepath2);
    
    FilePath filepath3;
    TEST_VERIFY(filepath3.IsEmpty());
    TEST_VERIFY(!filepath3.IsDirectoryPathname());
    
    filepath3 = filepath0;
    TEST_VERIFY(filepath0 == filepath3);
    TEST_VERIFY(filepath2 == filepath3);
    
    
    FilePath filepath4("~res:\\Gfx\\UI\\", "Screen\\texture.tex");
    TEST_VERIFY(!filepath4.IsEmpty());
    TEST_VERIFY(!filepath4.IsDirectoryPathname());
    
    TEST_VERIFY(filepath4.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath4.GetBasename() == "texture");
    TEST_VERIFY(filepath4.GetExtension() == ".tex");
    TEST_VERIFY(filepath4.GetDirectory() == "~res:\\Gfx\\UI\\Screen\\");
    
    TEST_VERIFY(filepath4.GetRelativePathname("~res:\\Gfx\\UI\\Screen\\") == "texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:\\Gfx\\UI\\") == "Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:\\Gfx\\") == "UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:\\") == "Gfx/UI/Screen/texture.tex");
    
    
    FilePath filepath5("~res:\\Gfx\\UI\\", "..\\Screen\\texture.tex");
    TEST_VERIFY(!filepath5.IsEmpty());
    TEST_VERIFY(!filepath5.IsDirectoryPathname());
    
    TEST_VERIFY(filepath5.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath5.GetBasename() == "texture");
    TEST_VERIFY(filepath5.GetExtension() == ".tex");
    TEST_VERIFY(filepath5.GetDirectory() == "~res:\\Gfx\\Screen\\");
    
    TEST_VERIFY(filepath5.GetRelativePathname("~res:\\Gfx\\Screen\\") == "texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:\\Gfx\\") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:\\Gfx\\") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:\\") == "Gfx/Screen/texture.tex");
    
    FilePath filepath6("~res:\\Gfx\\Screen\\texture.tex");
    TEST_VERIFY(filepath5 == filepath6);
    
    FilePath filepath7("~res:\\Gfx\\Screen\\");
    TEST_VERIFY(!filepath7.IsEmpty());
    TEST_VERIFY(filepath7.IsDirectoryPathname());
    
    FilePath filepath8 = filepath7 + "texture.tex";
    TEST_VERIFY(filepath8 == filepath6);
    
    
    FilePath::SetBundleName("c:\\TestProject\\Data");
    FilePath filepath9("~res:\\Gfx\\UI\\Screen\\texture.tex");
    FilePath filepath10("~res:\\Gfx\\", "..\\..\\Data\\Gfx\\UI\\Screen\\texture.tex");
    TEST_VERIFY(filepath9 == filepath10);
    TEST_VERIFY(filepath8 != filepath10);
    
    FilePath filepath11("texture.tex");
    TEST_VERIFY(!filepath11.IsEmpty());
    TEST_VERIFY(!filepath11.IsDirectoryPathname());
    
    TEST_VERIFY(filepath11.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath11.GetBasename() == "texture");
    TEST_VERIFY(filepath11.GetExtension() == ".tex");
    TEST_VERIFY(filepath11.GetDirectory() == FilePath("testFile").GetDirectory());
    
    filepath11.ReplaceBasename("image");
    TEST_VERIFY(filepath11.GetFilename() == "image.tex");
    
    filepath11.ReplaceExtension(".psd");
    TEST_VERIFY(filepath11.GetFilename() == "image.psd");
    
    filepath11.ReplaceFilename("music.mp3");
    TEST_VERIFY(filepath11.GetFilename() == "music.mp3");
    
    filepath11.ReplaceDirectory(String("c:\\Users\\Test"));
    TEST_VERIFY(filepath11.GetDirectory() == "c:\\Users\\Test\\");
    
    FilePath filepath12("c:\\Users\\Test");
    filepath12.MakeDirectoryPathname();
    DVASSERT(filepath12.IsDirectoryPathname());
    
    TEST_VERIFY(filepath12.GetFilename() == "");
    TEST_VERIFY(filepath12.GetBasename() == "");
    TEST_VERIFY(filepath12.GetExtension() == "");
    TEST_VERIFY(filepath12.GetDirectory() == "c:\\Users\\Test\\");
    
    
    FilePath filepath13("c:\\Users\\Test\\file");
    TEST_VERIFY(filepath13.GetFilename() == "file");
    TEST_VERIFY(filepath13.GetBasename() == "file");
    TEST_VERIFY(filepath13.GetExtension() == "");
    TEST_VERIFY(filepath13.GetDirectory() == "c:\\Users\\Test\\");
    
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath2.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath3.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath4.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath5.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath6.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath7.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/");
    TEST_VERIFY(filepath8.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath9.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath10.GetAbsolutePathname() == "c:/TestProject/Data/Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath11.GetAbsolutePathname() == "c:/Users/Test/music.mp3");
    TEST_VERIFY(filepath12.GetAbsolutePathname() == "c:/Users/Test/");
    TEST_VERIFY(filepath13.GetAbsolutePathname() == "c:/Users/Test/file");
    
    
    Logger::Debug("[FilePathTest] Win32 win style Done");
    
    FilePath::SetBundleName(oldProjectPathname);
}

void FilePathTest::BundleNameTest(PerfFuncData * data)
{
    Logger::Debug("[FilePathTest] BundleNameTest");

    FileSystem::Instance()->DeleteDirectory("~doc:/BundleTest/Data/TestData/FileSystemTest/Folder1/");

    FileSystem::eCreateDirectoryResult created = FileSystem::Instance()->CreateDirectory("~doc:/BundleTest/Data/TestData/FileSystemTest/Folder1/", true);
    TEST_VERIFY(created == FileSystem::DIRECTORY_CREATED);

    File *fileForWritting = File::Create("~doc:/BundleTest/Data/TestData/FileSystemTest/Folder1/file1.zip", File::CREATE | File::WRITE);
    if(!fileForWritting)
    {
        TEST_VERIFY(fileForWritting != NULL);
        return;
    }

    
    uint8 buffer[1024];
    TEST_VERIFY(fileForWritting->Write(buffer, 1024) == 1024);
    SafeRelease(fileForWritting);
    

    TEST_VERIFY(HasFileCorrectSize("~res:/TestData/FileSystemTest/Folder1/file1.zip", 31749));
    FilePath::AddResourcesFolder("~doc:/BundleTest/");
    TEST_VERIFY(HasFileCorrectSize("~res:/TestData/FileSystemTest/Folder1/file1.zip", 1024));
    TEST_VERIFY(!HasFileCorrectSize("~res:/TestData/FileSystemTest/Folder1/file1.zip", 31749));
    FilePath::RemoveResourcesFolder("~doc:/BundleTest/");
    TEST_VERIFY(HasFileCorrectSize("~res:/TestData/FileSystemTest/Folder1/file1.zip", 31749));

    Logger::Debug("[FilePathTest] BundleNameTest Done");
}

bool FilePathTest::HasFileCorrectSize(const FilePath &path, uint32 size)
{
    File *file = File::Create(path, File::OPEN | File::READ);
    if(!file)
    {
        Logger::Error("[FilePathTest::HasFileCorrectSize] Can't open file %s", path.GetAbsolutePathname().c_str());
        return false;
    }
    
    bool sizeIsCorrect = (file->GetSize() == size);
    SafeRelease(file);

    return sizeIsCorrect;
}

void FilePathTest::AddFunctionalityTest( PerfFuncData * data )
{
	Logger::Debug("[FilePathTest] Operation + Test");

	FilePath path1("/Folder/");
	FilePath path2 = path1 + "file.txt";
	FilePath path3 = path2 + ".lka";
	FilePath path4 = FilePath() + path2.GetAbsolutePathname();

	TEST_VERIFY(path2 == "/Folder/file.txt");
	TEST_VERIFY(path3 == "/Folder/file.txt.lka");
	TEST_VERIFY(path4 == path2);

	Logger::Debug("[FilePathTest] Operation + Done");
}

