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


#include "Commands2/KeyedArchiveCommand.h"


KeyedArchiveAddValueCommand::KeyedArchiveAddValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String &_key, const DAVA::VariantType &_val)
	: Command2(CMDID_KEYEDARCHIVE_ADD_KEY, "Add key to archive")
	, archive(_archive)
	, key(_key)
	, val(_val)
{ }

KeyedArchiveAddValueCommand::~KeyedArchiveAddValueCommand()
{ }

void KeyedArchiveAddValueCommand::Undo()
{
	if(NULL != archive)
	{
		archive->DeleteKey(key);
	}
}

void KeyedArchiveAddValueCommand::Redo()
{
	if(NULL != archive)
	{
		archive->SetVariant(key, val);
	}
}

KeyeadArchiveRemValueCommand::KeyeadArchiveRemValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String &_key)
	: Command2(CMDID_KEYEDARCHIVE_REM_KEY, "Rem key from archive")
	, archive(_archive)
	, key(_key)
{
	if(NULL != archive)
	{
		DAVA::VariantType *vPtr = archive->GetVariant(key);

		if(NULL != vPtr)
		{
			val = *vPtr;
		}
	}
}

KeyeadArchiveRemValueCommand::~KeyeadArchiveRemValueCommand()
{ }

void KeyeadArchiveRemValueCommand::Undo()
{
	if(NULL != archive)
	{
		archive->SetVariant(key, val);
	}
}

void KeyeadArchiveRemValueCommand::Redo()
{
	if(NULL != archive)
	{
		archive->DeleteKey(key);
	}
}

KeyeadArchiveSetValueCommand::KeyeadArchiveSetValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String &_key, const DAVA::VariantType &_val)
	: Command2(CMDID_KEYEDARCHIVE_SET_KEY, "Set archive value")
	, archive(_archive)
	, key(_key)
	, newVal(_val)
{
	if(NULL != archive)
	{
		oldVal = *archive->GetVariant(key);
	}
}

KeyeadArchiveSetValueCommand::~KeyeadArchiveSetValueCommand()
{ }

void KeyeadArchiveSetValueCommand::Undo()
{
	if(NULL != archive && archive->IsKeyExists(key))
	{
		archive->SetVariant(key, oldVal);
	}
}

void KeyeadArchiveSetValueCommand::Redo()
{
	if(NULL != archive && archive->IsKeyExists(key))
	{
		archive->SetVariant(key, newVal);
	}
}
