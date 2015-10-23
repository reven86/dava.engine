/* mcpp_out.h: declarations of OUTDEST data types for MCPP  */
#ifndef _MCPP_OUT_H
#define _MCPP_OUT_H

/* Choices for output destination */
typedef enum {
    MCPP_OUT,                        /* ~= fp_out    */
    MCPP_ERR,                        /* ~= fp_err    */
    MCPP_DBG,                        /* ~= fp_debug  */
    MCPP_NUM_OUTDEST
} OUTDEST;

#endif  /* _MCPP_OUT_H  */
