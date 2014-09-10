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
This source file is part of GIMPACT Library.

For the latest info, see http://gimpact.sourceforge.net/

Copyright (c) 2007 Francisco Leon Najera. C.C. 80087371.
email: projectileman@yahoo.com


This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "btContactProcessing.h"

#define MAX_COINCIDENT 8

struct CONTACT_KEY_TOKEN
{
	unsigned int m_key;
	int m_value;
	CONTACT_KEY_TOKEN()
    {
    }

    CONTACT_KEY_TOKEN(unsigned int key,int token)
    {
    	m_key = key;
    	m_value =  token;
    }

    CONTACT_KEY_TOKEN(const CONTACT_KEY_TOKEN& rtoken)
    {
    	m_key = rtoken.m_key;
    	m_value = rtoken.m_value;
    }

    inline bool operator <(const CONTACT_KEY_TOKEN& other) const
	{
		return (m_key < other.m_key);
	}

	inline bool operator >(const CONTACT_KEY_TOKEN& other) const
	{
		return (m_key > other.m_key);
	}

};

class CONTACT_KEY_TOKEN_COMP
{
	public:

		bool operator() ( const CONTACT_KEY_TOKEN& a, const CONTACT_KEY_TOKEN& b ) const
		{
			return ( a < b );
		}
};


void btContactArray::merge_contacts(
	const btContactArray & contacts, bool normal_contact_average)
{
	clear();

	int i;
	if(contacts.size()==0) return;


	if(contacts.size()==1)
	{
		push_back(contacts[0]);
		return;
	}

	btAlignedObjectArray<CONTACT_KEY_TOKEN> keycontacts;

	keycontacts.reserve(contacts.size());

	//fill key contacts

	for ( i = 0;i<contacts.size() ;i++ )
	{
		keycontacts.push_back(CONTACT_KEY_TOKEN(contacts[i].calc_key_contact(),i));
	}

	//sort keys
	keycontacts.quickSort(CONTACT_KEY_TOKEN_COMP());

	// Merge contacts
	int coincident_count=0;
	btVector3 coincident_normals[MAX_COINCIDENT];

	unsigned int last_key = keycontacts[0].m_key;
	unsigned int key = 0;

	push_back(contacts[keycontacts[0].m_value]);

	GIM_CONTACT * pcontact = &(*this)[0];

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
		    pcontact = &(*this)[this->size()-1];
        }
		last_key = key;
	}
}

void btContactArray::merge_contacts_unique(const btContactArray & contacts)
{
	clear();

	if(contacts.size()==0) return;

	if(contacts.size()==1)
	{
		push_back(contacts[0]);
		return;
	}

	GIM_CONTACT average_contact = contacts[0];

	for (int i=1;i<contacts.size() ;i++ )
	{
		average_contact.m_point += contacts[i].m_point;
		average_contact.m_normal += contacts[i].m_normal * contacts[i].m_depth;
	}

	//divide
	btScalar divide_average = 1.0f/((btScalar)contacts.size());

	average_contact.m_point *= divide_average;

	average_contact.m_normal *= divide_average;

	average_contact.m_depth = average_contact.m_normal.length();

	average_contact.m_normal /= average_contact.m_depth;

}

