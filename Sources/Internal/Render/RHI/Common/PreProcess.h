#pragma once

#include <string>

void PreProcessText(const char* text, const char** arg, unsigned argCount, std::string* result);
void SetPreprocessCurFile(const char* filename);
