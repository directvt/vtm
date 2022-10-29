// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_XML_HPP
#define NETXS_XML_HPP

#include "../ui/layout.hpp"
#include "../datetime/quartz.hpp"

namespace netxs::xml
{
    using namespace netxs::utf;
    using namespace netxs::ui::atoms;
    using namespace netxs::datetime;

    static constexpr auto spaces = " \n\r\t"sv;
    enum class type_old
    {
        none,
        token,
        open,
        close,
    };
    auto escape(qiew line)
    {
        auto crop = text{};
        crop.reserve(line.size() * 2);
        while (line)
        {
            auto c = line.pop_front();
            switch (c)
            {
                case '\033': crop.push_back('\\'); crop.push_back('e' ); break;
                case   '\\': crop.push_back('\\'); crop.push_back('\\'); break;
                case   '\"': crop.push_back('\\'); crop.push_back('\"'); break;
                case   '\n': crop.push_back('\\'); crop.push_back('n' ); break;
                case   '\r': crop.push_back('\\'); crop.push_back('r' ); break;
                case   '\t': crop.push_back('\\'); crop.push_back('t' ); break;
                case   '\a': crop.push_back('\\'); crop.push_back('a' ); break;
                default:
                    crop.push_back(c);
                    break;
            }
        }
        return crop;
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
                }
            }
            else crop.push_back(c);
        }
        return crop;
    }

    template<class T>
    auto take(view utf8) -> std::optional<T>
    {
        if (utf8.starts_with("0x"))
        {
            utf8.remove_prefix(2);
            return utf::to_int<T, 16>(utf8);
        }
        else return utf::to_int<T, 10>(utf8);
    }
    template<>
    auto take<text>(view utf8) -> std::optional<text>
    {
        return text{ utf8 };
    }
    template<>
    auto take<bool>(view utf8) -> std::optional<bool>
    {
        auto value = text{ utf8 };
        utf::to_low(value);
        return value.empty() || value.starts_with("1")  // 1 - true
                             || value.starts_with("on") // on
                             || value.starts_with("y")  // yes
                             || value.starts_with("t"); // true
    }
    template<>
    auto take<twod>(view utf8) -> std::optional<twod>
    {
        utf::trim_front(utf8, " ({[\"\'");
        if (auto x = utf::to_int(utf8))
        {
            utf::trim_front(utf8, " ,.x/:;");
            if (auto y = utf::to_int(utf8))
            {
                return twod{ x.value(), y.value() };
            }
        }
        return std::nullopt;
    }
    template<>
    auto take<period>(view utf8) -> std::optional<period>
    {
        using namespace std::chrono;
        utf::trim_front(utf8, " ({[\"\'");
        if (auto x = utf::to_int(utf8))
        {
            auto v = x.value();
            auto p = period{};
                 if (utf8.empty()
                  || utf8.starts_with("ms" )) return period{ milliseconds{ v } };
            else if (utf8.starts_with("us" )) return period{ microseconds{ v } };
            else if (utf8.starts_with("ns" )) return period{  nanoseconds{ v } };
            else if (utf8.starts_with("s"  )) return period{      seconds{ v } };
            else if (utf8.starts_with("min")) return period{      minutes{ v } };
            else if (utf8.starts_with("h"  )) return period{        hours{ v } };
            else if (utf8.starts_with("d"  )) return period{         days{ v } };
            else if (utf8.starts_with("w"  )) return period{        weeks{ v } };
        }
        return std::nullopt;
    }
    template<>
    auto take<rgba>(view utf8) -> std::optional<rgba>
    {
        auto tobyte = [](auto c)
        {
                 if (c >= '0' && c <= '9') return c - '0';
            else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            else                           return 0;
        };

        auto value = text{ utf8 };
        utf::to_low(value);
        auto result = rgba{};
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
                return result;
            }
            else if (shadow.size() >= 6) // hex: #rrggbb
            {
                result.chan.r = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                result.chan.g = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                result.chan.b = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                result.chan.a = 0xff;
                return result;
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
                return result;
            }
            else if (shadow.size() >= 6) // hex: 0xbbggrr
            {
                result.chan.b = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                result.chan.g = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                result.chan.r = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                result.chan.a = 0xff;
                return result;
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
        else if (auto c = utf::to_int(shadow)) // Single ANSI color value
        {
            if (c.value() >=0 && c.value() <=255)
            {
                result = rgba::color256[c.value()];
                return result;
            }
            else log(" xml: unknown ANSI 256-color value format: { ", value, " }, expected 0-255 decimal value");
        }
        return std::nullopt;
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

    struct document
    {
        struct suit
        {
            struct literal
            {
                using list = std::list<literal>;
                using iter = std::list<literal>::iterator;

                suit& boss;
                type  kind;
                iter  upto;
                frag  part;

                template<class ...Args>
                literal(suit& boss, type kind, Args&&... args)
                    : boss{ boss },
                      kind{ kind },
                      upto{ boss.data.end() },
                      part{ std::make_shared<text>(std::forward<Args>(args)...) }
                { }
                literal(suit& boss, type kind, frag part)
                    : boss{ boss },
                      kind{ kind },
                      upto{ boss.data.end() },
                      part{ part }
                { }
            };

            using list = literal::list;

            list data;
            bool live;
            bool fail;
            si32 deep;
            text file;

            suit(suit&&) = default;
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
            auto utf8()
            {
                auto crop = text{};
                for (auto& item : data)
                {
                    crop += *item.part;
                }
                return crop;
            }
        };

        struct elem;
        using sptr = netxs::sptr<elem>;
        using wptr = netxs::wptr<elem>;
        using byid = std::unordered_map<text, sptr>;
        using list = std::vector<frag>;
        using vect = std::vector<sptr>;
        using subs = std::unordered_map<text, vect, qiew::hash, qiew::equal>;

        struct elem
            : public std::enable_shared_from_this<elem>
        {
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

            auto enumerate(qiew path_str)
            {
                using utf::text;
                path_str = utf::trim(path_str, '/');
                auto root = shared_from_this();
                auto crop = vect{}; //auto& items = config.root->sub["menu"][0]->sub["item"]...;
                auto temp = text{};
                auto path = utf::divide(path_str, '/');
                if (path.size())
                {
                    auto head = path.begin();
                    auto tail = path.end();
                    auto item = root;
                    while (head != tail)
                    {
                        temp = *head++;
                        if (auto iter = item->sub.find(temp);
                                iter!= item->sub.end())
                        {
                            auto& i = iter->second;
                            crop.reserve(i.size());
                            if (head == tail)
                            {
                                for (auto& item : i)
                                {
                                    if (!item->is_template) crop.push_back(item);
                                }
                            }
                            else if (i.size() && i.front())
                            {
                                item = i.front();
                            }
                            else break;
                        }
                        else break;
                    }
                }
                return crop;
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
                static auto map = byid{};
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

            auto get_value()
            {
                auto crop = text{};
                for (auto& v : val_ptr_list)
                {
                    crop += *v;
                }
                return crop;
            }
            template<class T>
            auto take(qiew attr, T fallback = {})
            {
                if (auto iter = sub.find(attr); iter != sub.end())
                {
                    auto& item_set = iter->second;
                    if (item_set.size()) // Take the first item only.
                    {
                        auto crop = item_set.front()->get_value();
                        if (auto result = xml::take<T>(crop)) return result.value();
                        else                                  return fallback;
                    }
                }
                if (auto def_ptr = def_wptr.lock()) return def_ptr->take(attr, fallback);
                else                                return fallback;
            }

            static constexpr auto find_start         = "<"sv;
            static constexpr auto rawtext_delims     = " \t\n\r/><"sv;
            static constexpr auto token_delims       = " \t\n\r=*/><"sv;
            static constexpr auto view_comment_begin = "<!--"sv;
            static constexpr auto view_comment_close = "-->"sv;
            static constexpr auto view_close_tag     = "</"sv;
            static constexpr auto view_begin_tag     = "<"sv;
            static constexpr auto view_empty_tag     = "/>"sv;
            static constexpr auto view_slash         = "/"sv;
            static constexpr auto view_close_inline  = ">"sv;
            static constexpr auto view_quoted_text   = "\""sv;
            static constexpr auto view_equal         = "="sv;
            static constexpr auto view_defaults      = "*"sv;

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
                else if (temp.starts_with(view_slash        )) what = type::unknown;
                else if (temp.starts_with(view_close_inline )) what = type::close_inline;
                else if (temp.starts_with(view_quoted_text  )) what = type::quoted_text;
                else if (temp.starts_with(view_equal        )) what = type::equal;
                else if (temp.starts_with(view_defaults     )
                      && last == type::token)                  what = type::defaults;
                else if (view_spaces.find(temp.front()) != view::npos) what = type::whitespaces;
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
                return item;
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
                if (page.data.empty() || page.last_type() != type::whitespaces)
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
                    diff(temp, data, type::whitespaces);
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
        auto utf8()
        {
            return page.utf8();
        }
        auto enumerate(view path_str)
        {
            if (path_str == "/") return vect{ root };
            else
            {
                path_str = utf::trim(path_str, '/');
                auto tmp = utf::cutoff(path_str, '/');
                if (root && root->tag_ptr && *(root->tag_ptr) == tmp)
                {
                    return root->enumerate(path_str.substr(tmp.size()));
                }
            }
            return vect{};
        }
        auto show()
        {
            static constexpr auto top_token_fg = rgba{ 0xFFffd799 };
            static constexpr auto end_token_fg = rgba{ 0xFFb3966a };
            static constexpr auto token_fg     = rgba{ 0xFFdab883 };
            static constexpr auto liter_fg     = rgba{ 0xFF808080 };
            static constexpr auto comment_fg   = rgba{ 0xFF4e4e4e };
            static constexpr auto defaults_fg  = rgba{ 0xFF9e9e9e };
            static constexpr auto quotes_fg    = rgba{ 0xFFBBBBBB };
            static constexpr auto value_fg     = rgba{ 0xFFf09690 };
            static constexpr auto value_bg     = rgba{ 0xFF202020 };

            //test
            //auto tmp = page.data.front().upto;
            //auto clr = 0;

            auto yield = ansi::esc{};
            for (auto& item : page.data)
            {
                auto kind = item.kind;
                auto data_ptr = item.part;

                //test
                //if (item.upto == page.data.end() || tmp != item.upto)
                //{
                //    clr++;
                //    tmp = item.upto;
                //}

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
                //test
                //yield.bgc((tint)(clr % 8));
                if (kind == type::tag_value)
                {
                    auto temp = data;
                    if (fgc) yield.fgc(fgc).add(xml::escape(temp)).nil();
                    else     yield.add(xml::escape(temp));
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

        document(document&&) = default;
        document(view data, text file = "")
            : page{ file },
              root{ elem::root(page, data) }
        {
            if (page.fail) log(" xml: inconsistent xml data from ", page.file, "\n", show());
        }
    };

    struct settings
    {
        netxs::sptr<xml::document> fallback; // = std::make_shared<xml::document>(default_config, "");
        netxs::sptr<xml::document> document; // = fallback;
        xml::document::vect list{};
        xml::document::vect temp{};
        text cwd{};
        text defaults{};

        settings(view default_config)
            : fallback{ std::make_shared<xml::document>(default_config, "") },
              document{ fallback }
        { }
        settings(settings const& other)
            : fallback{ other.fallback },
              document{ other.document }
        { }
        settings() = default;

        auto cd(view path, view defpath = {})
        {
            defaults = utf::trim(defpath, '/');
            if (path.empty()) return faux;
            if (path.front() == '/')
            {
                path = utf::trim(path, '/');
                cwd = "/" + text{ path };
                temp = document->enumerate(cwd);
                if (temp.empty()) temp = fallback->enumerate(cwd);
            }
            else
            {
                path = utf::trim(path, '/');
                cwd += "/" + text{ path };
                if (temp.size())
                {
                    temp = temp.front()->enumerate(path);
                    if (temp.empty()) temp = fallback->enumerate(cwd);
                }
            }
            auto test = !!temp.size();
            if (!test)
            {
                log(" xml:" + ansi::fgc(redlt) + " xml path not found: " + ansi::nil() + cwd);
            }
            return test;
        }
        template<class T = si32>
        auto take(view path, T defval = {})
        {
            if (path.empty()) return defval;
            auto crop = text{};
            auto dest = text{};
            if (path.front() == '/')
            {
                dest = utf::trim(path, '/');
                list = document->enumerate(dest);
            }
            else
            {
                path = utf::trim(path, '/');
                dest = cwd + "/" + text{ path };
                if (temp.size()) list = temp.front()->enumerate(path);
                if (list.empty() && defaults.size())
                {
                    dest = defaults + "/" + text{ path };
                    list = document->enumerate(dest);
                }
            }
            if (list.empty()) list = fallback->enumerate(dest);
            if (list.size() ) crop = list.back()->get_value();
            else              log(" xml:" + ansi::fgc(redlt) + " xml path not found: " + ansi::nil() + dest);
            list.clear();
            if (auto result = xml::take<T>(crop)) return result.value();
            else
            {
                if (crop.size()) return take("/config/set/" + crop, defval);
                else             return defval;
            }
        }
        template<class T>
        auto take(view path, T defval, std::unordered_map<text, T> dict)
        {
            if (path.empty()) return defval;
            auto crop = take(path, ""s);
            auto iter = dict.find(crop);
            return iter == dict.end() ? defval
                                      : iter->second;
        }
        auto take(view path, cell defval)
        {
            if (path.empty()) return defval;
            auto fgc_path = text{ path } + '/' + "fgc";
            auto bgc_path = text{ path } + '/' + "bgc";
            auto crop = cell{};
            crop.fgc(take(fgc_path, defval.fgc()));
            crop.bgc(take(bgc_path, defval.bgc()));
            return crop;
        }
        auto take_list(view path)
        {
            auto list = document->enumerate(path);
            if (list.empty()) list = fallback->enumerate(path);
            return list;
        }
        auto utf8()
        {
            return document->utf8();
        }
        auto merge(view run_config)
        {
            //todo implement
        }
    };
}

#endif // NETXS_XML_HPP