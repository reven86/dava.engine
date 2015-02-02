/*
 * BacktraceCorkscrewImpl.h
 *
 *  Created on: Dec 20, 2014
 *      Author: l_sinyakov
 */

#ifndef BACKTRACECORKSCREWIMPL_H_
#define BACKTRACECORKSCREWIMPL_H_

#include "BacktraceInterface.h"
#include "libcorkscrew_stab.h"
namespace DAVA
{
class MemoryMapCorkscrewIterator:public MemoryMapIterator
{
public:
	MemoryMapCorkscrewIterator(map_info_t *map_info);
	virtual ~MemoryMapCorkscrewIterator(){}
	virtual bool Next() override;
	virtual void ToBegin() override;
	virtual const char * GetLib() const override;
	virtual pointer_size GetAddrStart() const override;
	virtual pointer_size GetAddrEnd() const override;
protected:
	map_info_t *map_info;
	map_info_t *now;
};

class MemoryMapCorkscrewInterface: public MemoryMapInterface
{
public:
	MemoryMapCorkscrewInterface();
	virtual ~MemoryMapCorkscrewInterface();
	virtual bool Resolve(pointer_size addr,const char **,pointer_size *) const override;
	virtual MemoryMapIterator & GetIterator() const;
	map_info_t* GetMapInfo(){return map_info;}
protected:

	map_info_t *map_info;
	mutable MemoryMapCorkscrewIterator iterator;
};

class BacktraceCorkscrewImpl: public DAVA::BacktraceInterface
{
public:

	virtual ~BacktraceCorkscrewImpl();
	static  BacktraceCorkscrewImpl* Load();
	virtual void BuildMemoryMap() override;
	virtual const MemoryMapInterface * GetMemoryMap() const override;
	//handler safe function
	virtual void Backtrace(Function<void(pointer_size)> onFrame,
			void * context = NULL, void * siginfo = NULL) override;
protected:
	BacktraceCorkscrewImpl();
	bool loaded;
	MemoryMapCorkscrewInterface * processMap;
};

} /* namespace DAVA */
#endif /* BACKTRACECORKSCREWIMPL_H_ */
