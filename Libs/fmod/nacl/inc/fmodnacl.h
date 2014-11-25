/* ============================================================================================ */
/* FMOD NaCL Specific header file. Copyright (c), Firelight Technologies Pty, Ltd. 2005-2014. */
/* ============================================================================================ */

#ifndef _FMODNACL_H
#define _FMODNACL_H

#include "ppapi/c/pp_instance.h"

/*
[STRUCTURE] 
[
    [DESCRIPTION]
    Use this structure to set the information required for Native-Client initialization.

    Pass this structure in as the "extradriverdata" parameter in System::init.

    [REMARKS]

    [PLATFORMS]
    Native Client

    [SEE_ALSO]
    System::init
]
*/
typedef struct FMOD_NACL_EXTRADRIVERDATA
{
	PP_Instance instance;        /* (IN)  Handle to the native client module instance */
} FMOD_NACL_EXTRADRIVERDATA;

#endif // #ifndef _FMODNACL_H