// Copyright (c) NetXS Group.
// Licensed under the MIT license.

//deprecated stuff

#ifndef NETXS_TEXT_TABLE_HPP
#define NETXS_TEXT_TABLE_HPP

#include <sstream>
#include <vector>

#include "text/utf.hpp"

namespace netxs
{
    namespace table
    {
        using utf::text;
        using cell    = std::vector<text>;
        using row     = std::vector<cell>;
        using content = std::vector<row>;

        text draw(content const& table)
        {
            auto line = utf::flux{};

            if (table.size() && table[0].size())
            {
                const auto top_left     = text{ "┌"   };
                const auto top_right    = text{ "┐\n" };
                const auto top_cross    = text{ "┬"   };
                const auto bottom_cross = text{ "┴"   };
                const auto bottom_left  = text{ "└"   };
                const auto bottom_right = text{ "┘\n" };
                const auto vertical     = text{ "│"   };
                const auto horizontal   = text{ "─"   };
                const auto cross        = text{ "┼"   };
                const auto left_cross   = text{ "├"   };
                const auto right_cross  = text{ "┤\n" };
                const auto space        = text{ " "   };
                const auto end_line     = text{ "\n"  };
                const auto empty        = text{};

                auto columns_count = table[0].size();
                auto widths = std::vector<size_t>(columns_count);
                for (auto& row : table)
                {
                    for (auto cell = 0; cell < row.size(); cell++)
                    {
                        auto max_width = 1 + utf::maxlen(row[cell]) + 1;
                        if (max_width > widths[cell])
                        {
                            widths[cell] = max_width;
                        }
                    }
                }

                auto stroke = [&](text const& left,
                                  text const& inner,
                                  text const& right)
                {
                    line << left;
                    for (auto i = 0; i + 1 < columns_count; i++)
                    {
                        line << utf::repeat(horizontal, widths[i]) << inner;
                    }
                    line << utf::repeat(horizontal, widths.back()) << right;
                };

                stroke(top_left, top_cross, top_right);
                for (auto r = 0; r < table.size(); r++)
                {
                    auto& row_values = table[r];
                    auto  rows_count = utf::maxlen(row_values);

                    for (auto j = 0; j < rows_count; j++)
                    {
                        line << vertical;
                        for (auto i = 0; i < columns_count; i++)
                        {
                            auto cell_line = text(j < row_values[i].size() ? row_values[i][j] : empty);
                            line << utf::adjust(space + cell_line, widths[i], space);
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