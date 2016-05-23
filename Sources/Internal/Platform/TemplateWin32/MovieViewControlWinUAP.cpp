#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/MovieViewControlWinUAP.h"
#include "Platform/TemplateWin32/PrivateMovieViewWinUAP.h"

namespace DAVA
{
MovieViewControl::MovieViewControl()
    : privateImpl(std::make_shared<PrivateMovieViewWinUAP>())
{
}

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

bool MovieViewControl::IsPlaying() const
{
    return privateImpl->IsPlaying();
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
