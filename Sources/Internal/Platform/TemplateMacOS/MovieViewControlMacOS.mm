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


#include "MovieViewControlMacOS.h"

#import <QTKit/QTKit.h>

namespace DAVA
{
MovieViewControl::MovieViewControl()
{
	movieView = [[QTMovieView alloc] init];
	NSView* openGLView = (NSView*)Core::Instance()->GetOpenGLView();
	[openGLView addSubview:(QTMovieView*)movieView];
}
	
MovieViewControl::~MovieViewControl()
{
	QTMovieView* player = (QTMovieView*)movieView;
	[player removeFromSuperview];
	[player release];
	
	movieView = nil;
}

void MovieViewControl::Initialize(const Rect& rect)
{
	SetRect(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
	NSURL* movieURL = [NSURL fileURLWithPath:[NSString stringWithCString:moviePath.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding]];
	QTMovie* movie = [QTMovie movieWithURL:movieURL error:nil];
	
	QTMovieView* player = (QTMovieView*)movieView;
	[player setPreservesAspectRatio: params.scalingMode == scalingModeAspectFit];
	[player setMovie:movie];
}
	
void MovieViewControl::SetRect(const Rect& rect)
{
	NSRect movieViewRect = [(QTMovieView*)movieView frame];
	
	movieViewRect.size.width = rect.dx * Core::GetVirtualToPhysicalFactor();
	movieViewRect.size.height = rect.dy * Core::GetVirtualToPhysicalFactor();
	
	movieViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
	movieViewRect.origin.y = (Core::Instance()->GetPhysicalScreenHeight() - rect.y - rect.dy) * DAVA::Core::GetVirtualToPhysicalFactor();
	
	movieViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
	movieViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;
	
	[(QTMovieView*)movieView setFrame: movieViewRect];
}

void MovieViewControl::SetVisible(bool isVisible)
{
	[(QTMovieView*)movieView setHidden:!isVisible];
}

void MovieViewControl::Play()
{
	[(QTMovieView*)movieView play:nil];
}

void MovieViewControl::Stop()
{
	Pause();
	[(QTMovieView*)movieView gotoBeginning:nil];
}
	
void MovieViewControl::Pause()
{
	[(QTMovieView*)movieView pause:nil];
}
	
void MovieViewControl::Resume()
{
	Play();
}
	
bool MovieViewControl::IsPlaying()
{
	QTMovie* curMovie = [(QTMovieView*)movieView movie];
	if (!curMovie)
	{
		return false;
	}
	
	return [curMovie rate] != 0;
}

}