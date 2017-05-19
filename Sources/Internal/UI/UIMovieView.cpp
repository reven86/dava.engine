#include "UI/UIMovieView.h"
#include "UI/UIControlSystem.h"

#include "Engine/Engine.h"

#if defined(DISABLE_NATIVE_MOVIEVIEW)
// Use stub movie control
#define DRAW_PLACEHOLDER_FOR_STUB_UIMOVIEVIEW
#include "Platform/MovieViewControlStub.h"
#include "Render/RenderHelper.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "UI/Private/iOS/MovieViewControliOS.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "UI/Private/OSX/MovieViewControlMacOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "UI/Private/Android/MovieViewControlAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "UI/Private/UWP/MovieViewControlUWP.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/MovieViewControlWin32.h"
#else
// UIMovieView is not implemented for this platform yet, using stub one.
#define DRAW_PLACEHOLDER_FOR_STUB_UIMOVIEVIEW
#include "Platform/MovieViewControlStub.h"
#include "Render/RenderHelper.h"
#endif
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Update/UIUpdateComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIMovieView)
{
    ReflectionRegistrator<UIMovieView>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIMovieView* o) { o->Release(); })
    .End();
}

UIMovieView::UIMovieView(const Rect& rect)
    : UIControl(rect)
#if defined(__DAVAENGINE_COREV2__)
    , movieViewControl(std::make_shared<MovieViewControl>(Engine::Instance()->PrimaryWindow()))
#else
    , movieViewControl(std::make_shared<MovieViewControl>())
#endif
{
    movieViewControl->Initialize(rect);
    UpdateControlRect();
    GetOrCreateComponent<UIUpdateComponent>();
}

UIMovieView::~UIMovieView()
{
    movieViewControl->OwnerIsDying();
}

void UIMovieView::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    movieViewControl->OpenMovie(moviePath, params);
}

void UIMovieView::SetPosition(const Vector2& position)
{
    UIControl::SetPosition(position);
    UpdateControlRect();
}

void UIMovieView::SetSize(const Vector2& newSize)
{
    UIControl::SetSize(newSize);
    UpdateControlRect();
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

bool UIMovieView::IsPlaying() const
{
    return movieViewControl->IsPlaying();
}

void UIMovieView::UpdateControlRect()
{
    Rect rect = GetAbsoluteRect();
    movieViewControl->SetRect(rect);
}

void UIMovieView::Draw(const UIGeometricData& parentGeometricData)
{
    UIControl::Draw(parentGeometricData);
    movieViewControl->Draw(parentGeometricData);
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

void UIMovieView::Update(float32 timeElapsed)
{
    UIControl::Update(timeElapsed);
    movieViewControl->Update();
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

void UIMovieView::OnActive()
{
    UIControl::OnActive();
    UpdateControlRect();
}

UIMovieView* UIMovieView::Clone()
{
    UIMovieView* uiMoviewView = new UIMovieView(GetRect());
    uiMoviewView->CopyDataFrom(this);
    return uiMoviewView;
}
} // namespace DAVA
