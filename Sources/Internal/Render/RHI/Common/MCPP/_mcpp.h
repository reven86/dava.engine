#pragma once

#define PREPROCESSED 0
#define HAVE_CONFIG_H 0
#define MCPP_LIB 1
#define DLL_EXPORT 0

#if defined(__DAVAENGINE_WIN32__)
#pragma warning(disable : 4018 4068 4101 4102 4146)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "Base/Platform.h"
#include "Base/BaseTypes.h"
#include "mcpp_out.h"

#if defined(__DAVAENGINE_WIN32__)
typedef struct _stat stat_t;
#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif
#else
typedef struct stat stat_t;
#endif

extern const char* MCPP_Text;

void mcpp__set_cur_file(const char* filename);

FILE* mcpp__fopen(const char* filename, const char* mode);
int mcpp__fclose(FILE* file);
int mcpp__ferror(FILE* file);
char* mcpp__fgets(char* buf, int max_size, FILE* file);
int mcpp__stat(const char* path, stat_t* buffer);

void mcpp__set_input(const void* data, unsigned data_sz);
void mcpp__startup();
void mcpp__cleanup();
void mcpp__shutdown();

struct mcpp_preprocessed_text
{
    char* buffer = nullptr;
    DAVA::uint32 pos = 0;
};

extern mcpp_preprocessed_text _mcpp_preprocessed_text;
int mcpp_fputc_impl(int ch, OUTDEST dst);
int mcpp_fputs_impl(const char* str, OUTDEST dst);
int mcpp_fprintf_impl(OUTDEST dst, const char* format, ...);
