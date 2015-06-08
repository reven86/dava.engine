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


#include "DAVAEngine.h"
#include "GameCore.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TexturePacker/CommandLineParser.h"

#include "TextureCompression/PVRConverter.h"

#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"
#include "TeamcityOutput/TeamcityOutput.h"

using namespace DAVA;
 
void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n"); 
    printf("\t-v or --verbose - detailed output\n");
    printf("\t-s or --silent - silent mode. Log only warnings and errors.\n");
    printf("\t-teamcity - extra output in teamcity format\n");
    printf("\t-md5mode - only process md5 for output resources\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
}


bool CheckPosition(int32 commandPosition)
{
    if(CommandLineParser::CheckPosition(commandPosition))
    {
        printf("Wrong arguments\n");
        PrintUsage();

        return false;
    }
    
    return true;
}


void DumpCommandLine()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    int32 count = CommandLineParser::GetCommandsCount();
    for(int32 i = 0; i < count; ++i)
    {
        String command = CommandLineParser::GetCommand(i);
        Logger::FrameworkDebug("command: %s, param: %s", command.c_str(), CommandLineParser::GetCommandParam(command).c_str());
    }
    
    Logger::FrameworkDebug("");
    
    count = commandLine.size();
    for(int32 i = 0; i < count; ++i)
    {
        String command = commandLine[i];
        Logger::FrameworkDebug("command: %s", command.c_str());
    }
    Logger::FrameworkDebug("");
}

void ProcessRecourcePacker()
{
    if(CommandLineParser::Instance()->GetVerbose())
    {
        DumpCommandLine();
    }

    ResourcePacker2D * resourcePacker = new ResourcePacker2D();
    
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    FilePath commandLinePath(commandLine[1]);
    commandLinePath.MakeDirectoryPathname();
    
    String lastDir = commandLinePath.GetDirectory().GetLastDirectoryName();
    FilePath outputh = commandLinePath + ("../../Data/" + lastDir + "/");
    
    resourcePacker->InitFolders(commandLinePath, outputh);
    
    if(resourcePacker->excludeDirectory.IsEmpty())
    {
        Logger::Error("[FATAL ERROR: Packer has wrong input pathname]");
        return;
    }
    
    if (resourcePacker->excludeDirectory.GetLastDirectoryName() != "DataSource")
    {
        Logger::Error("[FATAL ERROR: Packer working only inside DataSource directory]");
        return;
    }
    
    if(commandLine.size() < 3)
    {
        Logger::Error("[FATAL ERROR: PVRTexTool path need to be second parameter]");
        return;
    }
    
    
    auto toolFolderPath = resourcePacker->excludeDirectory + (commandLine[2] + "/");
#if defined (__DAVAENGINE_MACOS__)
	String pvrTexToolName = String("PVRTexToolCLI");
    String cacheToolName = "AssetCacheClient";
#elif defined (__DAVAENGINE_WIN32__)
	String pvrTexToolName = String("PVRTexToolCLI.exe");
    String cacheToolName = "AssetCacheClient.exe";
#endif
    
    PVRConverter::Instance()->SetPVRTexTool(toolFolderPath + pvrTexToolName);
    resourcePacker->SetCacheClientTool(toolFolderPath + cacheToolName);
    
    
    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::FrameworkDebug("[Resource Packer Started]");
    Logger::FrameworkDebug("[INPUT DIR] - [%s]", resourcePacker->inputGfxDirectory.GetAbsolutePathname().c_str());
    Logger::FrameworkDebug("[OUTPUT DIR] - [%s]", resourcePacker->outputGfxDirectory.GetAbsolutePathname().c_str());
    Logger::FrameworkDebug("[EXCLUDE DIR] - [%s]", resourcePacker->excludeDirectory.GetAbsolutePathname().c_str());
    
    PixelFormatDescriptor::InitializePixelFormatDescriptors();
    GPUFamilyDescriptor::SetupGPUParameters();
    
    
    eGPUFamily exportForGPU = GPU_ORIGIN;
    if(CommandLineParser::CommandIsFound(String("-gpu")))
    {
        String gpuName = CommandLineParser::GetCommandParam(String("-gpu"));
        exportForGPU = GPUFamilyDescriptor::GetGPUByName(gpuName);
		if (GPU_INVALID == exportForGPU)
		{
			exportForGPU = GPU_ORIGIN;
		}
    }
    
    if (CommandLineParser::CommandIsFound(String("-md5mode")))
    {
        resourcePacker->RecalculateMD5ForOutputDir();
    }
    else
    {
        resourcePacker->PackResources(exportForGPU);
    }
    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    Logger::FrameworkDebug("[Resource Packer Compile Time: %0.3lf seconds]", (float64)elapsedTime / 1000.0);
    
    SafeDelete(resourcePacker);
}

void FrameworkDidLaunched()
{
    Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);

	if (Core::Instance()->IsConsoleMode())
	{
        if(     CommandLineParser::GetCommandsCount() < 2
           ||   (CommandLineParser::CommandIsFound(String("-usage")))
           ||   (CommandLineParser::CommandIsFound(String("-help")))
           )
        {
            PrintUsage();
			return;
        }
		
        if(CommandLineParser::CommandIsFound(String("-exo")))
        {
            CommandLineParser::Instance()->SetExtendedOutput(true);
            
            Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
        }
        
        if(CommandLineParser::CommandIsFound(String("-v")) || CommandLineParser::CommandIsFound(String("--verbose")))
        {
            CommandLineParser::Instance()->SetVerbose(true);

            Logger::Instance()->SetLogLevel(Logger::LEVEL_FRAMEWORK);
        }

	if (CommandLineParser::CommandIsFound(String("-s")) || CommandLineParser::CommandIsFound(String("--silent")))
	{
		Logger::Instance()->SetLogLevel(Logger::LEVEL_WARNING);
	}

        if(CommandLineParser::CommandIsFound(String("-teamcity")))
        {
            CommandLineParser::Instance()->SetUseTeamcityOutput(true);

            DAVA::TeamcityOutput *out = new DAVA::TeamcityOutput();
            DAVA::Logger::AddCustomOutput(out);
        }

	}

    ProcessRecourcePacker();
}


void FrameworkWillTerminate()
{
}
