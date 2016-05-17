#include "Tests/UIMovieTest.h"

UIMovieTest::UIMovieTest()
    : BaseScreen("UIMovieTest")
{
}

void UIMovieTest::LoadResources()
{
    movieView = new UIMovieView(Rect(10, 10, 940, 600));
    movieView->OpenMovie("~res://TestData/MovieTest/bunny.m4v", OpenMovieParams());
    movieView->SetDebugDraw(true);
    AddControl(movieView);

    // Create the "player" buttons.
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(14);

    playButton = CreateUIButton(font, Rect(10, 620, 60, 20), "Play", &UIMovieTest::ButtonPressed);
    stopButton = CreateUIButton(font, Rect(80, 620, 60, 20), "Stop", &UIMovieTest::ButtonPressed);
    pauseButton = CreateUIButton(font, Rect(150, 620, 60, 20), "Pause", &UIMovieTest::ButtonPressed);
    resumeButton = CreateUIButton(font, Rect(220, 620, 60, 20), "Resume", &UIMovieTest::ButtonPressed);
    hideButton = CreateUIButton(font, Rect(290, 620, 60, 20), "Hide", &UIMovieTest::ButtonPressed);
    showButton = CreateUIButton(font, Rect(360, 620, 60, 20), "Show", &UIMovieTest::ButtonPressed);

    buttonScale0 = CreateUIButton(font, Rect(10, 660, 100, 20), "None", &UIMovieTest::ScaleButtonPressed);
    buttonScale1 = CreateUIButton(font, Rect(120, 660, 100, 20), "Aspect fit", &UIMovieTest::ScaleButtonPressed);
    buttonScale2 = CreateUIButton(font, Rect(10, 690, 100, 20), "Aspect fill", &UIMovieTest::ScaleButtonPressed);
    buttonScale3 = CreateUIButton(font, Rect(120, 690, 100, 20), "Fill", &UIMovieTest::ScaleButtonPressed);

    playerStateText = new UIStaticText(Rect(470, 620, 100, 20));
    playerStateText->SetFont(font);
    AddControl(playerStateText);

    SafeRelease(font);

    BaseScreen::LoadResources();
}

void UIMovieTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(movieView);

    SafeRelease(playButton);
    SafeRelease(stopButton);
    SafeRelease(pauseButton);
    SafeRelease(resumeButton);
    SafeRelease(hideButton);
    SafeRelease(showButton);

    SafeRelease(buttonScale0);
    SafeRelease(buttonScale1);
    SafeRelease(buttonScale2);
    SafeRelease(buttonScale2);

    SafeRelease(playerStateText);
}

void UIMovieTest::OnActive()
{
    movieView->Play();
}

void UIMovieTest::Update(float32 timeElapsed)
{
    UpdatePlayerStateText();
    BaseScreen::Update(timeElapsed);
}

void UIMovieTest::UpdatePlayerStateText()
{
    bool isPlaying = movieView->IsPlaying();
    playerStateText->SetText(isPlaying ? L"Playing" : L"Paused");
}

UIButton* UIMovieTest::CreateUIButton(Font* font, const Rect& rect, const String& text,
                                      void (UIMovieTest::*onClick)(BaseObject*, void*, void*))
{
    UIButton* button = new UIButton(rect);
    button->SetStateFont(0xFF, font);
    button->SetStateText(0xFF, StringToWString(text));
    button->SetStateFontColor(0xFF, Color::White);
    button->SetDebugDraw(true);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, onClick));
    AddControl(button);
    return button;
}

void UIMovieTest::ButtonPressed(BaseObject* obj, void* data, void* callerData)
{
    if (obj == playButton)
        movieView->Play();
    else if (obj == stopButton)
        movieView->Stop();
    else if (obj == pauseButton)
        movieView->Pause();
    else if (obj == resumeButton)
        movieView->Resume();
    else if (obj == hideButton)
        movieView->SetVisibilityFlag(false);
    else if (obj == showButton)
        movieView->SetVisibilityFlag(true);
}

void UIMovieTest::ScaleButtonPressed(BaseObject* obj, void* data, void* callerData)
{
    eMovieScalingMode scaleMode = scalingModeNone;
    if (obj == buttonScale0)
        scaleMode = scalingModeNone;
    else if (obj == buttonScale1)
        scaleMode = scalingModeAspectFit;
    else if (obj == buttonScale2)
        scaleMode = scalingModeAspectFill;
    else if (obj == buttonScale3)
        scaleMode = scalingModeFill;

    movieView->OpenMovie("~res://TestData/MovieTest/bunny.m4v", OpenMovieParams(scaleMode));
    movieView->Play();
}
