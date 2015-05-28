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


#include "yaml-cpp/node.h"
#include "yaml-cpp/exceptions.h"
#include "iterpriv.h"

namespace YAML
{
	Iterator::Iterator(): m_pData(new IterPriv)
	{
	}

	Iterator::Iterator(std::auto_ptr<IterPriv> pData): m_pData(pData)
	{
	}

	Iterator::Iterator(const Iterator& rhs): m_pData(new IterPriv(*rhs.m_pData))
	{
	}

	Iterator& Iterator::operator = (const Iterator& rhs)
	{
		if(this == &rhs)
			return *this;

		m_pData.reset(new IterPriv(*rhs.m_pData));
		return *this;
	}

	Iterator::~Iterator()
	{
	}

	Iterator& Iterator::operator ++ ()
	{
		if(m_pData->type == IterPriv::IT_SEQ)
			++m_pData->seqIter;
		else if(m_pData->type == IterPriv::IT_MAP)
			++m_pData->mapIter;

		return *this;
	}

	Iterator Iterator::operator ++ (int)
	{
		Iterator temp = *this;

		if(m_pData->type == IterPriv::IT_SEQ)
			++m_pData->seqIter;
		else if(m_pData->type == IterPriv::IT_MAP)
			++m_pData->mapIter;

		return temp;
	}

	const Node& Iterator::operator * () const
	{
		if(m_pData->type == IterPriv::IT_SEQ)
			return **m_pData->seqIter;

		throw BadDereference();
	}

	const Node *Iterator::operator -> () const
	{
		if(m_pData->type == IterPriv::IT_SEQ)
			return *m_pData->seqIter;

		throw BadDereference();
	}

	const Node& Iterator::first() const
	{
		if(m_pData->type == IterPriv::IT_MAP)
			return *m_pData->mapIter->first;

		throw BadDereference();
	}

	const Node& Iterator::second() const
	{
		if(m_pData->type == IterPriv::IT_MAP)
			return *m_pData->mapIter->second;

		throw BadDereference();
	}

	bool operator == (const Iterator& it, const Iterator& jt)
	{
		if(it.m_pData->type != jt.m_pData->type)
			return false;

		if(it.m_pData->type == IterPriv::IT_SEQ)
			return it.m_pData->seqIter == jt.m_pData->seqIter;
		else if(it.m_pData->type == IterPriv::IT_MAP)
			return it.m_pData->mapIter == jt.m_pData->mapIter;

		return true;
	}

	bool operator != (const Iterator& it, const Iterator& jt)
	{
		return !(it == jt);
	}
}
