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


#include <iostream>

#include <FileSystem/FileSystem.h>
#include <FileSystem/ResourceArchive.h>
#include <FileSystem/FileList.h>
#include <CommandLine/ProgramOptions.h>

using namespace DAVA;

class Packer
{
public:
	Packer(const String & _resourceName, const String & _packedDir)
        : fileSystem{ new FileSystem() }
        , resourceArchive(new ResourceArchive(), [](ResourceArchive* ar){SafeRelease(ar); })
        , resourceName(_resourceName)
        , packedDir(_packedDir) 
	{
	}

	~Packer()
	{
        fileSystem->Release();
	}
	

	bool Pack(const String& compression)
	{
        auto dirName = ExtractName(packedDir);
		CollectAllFilesInDirectory(packedDir, "");
	

		std::cout << "Saving archive... " << '\n';
		std::cout << "file count:" << resourceCount << '\n';
			
		// saving process
		auto name = fileNameList.begin();
	
		auto resourceRealSum = 0;
		auto resourcePackedSum = 0;


		for (auto file = 0; file < resourceCount; ++file)
		{
			int32 resourcePackedSize; 
			int32 resourceRealSize;
						
			if (-1 == resourceArchive->SaveProgress(&resourcePackedSize, &resourceRealSize))
			{
				std::cout << "*** Resource Archive Error\n";
				std::cout << "file:" << *name << '\n';
				return false;
			}

			resourceRealSum += resourceRealSize;
			resourcePackedSum += resourcePackedSize;

			// process information
			std::cout << "file:" << *name;
			std::cout << "size:" << resourceRealSize << " packed: " << resourcePackedSize << '\n';

			++name;
		}

		// pack summary	
		std::cout << "Summary:\n"
		          << "size: " << resourceRealSum << '\n'
		          << "packed size: " << resourcePackedSum << '\n'
		          << "compression rate: " << (static_cast<float32>(resourcePackedSum) / resourceRealSum) << '\n';
		
        return true;
	}	

	String ExtractName(const String & name)
	{		
		auto n = name.rfind("/");
        if (n == String::npos)
        {
            return name;
        }
		else
		{
            if (n == name.length())
            {
                n = name.rfind("/", n - 1);
            }

			return name.substr(n);
		}
	}
	
	void CollectAllFilesInDirectory(const String & pathDirName, const String & redusedPath)
	{
        FilePath pathToDir = pathDirName;
        fileSystem->SetCurrentWorkingDirectory(pathToDir);
        auto includeHidden = false;
        auto * fileList = new FileList(pathToDir, includeHidden);
		for (auto file = 0; file < fileList->GetCount(); ++file)
		{
			if (fileList->IsDirectory(file))
			{
                auto directoryName = fileList->GetFilename(file);
                if ((directoryName != "..") && (directoryName != "."))
				{
                    std::cout << "Directory: " << directoryName << '\n';
                    CollectAllFilesInDirectory(directoryName + '/', redusedPath + directoryName + '/');
				}
			}else
			{
                auto filename = fileList->GetFilename(file);
				auto pathname = redusedPath + filename;
				
				std::cout << "file: " << pathname << '\n';
				fileNameList.push_back(pathname);

				resourceArchive->AddFile(pathname);
				resourceCount++;
			}
		}
		fileList->Release();
	}
private:

    FileSystem*	    fileSystem = nullptr;
    std::unique_ptr<ResourceArchive, void(*)(ResourceArchive*)> resourceArchive;

	List<String>	fileNameList;

	String			resourceName;
	String			packedDir;
	int32			resourceCount = 0;
};

void FrameworkDidLaunched()
{
    
}

void FrameworkWillTerminate()
{
    
}

int PackDirectoryIntoPakfile(const String& dir, const String& pak, const String& compression)
{
    auto result = EXIT_FAILURE;

    std::cout << "===================================================\n"
              << "=== Packer started\n"
              << "=== Pack directory: " << dir << '\n'
              << "=== Pack archiveName: " << pak << '\n'
              << "===================================================\n";

    auto dirWithSlash = (dir.back() == '/' ? dir : dir + '/');

    Packer packer(pak, dirWithSlash);

    if (packer.Pack(compression))
    {
        result = EXIT_SUCCESS;
    }

    return result;
}

int UnpackPackfileIntoDirectory(const String& pak, const String& dir)
{
    auto fs = new FileSystem();
    DVASSERT(fs);
    auto programmPath = fs->GetCurrentWorkingDirectory().GetAbsolutePathname();

    auto pathArchiveNameDir = programmPath + "/" + pak;

    std::cout << "===================================================\n"
        << "=== Unpacker started\n"
        << "=== Unpack directory: " << dir << '\n'
        << "=== Unpack archiveName: " << pak << '\n'
        << "===================================================\n";


    fs->CreateDirectory(dir);

    std::unique_ptr<ResourceArchive, void (*)(ResourceArchive*)> ra(new ResourceArchive(), [](ResourceArchive*ptr){ SafeRelease(ptr); });
    ra->Open(pathArchiveNameDir);

    // TODO
    return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
    auto result = EXIT_FAILURE;

    ProgramOptions packOptions("pack");
    packOptions.AddOption("--compression", VariantType(String("lz4")), "compression method, lz4 - default");
    packOptions.AddArgument("directory");
    packOptions.AddArgument("pakfile");

    ProgramOptions unpackOptions("unpack");
    unpackOptions.AddArgument("directory");
    unpackOptions.AddArgument("pakfile");

    if (packOptions.Parse(argc, argv))
    {
        auto compression = packOptions.GetOption("--compression").AsString();

        auto dirName = packOptions.GetArgument("directory");
        auto pakFile = packOptions.GetArgument("pakfile");

        result = PackDirectoryIntoPakfile(dirName, pakFile, compression);
    } else if (unpackOptions.Parse(argc, argv))
    {
        auto pakFile = unpackOptions.GetArgument("pakfile");
        auto dirName = unpackOptions.GetArgument("directory");
        
        result = UnpackPackfileIntoDirectory(pakFile, dirName);
    } else
    {
        packOptions.PrintUsage();
        unpackOptions.PrintUsage();
    }

	return result;
}



