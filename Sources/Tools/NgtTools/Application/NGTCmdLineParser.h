#pragma once

#include <core_generic_plugin/interfaces/i_command_line_parser.hpp>
#include <core_dependency_system/i_interface.hpp>

namespace NGTLayer
{
class NGTCmdLineParser
: public wgt::Implements<wgt::ICommandLineParser>
{
public:
    NGTCmdLineParser(int argc_, char** argv_);

    int argc() const override;
    char** argv() const override;

    bool getFlag(const char* arg) const override;
    const char* getParam(const char* arg) const override;
    std::string getParamStr(const char* arg) const override;
    std::wstring getParamStrW(const char* arg) const override;

private:
    int m_argc;
    char** m_argv;
};
} // namespace NGTLayer