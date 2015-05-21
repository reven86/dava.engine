/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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

#ifndef __DAVAENGINE_LIBUNWIND_STAB_H__
#define __DAVAENGINE_LIBUNWIND_STAB_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

//__arm__ should only be defined when compiling on arm32
#if defined(__arm__)
#include <link.h>
#include <signal.h>

#if __clang_minor__ <= 5
///------------------------------------------------------------------------
///--- ucontext is not proided in NDK-10
///--- begin stab from <sys/ucontext.h>
///------------------------------------------------------------------------
enum {
    REG_R0 = 0,
    REG_R1,
    REG_R2,
    REG_R3,
    REG_R4,
    REG_R5,
    REG_R6,
    REG_R7,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
};

#define NGREG 18 /* Like glibc. */

typedef int greg_t;
typedef greg_t gregset_t[NGREG];

#include <asm/sigcontext.h>
typedef struct sigcontext mcontext_t;
///! Importaint: This structure might have diffrent structure past  sigset_t uc_sigmask;
/// depending on linux kernel version and platform(vary from device to device)
/// so avoid any operations with it and it's members
typedef struct ucontext {
    unsigned long uc_flags;
    struct ucontext* uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    sigset_t uc_sigmask;
    // Android has a wrong (smaller) sigset_t on ARM.
    uint32_t __padding_rt_sigset;
    // The kernel adds extra padding after uc_sigmask to match glibc sigset_t on ARM.
    char __padding[120];
    unsigned long uc_regspace[128] __attribute__((__aligned__(8)));
} ucontext_t;
///------------------------------------------------------------------------
///--- end stab from <sys/ucontext.h>
///------------------------------------------------------------------------
#endif // if __clang_minor <= 5
///------------------------------------------------------------------------
///--- libunwind.h is not provided in NDK-10 as of yet, so we have to stab
///--- begin stab from libunwind-arm.h
///------------------------------------------------------------------------
#define UNW_LOCAL_ONLY
typedef enum
{
    UNW_ARM_R0,
    UNW_ARM_R1,
    UNW_ARM_R2,
    UNW_ARM_R3,
    UNW_ARM_R4,
    UNW_ARM_R5,
    UNW_ARM_R6,
    UNW_ARM_R7,
    UNW_ARM_R8,
    UNW_ARM_R9,
    UNW_ARM_R10,
    UNW_ARM_R11,
    UNW_ARM_R12,
    UNW_ARM_R13,
    UNW_ARM_R14,
    UNW_ARM_R15,
    
    /* VFPv2 s0-s31 (obsolescent numberings).  */
    UNW_ARM_S0 = 64,
    UNW_ARM_S1,
    UNW_ARM_S2,
    UNW_ARM_S3,
    UNW_ARM_S4,
    UNW_ARM_S5,
    UNW_ARM_S6,
    UNW_ARM_S7,
    UNW_ARM_S8,
    UNW_ARM_S9,
    UNW_ARM_S10,
    UNW_ARM_S11,
    UNW_ARM_S12,
    UNW_ARM_S13,
    UNW_ARM_S14,
    UNW_ARM_S15,
    UNW_ARM_S16,
    UNW_ARM_S17,
    UNW_ARM_S18,
    UNW_ARM_S19,
    UNW_ARM_S20,
    UNW_ARM_S21,
    UNW_ARM_S22,
    UNW_ARM_S23,
    UNW_ARM_S24,
    UNW_ARM_S25,
    UNW_ARM_S26,
    UNW_ARM_S27,
    UNW_ARM_S28,
    UNW_ARM_S29,
    UNW_ARM_S30,
    UNW_ARM_S31,
    
    /* FPA register numberings.  */
    UNW_ARM_F0 = 96,
    UNW_ARM_F1,
    UNW_ARM_F2,
    UNW_ARM_F3,
    UNW_ARM_F4,
    UNW_ARM_F5,
    UNW_ARM_F6,
    UNW_ARM_F7,
    
    /* iWMMXt GR register numberings.  */
    UNW_ARM_wCGR0 = 104,
    UNW_ARM_wCGR1,
    UNW_ARM_wCGR2,
    UNW_ARM_wCGR3,
    UNW_ARM_wCGR4,
    UNW_ARM_wCGR5,
    UNW_ARM_wCGR6,
    UNW_ARM_wCGR7,
    
    /* iWMMXt register numberings.  */
    UNW_ARM_wR0 = 112,
    UNW_ARM_wR1,
    UNW_ARM_wR2,
    UNW_ARM_wR3,
    UNW_ARM_wR4,
    UNW_ARM_wR5,
    UNW_ARM_wR6,
    UNW_ARM_wR7,
    UNW_ARM_wR8,
    UNW_ARM_wR9,
    UNW_ARM_wR10,
    UNW_ARM_wR11,
    UNW_ARM_wR12,
    UNW_ARM_wR13,
    UNW_ARM_wR14,
    UNW_ARM_wR15,
    
    /* Two-byte encodings from here on.  */
    
    /* SPSR.  */
    UNW_ARM_SPSR = 128,
    UNW_ARM_SPSR_FIQ,
    UNW_ARM_SPSR_IRQ,
    UNW_ARM_SPSR_ABT,
    UNW_ARM_SPSR_UND,
    UNW_ARM_SPSR_SVC,
    
    /* User mode registers.  */
    UNW_ARM_R8_USR = 144,
    UNW_ARM_R9_USR,
    UNW_ARM_R10_USR,
    UNW_ARM_R11_USR,
    UNW_ARM_R12_USR,
    UNW_ARM_R13_USR,
    UNW_ARM_R14_USR,
    
    /* FIQ registers.  */
    UNW_ARM_R8_FIQ = 151,
    UNW_ARM_R9_FIQ,
    UNW_ARM_R10_FIQ,
    UNW_ARM_R11_FIQ,
    UNW_ARM_R12_FIQ,
    UNW_ARM_R13_FIQ,
    UNW_ARM_R14_FIQ,
    
    /* IRQ registers.  */
    UNW_ARM_R13_IRQ = 158,
    UNW_ARM_R14_IRQ,
    
    /* ABT registers.  */
    UNW_ARM_R13_ABT = 160,
    UNW_ARM_R14_ABT,
    
    /* UND registers.  */
    UNW_ARM_R13_UND = 162,
    UNW_ARM_R14_UND,
    
    /* SVC registers.  */
    UNW_ARM_R13_SVC = 164,
    UNW_ARM_R14_SVC,
    
    /* iWMMXt control registers.  */
    UNW_ARM_wC0 = 192,
    UNW_ARM_wC1,
    UNW_ARM_wC2,
    UNW_ARM_wC3,
    UNW_ARM_wC4,
    UNW_ARM_wC5,
    UNW_ARM_wC6,
    UNW_ARM_wC7,
    
    /* VFPv3/Neon 64-bit registers.  */
    UNW_ARM_D0 = 256,
    UNW_ARM_D1,
    UNW_ARM_D2,
    UNW_ARM_D3,
    UNW_ARM_D4,
    UNW_ARM_D5,
    UNW_ARM_D6,
    UNW_ARM_D7,
    UNW_ARM_D8,
    UNW_ARM_D9,
    UNW_ARM_D10,
    UNW_ARM_D11,
    UNW_ARM_D12,
    UNW_ARM_D13,
    UNW_ARM_D14,
    UNW_ARM_D15,
    UNW_ARM_D16,
    UNW_ARM_D17,
    UNW_ARM_D18,
    UNW_ARM_D19,
    UNW_ARM_D20,
    UNW_ARM_D21,
    UNW_ARM_D22,
    UNW_ARM_D23,
    UNW_ARM_D24,
    UNW_ARM_D25,
    UNW_ARM_D26,
    UNW_ARM_D27,
    UNW_ARM_D28,
    UNW_ARM_D29,
    UNW_ARM_D30,
    UNW_ARM_D31,
    
    /* For ARM, the CFA is the value of SP (r13) at the call site in the
     previous frame.  */
    UNW_ARM_CFA,
    
    UNW_TDEP_LAST_REG = UNW_ARM_D31,
    
    UNW_TDEP_IP = UNW_ARM_R14,  /* A little white lie.  */
    UNW_TDEP_SP = UNW_ARM_R13,
    UNW_TDEP_EH = UNW_ARM_R0   /* FIXME.  */
}
arm_regnum_t;
typedef enum
{
    UNW_REG_IP = UNW_TDEP_IP,        /* (rw) instruction pointer (pc) */
    UNW_REG_SP = UNW_TDEP_SP,        /* (ro) stack pointer */
    UNW_REG_EH = UNW_TDEP_EH,        /* (rw) exception-handling reg base */
    UNW_REG_LAST = UNW_TDEP_LAST_REG
}
unw_frame_regnum_t;

