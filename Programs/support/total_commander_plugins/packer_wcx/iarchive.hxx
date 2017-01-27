#pragma once

#include "pack_format.hxx"
#include "pack_meta_data.hxx"

extern std::ofstream l; // for debug logging

struct iarchive
{
    virtual ~iarchive() = default;

    virtual const std::vector<pack_format::file_info>& get_files_info() const = 0;

    virtual const pack_format::file_info* get_file_info(const std::string& relative) const = 0;

    virtual bool has_file(const std::string& relative) const = 0;

    virtual bool load_file(const std::string& relative, std::vector<uint8_t>& output) = 0;

    virtual bool has_meta() const = 0;

    virtual const pack_meta_data& get_meta() const = 0;

    virtual std::string print_meta() const = 0;

    int32_t file_index = 0;
    std::string archive_name;
    std::string last_file_name;
};

iarchive* create(const std::string& file);
