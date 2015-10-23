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

#if !defined __STATSET_HPP__
#define __STATSET_HPP__
//==============================================================================
//
//  Global Set of (usually performance-related) per-frame stats
//
//==============================================================================
//
//  externals:

    #include "../rhi_Type.h"


//==============================================================================
//
//  publics:

class
StatSet
{
public:

    static unsigned     AddStat( const char* full_name, const char* short_name, unsigned parent_id=InvalidIndex );
    static unsigned     AddPermanentStat( const char* full_name, const char* short_name, unsigned parent_id=InvalidIndex );
    
    static void         ResetAll();

    static void         SetStat( unsigned id, unsigned value );
    static void         IncStat( unsigned id, unsigned delta );
    static void         DecStat( unsigned id, unsigned delta );

    static unsigned     StatValue( unsigned id );

    static unsigned     StatID( const char* name );
    static const char*  StatFullName( unsigned id );
    static void         DumpStats();
};




//==============================================================================
#endif // __STATSET_HPP__

