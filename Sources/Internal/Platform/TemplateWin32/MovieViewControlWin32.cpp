#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "MovieViewControlWin32.h"

#include "Platform/TemplateWin32/FfmpegPlayer.h"
#include "Render/PixelFormatDescriptor.h"
#include "UI/UIControl.h"

namespace DAVA
{
MovieViewControl::MovieViewControl()
    : ffmpegPlayer(new FfmpegPlayer())
    , videoBackground(new UIControlBackground())
{
    ffmpegPlayer->Initialize(Rect());
}

MovieViewControl::~MovieViewControl()
{
    SafeDelete(ffmpegPlayer);
    SafeRelease(videoTexture);
    SafeRelease(videoBackground);
    SafeDeleteArray(videoTextureBuffer);
}

void MovieViewControl::Initialize(const Rect& rect)
{
    ffmpegPlayer->Initialize(rect);
}

void MovieViewControl::SetRect(const Rect& rect)
{
    ffmpegPlayer->SetRect(rect);
}

void MovieViewControl::SetVisible(bool isVisible)
{
    ffmpegPlayer->SetVisible(isVisible);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    ffmpegPlayer->OpenMovie(moviePath, params);

    switch (params.scalingMode)
    {
    case DAVA::scalingModeNone:
        videoBackground->SetDrawType(UIControlBackground::eDrawType::DRAW_ALIGNED);
        break;
    case DAVA::scalingModeAspectFit:
        videoBackground->SetDrawType(UIControlBackground::eDrawType::DRAW_SCALE_PROPORTIONAL);
        break;
    case DAVA::scalingModeAspectFill:
        videoBackground->SetDrawType(UIControlBackground::eDrawType::DRAW_SCALE_PROPORTIONAL_ONE);
        break;
    case DAVA::scalingModeFill:
        videoBackground->SetDrawType(UIControlBackground::eDrawType::DRAW_SCALE_TO_RECT);
        break;
    default:
        break;
    }
}

void MovieViewControl::Play()
{
    ffmpegPlayer->Play();
    Vector2 res = ffmpegPlayer->GetResolution();
    textureWidth = NextPowerOf2(static_cast<uint32>(res.dx));
    textureHeight = NextPowerOf2(static_cast<uint32>(res.dy));
    uint32 size = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(ffmpegPlayer->GetPixelFormat());

    SafeDeleteArray(videoTextureBuffer);
    videoTextureBuffer = new uint8[size];

    Memset(videoTextureBuffer, 0, size);
}

void MovieViewControl::Stop()
{
    ffmpegPlayer->Stop();
    SafeDeleteArray(videoTextureBuffer);
    SafeRelease(videoTexture);
}

void MovieViewControl::Pause()
{
    ffmpegPlayer->Pause();
}

void MovieViewControl::Resume()
{
    ffmpegPlayer->Resume();
}

bool MovieViewControl::IsPlaying() const
{
    return ffmpegPlayer->IsPlaying();
}

void MovieViewControl::Update()
{
    if (nullptr == ffmpegPlayer)
        return;

    ffmpegPlayer->Update();

    if (FfmpegPlayer::STOPPED == ffmpegPlayer->GetState())
        SafeRelease(videoTexture);

    FfmpegPlayer::DrawVideoFrameData drawData = ffmpegPlayer->GetDrawData();

    if (nullptr == videoTextureBuffer || PixelFormatDescriptor::TEXTURE_FORMAT_INVALID == drawData.format || 0 == drawData.data.size())
        return;

    Memcpy(videoTextureBuffer, drawData.data.data(), drawData.data.size());
    if (nullptr == videoTexture)
    {
        videoTexture = Texture::CreateFromData(drawData.format, videoTextureBuffer, textureWidth, textureHeight, false);
        Sprite* videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, drawData.frameWidth, drawData.frameHeight, static_cast<float32>(drawData.frameWidth), static_cast<float32>(drawData.frameHeight));
        videoBackground->SetSprite(videoSprite);
        videoSprite->Release();
    }
    else
    {
        videoTexture->TexImage(0, textureWidth, textureHeight, videoTextureBuffer, static_cast<uint32>(drawData.data.size()), Texture::INVALID_CUBEMAP_FACE);
    }
}

void MovieViewControl::Draw(const UIGeometricData& parentGeometricData)
{
    if (videoTexture)
        videoBackground->Draw(parentGeometricData);
}
}

#endif