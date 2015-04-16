#if !defined __PREPROCESS_H__
#define __PREPROCESS_H__

#include <string>


void    PreProcessText( const char* text, std::string* result );
void    PreProcessText( const char* text, const char** arg, unsigned argCount, std::string* result );


#endif //__PREPROCESS_H__
