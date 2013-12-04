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

jclass JniMovieViewControl::gJavaClass = NULL;
const char* JniMovieViewControl::gJavaClassName = NULL;

jclass JniMovieViewControl::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniMovieViewControl::GetJavaClassName() const
{
	return gJavaClassName;
}

JniMovieViewControl::JniMovieViewControl(uint32 id)
{
	this->id = id;
}

void JniMovieViewControl::Initialize(const Rect& _rect)
{
	jmethodID mid = GetMethodID("Initialize", "(IFFFF)V");
	if (mid)
	{
		Rect rect = V2P(_rect);
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, rect.x, rect.y, rect.dx, rect.dy);
	}
}

void JniMovieViewControl::Uninitialize()
{
	jmethodID mid = GetMethodID("Uninitialize", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id);
}

void JniMovieViewControl::SetRect(const Rect& _rect)
{
	jmethodID mid = GetMethodID("SetRect", "(IFFFF)V");
	if (mid)
	{
		Rect rect = V2P(_rect);
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, rect.x, rect.y, rect.dx, rect.dy);
	}
}

void JniMovieViewControl::SetVisible(bool isVisible)
{
	jmethodID mid = GetMethodID("SetVisible", "(IZ)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, isVisible);
}

void JniMovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
	jmethodID mid = GetMethodID("OpenMovie", "(ILjava/lang/String;I)V");
	if (mid)
	{
		jstring jMoviePath = GetEnvironment()->NewStringUTF(moviePath.GetAbsolutePathname().c_str());
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, jMoviePath, params.scalingMode);
		GetEnvironment()->DeleteLocalRef(jMoviePath);
	}
}

void JniMovieViewControl::Play()
{
	jmethodID mid = GetMethodID("Play", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id);
}

void JniMovieViewControl::Stop()
{
	jmethodID mid = GetMethodID("Stop", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id);
}

void JniMovieViewControl::Pause()
{
	jmethodID mid = GetMethodID("Pause", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id);
}

void JniMovieViewControl::Resume()
{
	jmethodID mid = GetMethodID("Resume", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id);
}

bool JniMovieViewControl::IsPlaying()
{
	jmethodID mid = GetMethodID("IsPlaying", "(I)Z");
	if (mid)
		return GetEnvironment()->CallStaticBooleanMethod(GetJavaClass(), mid, id);
	return false;
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
