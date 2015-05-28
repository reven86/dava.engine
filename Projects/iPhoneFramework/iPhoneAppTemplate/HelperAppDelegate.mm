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


#import "HelperAppDelegate.h"
#include "UIScreenManager.h"
#include "ApplicationCore.h"
#include "MemoryManager.h"

extern  void FrameworkWillTerminate();

@implementation HelperAppDelegate

- (void)applicationDidFinishLaunching:(UIApplication *)application 
{
	DAVA::Core::GetApplicationCore()->OnAppStarted();
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	DAVA::Core::GetApplicationCore()->Resume();
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	DAVA::Core::GetApplicationCore()->Suspend();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	DAVA::Core::GetApplicationCore()->GoBackground();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	DAVA::Core::GetApplicationCore()->Resume();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	NSLog(@"Application termination started");
	DAVA::Core::GetApplicationCore()->OnAppFinished();
	NSLog(@"System release started");
	DAVA::UIScreenManager::Instance()->Release(); 
	DAVA::Core::Release();

	DAVA::Sprite::DumpSprites();
	DAVA::Texture::DumpTextures();
#ifdef ENABLE_MEMORY_MANAGER
	if (DAVA::MemoryManager::Instance() != 0)
	{
		DAVA::MemoryManager::Instance()->FinalLog();
	}
#endif
	FrameworkWillTerminate();
	NSLog(@"Application termination finished");
}

@end
