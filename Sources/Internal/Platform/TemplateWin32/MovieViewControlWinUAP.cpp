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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/MovieViewControlWinUAP.h"
#include "Platform/TemplateWin32/PrivateMovieViewWinUAP.h"

namespace DAVA
{

MovieViewControl::MovieViewControl()
    : privateImpl(std::make_shared<PrivateMovieViewWinUAP>())
{}

MovieViewControl::~MovieViewControl()
{
    // Tell private implementation that owner is sentenced to death
    privateImpl->OwnerAtPremortem();
}

void MovieViewControl::Initialize(const Rect& rect)
{
    privateImpl->Initialize(rect);
}

void MovieViewControl::SetRect(const Rect& rect)
{
    privateImpl->SetRect(rect);
}

void MovieViewControl::SetVisible(bool isVisible)
{
    privateImpl->SetVisible(isVisible);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    privateImpl->OpenMovie(moviePath, params);
}

void MovieViewControl::Play()
{
    privateImpl->Play();
}

void MovieViewControl::Stop()
{
    privateImpl->Stop();
}

void MovieViewControl::Pause()
{
    privateImpl->Pause();
}

void MovieViewControl::Resume()
{
    privateImpl->Resume();
}

bool MovieViewControl::IsPlaying()
{
    return privateImpl->IsPlaying();
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
