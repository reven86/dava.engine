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


#include "TextureBrowser/TextureConvertorWork.h"

JobStack::JobStack()
	: head(NULL)
	, itemsCount(0)
{ }

JobStack::~JobStack()
{
	JobItemWrapper *item;

	while(NULL != head)
	{
		item = head;
		head = head->next;

		delete item;
	}
}

bool JobStack::push(const JobItem &item)
{
	bool ret = true;
	JobItemWrapper *i = head;

	// remember force value
    eTextureConvertMode convertMode = item.convertMode;

	// search for the same works in list and remove it
	while(NULL != i)
	{
		if(i->type == item.type && NULL != i->descriptor && i->descriptor == item.descriptor)
		{
			if(NULL != i->prev)
			{
				i->prev->next = i->next;
			}

			if(NULL != i->next)
			{
				i->next->prev = i->prev;
			}

			if(i == head)
			{
				head = i->next;
			}

            // if this job has more strict convert mode, we should move it to the new job
			if(i->convertMode < convertMode)
			{
                convertMode = i->convertMode;
			}

			delete i;
			itemsCount--;

			ret = false;
			break;
		}

		i = i->next;
	}

	// add new work
	i = new JobItemWrapper(item);

	// restore convert mode value
    i->convertMode = convertMode;

	if(NULL != head)
	{
		head->prev = i;
		i->next = head;
	}

	head = i;
	itemsCount++;

	return ret;
}

JobItem* JobStack::pop()
{
	JobItemWrapper *item = head;

	if(NULL != head)
	{
		head = head->next;
		if(NULL != head)
		{
			head->prev = NULL;
		}

		itemsCount--;
	}

	return item;
}

int JobStack::size()
{
	return itemsCount;
}

JobStack::JobItemWrapper::JobItemWrapper(const JobItem &item)
	: JobItem(item)
	, next(NULL)
	, prev(NULL)
{ }