typedef uint32_t unw_word_t;
/* FIXME for ARM. Too big?  What do other things use for similar tasks?  */
#define UNW_TDEP_CURSOR_LEN    4096
typedef struct unw_cursor
{
    unw_word_t opaque[UNW_TDEP_CURSOR_LEN];
}
unw_cursor_t;
/* On ARM, we define our own unw_tdep_context instead of using ucontext_t.
 This allows us to support systems that don't support getcontext and
 therefore do not define ucontext_t.  */
typedef struct unw_tdep_context
{
    unsigned long regs[16];
}
unw_tdep_context_t;
/* This type encapsulates the entire (preserved) machine-state.  */
typedef unw_tdep_context_t unw_context_t;
/* There is no getcontext() on ARM.  Use a stub version which only saves GP
 registers.  FIXME: Not ideal, may not be sufficient for all libunwind
 use cases.  Stores pc+8, which is only approximately correct, really.  */
#ifndef __thumb__
#define unw_tdep_getcontext(uc) (({                    \
unw_tdep_context_t *unw_ctx = (uc);                    \
register unsigned long *unw_base asm ("r0") = unw_ctx->regs;        \
__asm__ __volatile__ (                        \
"stmia %[base], {r0-r15}"                        \
: : [base] "r" (unw_base) : "memory");                \
}), 0)
#else /* __thumb__ */
#define unw_tdep_getcontext(uc) (({                    \
unw_tdep_context_t *unw_ctx = (uc);                    \
register unsigned long *unw_base asm ("r0") = unw_ctx->regs;        \
__asm__ __volatile__ (                        \
".align 2\nbx pc\nnop\n.code 32\n"                    \
"stmia %[base], {r0-r15}\n"                        \
"orr %[base], pc, #1\nbx %[base]"                    \
: [base] "+r" (unw_base) : : "memory", "cc");            \
}), 0)
#endif
typedef struct
{
    /* no arm-specific auxiliary proc-info */
    /* ANDROID support update. */
    char __reserved;
    /* End of ANDROID update. */
}
unw_tdep_proc_info_t;
typedef struct unw_proc_info
{
    unw_word_t start_ip;    /* first IP covered by this procedure */
    unw_word_t end_ip;        /* first IP NOT covered by this procedure */
    unw_word_t lsda;        /* address of lang.-spec. data area (if any) */
    unw_word_t handler;        /* optional personality routine */
    unw_word_t gp;        /* global-pointer value for this procedure */
    unw_word_t flags;        /* misc. flags */
    
    int format;            /* unwind-info format (arch-specific) */
    int unwind_info_size;    /* size of the information (if applicable) */
    void *unwind_info;        /* unwind-info (arch-specific) */
    unw_tdep_proc_info_t extra;    /* target-dependent auxiliary proc-info */
}
unw_proc_info_t;
typedef struct unw_map_cursor
{
    void *map_list;
    void *cur_map;
}
unw_map_cursor_t;

