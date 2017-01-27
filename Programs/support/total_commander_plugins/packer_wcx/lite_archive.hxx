#pragma once

#include "iarchive.hxx"
#include "pack_meta_data.hxx"

#include <fstream>

class lite_archive final : public iarchive
{
public:
    lite_archive(const std::string& file);
    ~lite_archive() override = default;

    const std::vector<pack_format::file_info>& get_files_info() const override;

    const pack_format::file_info* get_file_info(const std::string& relative) const override;

    bool has_file(const std::string& relative) const override;

    bool load_file(const std::string& relative, std::vector<uint8_t>& output) override;

    bool has_meta() const override;

    const pack_meta_data& get_meta() const override;

    std::string print_meta() const override;

private:
    std::ifstream ifile;
    std::string file_name;
    pack_format::lite_pack::footer footer;
    std::vector<pack_format::file_info> file_info;
};
