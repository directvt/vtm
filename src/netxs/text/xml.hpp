// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_XML_HPP
#define NETXS_XML_HPP

#include "../ui/layout.hpp"

namespace netxs::xml
{
    using namespace netxs::utf;
    using namespace netxs::ui::atoms;
    using dict = std::unordered_map<text, text>;

    static constexpr auto spaces = " \n\r\t";
    enum class type
    {
        none,
        token,
        open,
        close,
    };
    auto escape(text line)
    {
        utf::change(line, "\\"s, "\\\\"s);
        utf::change(line, "\""s, "\\\""s);
        return line;
    }
    auto unescape(qiew line)
    {
        auto crop = text{};
        crop.reserve(line.size());
        while (line)
        {
            auto c = line.pop_front();
            if (c == '\\' && line)
            {
                auto c = line.pop_front();
                switch (c)
                {
                    case 'e' : crop.push_back('\x1b'); break;
                    case 't' : crop.push_back('\t'  ); break;
                    case 'r' : crop.push_back('\n'  ); break;
                    case 'n' : crop.push_back('\n'  ); break;
                    case 'a' : crop.push_back('\a'  ); break;
                    case '\"': crop.push_back('\"'  ); break;
                    case '\'': crop.push_back('\''  ); break;
                    default:   crop.push_back('\\'  );
                               crop.push_back(c     ); break;
                };
            }
            else crop.push_back(c);
        }
        return crop;
    }
    auto open(view& data, type& next)
    {
        utf::trim_front(data, spaces);
        auto iter = data.find('<');
        if (iter != view::npos)
        {
            data.remove_prefix(iter + 1);
            next = type::open;
            return true;
        }
        else
        {
            next = type::none;
            return faux;
        }
    }
    auto attr(view& data, text& item, type& next)
    {
        utf::trim_front(data, spaces);
        item.clear();
        if (data.empty()) next = type::none;
        else
        {
            auto c = data.front();
            if (c == '/')
            {
                data.remove_prefix(1);
                if (data.size() && data.front() == '>')
                {
                    data.remove_prefix(1);
                    next = type::close;
                }
                else next = type::none;
            }
            else
            {
                item = get_tail(data, " \t\n\r=");
                next = type::token;
                utf::to_low(item);
            }
        }
        return next == type::token;
    }
    auto value(view& data)
    {
        auto crop = text{};
        utf::trim_front(data, spaces);

        if (data.empty() || data.front() != '=') return crop;
        data.remove_prefix(1); // remove '='.
        utf::trim_front(data, spaces);

        auto delim = data.front();
        if (delim != '/')
        {
            if (delim != '\'' && delim != '\"') crop = utf::get_tail(data, " \t\n\r/>");
            else                                crop = utf::get_quote(data, view(&delim, 1));
            crop = xml::unescape(crop);
        }
        return crop;
    }
    template<class T>
    auto take(dict& item, text const& attr, T fallback = {})
    { }
    template<>
    auto take<si32>(dict& item, text const& attr, si32 result)
    {
        if (auto v = utf::to_int(item[attr]))
        {
            result = v.value();
        }
        return result;
    }
    template<>
    auto take<view>(dict& item, text const& attr, view result)
    {
        if (auto iter = item.find(attr); iter != item.end())
        {
            result = view{ iter->second };
        }
        return result;
    }
    template<>
    auto take<bool>(dict& item, text const& attr, bool result)
    {
        if (auto iter = item.find(attr); iter != item.end())
        {
            auto& value = utf::to_low(iter->second);
            result = value.empty() || value.starts_with("1")  // 1 - true
                                   || value.starts_with("o")  // on
                                   || value.starts_with("y")  // yes
                                   || value.starts_with("t"); // true
        }
        return result;
    }
    template<>
    auto take<twod>(dict& item, text const& attr, twod result)
    {
        if (auto iter = item.find(attr); iter != item.end())
        {
            auto shadow = view{ iter->second };
            utf::trim_front(shadow, " ({[\"\'");
            if (auto x = utf::to_int(shadow))
            {
                utf::trim_front(shadow, " ,.x/:;");
                if (auto y = utf::to_int(shadow))
                {
                    result.x = x.value();
                    result.y = y.value();
                }
            }
        }
        return result;
    }
    template<>
    auto take<rgba>(dict& item, text const& attr, rgba result)
    {
        auto tobyte = [](auto c)
        {
                 if (c >= '0' && c <= '9') return c - '0';
            else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            else                           return 0;
        };

        if (auto iter = item.find(attr); iter != item.end())
        {
            auto& value = utf::to_low(iter->second);
            auto shadow = view{ value };
            utf::trim_front(shadow, " ({[\"\'");
            if (shadow.starts_with('#')) // hex: #rrggbbaa
            {
                shadow.remove_prefix(1);
                if (shadow.size() >= 8) // hex: #rrggbbaa
                {
                    result.chan.r = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                    result.chan.g = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                    result.chan.b = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                    result.chan.a = (tobyte(shadow[6]) << 4) + tobyte(shadow[7]);
                }
                else if (shadow.size() >= 6) // hex: #rrggbb
                {
                    result.chan.r = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                    result.chan.g = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                    result.chan.b = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                    result.chan.a = 0xff;
                }
                else log(" xml: unknown hex rgba format: { ", value, " }, expected #rrggbbaa or #rrggbb rgba hex value");
            }
            else if (shadow.starts_with("0x")) // hex: 0xaabbggrr
            {
                shadow.remove_prefix(2);
                if (shadow.size() >= 8) // hex: 0xaabbggrr
                {
                    result.chan.a = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                    result.chan.b = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                    result.chan.g = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                    result.chan.r = (tobyte(shadow[6]) << 4) + tobyte(shadow[7]);
                }
                else if (shadow.size() >= 6) // hex: 0xbbggrr
                {
                    result.chan.b = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                    result.chan.g = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                    result.chan.r = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                    result.chan.a = 0xff;
                }
                else log(" xml: unknown hex rgba format: { ", value, " }, expected 0xaabbggrr or 0xbbggrr rgba hex value");
            }
            else if (utf::check_any(shadow, ",;/")) // dec: 000,000,000,000
            {
                if (auto r = utf::to_int(shadow))
                {
                    result.chan.r = r.value();
                    utf::trim_front(shadow, ",./:;");
                    if (auto g = utf::to_int(shadow))
                    {
                        result.chan.g = g.value();
                        utf::trim_front(shadow, ",./:;");
                        if (auto b = utf::to_int(shadow))
                        {
                            result.chan.b = b.value();
                            utf::trim_front(shadow, ",./:;");
                            if (auto a = utf::to_int(shadow)) result.chan.a = a.value();
                            else                              result.chan.a = 0xff;
                            return result;
                        }
                    }
                }
                log(" xml: unknown rgba format: { ", value, " }, expected 000,000,000,000 decimal rgba value");
            }
            else // Single ANSI color value
            {
                if (auto c = utf::to_int(shadow); c.value() >=0 && c.value() <=255)
                {
                    result = rgba::color256[c.value()];
                }
                else log(" xml: unknown ANSI 256-color value format: { ", value, " }, expected 0-255 decimal value");
            }
        }
        return result;
    }
}

#endif // NETXS_XML_HPP