typedef struct unw_map
{
    unw_word_t start;
    unw_word_t end;
    char *path;
    int flags;
}
unw_map_t;
///------------------------------------------------------------------------
///--- end stab libunwind-arm.h
///------------------------------------------------------------------------


///------------------------------------------------------------------------
///--- here we provide function defenitions from libunwind
///------------------------------------------------------------------------

typedef int (*t_unw_init_local) (unw_cursor_t *, unw_context_t *);
extern t_unw_init_local unw_init_local;

typedef int (*t_unw_step) (unw_cursor_t *);
extern t_unw_step unw_step ;

typedef int (*t_unw_get_reg) (unw_cursor_t *, int, unw_word_t *);
extern t_unw_get_reg unw_get_reg;

typedef int (*t_unw_backtrace)(void **buffer, int size);
extern t_unw_backtrace unw_backtrace ;

typedef int (*t_unw_get_proc_info) (unw_cursor_t *, unw_proc_info_t *);
extern t_unw_get_proc_info unw_get_proc_info ;

typedef int (*t_unw_map_local_create) (void);
extern t_unw_map_local_create unw_map_local_create ;

typedef void (*t_unw_map_local_destroy)(void);
extern t_unw_map_local_destroy unw_map_local_destroy ;

typedef int (*t_unw_is_signal_frame)(unw_cursor_t *);
extern t_unw_is_signal_frame unw_is_signal_frame;

