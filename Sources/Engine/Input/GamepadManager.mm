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



#include <GameController/GameController.h>
#import "GamepadManager.h"
#import "GamepadDevice.h"

using namespace DAVA;

@interface GamepadManagerIOS : NSObject

- (id)initWithParent:(GamepadManager *)parent;
- (GamepadDevice *)getCurrentGamepad;
@property(nonatomic, retain) GCController *currentController;

@end


namespace DAVA
{

    GamepadManager::GamepadManager()
    {
        if(NSFoundationVersionNumber > NSFoundationVersionNumber_iOS_6_1)
        {
            m_gamepadManagerIOS = [[GamepadManagerIOS alloc] initWithParent:this];
        }
    }

    GamepadDevice *GamepadManager::GetCurrentGamepad()
    {
        if(NSFoundationVersionNumber > NSFoundationVersionNumber_iOS_6_1)
        {
            return [(GamepadManagerIOS *)m_gamepadManagerIOS getCurrentGamepad];
        }
        else
        {
            return NULL;
        }
    }
}


@implementation GamepadManagerIOS {
@private
    bool _isListening;
    GamepadManager *_parent;
    RefPtr<GamepadDevice> _currentGamepad;
}

@synthesize currentController = _currentController;

- (id)initWithParent:(GamepadManager *)parent {
    self = [super init];
    if (self) {
        _isListening = NO;
        _parent = parent;
        [self startListening];
    }

    return self;
}

- (void)dealloc {
    [self stopListening];
    self.currentController = nil;
    [super dealloc];
}

- (GamepadDevice *)getCurrentGamepad {
    return _currentGamepad.Get();
}

- (void)startListening {
    if(!_isListening) {
        [self findController];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidConnect) name:@"GCControllerDidConnectNotification" object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidDisconnect) name:@"GCControllerDidDisconnectNotification" object:nil];
        _isListening = YES;
    }
}

- (void)stopListening {
    if(_isListening) {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        _isListening = NO;
    }
}

- (void)gameControllerDidConnect {
    [self findController];
}

- (void)gameControllerDidDisconnect {
    if(_currentController) {
        _currentController.gamepad.valueChangedHandler = nil;
        _currentController.extendedGamepad.valueChangedHandler = nil;
        _currentController = nil;
        _currentGamepad = NULL;
    }
}

- (eDavaGamepadProfile)getProfile
{
    if(_currentController == nil) {
        return eDavaGamepadProfile_None;
    } else {
        return _currentController.extendedGamepad ? eDavaGamepadProfile_ExtendedProfile : eDavaGamepadProfile_SimpleProfile;
    }
}


- (void)findController {
    self.currentController = nil;
    NSArray *controllers = [GCController controllers];
    if(controllers.count) {
        self.currentController = [controllers objectAtIndex:0];
        _currentGamepad = RefPtr<GamepadDevice>(new GamepadDevice(_currentController));
    }
}

@end


