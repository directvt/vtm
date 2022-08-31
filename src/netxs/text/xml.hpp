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
    enum class type_old
    {
        none,
        token,
        open,
        close,
    };
    template<class T>
    auto escape(T&& line)
    {
        auto temp = text{ std::forward<T>(line) };
        utf::change(temp, "\\"s,   "\\\\"s);
        utf::change(temp, "\""s,   "\\\""s);
        utf::change(temp, "\x1b"s, "\\e"s );
        utf::change(temp, "\n"s,   "\\n"s );
        utf::change(temp, "\r"s,   "\\r"s );
        utf::change(temp, "\t"s,   "\\t"s );
        utf::change(temp, "\a"s,   "\\a"s );
        return temp;
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
    auto open(view& data, type_old& next)
    {
        auto iter = data.find('<');
        if (iter != view::npos)
        {
            data.remove_prefix(iter + 1);
            next = type_old::open;
            return true;
        }
        else
        {
            next = type_old::none;
            return faux;
        }
    }
    auto attr(view& data, text& item, type_old& next)
    {
        utf::trim_front(data, spaces);
        item.clear();
        if (data.empty()) next = type_old::none;
        else
        {
            auto c = data.front();
            if (c == '/')
            {
                data.remove_prefix(1);
                if (data.size() && data.front() == '>')
                {
                    data.remove_prefix(1);
                    next = type_old::close;
                }
                else next = type_old::none;
            }
            else
            {
                item = get_tail(data, " \t\n\r=");
                next = type_old::token;
                utf::to_low(item);
            }
        }
        return next == type_old::token;
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


    using frag = netxs::sptr<text>;

    enum type
    {
        na,            // start of file
        eof,           // end of file
        top_token,     // open tag name
        end_token,     // close tag name
        token,         // name
        raw_text,      //         ex: raw text
        quotes,        // '"'     ex: " or '
        quoted_text,   // '"'     ex: " text "
        begin_tag,     // '<'     ex: <name ...
        close_tag,     // '</'    ex: ... </name>
        comment_begin, // '<!--'  ex: ... <!-- ...
        comment_close, // '-->'   ex: ... -->
        close_inline,  // '>'     ex: ... >
        empty_tag,     // '/>'    ex: ... />
        equal,         // '='     ex: name=value
        defaults,      // '*'     ex: name*
        whitespaces,   // ' '     ex: \s\t\r\n...
        unknown,       //
        tag_value,     //
    };

    class document
    {
        class suit
        {
            class literal
            {
            public:
                using list = std::list<literal>;
                using iter = std::list<literal>::iterator;

                suit& boss;
                iter  upto;
                type  kind;
                frag  part;

                template<class ...Args>
                literal(suit& boss, type kind, Args&&... args)
                    : boss{ boss },
                      kind{ kind },
                      part{ std::make_shared<text>(std::forward<Args>(args)...) }
                { }
                literal(suit& boss, type kind, frag part)
                    : boss{ boss },
                      kind{ kind },
                      part{ part }
                { }
            };

        public:
            using list = literal::list;

            list data;
            bool live;
            bool fail;
            si32 deep;
            text file;

            suit(text file = {})
                : live{ true },
                  fail{ faux },
                  file{ file },
                  deep{ 0    }
            { }
           ~suit() { live = faux; } 

            auto& last_type()
            {
                return data.back().kind;
            }
            auto last_type(type what)
            {
                return data.size() && last_type() == what;
            }
            auto last_iter()
            {
                return std::prev(data.end());
            }
            template<class ...Args>
            auto append(type kind, Args&&... args)
            {
                auto& item = data.emplace_back(*this, kind, std::forward<Args>(args)...);
                return item.part;
            }
            auto lines()
            {
                auto count = 0_sz;
                for (auto& item : data)
                {
                    auto part = *item.part;
                    count += std::count(part.begin(), part.end(), '\n');
                }
                return std::max(1_sz, count);
            }
        };

        class elem;
        using sptr = netxs::sptr<elem>;
        using wptr = netxs::wptr<elem>;
        using byid = std::unordered_map<text, sptr>;
        using list = std::vector<frag>;
        using vect = std::vector<sptr>;
        using subs = std::unordered_map<text, vect>;

        class elem
            : public std::enable_shared_from_this<elem>
        {
        public:
            elem(suit& page, wptr parent_wptr)
                : page{ page },
                  parent_wptr{ parent_wptr }
            { }
           ~elem()
            {
                if (start_iter->boss.live)
                {
                    sub.clear();
                    auto head = start_iter;
                    auto tail = std::next(start_iter->upto);
                    start_iter->boss.data.erase(head, tail);
                }
            }

            static auto create(suit& page, wptr parent_wptr = {})
            {
                return std::make_shared<elem>(page, parent_wptr);
            }
            static auto root(suit& page, view shadow)
            {
                auto root_ptr = create(page);
                root_ptr->start(shadow);
                return root_ptr;
            }

            static auto& map()
            {
                static byid map;
                return map;
            }
            template<class T>
            static auto map(T&& key, sptr item)
            {
                map().emplace(std::forward<T>(key), item);
            }

            using iter = suit::list::iterator;

            suit& page;
            frag  tag_ptr;
            list  val_ptr_list;
            subs  sub;
            wptr  parent_wptr;
            wptr  def_wptr;
            bool  is_template{};
            iter  start_iter;

            static constexpr auto spaces = " \n\r\t";
            static constexpr auto find_start = "<";
            static constexpr auto rawtext_delims = " \t\n\r/><";
            static constexpr auto token_delims = " \t\n\r=*/><";

            static constexpr view view_comment_begin = "<!--";
            static constexpr view view_comment_close = "-->" ;
            static constexpr view view_close_tag     = "</"  ;
            static constexpr view view_begin_tag     = "<"   ;
            static constexpr view view_empty_tag     = "/>"  ;
            static constexpr view view_slash         = "/"   ;
            static constexpr view view_close_inline  = ">"   ;
            static constexpr view view_quoted_text   = "\""  ;
            static constexpr view view_equal         = "="   ;
            static constexpr view view_defaults      = "*"   ;

            auto peek(view temp, type& what, type& last)
            {
                last = what;
                if (last == type::na)
                {
                    if (!temp.starts_with(view_comment_begin)
                     && !temp.starts_with(view_close_tag    )
                     &&  temp.starts_with(view_begin_tag    )) what = type::begin_tag;
                    else return;
                }
                else if (temp.empty())                         what = type::eof;
                else if (temp.starts_with(view_comment_begin)) what = type::comment_begin;
                else if (temp.starts_with(view_close_tag    )) what = type::close_tag;
                else if (temp.starts_with(view_begin_tag    )) what = type::begin_tag;
                else if (temp.starts_with(view_empty_tag    )) what = type::empty_tag;
                else if (temp.starts_with(view_slash        ))
                         what = type::unknown;
                else if (temp.starts_with(view_close_inline )) what = type::close_inline;
                else if (temp.starts_with(view_quoted_text  )) what = type::quoted_text;
                else if (temp.starts_with(view_equal        )) what = type::equal;
                else if (temp.starts_with(view_defaults     )
                      && last == type::token)        what = type::defaults;
                else if (last == type::close_tag
                      || last == type::begin_tag
                      || last == type::token
                      || last == type::defaults
                      || last == type::raw_text
                      || last == type::quoted_text)  what = type::token;
                else                                 what = type::raw_text;
            }
            auto take_token(view& data)
            {
                auto item = utf::get_tail(data, token_delims);
                utf::to_low(item);
                return std::move(item);
            }
            template<bool Append_page = true>
            auto take_value(view& data)
            {
                auto item_ptr = frag{};
                if (data.size())
                {
                    auto delim = data.front();
                    if (delim != '\'' && delim != '\"')
                    {
                        auto crop = utf::get_tail(data, rawtext_delims);
                        item_ptr = std::make_shared<text>(xml::unescape(crop));
                        if constexpr (Append_page)
                        {
                            page.append(type::tag_value, item_ptr);
                        }
                    }
                    else
                    {
                        auto delim_view = view(&delim, 1);
                        auto crop = utf::get_quote(data, delim_view);
                        item_ptr = std::make_shared<text>(xml::unescape(crop));
                        if constexpr (Append_page) 
                        {
                            page.append(type::quotes, delim_view);
                            page.append(type::tag_value, item_ptr);
                            page.append(type::quotes, delim_view);
                        }
                    }
                }
                else item_ptr = std::make_shared<text>("");
                return item_ptr;
            }
            auto skip(view& data, type what)
            {
                auto temp = data;
                switch (what)
                {
                    case type::comment_begin: data.remove_prefix(view_comment_begin.size()); break;
                    case type::comment_close: data.remove_prefix(view_comment_close.size()); break;
                    case type::close_tag:     data.remove_prefix(view_close_tag    .size()); break;
                    case type::begin_tag:     data.remove_prefix(view_begin_tag    .size()); break;
                    case type::empty_tag:     data.remove_prefix(view_empty_tag    .size()); break;
                    case type::close_inline:  data.remove_prefix(view_close_inline .size()); break;
                    case type::quoted_text:   data.remove_prefix(view_quoted_text  .size()); break;
                    case type::equal:         data.remove_prefix(view_equal        .size()); break;
                    case type::defaults:      data.remove_prefix(view_defaults     .size()); break;

                    case type::token:
                    case type::top_token:
                    case type::end_token:     take_token(data); break;

                    case type::raw_text:
                    case type::quotes:
                    case type::tag_value:     take_value<faux>(data); break;

                    case type::whitespaces:   utf::trim_front(data, spaces); break;

                    case type::na:            utf::get_tail<faux>(data, find_start); break;

                    case type::unknown:       if (data.size()) data.remove_prefix(1); break;
                    default: break;
                }
                return temp.substr(0, temp.size() - data.size());
            }
            auto trim(view& data)
            {
                auto temp = utf::trim_front(data, spaces);
                if (temp.size()) page.append(type::whitespaces, std::move(temp));
            }
            auto diff(view& data, view& temp, type kind = type::whitespaces)
            {
                auto delta = temp.size() - data.size();
                     if (delta > 0) page.append(kind, temp.substr(0, delta));
                else if (delta < 0) log(" xml: unexpected data");
            }
            auto take_token_ptr(view& data)
            {
                auto item = utf::get_tail(data, token_delims);
                utf::to_low(item);
                auto item_ptr = std::make_shared<text>(std::move(item));
                page.append(type::token, item_ptr);
                return item_ptr;
            }
            auto fail(type what, type last, view data)
            {
                auto str = [&](type what)
                {
                    switch (what)
                    {
                        case type::na:            return view{ "{START}" }   ;
                        case type::eof:           return view{ "{EOF}" }     ;
                        case type::token:         return view{ "{token}" }   ;
                        case type::raw_text:      return view{ "{raw text}" };
                        case type::quoted_text:   return view_quoted_text    ;
                        case type::begin_tag:     return view_begin_tag      ;
                        case type::close_tag:     return view_close_tag      ;
                        case type::comment_begin: return view_comment_begin  ;
                        case type::comment_close: return view_comment_close  ;
                        case type::close_inline:  return view_close_inline   ;
                        case type::empty_tag:     return view_empty_tag      ;
                        case type::equal:         return view_equal          ;
                        case type::defaults:      return view_defaults       ;
                        default:                  return view{ "{unknown}" } ;
                    };
                };
                log(" xml: unexpected '", str(what), "' after '", str(last), "' at ", page.file, ":", page.lines());
                page.fail = true;
            }
            auto check_spaces()
            {
                auto& a = page.data.back();
                if (page.last_type() != type::whitespaces)
                {
                    page.append(type::whitespaces, "");
                }
            }
            auto take_pair(view& data, type& what, type& last, type token_kind)
            {
                tag_ptr = take_token_ptr(data);
                auto& last_type = page.last_type();
                if (last_type == type::token) last_type = token_kind;

                auto temp = data;
                utf::trim_front(temp, spaces);
                peek(temp, what, last);

                if (what == type::defaults)
                {
                    diff(data, temp, type::defaults);
                    data=temp;
                    is_template = true;
                    auto& last_type = page.last_type();
                    if (last_type == type::top_token || last_type == type::token)
                    {
                        last_type = type::defaults;
                    }
                    page.append(type::defaults, skip(data, what));
                    temp = data;
                    utf::trim_front(temp, spaces);
                    peek(temp, what, last);
                }

                if (what == type::equal)
                {
                    diff(data, temp, type::whitespaces);
                    data=temp;

                    page.append(type::equal, skip(data, what));
                    trim(data);
                    peek(data, what, last);
                    if (what == type::quoted_text || what == type::raw_text)
                    {
                        val_ptr_list.push_back(take_value(data));
                    }
                    else fail(what, last, data);
                }
            }
            auto open_fragment()
            {
                check_spaces();
                start_iter = page.last_iter();
            }
            auto close_fragment()
            {
                auto final_iter = page.last_iter();
                start_iter->upto = final_iter;
                final_iter->upto = start_iter;
            }
            void parse(view& data)
            {
                page.deep++;
                auto what = type::na;
                auto last = type::na;
                auto defs = std::unordered_map<text, wptr>{};
                auto fire = faux;
                auto push = [&](sptr& item)
                {
                    auto& sub_tag = *(item->tag_ptr);
                    if (item->is_template) defs[sub_tag] = item;
                    else
                    {
                        auto iter = defs.find(sub_tag);
                        if (iter != defs.end())
                        {
                            item->def_wptr = iter->second;
                        }
                    }
                    sub[sub_tag].push_back(item);
                };

                trim(data);
                open_fragment();

                peek(data, what, last);
                if (what == type::begin_tag)
                {
                    page.append(type::begin_tag, skip(data, what));
                    trim(data);
                    peek(data, what, last);
                    if (what == type::token)
                    {
                        take_pair(data, what, last, type::top_token);
                        trim(data);
                        peek(data, what, last);

                        if (what == type::token)
                        {
                            do // Proceed inlined subs
                            {
                                auto item = elem::create(page, shared_from_this());
                                item->open_fragment();
                                item->take_pair(data, what, last, type::token);
                                item->close_fragment();
                                push(item);
                                trim(data);
                                peek(data, what, last);
                            }
                            while (what == type::token);
                        }

                        if (what == type::empty_tag) // />
                        {
                            page.append(type::empty_tag, skip(data, what));
                        }
                        else if (what == type::close_inline) // proceed nested subs
                        {
                            page.append(type::close_inline, skip(data, what));
                            do
                            {
                                auto temp = data;
                                utf::trim_front(temp, spaces);
                                peek(temp, what, last);
                                do
                                {
                                    if (what == type::quoted_text)
                                    {
                                        diff(temp, data, type::quoted_text);
                                        data = temp;
                                        val_ptr_list.push_back(take_value(data));
                                        trim(data);
                                        temp = data;
                                    }
                                    else if (what == type::raw_text)
                                    {
                                        auto size = data.find('<');
                                        val_ptr_list.push_back(std::make_shared<text>(data.substr(0, size)));
                                        if (size == view::npos)
                                        {
                                            page.append(type::unknown, data);
                                            data = {};
                                            last = what;
                                            what = type::eof;
                                            break;
                                        }
                                        page.append(type::raw_text, data.substr(0, size));
                                        data.remove_prefix(size);
                                        temp = data;
                                    }
                                    else if (what == type::begin_tag && page.deep < 30)
                                    {
                                        trim(data);
                                        data = temp;
                                        auto item = elem::create(page, shared_from_this());
                                        item->parse(data);
                                        push(item);
                                        temp = data;
                                        utf::trim_front(temp, spaces);
                                    }
                                    else if (what == type::comment_begin) // <!--
                                    {
                                        auto size = data.find(view_comment_close);
                                        if (size == view::npos)
                                        {
                                            page.append(type::unknown, data);
                                            data = {};
                                            last = what;
                                            what = type::eof;
                                            break;
                                        }
                                        size += view_comment_close.size();
                                        page.append(type::comment_begin, data.substr(0, size));
                                        data.remove_prefix(size);
                                        temp = data;
                                        utf::trim_front(temp, spaces);
                                    }
                                    else if (what != type::close_tag
                                          && what != type::eof)
                                    {
                                        fail(what, last, data);
                                        skip(temp, what);
                                        diff(temp, data, type::unknown);
                                        data = temp;
                                    }
                                    peek(temp, what, last);
                                }
                                while (what != type::close_tag
                                    && what != type::eof);

                                if (what == type::close_tag) // </token>
                                {
                                    auto skip_frag = skip(temp, what);
                                    auto trim_frag = utf::trim_front(temp, spaces);
                                    peek(temp, what, last);
                                    if (what == type::token)
                                    {
                                        auto token = take_token(temp);
                                        trim(data);
                                        if (token == *tag_ptr)
                                        {
                                                                  page.append(type::close_tag, skip_frag);
                                            if (trim_frag.size()) page.append(type::whitespaces, trim_frag);
                                                                  page.append(type::end_token, tag_ptr);
                                            data = temp;
                                            auto tail = data.find('>');
                                            if (tail != view::npos) data.remove_prefix(tail + 1);
                                            else                    data = {};
                                            diff(data, temp, type::close_tag);
                                            break;
                                        }
                                        else
                                        {
                                            what = type::unknown;
                                                                  page.append(what, skip_frag);
                                            if (trim_frag.size()) page.append(what, trim_frag);
                                                                  page.append(what, token);
                                            data = temp;
                                            auto tail = data.find('>');
                                            if (tail != view::npos) data.remove_prefix(tail + 1);
                                            else                    data = {};
                                            diff(data, temp, what);
                                            log(" xml: unexpected closing tag name '", token, "', expected: '", *tag_ptr, "' at ", page.file, ":", page.lines());
                                            continue; // Repeat until eof or success.
                                        }
                                    }
                                    else
                                    {
                                        diff(temp, data, type::unknown);
                                        data = temp;
                                        fail(what, last, data);
                                        continue; // Repeat until eof or success.
                                    }
                                }
                                else if (what == type::eof)
                                {
                                    trim(data);
                                    if (!page.last_type(type::eof)) log(" xml: unexpected {EOF}");
                                }
                            }
                            while (data.size());
                        }
                        else fire = true;
                    }
                    else fire = true;
                }
                else fire = true;

                if (!tag_ptr)
                {
                    auto head = page.data.rbegin();
                    auto tail = page.data.rend();
                    while (head != tail)
                    {
                        auto& item = *head++;
                        auto kind = item.kind;
                        item.kind = type::unknown;
                        if (kind == type::begin_tag) break;
                    }
                    
                    tag_ptr = std::make_shared<text>("");
                    page.append(type::tag_value, tag_ptr);
                    log(" xml: empty tag name at ", page.file, ":", page.lines());
                }
                if (fire) fail(what, last, data);
                if (what == type::eof) page.append(what, "");
                close_fragment();
                page.deep--;
            }
            void start(view& data)
            {
                auto temp = data;
                auto what = type::na;
                auto last = type::na;
                auto empty = utf::trim_front(temp, spaces);
                peek(temp, what, last);
                while (what != type::begin_tag && what != type::eof) // Skip all non-xml data.
                {
                    if (what == type::na) fail(type::raw_text, last, data);
                    else                  fail(what, last, data);
                    page.append(type::unknown, empty);
                    page.append(type::unknown, skip(temp, what));
                    data = temp;
                    empty = utf::trim_front(temp, spaces);
                    peek(temp, what, last);
                }
                parse(data);
            }
            text show(sz_t indent = 0)
            {
                auto data = text{};
                data += text(indent, ' ') + '<' + *tag_ptr;
                if (is_template) data += view_defaults;

                if (val_ptr_list.size())
                {
                    auto val = text{};
                    for (auto& val_ptr : val_ptr_list)
                    {
                        val += *val_ptr;
                    }
                    if (val.size())
                    {
                        if (utf::check_any(val, rawtext_delims)) data += "=\"" + xml::escape(val) + "\"";
                        else                                     data += '='   + xml::escape(val) + ' ';
                    }
                }

                if (sub.empty()) data += "/>\n";
                else
                {
                    data += ">\n";
                    for (auto& [sub_tag, sub_list] : sub)
                    {
                        for (auto& item : sub_list)
                        {
                            data += item->show(indent + 4);
                        }
                    }
                    data += text(indent, ' ') + "</" + *tag_ptr + ">\n";
                }

                return data;
            }
        };

    public:
        suit page;
        sptr root;

        template<class T>
        auto get(T&& path)
        {

        }
        template<class T>
        auto set(T&& path)
        {
            
        }
        auto show()
        {
            static const rgba top_token_fg = 0xFFffd799;
            static const rgba end_token_fg = 0xFFb3966a;
            static const rgba token_fg     = 0xFFdab883;
            static const rgba liter_fg     = 0xFF808080;
            static const rgba comment_fg   = 0xFF4e4e4e;
            static const rgba defaults_fg  = 0xFF9e9e9e;
            static const rgba quotes_fg    = 0xFFBBBBBB;
            static const rgba value_fg     = 0xFFf09690;
            static const rgba value_bg     = 0xFF202020;

            auto yield = ansi::esc{};
            for (auto& item : page.data)
            {
                auto kind = item.kind;
                auto data_ptr = item.part;

                auto fgc = rgba{};
                auto bgc = rgba{};
                switch (kind)
                {
                    case eof:           fgc = redlt;        break;
                    case top_token:     fgc = top_token_fg; break;
                    case end_token:     fgc = end_token_fg; break;
                    case token:         fgc = token_fg;     break;
                    case raw_text:      fgc = yellowdk;     break;
                    case quoted_text:   fgc = yellowdk;     break;
                    case comment_begin: fgc = comment_fg;   break;
                    case comment_close: fgc = comment_fg;   break;
                    case begin_tag:     fgc = liter_fg;     break;
                    case close_tag:     fgc = liter_fg;     break;
                    case close_inline:  fgc = liter_fg;     break;
                    case empty_tag:     fgc = liter_fg;     break;
                    case equal:         fgc = liter_fg;     break;
                    case quotes:        fgc = quotes_fg;    break;
                    case defaults:      fgc = defaults_fg;  break;
                    case unknown:       fgc = redlt;        break;
                    case tag_value:     fgc = value_fg;
                                        bgc = value_bg;     break;
                    default: break;
                }
                auto& data = *data_ptr;
                if (kind == type::tag_value)
                {
                    auto temp = data;
                    if (fgc)
                    {
                        //if (bgc) yield.fgc(fgc).bgc(bgc);
                        //else     yield.fgc(fgc);
                        //yield.add(xml::escape(temp)).nil();
                        yield.fgc(fgc).add(xml::escape(temp)).nil();
                    }
                    else yield.add(xml::escape(temp));
                }
                else
                {
                    if (fgc) yield.fgc(fgc).add(data).nil();
                    else     yield.add(data);
                }
            }

            auto count = 1;
            auto width = 0_sz;
            auto lines = page.lines();
            while (lines)
            {
                lines /= 10;
                width++;
            }
            auto numerate = [&]
            {
                return ansi::pushsgr().fgc(liter_fg) + utf::adjust(std::to_string(count++), width, ' ', true) + ": " + ansi::popsgr();
            };
            yield = numerate() + yield;
            utf::for_each(yield, "\n", [&]{ return "\n" + numerate(); });
            return yield;
        }

        document(view data, text file = "")
            : page{ file },
              root{ elem::root(page, data) }
        {
            if (page.fail) log(" xml: inconsistent xml data from ", page.file, "\n", show());
        }
    };
}

#endif // NETXS_XML_HPP