typedef int (*t_unw_handle_signal_frame)(unw_cursor_t *);
extern t_unw_handle_signal_frame unw_handle_signal_frame ;

typedef int (*t_unw_getcontext)(unw_context_t*);
extern t_unw_getcontext  unw_getcontext ;

typedef int (*t_unw_map_local_cursor_valid)(unw_map_cursor_t*);
extern t_unw_map_local_cursor_valid unw_map_local_cursor_valid;

typedef void (*t_unw_map_local_cursor_get) (unw_map_cursor_t *);
extern t_unw_map_local_cursor_get unw_map_local_cursor_get;

typedef int (*t_unw_map_local_cursor_get_next) (unw_map_cursor_t *, unw_map_t *);
extern t_unw_map_local_cursor_get_next unw_map_local_cursor_get_next;

typedef void (*t_unw_map_cursor_reset)(unw_map_cursor_t *map_cursor);
extern t_unw_map_cursor_reset unw_map_cursor_reset ;

typedef void (*t_unw_map_cursor_create) (unw_map_cursor_t *, pid_t);
extern t_unw_map_cursor_create unw_map_cursor_create;

typedef void (*t_unw_map_cursor_destroy) (unw_map_cursor_t *);
extern t_unw_map_cursor_destroy unw_map_cursor_destroy ;

typedef int (*t_unw_map_cursor_get_next) (unw_map_cursor_t *, unw_map_t *);
extern t_unw_map_cursor_get_next unw_map_cursor_get_next ;

typedef int (*t_unw_get_proc_name) (unw_cursor_t *cp, char *bufp, size_t len, unw_word_t *offp);
extern t_unw_get_proc_name unw_get_proc_name ;

///------------------------------------------------------------------------
///--- function to load libunwind dynamicly
///------------------------------------------------------------------------
bool DynLoadLibunwind();
#else
typedef uint32_t  kernel_sigmask_t[2];
typedef struct ucontext {
    uint32_t uc_flags;
    struct ucontext* uc_link;
    stack_t uc_stack;
    sigcontext uc_mcontext;
    kernel_sigmask_t uc_sigmask;
} ucontext_t;
#endif //#if defined(__arm__)
#endif //#if defined(__DAVAENGINE_ANDROID__)
#endif /* #ifndef __DAVAENGINE_LIBUNWIND_STAB_H__ */
