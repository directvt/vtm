// Copyright (c) NetXS Group.
// Licensed under the MIT license.

//deprecated stuff

#ifndef NETXS_TEXT_TABLE_HPP
#define NETXS_TEXT_TABLE_HPP

#include <sstream>
#include <vector>

#include "text/utf.hpp"

namespace utils
{
    namespace table
    {
        template<class char_T>
        inline std::basic_string<char_T> convert(std::string const& str)
        {
            return std::basic_string<char_T>();
        }
        template <>
        inline std::basic_string<char> convert<char>(std::string const& str)
        {
            return str;
        }
        template <>
        inline std::basic_string<wchar_t> convert<wchar_t>(std::string const& str)
        {
            return utf::to_utf(str);
        }

        template<class T>
        using cell = std::vector<std::basic_string<T>>;

        template<class T>
        using row = std::vector<cell<T>>;

        template<class T>
        using content = std::vector<row<T>>;

        template<class T, class W = std::conditional<std::is_same<char, T>::value, wchar_t, char>::type>
        cell<W> to_utf(cell<T>& obj)
        {
            cell<W> values;
            for (auto& line : obj)
            {
                values.push_back(utils::utf::to_utf(line));
            }
            return values;
        }

        template<class T, class W = std::conditional<std::is_same<char, T>::value, wchar_t, char>::type>
        row<W> to_utf(row<T>& obj)
        {
            row<W> values;
            for (auto& cell : obj)
            {
                values.push_back(to_utf(cell));
            }
            return values;
        }

        template<class T>
        std::basic_string<T> draw(content<T> const& table)
        {
            std::basic_stringstream<T>	line;

            if (table.size() && table[0].size())
            {
                const std::basic_string<T> top_left		= convert<T>("┌");
                const std::basic_string<T> top_right	= convert<T>("┐\n");
                const std::basic_string<T> top_cross	= convert<T>("┬");
                const std::basic_string<T> bottom_cross = convert<T>("┴");
                const std::basic_string<T> bottom_left	= convert<T>("└");
                const std::basic_string<T> bottom_right = convert<T>("┘\n");
                const std::basic_string<T> vertical		= convert<T>("│");
                const std::basic_string<T> horizontal	= convert<T>("─");
                const std::basic_string<T> cross		= convert<T>("┼");
                const std::basic_string<T> left_cross	= convert<T>("├");
                const std::basic_string<T> right_cross	= convert<T>("┤\n");
                const std::basic_string<T> space		= convert<T>(" ");
                const std::basic_string<T> end_line		= convert<T>("\n");
                const std::basic_string<T> empty;

                size_t columns_count = table[0].size();
                std::vector<size_t> widths(columns_count);
                for (auto& row : table)
                {
                    for (size_t cell = 0; cell < row.size(); cell++)
                    {
                        size_t max_width = 1 + utils::utf::maxlen(row[cell]) + 1;
                        if (max_width > widths[cell])
                        {
                            widths[cell] = max_width;
                        }
                    }
                }

                auto stroke = [&](std::basic_string<T> const& left,
                                  std::basic_string<T> const& inner,
                                  std::basic_string<T> const& right)
                {
                    line << left;
                    for (size_t i = 0; i + 1 < columns_count; i++)
                    {
                        line << utils::utf::repeat(horizontal, widths[i]) << inner;
                    }
                    line << utils::utf::repeat(horizontal, widths.back()) << right;
                };

                stroke(top_left, top_cross, top_right);
                for (size_t r = 0; r < table.size(); r++)
                {
                    auto&  row_values = table[r];
                    size_t rows_count = utils::utf::maxlen(row_values);

                    for (size_t j = 0; j < rows_count; j++)
                    {
                        line << vertical;
                        for (size_t i = 0; i < columns_count; i++)
                        {
                            std::basic_string<T> cell_line = (j < row_values[i].size()) ? row_values[i][j] : empty;
                            line << utils::utf::adjust(space + cell_line, widths[i], space);
                            line << vertical;
                        }
                        line << end_line;
                    }

                    if (r + 1 < table.size())
                    {
                        stroke(left_cross, cross, right_cross);
                    }
                    else
                    {
                        stroke(bottom_left, bottom_cross, bottom_right);
                    }
                }
            }

            return line.str();
        }
    }
}

#endif // NETXS_TEXT_TABLE_HPP