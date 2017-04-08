#ifndef __DAVAENGINE_UIMOVIEVIEW__H__
#define __DAVAENGINE_UIMOVIEVIEW__H__

#include "Base/BaseTypes.h"

#include "UI/UIControl.h"
#include "UI/IMovieViewControl.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
// The purpose of UIMovieView class is to display movies.
class UIMovieView : public UIControl
{
    DAVA_VIRTUAL_REFLECTION(UIMovieView, UIControl);

public:
    UIMovieView(const Rect& rect = Rect());

protected:
    virtual ~UIMovieView();

public:
    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params);

    // Overloaded virtual methods.
    void SetPosition(const Vector2& position) override;
    void SetSize(const Vector2& newSize) override;

    void SystemDraw(const UIGeometricData& geometricData, const DAVA::UIControlBackground* parentBackground) override;
    void Draw(const UIGeometricData& parentGeometricData) override;
    void Update(float32 timeElapsed) override;

    void OnVisible() override;
    void OnInvisible() override;
    void OnActive() override;

    UIMovieView* Clone() override;

    // Start/stop the video playback.
    void Play();
    void Stop();

    // Pause/resume the playback.
    void Pause();
    void Resume();

    // Whether the movie is being played?
    bool IsPlaying() const;

private:
    void UpdateControlRect();

protected:
    // Platform-specific implementation of the Movie Control.
    // Make impl to be controlled by std::shared_ptr as on some platforms (e.g. uwp, android)
    // impl can live longer than its owner: native control can queue callback in UI thread
    // but impl's owner is already dead
    std::shared_ptr<IMovieViewControl> movieViewControl;
};

} // namespace DAVA

#endif // __DAVAENGINE_UIMOVIEVIEW__H__
