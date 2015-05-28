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


#import <UIKit/UIKit.h>
#include "DAVAEngine.h"

using namespace DAVA;

extern void FrameworkDidLaunched();

int main(int argc, char *argv[]) {
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	Core::Create();
	FrameworkDidLaunched();

	//TODO: fix this ugly realisation. Orientation need to be geted from the info.plist 
	// and must be inited as part of the core 		
	// DAVA::Core::Init(DAVA::RenderManager::ORIENTATION_PORTRAIT, 320, 480); for example
	// orientation is not a part of the rendering - orientation is part of an application at all
	KeyedArchive * options = DAVA::Core::GetOptions();
	String orientation = options->GetString("orientation", "portrait");
	if (orientation == "portrait")
	{
		DAVA::RenderManager::Init(DAVA::RenderManager::ORIENTATION_PORTRAIT, 320, 480);
	}
	else if (orientation == "landscape_left")
	{
		DAVA::RenderManager::Init(DAVA::RenderManager::ORIENTATION_LANDSCAPE_LEFT, 320, 480);
	}
	else if (orientation == "landscape_right")
	{
		DAVA::RenderManager::Init(DAVA::RenderManager::ORIENTATION_LANDSCAPE_RIGHT, 320, 480);
	}
	else if (orientation == "portrait_upside_down")
	{
		DAVA::RenderManager::Init(DAVA::RenderManager::ORIENTATION_PORTRAIT_UPSIDE_DOWN, 320, 480);
	}
	
	int retVal = UIApplicationMain(argc, argv, nil, nil);
	
	[pool release];
    return retVal;
}
