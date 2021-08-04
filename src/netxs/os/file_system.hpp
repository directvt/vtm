// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_FILE_SYSTEM_HPP
#define NETXS_FILE_SYSTEM_HPP

#include "../text/utf.hpp"

#include <filesystem>

namespace netxs::os
{
    using utf::text;
    using utf::view;

    inline auto same_path (view path, view same);
    inline auto make_link (view path, view link);
    inline auto copy_file (view file, view copy);
    // It's the same as a path in case of c++17.
    // Rename always overwrites.
    // Rename fails if new_p names a non-existing directory ending with a directory separator.
    inline auto move_file (view file, view move);
    inline auto kill_path (view path);
    inline auto test_path (view path);
    inline auto make_path (view path);
    inline auto work_path (view path = {});
    inline auto file_name (view path);
    inline auto take_temp ();

    auto same_path (view path, view same)
    {
        std::error_code ec;
        return std::filesystem::equivalent(path, same, ec);
    }
    auto make_link (view path, view link)
    {
        std::error_code ec;
        std::filesystem::create_symlink(path, link, ec);
        return !ec;
    }
    auto copy_file (view file, view copy)
    {
        std::error_code ec;
        return std::filesystem::copy_file(file, copy, ec);
    }
    auto move_file (view file, view move)
    {
        std::error_code ec;
        std::filesystem::rename(file, move, ec);
        return !ec;
    }
    auto kill_path (view path)
    {
        std::error_code ec;
        return std::filesystem::remove(path, ec);
    }
    auto test_path (view path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec);
    }
    auto make_path (view path)
    {
        std::error_code ec;
        return std::filesystem::create_directory(path, ec);
    }
    auto work_path (view path)
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
    auto file_name (view path)
    {
        return std::filesystem::path(path).filename().string();
    }
    auto take_temp ()
    {
        std::error_code ec;
        return utf::to_utf(std::filesystem::temp_directory_path(ec).wstring());
    }
}

#endif // NETXS_FILE_SYSTEM_HPP