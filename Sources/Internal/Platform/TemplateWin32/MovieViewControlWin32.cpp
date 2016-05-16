#include "MovieViewControlWin32.h"

#include "Platform/TemplateWin32/FfmpegPlayer.h"
#include "Render/PixelFormatDescriptor.h"
#include "UI/UIControl.h"

namespace DAVA
{
MovieViewControl::MovieViewControl()
    : ffmpegDecoder(new FfmpegPlayer())
    , videoBackground(new UIControlBackground())
{
    ffmpegDecoder->Initialize(Rect());
    videoBackground->SetDrawType(UIControlBackground::eDrawType::DRAW_SCALE_PROPORTIONAL);
}

MovieViewControl::~MovieViewControl()
{
    SafeDelete(ffmpegDecoder);
    SafeRelease(videoTexture);
    SafeRelease(videoBackground);
    SafeDeleteArray(videoTextureBuffer);
}

void MovieViewControl::Initialize(const Rect& rect)
{
    ffmpegDecoder->Initialize(rect);
}

void MovieViewControl::SetRect(const Rect& rect)
{
    ffmpegDecoder->SetRect(rect);
}

void MovieViewControl::SetVisible(bool isVisible)
{
    ffmpegDecoder->SetVisible(isVisible);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    ffmpegDecoder->OpenMovie(moviePath, params);
}

void MovieViewControl::Play()
{
    ffmpegDecoder->Play();
    Vector2 res = ffmpegDecoder->GetResolution();
    textureWidth = NextPowerOf2(static_cast<uint32>(res.dx));
    textureHeight = NextPowerOf2(static_cast<uint32>(res.dy));
    uint32 size = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(ffmpegDecoder->GetPixelFormat());

    SafeDeleteArray(videoTextureBuffer);
    videoTextureBuffer = new uint8[size];

    Memset(videoTextureBuffer, 0, size);
}

void MovieViewControl::Stop()
{
    ffmpegDecoder->Stop();
    SafeDeleteArray(videoTextureBuffer);
    SafeRelease(videoTexture);
}

void MovieViewControl::Pause()
{
    ffmpegDecoder->Pause();
}

void MovieViewControl::Resume()
{
    ffmpegDecoder->Resume();
}

bool MovieViewControl::IsPlaying() const
{
    return ffmpegDecoder->IsPlaying();
}

void MovieViewControl::Update()
{
    if (nullptr == ffmpegDecoder)
        return;

    ffmpegDecoder->Update();

    if (FfmpegPlayer::STOPPED == ffmpegDecoder->GetState())
        SafeRelease(videoTexture);

    FfmpegPlayer::DrawVideoFrameData* drawData = ffmpegDecoder->GetDrawData();

    if (nullptr == drawData || nullptr == videoTextureBuffer)
        return;

    Memcpy(videoTextureBuffer, drawData->data, drawData->dataSize);
    if (nullptr == videoTexture)
    {
        videoTexture = Texture::CreateFromData(drawData->format, videoTextureBuffer, textureWidth, textureHeight, false);
        Sprite* videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, drawData->frameWidth, drawData->frameHeight, static_cast<float32>(drawData->frameWidth), static_cast<float32>(drawData->frameHeight));
        videoBackground->SetSprite(videoSprite);
        videoSprite->Release();
    }
    else
    {
        videoTexture->TexImage(0, textureWidth, textureHeight, videoTextureBuffer, drawData->dataSize, Texture::INVALID_CUBEMAP_FACE);
    }
    SafeDelete(drawData);
}

void MovieViewControl::Draw(const UIGeometricData& parentGeometricData)
{
    if (videoTexture)
        videoBackground->Draw(parentGeometricData);
}
}