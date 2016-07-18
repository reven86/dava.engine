/*-------------------------------------------------------------------------------------------------
file:   rc.h
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once
#include "common.h"

class CRC_;

//#ifdef _DEBUG
typedef CRC_ CRC;
/*
#else
typedef ERR_ID CRC;
#endif
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
class SYSTEM_API CRC_
{
public:
    CRC_(void);
    CRC_(ERR_ID id_err);
    CRC_(CRC_& r);
    CRC_& operator=(CRC_& r);
    ~CRC_(void);

    bool operator==(CRC_& r)
    {
        return m_id_err == r.m_id_err;
    }
    bool operator==(ERR_ID id_err)
    {
        return m_id_err == id_err;
    }
    ERR_ID GetErrId(void)
    {
        return m_id_err;
    }
    operator bool()
    {
        return m_id_err == ERR_OK;
    }

    void Resolve(void);

private:
    bool m_resolved;
    ERR_ID m_id_err;
};
