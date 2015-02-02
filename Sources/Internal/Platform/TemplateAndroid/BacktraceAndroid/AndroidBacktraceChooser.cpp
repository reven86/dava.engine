#include "AndroidBacktraceChooser.h"

namespace DAVA 
{
BacktraceInterface * AndroidBacktraceChooser::backtraceProvider = nullptr;
BacktraceInterface* AndroidBacktraceChooser::ChooseBacktraceAndroid()
{
	

	// try to create at least one bactrace provider
	if(backtraceProvider == nullptr)
	{
		backtraceProvider = BacktraceCorkscrewImpl::Load(); 
		#if defined(__arm__)
		if(backtraceProvider == nullptr)
		{
			backtraceProvider = BacktraceUnwindImpl::Load();
			if(backtraceProvider == nullptr)
			{
				return nullptr;
			}
		}
		#endif
		if(backtraceProvider == nullptr)
		{
			return nullptr;
		}
	
		// building memory memp of process at this point
		// all important libs are likely to be loaded at this point
		backtraceProvider->BuildMemoryMap();
	}
	else
	{
		return backtraceProvider;
	}


}
void AndroidBacktraceChooser::ReleaseBacktraceInterface()
{
	if(backtraceProvider != nullptr)
		delete backtraceProvider;
	backtraceProvider = nullptr;
}
}