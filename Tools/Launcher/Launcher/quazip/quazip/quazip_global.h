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


/**
Copyright (C) 2005-2011 Sergey A. Tachenov

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant, see
quazip/(un)zip.h files for details, basically it's zlib license.
 */

#ifndef QUAZIP_GLOBAL_H
#define QUAZIP_GLOBAL_H

#include <QtCore/qglobal.h>

/**
  This is automatically defined when building a static library, but when
  including QuaZip sources directly into a project, QUAZIP_STATIC should
  be defined explicitly to avoid possible troubles with unnecessary
  importing/exporting.
  */
#ifdef QUAZIP_STATIC
#define QUAZIP_EXPORT
#else
/**
 * When building a DLL with MSVC, QUAZIP_BUILD must be defined.
 * qglobal.h takes care of defining Q_DECL_* correctly for msvc/gcc.
 */
#if defined(QUAZIP_BUILD)
	#define QUAZIP_EXPORT Q_DECL_EXPORT
#else
	#define QUAZIP_EXPORT Q_DECL_IMPORT
#endif
#endif // QUAZIP_STATIC

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

#endif // QUAZIP_GLOBAL_H
