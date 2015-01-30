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


#include "MovieViewControlAndroid.h"

namespace DAVA {

JniMovieViewControl::JniMovieViewControl(uint32 id)
    : jniMovieViewControl("com/dava/framework/JNIMovieViewControl")
{
	this->id = id;

    initialize = jniMovieViewControl.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("Initialize");
    uninitialize = jniMovieViewControl.GetStaticMethod<void, jint>("Uninitialize");
    setRect = jniMovieViewControl.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("SetRect");
    setVisible = jniMovieViewControl.GetStaticMethod<void, jint, jboolean>("SetVisible");
    openMovie = jniMovieViewControl.GetStaticMethod<void, jint, jstring, jint>("OpenMovie");
    play = jniMovieViewControl.GetStaticMethod<void, jint>("Play");
    stop = jniMovieViewControl.GetStaticMethod<void, jint>("Stop");
    pause = jniMovieViewControl.GetStaticMethod<void, jint>("Pause");
    resume = jniMovieViewControl.GetStaticMethod<void, jint>("Resume");
    isPlaying = jniMovieViewControl.GetStaticMethod<jboolean, jint>("IsPlaying");
}

void JniMovieViewControl::Initialize(const Rect& _rect)
{
    Rect rect = JNI::V2P(_rect);
    initialize(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniMovieViewControl::Uninitialize()
{
    uninitialize(id);
}

void JniMovieViewControl::SetRect(const Rect& _rect)
{
    Rect rect = JNI::V2P(_rect);
    setRect(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniMovieViewControl::SetVisible(bool isVisible)
{
	setVisible(id, isVisible);
}

void JniMovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    JNIEnv *env = JNI::GetEnv();
    jstring jMoviePath = env->NewStringUTF(moviePath.GetAbsolutePathname().c_str());

    openMovie(id, jMoviePath, params.scalingMode);

    env->DeleteLocalRef(jMoviePath);
}

void JniMovieViewControl::Play()
{
    play(id);
}

void JniMovieViewControl::Stop()
{
    stop(id);
}

void JniMovieViewControl::Pause()
{
    pause(id);
}

void JniMovieViewControl::Resume()
{
    resume(id);
}

bool JniMovieViewControl::IsPlaying()
{
    return isPlaying(id);
}

MovieViewControl::MovieViewControl() :
	jniMovieViewControl((uint32)this)
{

}

MovieViewControl::~MovieViewControl()
{
	jniMovieViewControl.Uninitialize();
}

void MovieViewControl::Initialize(const Rect& rect)
{
	jniMovieViewControl.Initialize(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
	jniMovieViewControl.OpenMovie(moviePath, params);
}

void MovieViewControl::SetRect(const Rect& rect)
{
	jniMovieViewControl.SetRect(rect);
}

void MovieViewControl::SetVisible(bool isVisible)
{
	jniMovieViewControl.SetVisible(isVisible);
}

void MovieViewControl::Play()
{
	jniMovieViewControl.Play();
}

void MovieViewControl::Stop()
{
	jniMovieViewControl.Stop();
}

void MovieViewControl::Pause()
{
	jniMovieViewControl.Pause();
}

void MovieViewControl::Resume()
{
	jniMovieViewControl.Resume();
}

bool MovieViewControl::IsPlaying()
{
	return jniMovieViewControl.IsPlaying();
}

}
