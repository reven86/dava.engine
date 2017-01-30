#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "iarchive.hxx"
#include "pack_format.hxx"
#include "pack_meta_data.hxx"

class pack_archive final : public iarchive
{
public:
    explicit pack_archive(const std::string& archiveName);

    const std::vector<pack_format::file_info>& get_files_info() const override;

    const pack_format::file_info* get_file_info(const std::string& relative) const override;

    bool has_file(const std::string& relative) const override;

    bool
    load_file(const std::string& relativeFilePath, std::vector<uint8_t>& output) override;

    bool has_meta() const override;

    const pack_meta_data& get_meta() const override;

    std::string print_meta() const override;

private:
    std::ifstream file;
    pack_format::pack_file pack_file;
    std::unique_ptr<pack_meta_data> pack_meta;
    std::unordered_map<std::string, pack_format::file_table_entry*> map_file_data;
    std::vector<pack_format::file_info> files_info;
};
