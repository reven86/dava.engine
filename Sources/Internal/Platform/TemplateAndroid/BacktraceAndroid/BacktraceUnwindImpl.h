/*
 * BacktraceUnwindImpl.h
 *
 *  Created on: Dec 19, 2014
 *      Author: l_sinyakov
 */

#ifndef BACKTRACEUNWINDIMPL_H_
#define BACKTRACEUNWINDIMPL_H_

#include "BacktraceInterface.h"
#include "Base/BaseTypes.h"
#include "libunwind_stab.h"
#if defined(__arm__)
namespace DAVA
{
class MemoryMapUnwindIterator:public MemoryMapIterator
{
public:
	MemoryMapUnwindIterator(Vector<unw_map_t>::const_iterator begin,Vector<unw_map_t>::const_iterator end);
	virtual ~MemoryMapUnwindIterator(){}
	virtual bool Next() override;
	virtual void ToBegin() override;
	virtual const char * GetLib() const override;
	virtual pointer_size GetAddrStart() const override;
	virtual pointer_size GetAddrEnd() const override;
protected:
	Vector<unw_map_t>::const_iterator begin;
	Vector<unw_map_t>::const_iterator end;
	Vector<unw_map_t>::const_iterator now;
};

class MemoryMapUnwind: public MemoryMapInterface
{
public:
	MemoryMapUnwind();
	virtual ~MemoryMapUnwind();
	virtual bool Resolve(pointer_size addr,const char **,pointer_size *) const override;
	virtual MemoryMapIterator & GetIterator() const override;

private:

	//vector is ok should be loaded long before crash
	Vector<unw_map_t> memoryMap;
	unw_map_cursor_t mapCursor;
	mutable MemoryMapUnwindIterator iterator;
};
class BacktraceUnwindImpl:public BacktraceInterface
{
public:
	virtual ~BacktraceUnwindImpl();
	static BacktraceUnwindImpl * Load();
	virtual void BuildMemoryMap() override;
	virtual const MemoryMapInterface * GetMemoryMap() const override;
	//handler safe function
	virtual void Backtrace(Function<void(pointer_size)> onFrame, void * context = NULL , void * siginfo = NULL) override;
protected:
	BacktraceUnwindImpl();
	bool loaded;
	MemoryMapUnwind * processMap;

};

} /* namespace DAVA */
#endif /* __arm__*/
#endif /* BACKTRACEUNWINDIMPL_H_ */
