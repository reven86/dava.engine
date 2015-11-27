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



/*
-----------------------------------------------------------------------------
This source file is part of GIMPACT Library.

For the latest info, see http://gimpact.sourceforge.net/

Copyright (c) 2006 Francisco Leon Najera. C.C. 80087371.
email: projectileman@yahoo.com

 This library is free software; you can redistribute it and/or
 modify it under the terms of EITHER:
   (1) The GNU Lesser General Public License as published by the Free
       Software Foundation; either version 2.1 of the License, or (at
       your option) any later version. The text of the GNU Lesser
       General Public License is included with this library in the
       file GIMPACT-LICENSE-LGPL.TXT.
   (2) The BSD-style license that is included with this library in
       the file GIMPACT-LICENSE-BSD.TXT.
   (3) The zlib/libpng license that is included with this library in
       the file GIMPACT-LICENSE-ZLIB.TXT.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 GIMPACT-LICENSE-LGPL.TXT, GIMPACT-LICENSE-ZLIB.TXT and GIMPACT-LICENSE-BSD.TXT for more details.

-----------------------------------------------------------------------------
*/

#include "gim_contact.h"

#define MAX_COINCIDENT 8

void gim_contact_array::merge_contacts(
	const gim_contact_array & contacts, bool normal_contact_average)
{
	clear();

	if(contacts.size()==1)
	{
		push_back(contacts.back());
		return;
	}

	gim_array<GIM_RSORT_TOKEN> keycontacts(contacts.size());
	keycontacts.resize(contacts.size(),false);

	//fill key contacts

	GUINT i;

	for (i = 0;i<contacts.size() ;i++ )
	{
		keycontacts[i].m_key = contacts[i].calc_key_contact();
		keycontacts[i].m_value = i;
	}

	//sort keys
	gim_heap_sort(keycontacts.pointer(),keycontacts.size(),GIM_RSORT_TOKEN_COMPARATOR());

	// Merge contacts

	GUINT coincident_count=0;
	btVector3 coincident_normals[MAX_COINCIDENT];

	GUINT last_key = keycontacts[0].m_key;
	GUINT key = 0;

	push_back(contacts[keycontacts[0].m_value]);
	GIM_CONTACT * pcontact = &back();



	for( i=1;i<keycontacts.size();i++)
	{
	    key = keycontacts[i].m_key;
		const GIM_CONTACT * scontact = &contacts[keycontacts[i].m_value];

		if(last_key ==  key)//same points
		{
			//merge contact
			if(pcontact->m_depth - CONTACT_DIFF_EPSILON > scontact->m_depth)//)
			{
				*pcontact = *scontact;
                coincident_count = 0;
			}
			else if(normal_contact_average)
			{
				if(btFabs(pcontact->m_depth - scontact->m_depth)<CONTACT_DIFF_EPSILON)
                {
                    if(coincident_count<MAX_COINCIDENT)
                    {
                    	coincident_normals[coincident_count] = scontact->m_normal;
                        coincident_count++;
                    }
                }
			}
		}
		else
		{//add new contact

		    if(normal_contact_average && coincident_count>0)
		    {
		    	pcontact->interpolate_normals(coincident_normals,coincident_count);
		        coincident_count = 0;
		    }

		    push_back(*scontact);
		    pcontact = &back();
        }
		last_key = key;
	}
}

void gim_contact_array::merge_contacts_unique(const gim_contact_array & contacts)
{
	clear();

	if(contacts.size()==1)
	{
		push_back(contacts.back());
		return;
	}

	GIM_CONTACT average_contact = contacts.back();

	for (GUINT i=1;i<contacts.size() ;i++ )
	{
		average_contact.m_point += contacts[i].m_point;
		average_contact.m_normal += contacts[i].m_normal * contacts[i].m_depth;
	}

	//divide
	GREAL divide_average = 1.0f/((GREAL)contacts.size());

	average_contact.m_point *= divide_average;

	average_contact.m_normal *= divide_average;

	average_contact.m_depth = average_contact.m_normal.length();

	average_contact.m_normal /= average_contact.m_depth;

}

