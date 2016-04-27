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

#ifndef __FRAMEWORK__MOUSECAPTURE_H__
#define __FRAMEWORK__MOUSECAPTURE_H__

#include <memory>
#include "Base/BaseObject.h"

namespace DAVA
{
class UIEvent;
struct MouseDeviceContext;
class MouseDeviceInterface;

enum class eCaptureMode
{
    OFF = 0, //!< Disable any capturing (send absolute xy)
    FRAME, //!< Capture system cursor into window rect (send absolute xy)
    PINING //!<< Capture system cursor on current position (send xy move delta)
};

class MouseDevice final : public BaseObject
{
public:
    MouseDevice();
    ~MouseDevice();
    MouseDevice(const MouseDevice&) = delete;
    MouseDevice& operator=(const MouseDevice&) = delete;

    void SetMode(eCaptureMode newMode);
    eCaptureMode GetMode() const;
    bool IsPinningEnabled() const;
    // Deprecated, only for UIControlSystem internal using
    DAVA_DEPRECATED(bool SkipEvents(const UIEvent* event));

private:
    void SetSystemMode(eCaptureMode sysMode);
    MouseDeviceContext* context;
    MouseDeviceInterface* privateImpl;
};

class MouseDeviceInterface
{
public:
    virtual void SetMode(eCaptureMode newMode) = 0;
    virtual void SetCursorInCenter() = 0;
    virtual bool SkipEvents(const UIEvent* event) = 0;
    virtual ~MouseDeviceInterface() = default;
};

} //  namespace DAVA

#endif //  __FRAMEWORK__MOUSECAPTURE_H__