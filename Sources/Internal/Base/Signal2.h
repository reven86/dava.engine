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


#ifndef __DAVA_SIGNAL_H__
#define __DAVA_SIGNAL_H__

#include "Function.h"
#include "FunctionTraits.h"
#include <vector> 

namespace DAVA
{
	template<typename F>
	struct SignalBase
	{
		typedef typename FuncTraits<F>::FunctionType FunctionType;
		typedef typename FunctionType::ParamType1 ParamType1;
		typedef typename FunctionType::ParamType2 ParamType2;
		typedef typename FunctionType::ParamType3 ParamType3;
		typedef typename FunctionType::ParamType4 ParamType4;
		typedef typename FunctionType::ParamType5 ParamType5;
		typedef typename FunctionType::ParamType6 ParamType6;
		typedef typename FunctionType::ParamType7 ParamType7;
		typedef typename FunctionType::ParamType8 ParamType8;

		void Connect(FunctionType &fn) 
		{
			slots.push_back(fn);
		}

		void Disconect(FunctionType &fn)
		{
			for (int i = 0; i < slots.size(); ++i)
			{
			}
		}

	protected:
		std::vector<FunctionType> slots;
	};

	template<typename T>
	struct Signal
	{
	};

	template<typename R>
	struct Signal<R()> : SignalBase< Function<R()> >
	{
		typedef SignalBase< Function<void()> > Base;

		void Emit() 
		{ 
			for (size_t i = 0; i < Base::slots.size(); ++i)
			{
				Base::slots[i]();
			}
		}
	};

	template<typename R, typename P1, typename P2, typename P3>
	struct Signal<R(P1, P2, P3)> : SignalBase< Function<R(P1, P2, P3)> >
	{
		typedef SignalBase< Function<R(P1, P2, P3)> > Base;

		void Emit(P1 p1, P2 p2, P3 p3)
		{
			for (size_t i = 0; i < Base::slots.size(); ++i)
			{
				Base::slots[i](p1, p2, p3);
			}
		}
	};

}

#endif // __DAVA_SIGNAL_H__
