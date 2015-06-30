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


#ifndef __KEYEDARCHIVE_COMMAND_H__
#define __KEYEDARCHIVE_COMMAND_H__

#include "Commands2/Command2.h"

class KeyedArchiveAddValueCommand : public Command2
{
public:
	KeyedArchiveAddValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String &_key, const DAVA::VariantType &_val);
	~KeyedArchiveAddValueCommand();

	virtual void Undo();
	virtual void Redo();

	virtual DAVA::Entity* GetEntity() const
	{
		return NULL;
	}

	DAVA::KeyedArchive* archive;
	DAVA::String key;
	DAVA::VariantType val;
};

class KeyeadArchiveRemValueCommand : public Command2
{
public:
	KeyeadArchiveRemValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String &_key);
	~KeyeadArchiveRemValueCommand();

	virtual void Undo();
	virtual void Redo();

	virtual DAVA::Entity* GetEntity() const
	{
		return NULL;
	}

	DAVA::KeyedArchive* archive;
	DAVA::String key;
	DAVA::VariantType val;
};

class KeyeadArchiveSetValueCommand : public Command2
{
public:
	KeyeadArchiveSetValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String &_key, const DAVA::VariantType &_val);
	~KeyeadArchiveSetValueCommand();

	virtual void Undo();
	virtual void Redo();

	virtual DAVA::Entity* GetEntity() const
	{
		return NULL;
	}

	DAVA::KeyedArchive* archive;
	DAVA::String key;
	DAVA::VariantType oldVal;
	DAVA::VariantType newVal;
};

#endif // __KEYEDARCHIVE_COMMAND_H__
