#pragma once

#include <string>

void PreProcessText(const char* text, const char** arg, unsigned argCount, std::string* result);
void SetPreprocessCurFile(const char* filename);

struct ShaderPreprocessScope
{
    ShaderPreprocessScope();
    ~ShaderPreprocessScope();

private:
    ShaderPreprocessScope(const ShaderPreprocessScope&) = delete;
    ShaderPreprocessScope(ShaderPreprocessScope&&) = delete;
    ShaderPreprocessScope operator = (const ShaderPreprocessScope&) = delete;
};
