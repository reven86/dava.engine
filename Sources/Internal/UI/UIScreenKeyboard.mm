//
//  UIScreenKeyboard.m
//  TestBed
//
//  Created by m_polubisok on 3/21/16.
//
//

#include "UIScreenKeyboard.h"
#import <Foundation/Foundation.h>

#import "Platform/TemplateiOS/RenderView.h"
#import "Platform/TemplateiOS/HelperAppDelegate.h"
#import "Platform/TemplateiOS/RenderViewController.h"

#include "UI/UIScreenManager.h"

namespace DAVA
{
void UIScreenKeyboard::OpenKeyboard()
{
    RenderViewController* controller = static_cast<RenderViewController*>(UIScreenManager::Instance()->GetController(CONTROLLER_GL));
    RenderView* renderView = controller.renderView;

    //Keyboard type sample
    renderView.keyboardType = UIKeyboardTypeEmailAddress;
    renderView.returnKeyType = UIReturnKeyJoin;

    [renderView canBecomeFirstResponder];
    [renderView becomeFirstResponder];
}

void UIScreenKeyboard::CloseKeyboard()
{
    RenderViewController* controller = static_cast<RenderViewController*>(UIScreenManager::Instance()->GetController(CONTROLLER_GL));
    RenderView* renderView = controller.renderView;

    [renderView resignFirstResponder];
}
}