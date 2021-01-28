// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_FILE_SYSTEM_H
#define NETXS_FILE_SYSTEM_H

#include "../text/utf.h"

#include <filesystem>

namespace netxs::os
{
    using namespace netxs::utf;

    inline bool same_path (view path, view same);
    inline bool make_link (view path, view link);
    inline bool copy_file (view file, view copy);
    // It's the same as a path in case of c++17
    // Rename overites always
    // Rename fails if new_p names a non - existing directory ending with a directory separator
    inline bool move_file (view file, view move);
    inline bool kill_path (view path);
    inline bool test_path (view path);
    inline bool make_path (view path);
    inline text work_path (view path = {});
    inline text file_name (view path);
    inline text take_temp ();

    bool same_path (view path, view same)
    {
        std::error_code ec;
        return std::filesystem::equivalent(path, same, ec);
    }
    bool make_link (view path, view link)
    {
        std::error_code ec;
        std::filesystem::create_symlink(path, link, ec);
        return !ec;
    }
    bool copy_file (view file, view copy)
    {
        std::error_code ec;
        return std::filesystem::copy_file(file, copy, ec);
    }
    bool move_file (view file, view move)
    {
        std::error_code ec;
        std::filesystem::rename(file, move, ec);
        return !ec;
    }
    bool kill_path (view path)
    {
        std::error_code ec;
        return std::filesystem::remove(path, ec);
    }
    bool test_path (view path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec);
    }
    bool make_path (view path)
    {
        std::error_code ec;
        return std::filesystem::create_directory(path, ec);
    }
    text work_path (view path)
    {
        text result;
        if (path.empty())
        {
            result = utf::to_utf(std::filesystem::current_path().wstring());
        }
        else
        {
            std::error_code ec;
            std::filesystem::current_path(path, ec);
            if (!ec) result = path;
        }
        return result;
    }
    text file_name (view path)
    {
        return std::filesystem::path(path).filename().string();
    }
    text take_temp ()
    {
        std::error_code ec;
        return utf::to_utf(std::filesystem::temp_directory_path(ec).wstring());
    }
}

#endif // NETXS_FILE_SYSTEM_H