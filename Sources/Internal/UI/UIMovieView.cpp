#include "Base/Platform.h"

#include "UI/UIMovieView.h"

#if defined(__DAVAENGINE_IPHONE__)
#include "Platform/TemplateiOS/MovieViewControliOS.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Platform/TemplateMacOS/MovieViewControlMacOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/MovieViewControlAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Platform/TemplateWin32/MovieViewControlWinUAP.h"
#else
// UIMovieView is not implemented for this platform yet, using stub one.
#define DRAW_PLACEHOLDER_FOR_STUB_UIMOVIEVIEW
#include "Platform/MovieViewControlStub.h"
#include "Render/RenderHelper.h"
#endif
#include "Render/2D/Systems/RenderSystem2D.h"

namespace DAVA
{
UIMovieView::UIMovieView(const Rect& rect)
    : UIControl(rect)
    , movieViewControl(new MovieViewControl())
{
    movieViewControl->Initialize(rect);
}

UIMovieView::~UIMovieView()
{
    SafeDelete(movieViewControl);
}

void UIMovieView::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    movieViewControl->OpenMovie(moviePath, params);
}

void UIMovieView::SetPosition(const Vector2& position)
{
    UIControl::SetPosition(position);

    Rect newRect = GetRect();
    movieViewControl->SetRect(newRect);
}

void UIMovieView::SetSize(const Vector2& newSize)
{
    UIControl::SetSize(newSize);

    Rect newRect = GetRect();
    movieViewControl->SetRect(newRect);
}

void UIMovieView::Play()
{
    movieViewControl->Play();
}

void UIMovieView::Stop()
{
    movieViewControl->Stop();
}

void UIMovieView::Pause()
{
    movieViewControl->Pause();
}

void UIMovieView::Resume()
{
    movieViewControl->Resume();
}

bool UIMovieView::IsPlaying()
{
    return movieViewControl->IsPlaying();
}

void UIMovieView::SystemDraw(const UIGeometricData& geometricData)
{
    UIControl::SystemDraw(geometricData);

#if defined(DRAW_PLACEHOLDER_FOR_STUB_UIMOVIEVIEW)
    static Color drawColor(Color(1.0f, 0.4f, 0.8f, 1.0f));

    Rect absRect = GetAbsoluteRect();
    RenderSystem2D::Instance()->DrawRect(absRect, drawColor);

    float32 minRadius = Min(GetSize().x, GetSize().y);
    RenderSystem2D::Instance()->DrawCircle(absRect.GetCenter(), minRadius / 2, drawColor);
    RenderSystem2D::Instance()->DrawCircle(absRect.GetCenter(), minRadius / 3, drawColor);
    RenderSystem2D::Instance()->DrawCircle(absRect.GetCenter(), minRadius / 4, drawColor);
#endif
}

void UIMovieView::OnVisible()
{
    UIControl::OnVisible();
    movieViewControl->SetVisible(true);
}

void UIMovieView::OnInvisible()
{
    UIControl::OnInvisible();
    movieViewControl->SetVisible(false);
}

UIMovieView* UIMovieView::Clone()
{
    UIMovieView* uiMoviewView = new UIMovieView(GetRect());
    uiMoviewView->CopyDataFrom(this);
    return uiMoviewView;
}

} // namespace DAVA
