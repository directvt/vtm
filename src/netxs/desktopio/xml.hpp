// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "canvas.hpp"

namespace netxs::xml
{
    template<class T>
    auto take(qiew utf8) -> std::optional<T>
    {
        utf::trim_front(utf8);
        if (utf8.starts_with("0x"))
        {
            utf8.remove_prefix(2);
            return utf::to_int<T, 16>(utf8);
        }
        else return utf8 ? utf::to_int<T, 10>(utf8)
                         : std::nullopt;
    }
    template<>
    auto take<fp32>(qiew utf8) -> std::optional<fp32>
    {
        utf::trim_front(utf8);
        return utf8 ? utf::to_int<fp32>(utf8)
                    : std::nullopt;
    }
    template<>
    auto take<text>(qiew utf8) -> std::optional<text>
    {
        return utf8.str();
    }
    template<>
    auto take<bool>(qiew utf8) -> std::optional<bool>
    {
        utf::trim_front(utf8);
        auto value = utf::to_lower(utf8.str());
        if (value == "1"
         || value == "on"
         || value == "yes"
         || value == "true")
        {
            return true;
        }
        else if (value == "0"
              || value == "off"
              || value == "no"
              || value == "faux"
              || value == "false")
        {
            return faux;
        }
        else return std::nullopt;
    }
    template<>
    auto take<twod>(qiew utf8) -> std::optional<twod>
    {
        utf::trim_front(utf8, " ({[\"\'");
        if (utf8)
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
    auto take<dent>(qiew utf8) -> std::optional<dent>
    {
        utf::trim_front(utf8, " ({[\"\'");
        if (utf8)
        if (auto l = utf::to_int(utf8))
        {
            utf::trim_front(utf8, " ,.x/:;");
            if (auto r = utf::to_int(utf8))
            {
                utf::trim_front(utf8, " ,.x/:;");
                if (auto t = utf::to_int(utf8))
                {
                    utf::trim_front(utf8, " ,.x/:;");
                    if (auto b = utf::to_int(utf8))
                    {
                        return dent{ l.value(), r.value(), t.value(), b.value() };
                    }
                    else return dent{ l.value(), r.value(), t.value() };
                }
                else return dent{ l.value(), r.value() };
            }
            else return dent{ l.value() };
        }
        return std::nullopt;
    }
    template<>
    auto take<span>(qiew utf8) -> std::optional<span>
    {
        using namespace std::chrono;
        utf::trim_front(utf8, " ({[\"\'");
        if (utf8)
        if (auto x = utf::to_int(utf8))
        {
            auto v = x.value();
            auto p = span{};
                 if (utf8.empty()
                  || utf8.starts_with("ms" )) return span{ milliseconds{ v } };
            else if (utf8.starts_with("us" )) return span{ microseconds{ v } };
            else if (utf8.starts_with("ns" )) return span{  nanoseconds{ v } };
            else if (utf8.starts_with("s"  )) return span{      seconds{ v } };
            else if (utf8.starts_with("min")) return span{      minutes{ v } };
            else if (utf8.starts_with("h"  )) return span{        hours{ v } };
            else if (utf8.starts_with("d"  )) return span{         days{ v } };
            else if (utf8.starts_with("w"  )) return span{        weeks{ v } };
        }
        return std::nullopt;
    }
    template<>
    auto take<argb>(qiew utf8) -> std::optional<argb>
    {
        if (!utf8) return std::nullopt;
        auto tobyte = [](auto c)
        {
                 if (c >= '0' && c <= '9') return (byte)(c - '0');
            else if (c >= 'a' && c <= 'f') return (byte)(c - 'a' + 10);
            else                           return (byte)(0);
        };
        auto value = utf::to_lower(utf8.str());
        auto result = argb{};
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
            //log("%%Unknown hex color format: { %value% }, expected #rrggbbaa or #rrggbb color hex value", prompt::xml, value);
        }
        else if (shadow.starts_with("0x")) // hex: 0xaarrggbb
        {
            shadow.remove_prefix(2);
            if (shadow.size() >= 8) // hex: 0xaarrggbb
            {
                result.chan.a = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                result.chan.r = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                result.chan.g = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                result.chan.b = (tobyte(shadow[6]) << 4) + tobyte(shadow[7]);
                return result;
            }
            else if (shadow.size() >= 6) // hex: 0xrrggbb
            {
                result.chan.a = 0xff;
                result.chan.r = (tobyte(shadow[0]) << 4) + tobyte(shadow[1]);
                result.chan.g = (tobyte(shadow[2]) << 4) + tobyte(shadow[3]);
                result.chan.b = (tobyte(shadow[4]) << 4) + tobyte(shadow[5]);
                return result;
            }
            //log("%%Unknown hex color format: { %value% }, expected 0xaarrggbb or 0xrrggbb color hex value", prompt::xml, value);
        }
        else if (utf::check_any(shadow, ",;/")) // dec: 000,000,000,000
        {
            if (auto r = utf::to_int(shadow))
            {
                result.chan.r = (byte)r.value();
                utf::trim_front(shadow, ",./:;");
                if (auto g = utf::to_int(shadow))
                {
                    result.chan.g = (byte)g.value();
                    utf::trim_front(shadow, ",./:;");
                    if (auto b = utf::to_int(shadow))
                    {
                        result.chan.b = (byte)b.value();
                        utf::trim_front(shadow, ",./:;");
                        if (auto a = utf::to_int(shadow)) result.chan.a = (byte)a.value();
                        else                              result.chan.a = 0xff;
                        return result;
                    }
                }
            }
            //log("%%Unknown hex color format: { %value% }, expected 000,000,000,000 decimal (r,g,b,a) color value", prompt::xml, value);
        }
        else if (auto c = utf::to_int(shadow)) // Single ANSI color value
        {
            if (c.value() >=0 && c.value() <=255)
            {
                result = argb::vt256[c.value()];
                return result;
            }
            //log("%%Unknown ANSI 256-color value format: { %value% }, expected 0-255 decimal value", prompt::xml, value);
        }
        return std::nullopt;
    }
    template<class T>
    auto take_or(qiew utf8, T fallback)
    {
        if (auto v = take<T>(utf8))
        {
            return v.value();
        }
        else
        {
            return fallback;
        }
    }

    auto operator - (view a, view b)
    {
        return a.substr(0, a.size() - b.size());
    }

    struct document
    {
        enum class type
        {
            na,            // Start of file
            eof,           // End of file
            eol,           // End of line
            top_token,     // Open tag name
            end_token,     // Close tag name
            token,         // Tag name
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
            value_begin,   //         ex: Value begin marker (right after equal)
            value_end,     //         ex: Value end marker (right after value)
            new_list,      // '*'     ex: name*
            lua_op_shl,    // '<<'    ex: Lua's shift left operator
            lua_op_less,   // '< '    ex: Lua's less than operator
            lua_op_less_eq,// '<='    ex: Lua's less than or equal operator
            compact,       // '/[^>]' ex: Compact syntax: <name/nested_block1/nested_block2=value param=value />
            spaces,        // ' '     ex: \s\t\r\n...
            unknown,       //
            tag_value,     // Quoted value.               ex: object="value"
            tag_numvalue,  // Value begins with digit.    ex: object=123ms
            tag_reference, // Non-quoted value.           ex: object=reference/to/value
            tag_joiner,    // Value joiner.               ex: object="value" | reference
            raw_reference, // Reference from outside.     ex: <object> "" | reference </object>
            raw_quoted,    // Quoted text from outside.   ex: <object> "quoted text" | reference </object>
            error,         // Inline error message.
        };

        struct literal;
        using fptr = netxs::sptr<literal>;
        using heap = std::vector<fptr>;

        struct literal
        {
            using wptr = netxs::wptr<literal>;

            wptr prev; // literal: Pointer to the prev.
            fptr next; // literal: Pointer to the next.
            bool busy; // literal: Reference loop detector mark.
            text utf8; // literal: Content data.
            type kind; // literal: Content type.

            literal(type kind, view utf8 = {})
                : busy{      },
                  utf8{ utf8 },
                  kind{ kind }
            { }
        };

        struct suit
        {
            fptr data; // suit: Linked list start.
            bool fail; // suit: Broken format.
            text file; // suit: Data source name.
            fptr back; // suit: Linked list end.

            suit(suit&&) = default;
            suit(view file = {})
                : data{ ptr::shared<literal>(type::na) },
                  fail{ faux },
                  file{ file },
                  back{ data }
            { }

            void init(view filename = {})
            {
                data = ptr::shared<literal>(type::na);
                fail = faux;
                file = filename;
                back = data;
            }
            void _append(type kind, view utf8)
            {
                auto frag = ptr::shared<literal>(kind, utf8);
                frag->prev = back;
                back->next = frag;
                back = frag;
            }
            auto append(type kind, view utf8 = {})
            {
                _append(kind, utf8);
                return back;
            }
            void append_if_nonempty(type kind, view utf8)
            {
                if (utf8.size())
                {
                    _append(kind, utf8);
                }
            }
            void clear_between(fptr begin, fptr end)
            {
                end->prev = begin;
                begin->next = end;
            }
            void insert_between(fptr begin, fptr end, heap& frag_ptr_list)
            {
                //todo optimize
                end->prev = begin;
                begin->next = end;
                auto insertion_point = begin;
                for (auto& frag_ptr : frag_ptr_list)
                {
                    auto& kind = frag_ptr->kind;
                    auto& utf8 = frag_ptr->utf8;
                    auto frag = ptr::shared<literal>(kind, utf8);
                    auto prev_next = insertion_point->next;
                    insertion_point->next = frag;
                    frag->prev = insertion_point;
                    frag->next = prev_next;
                    insertion_point = frag;
                }
            }
            auto lines()
            {
                auto count = 0_sz;
                auto next = data;
                while (next)
                {
                    auto& utf8 = next->utf8;
                    count += std::count(utf8.begin(), utf8.end(), '\n');
                    next = next->next;
                }
                return std::max(1_sz, count);
            }
            auto utf8()
            {
                auto crop = text{};
                auto next = data;
                while (next)
                {
                    crop+= next->utf8;
                    next = next->next;
                }
                return crop;
            }
            auto show()
            {
                static constexpr auto top_token_fg = argb{ 0xFF'99'd7'ff };
                static constexpr auto end_token_fg = argb{ 0xFF'6a'96'b3 };
                static constexpr auto token_fg     = argb{ 0xFF'83'b8'da };
                static constexpr auto liter_fg     = argb{ 0xFF'80'80'80 };
                static constexpr auto comment_fg   = argb{ 0xFF'4e'4e'4e };
                static constexpr auto new_list_fg  = argb{ 0xFF'9e'9e'9e };
                static constexpr auto quotes_fg    = argb{ 0xFF'BB'BB'BB };
                static constexpr auto value_fg     = argb{ 0xFF'90'96'f0 };
                static constexpr auto value_bg     = argb{ 0xFF'20'20'20 };

                //test
                //auto tmp = page.data.front().upto;
                //auto clr = 0;

                auto yield = ansi::escx{};
                auto next = data;
                while (next)
                {
                    auto  prev = next;
                    auto& utf8 = prev->utf8;
                    auto  kind = prev->kind;
                    next = next->next;

                    //test
                    //if (prev->upto == page.data.end() || tmp != prev->upto)
                    //{
                    //    clr++;
                    //    tmp = prev->upto;
                    //}

                    auto fgc = argb{};
                    auto bgc = argb{};
                    auto und = faux;
                    switch (kind)
                    {
                        case type::eof:           fgc = redlt;        break;
                        case type::top_token:     fgc = top_token_fg; break;
                        case type::end_token:     fgc = end_token_fg; break;
                        case type::compact:       fgc = end_token_fg; break;
                        case type::token:         fgc = token_fg;     break;
                        case type::comment_begin: fgc = comment_fg;   break;
                        case type::comment_close: fgc = comment_fg;   break;
                        case type::begin_tag:     fgc = liter_fg;     break;
                        case type::close_tag:     fgc = liter_fg;     break;
                        case type::close_inline:  fgc = liter_fg;     break;
                        case type::empty_tag:     fgc = liter_fg;     break;
                        case type::equal:         fgc = liter_fg;     break;
                        case type::quotes:        fgc = quotes_fg;    break;
                        case type::new_list:      fgc = new_list_fg;  break;
                        case type::unknown:       fgc = redlt;        break;
                        case type::tag_joiner:    fgc = liter_fg;     break;
                        case type::tag_reference: fgc = end_token_fg; und = true; break;
                        case type::raw_reference: fgc = end_token_fg; und = true;  break;
                        case type::raw_text:      fgc = value_fg;     break;
                        case type::quoted_text:
                        case type::raw_quoted:
                        case type::tag_numvalue:
                        case type::tag_value:     fgc = value_fg;
                                                  bgc = value_bg;     break;
                        case type::error:         fgc = whitelt;
                                                  bgc = reddk;
                                                  yield += ' ';       break;
                        default: break;
                    }
                    //test
                    //yield.bgc((tint)(clr % 8));

                    if (utf8.size())
                    {
                        if (und)
                        {
                                 if (bgc) yield.fgc(fgc).bgc(bgc).und(true).add(utf8).nil();
                            else if (fgc) yield.fgc(fgc)         .und(true).add(utf8).nil();
                            else          yield                  .und(true).add(utf8).nil();
                        }
                        else
                        {
                                 if (bgc) yield.fgc(fgc).bgc(bgc).add(utf8).nil();
                            else if (fgc) yield.fgc(fgc)         .add(utf8).nil();
                            else          yield                  .add(utf8);
                        }
                    }
                }

                auto count = 1;
                auto width = 0_sz;
                auto total = lines();
                while (total)
                {
                    total /= 10;
                    width++;
                }
                auto numerate = [&]
                {
                    return ansi::pushsgr().fgc(liter_fg) + utf::adjust(std::to_string(count++), width, ' ', true) + ": " + ansi::popsgr();
                };
                yield = numerate() + yield;
                utf::for_each(yield, "\n", [&]{ return "\n" + numerate(); });
                yield.add('\n');
                return yield;
            }
        };

        struct elem;
        using sptr = netxs::sptr<elem>;
        using wptr = netxs::wptr<elem>;
        using vect = std::vector<sptr>;
        using subs = utf::unordered_map<text, vect>;

        struct elem
        {
            enum form
            {
                node,
                attr,
                flat,
                pact, // Element has compact form (<element/elem2/elem3 ... />).
            };

            fptr from; // elem: Pointer to the begging of the semantic block.
            fptr name; // elem: Tag name.
            fptr insA; // elem: Insertion point for inline subelements.
            fptr insB; // elem: Insertion point for nested subelements.
            fptr upto; // elem: Pointer to the end of the semantic block.
            fptr vbeg; // elem: Value semantic block begin.
            fptr vend; // elem: Value semantic block end.
            heap body; // elem: Value fragments.
            subs hive; // elem: Subelements.
            bool base; // elem: Merge overwrite priority (new list if true).
            form mode; // elem: Element storage form.
            wptr boss; // elem: Parent context.

            elem()
                : base{ faux },
                  mode{ node }
            { }
           ~elem()
            {
                hive.clear();
                vbeg.reset();
                vend.reset();
                if (auto prev = from->prev.lock())
                {
                    auto next = upto->next;
                              prev->next = next;
                    if (next) next->prev = prev;
                }
            }

            auto get_parent_ptr()
            {
                return boss.lock();
            }
            auto is_quoted()
            {
                if (body.size() == 1)
                {
                    auto& value_placeholder = body.front();
                    if (value_placeholder->kind == type::tag_value) // equal [spaces] quotes tag_value quotes
                    if (auto quote_placeholder = value_placeholder->prev.lock())
                    if (quote_placeholder->kind == type::quotes && quote_placeholder->utf8.size())
                    {
                        auto c = quote_placeholder->utf8.front();
                        return c == '\"' || c == '\'';
                    }
                }
                return faux;
            }
            template<bool WithTemplate = faux>
            auto get_list3(qiew path_str, vect& crop)
            {
                utf::trim(path_str, '/');
                auto anchor = this;
                auto temp = text{};
                auto path = utf::split(path_str, '/');
                if (path.size())
                {
                    auto head = path.begin();
                    auto tail = path.end();
                    while (head != tail)
                    {
                        temp = *head++;
                        if (auto iter = anchor->hive.find(temp);
                                 iter!= anchor->hive.end())
                        {
                            auto& i = iter->second;
                            crop.reserve(i.size());
                            if (head == tail)
                            {
                                for (auto& item : i)
                                {
                                    if constexpr (WithTemplate) crop.push_back(item);
                                    else       if (!item->base) crop.push_back(item);
                                }
                            }
                            else if (i.size() && i.front())
                            {
                                anchor = &(*(i.front()));
                            }
                            else break;
                        }
                        else break;
                    }
                }
            }
            auto _concat_values()
            {
                auto value = text{};
                for (auto& value_placeholder : body)
                {
                    value += value_placeholder->utf8;
                }
                utf::unescape(value);
                return value;
            }
            auto _unsync_body(elem& item)
            {
                return body.size() != item.body.size()
                    || !std::ranges::equal(body, item.body, [](auto& s, auto& d){ return s->utf8 == d->utf8; });
                    //|| !std::equal(body.begin(), body.end(), item.body.begin(), [&](auto& s, auto& d){ return s->utf8 == d->utf8; });
            }
            void sync_value(elem& item)
            {
                if (item.body.size() && _unsync_body(item)) // An empty incoming body does nothing.
                {
                    auto dst_begin = vbeg;
                    auto dst_end = vend;
                    auto src_begin = item.vbeg;
                    auto src_end = item.vend;
                    auto insertion_point = dst_begin;
                    auto item_iterator = src_begin;
                    // Clear dst frags.
                    dst_end->prev = dst_begin;
                    dst_begin->next = dst_end;
                    body.clear();
                    // Copy src frags to dst.
                    while (item_iterator != src_end)
                    {
                        item_iterator = item_iterator->next;
                        auto& kind = item_iterator->kind;
                        auto& utf8 = item_iterator->utf8;
                        auto frag_ptr = ptr::shared<literal>(kind, utf8);
                        if (kind == type::tag_reference
                         || kind == type::quoted_text
                         || kind == type::tag_numvalue)
                        {
                            body.push_back(frag_ptr);
                        }
                        auto prev_next = insertion_point->next;
                        insertion_point->next = frag_ptr;
                        frag_ptr->prev = insertion_point;
                        frag_ptr->next = prev_next;
                        insertion_point = frag_ptr;
                    }
                    //todo inject raw literals
                    for (auto& frag_ptr : item.body)
                    {
                        auto& kind = frag_ptr->kind;
                        if (kind == type::raw_reference
                         || kind == type::raw_quoted
                         || kind == type::raw_text
                         || kind == type::unknown)
                        {
                            auto& utf8 = frag_ptr->utf8;
                            auto copy_frag_ptr = ptr::shared<literal>(kind, utf8);
                            body.push_back(copy_frag_ptr);
                        }
                    }
                    //todo sync suit
                    //
                    // inline:
                    //vbeg
                    //type::tag_reference
                    //type::quoted_text
                    //type::tag_numvalue
                    //vend
                    //
                    // outside:
                    //insB
                    //type::raw_reference
                    //type::raw_quoted
                    //type::raw_text
                    //type::unknown
                }
            }
            auto snapshot()
            {
                auto crop = text{};
                auto head = from;
                while (head)
                {
                    crop += head->utf8;
                    if (head == upto) break;
                    head = head->next;
                }
                if (crop.starts_with('\n')
                 || crop.starts_with('\r'))
                {
                    auto temp = view{ crop };
                    auto dent = text{ utf::pop_front_chars(temp, whitespaces) };
                    crop = temp;
                    utf::replace_all(crop, dent, "\n");
                }
                return crop;
            }
        };

        static constexpr auto find_start          = "<"sv;
        static constexpr auto rawtext_delims      = std::tuple{ " "sv, "/>"sv, ">"sv, "<"sv, "\n"sv, "\r"sv, "\t"sv };
        static constexpr auto reference_delims    = std::tuple_cat(rawtext_delims, std::tuple{ "|"sv, "\'"sv, "\""sv, "="sv });
        static constexpr auto token_delims        = " \t\n\r=*/><"sv;
        static constexpr auto view_comment_begin  = "<!--"sv;
        static constexpr auto view_comment_close  = "-->"sv;
        static constexpr auto view_close_tag      = "</"sv;
        static constexpr auto view_begin_tag      = "<"sv;
        static constexpr auto view_empty_tag      = "/>"sv;
        static constexpr auto view_slash          = "/"sv;
        static constexpr auto view_compact        = "/"sv;
        static constexpr auto view_close_inline   = ">"sv;
        static constexpr auto view_quoted_text    = "\""sv;
        static constexpr auto view_quoted_text_2  = "\'"sv;
        static constexpr auto view_equal          = "="sv;
        static constexpr auto view_new_list       = "*"sv;
        static constexpr auto view_lua_op_shl     = "<<"sv;
        static constexpr auto view_lua_op_less    = "< "sv;
        static constexpr auto view_lua_op_less_eq = "<="sv;
        static constexpr auto view_tag_joiner     = "|"sv;

        suit page;
        sptr root;

        document() = default;
        document(document&&) = default;
        document(view data, view file = {})
            : page{ file },
              root{ ptr::shared<elem>()}
        {
            read(data);
        }
        operator bool () const { return root ? !root->hive.empty() : faux; }

        void load(view data, view file = {})
        {
            page.init(file);
            root = ptr::shared<elem>();
            read(data);
        }
        template<bool WithTemplate = faux>
        auto take_ptr_list(view path)
        {
            auto item_ptr_list = vect{};
            if (root)
            {
                utf::trim(path, '/');
                if (path.empty())
                {
                    item_ptr_list.push_back(root);
                }
                else
                {
                    root->get_list3<WithTemplate>(path, item_ptr_list);
                }
            }
            return item_ptr_list;
        }
        auto join(view path, vect const& list)
        {
            utf::trim(path, '/');
            auto [parent_path, branch_path] = utf::split_back(path, '/');
            auto dest_hosts = take_ptr_list(parent_path);
            auto parent_ptr = dest_hosts.size() ? dest_hosts.front() : root;
            if (parent_ptr->mode == elem::form::pact)
            {
                log("%%Destination path is not suitable for merging '%parent_path%'", prompt::xml, parent_path);
                return;
            }
            auto& hive = parent_ptr->hive;
            auto iter = hive.find(branch_path);
            if (iter == hive.end())
            {
                iter = hive.emplace(branch_path , vect{}).first;
            }
            auto& dest_list = iter->second;
            if (dest_list.size() && dest_list.front()->base == faux) // Start a new list if the existing list was not declared as a list using an asterisk.
            {
                dest_list.clear();
            }
            for (auto& item_ptr : list) if (item_ptr && item_ptr->name->utf8 == branch_path)
            {
                //todo unify
                if (item_ptr->base)
                {
                    dest_list.clear();
                }
                auto mode = item_ptr->mode;
                auto from = item_ptr->from;
                auto upto = item_ptr->upto;
                auto next = upto->next;
                if (auto gate = mode == elem::form::attr ? parent_ptr->insA : parent_ptr->insB)
                if (auto prev = gate->prev.lock())
                if (auto past = from->prev.lock())
                {
                    from->prev = prev;
                    upto->next = gate;
                    gate->prev = upto;
                    prev->next = from;
                    past->next = next;  // Release an element from the previous list.
                    if (next) next->prev = past;
                    item_ptr->boss = parent_ptr;
                    dest_list.push_back(item_ptr);
                    if (mode != elem::form::attr) // Prepend '\n    <' to item when inserting it to gate==insB.
                    {
                        if (from->utf8.empty()) // Checking indent. Take indent from parent + pads if it is absent.
                        {
                            if (parent_ptr->from->utf8.empty()) // Most likely this is the root namespace.
                            {
                                from->utf8 = "\n";
                            }
                            else // Ordinary nested item.
                            {
                                from->utf8 = parent_ptr->from->utf8 + "    ";
                            }
                        }
                        //todo revise
                        if (from->next && from->next->kind == type::begin_tag) // Checking begin_tag.
                        {
                            auto shadow = view{ from->next->utf8 };
                            if (utf::pop_front_chars(shadow, whitespaces).empty()) // Set it to '<' if it is absent.
                            {
                                from->next->utf8 = "<";
                            }
                        }
                    }
                    continue;
                }
                log("%%Unexpected format for item '%parent_path%/%item->name->utf8%'", prompt::xml, parent_path, item_ptr->name->utf8);
            }
        }
        // xml: Attach the item list to the specified path.
        void attach(view mount_point, vect const& sub_list)
        {
            auto dest_list = take_ptr_list(mount_point);
            if (dest_list.size())
            {
                auto& parent_ptr = dest_list.front();
                if (parent_ptr->mode == elem::form::pact)
                {
                    log("%%Destination path is not suitable for merging '%parent_path%'", prompt::xml, mount_point);
                    return;
                }
                auto& parent_hive = parent_ptr->hive;
                auto connect = [&](auto& subitem_name)
                {
                    auto iter = parent_hive.find(subitem_name);
                    if (iter == parent_hive.end())
                    {
                        iter = parent_hive.emplace(subitem_name, vect{}).first;
                    }
                    return iter;
                };
                auto iter = connect(sub_list.front()->name->utf8);
                for (auto& item_ptr : sub_list)
                {
                    auto& current_item_name = iter->first;
                    auto& subitem_name = sub_list.front()->name->utf8;
                    if (current_item_name != subitem_name) // The case when the list is heterogeneous.
                    {
                        iter = connect(subitem_name);
                    }
                    //todo unify
                    auto& dest_list2 = iter->second;
                    if (item_ptr->base)
                    {
                        dest_list2.clear();
                    }
                    auto mode = item_ptr->mode;
                    auto from = item_ptr->from;
                    auto upto = item_ptr->upto;
                    auto next = upto->next;
                    if (auto gate = mode == elem::form::attr ? parent_ptr->insA : parent_ptr->insB)
                    if (auto prev = gate->prev.lock())
                    if (auto past = from->prev.lock())
                    {
                        from->prev = prev;
                        upto->next = gate;
                        gate->prev = upto;
                        prev->next = from;
                        past->next = next;  // Release an element from the previous list.
                        if (next) next->prev = past;
                        item_ptr->boss = parent_ptr;
                        dest_list2.push_back(item_ptr);
                        continue;
                    }
                    log("%%Unexpected format for item '%mount_point%%item%'", prompt::xml, mount_point, item_ptr->name->utf8);
                }
            }
            else
            {
                log("%%Destination path not found '%mount_point%'", prompt::xml, mount_point);
            }
        }
        void overlay(sptr item_ptr, text path = {})
        {
            auto& item = *item_ptr;
            auto& name = item.name->utf8;
            path += "/" + name;
            auto dest_list = take_ptr_list<true>(path);
            auto is_dest_list = (dest_list.size() && dest_list.front()->base) || dest_list.size() > 1;
            if (is_dest_list || dest_list.empty())
            {
                join(path, { item_ptr });
            }
            else
            {
                auto& dest_ptr = dest_list.front();
                dest_ptr->sync_value(item);
                for (auto& [sub_name, sub_list] : item.hive) // Proceed subelements.
                {
                    auto count = sub_list.size();
                    if (count == 1 && sub_list.front()->base == faux)
                    {
                        overlay(sub_list.front(), path);
                    }
                    else if (count) // It is a list.
                    {
                        join(path + "/" + sub_name, sub_list);
                    }
                    else
                    {
                        log("%%Unexpected tag without data: %tag%", prompt::xml, sub_name);
                    }
                }
            }
        }

    private:
        vect compacted;
        void fail(text msg)
        {
            page.fail = true;
            page.append(type::error, msg);
            log("%%%msg% at %page.file%:%lines%", prompt::xml, msg, page.file, page.lines());
        }
        void fail(type last, type what)
        {
            auto str = [&](type what)
            {
                switch (what)
                {
                    case type::na:              return view{ "{START}" }    ;
                    case type::eof:             return view{ "{EOF}" }      ;
                    case type::eol:             return view{ "{EOL}" }      ;
                    case type::token:           return view{ "{token}" }    ;
                    case type::raw_text:        return view{ "{raw text}" } ;
                    case type::compact:         return view{ "{compact}" }  ;
                    case type::tag_reference:   return view{ "{reference}" };
                    case type::raw_reference:   return view{ "{reference}" };
                    case type::tag_value:       return view{ "{value}" }    ;
                    case type::quoted_text:     return view_quoted_text     ;
                    case type::raw_quoted:      return view_quoted_text     ;
                    case type::begin_tag:       return view_begin_tag       ;
                    case type::close_tag:       return view_close_tag       ;
                    case type::comment_begin:   return view_comment_begin   ;
                    case type::comment_close:   return view_comment_close   ;
                    case type::close_inline:    return view_close_inline    ;
                    case type::empty_tag:       return view_empty_tag       ;
                    case type::equal:           return view_equal           ;
                    case type::new_list:        return view_new_list        ;
                    case type::lua_op_shl:      return view_lua_op_shl      ;
                    case type::lua_op_less:     return view_lua_op_less     ;
                    case type::lua_op_less_eq:  return view_lua_op_less_eq  ;
                    default:                    return view{ "{unknown}" }  ;
                }
            };
            fail(ansi::add("Unexpected '", str(what), "' after '", str(last), "'"));
        }
        auto peek(view& data, type& what, type& last)
        {
            last = what;
            if (data.empty()) what = type::eof;
            else if (data.starts_with(view_comment_begin)) what = type::comment_begin;
            else if (last == type::na && data.starts_with(view_begin_tag))
            {
                if (data.starts_with(view_close_tag)) what = type::close_tag;
                else                                  what = type::begin_tag;
            }
            else if (data.starts_with(view_close_tag    )) what = type::close_tag;
            else if (data.starts_with(view_begin_tag    )) what = type::begin_tag;
            else if (data.starts_with(view_empty_tag    )) what = type::empty_tag;
            else if (data.starts_with(view_close_inline )) what = type::close_inline;
            else if (data.starts_with(view_slash        ))
            {
                if (last == type::token) what = type::compact;
                else                     what = type::raw_text;
            }
            else if (data.starts_with(view_quoted_text  )
                  || data.starts_with(view_quoted_text_2)) what = type::quoted_text;
            else if (data.starts_with(view_equal        )) what = type::equal;
            else if (data.starts_with(view_tag_joiner   )
                  && (last == type::quoted_text
                   || last == type::tag_value
                   || last == type::tag_reference))        what = type::tag_joiner;
            else if (data.starts_with(view_new_list     )
                  && last == type::token)                  what = type::new_list;
            else if (whitespaces.find(data.front()) != view::npos) what = type::spaces;
            else if (last == type::close_tag
                  || last == type::begin_tag
                  || last == type::token
                  || last == type::new_list
                  || last == type::raw_text
                  || last == type::tag_value
                  || last == type::tag_reference
                  || last == type::compact
                  || last == type::quoted_text) what = type::token;
            else                                what = type::raw_text;
        }
        auto skip(view& data, type kind)
        {
            auto temp = data;
            switch (kind)
            {
                case type::comment_begin: data.remove_prefix(view_comment_begin.size()); break;
                case type::comment_close: data.remove_prefix(view_comment_close.size()); break;
                case type::close_tag:     data.remove_prefix(view_close_tag    .size()); break;
                case type::begin_tag:     data.remove_prefix(view_begin_tag    .size()); break;
                case type::empty_tag:     data.remove_prefix(view_empty_tag    .size()); break;
                case type::close_inline:  data.remove_prefix(view_close_inline .size()); break;
                case type::quoted_text:   data.remove_prefix(view_quoted_text  .size()); break;
                case type::equal:         data.remove_prefix(view_equal        .size()); break;
                case type::new_list:      data.remove_prefix(view_new_list     .size()); break;
                case type::tag_joiner:    data.remove_prefix(view_tag_joiner   .size()); break;
                case type::compact:       data.remove_prefix(view_compact      .size()); break;
                case type::token:
                case type::top_token:
                case type::end_token:     utf::eat_tail(data, token_delims); break;
                case type::raw_text:      utf::take_front(data, rawtext_delims); break;
                case type::tag_numvalue:
                case type::tag_reference: utf::take_front(data, reference_delims); break;
                case type::quotes:
                case type::tag_value:     utf::take_quote(data, data.front()); break;
                case type::spaces:        utf::trim_front(data, whitespaces); break;
                case type::na:            utf::take_front(data, find_start); break;
                case type::unknown:
                default:
                    data.remove_prefix(std::min(1, (si32)data.size()));
                    break;
            }
            return temp.substr(0, temp.size() - data.size());
        }
        auto take_pair(sptr& item_ptr, view& data, type& what, type& last, type kind)
        {
            item_ptr->name = page.append(            kind,         utf::take_front(data, token_delims));
                             page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
            peek(data, what, last);
            if (what == type::new_list)
            {
                item_ptr->base = true;
                page.append(            type::new_list, utf::pop_front(data, view_new_list.size()));
                page.append_if_nonempty(type::spaces,   utf::pop_front_chars(data, whitespaces));
                peek(data, what, last);
            }
            if (what == type::equal)
            {
                                 page.append(            type::equal,  utf::pop_front(data, view_equal.size()));
                                 page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
                item_ptr->vbeg = page.append(            type::value_begin);
                peek(data, what, last);
                auto not_empty = true;
                do
                {
                    if (what == type::quoted_text)
                    {
                        what = type::tag_value;
                        // #quoted_text
                        auto delim = data.front();
                        auto delim_view = view(&delim, 1);
                                        page.append(type::quotes, delim_view);
                        auto frag_ptr = page.append(type::quoted_text, utf::take_quote(data, delim));
                                        page.append(type::quotes, delim_view);
                        item_ptr->body.push_back(frag_ptr);
                    }
                    else if (what == type::raw_text) // Expected reference or number.
                    {
                        auto is_digit = netxs::onlydigits.find(data.front()) != text::npos;
                        what = is_digit ? type::tag_numvalue
                                        : type::tag_reference;
                        // #reference or number
                        auto frag_ptr = page.append(what, utf::take_front(data, reference_delims));
                        item_ptr->body.push_back(frag_ptr);
                    }
                    else
                    {
                        fail(last, what);
                        break;
                    }
                    page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
                    peek(data, what, last);
                    not_empty = what == type::tag_joiner;
                    if (not_empty) // Eat tag_joiner.
                    {
                        page.append(type::tag_joiner, utf::pop_front(data, view_tag_joiner.size()));
                        page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
                        peek(data, what, last);
                    }
                }
                while (not_empty);
                item_ptr->vend = page.append(type::value_end);
            }
            else if (what != type::compact) // Add placeholder for absent value.
            {
                                 page.append(type::equal);
                item_ptr->vbeg = page.append(type::value_begin);
                item_ptr->vend = page.append(type::value_end);
            }
        }
        auto open(sptr& item_ptr)
        {
            if (!page.data || page.back->kind != type::spaces)
            {
                page.append(type::spaces);
            }
            item_ptr->from = page.back;
        }
        auto seal(sptr& item_ptr)
        {
            item_ptr->upto = page.back;
        }
        auto note(view& data, type& what, type& last)
        {
            auto size = data.find(view_comment_close);
            if (size == view::npos)
            {
                page.append(type::unknown, data);
                data = {};
                last = what;
                what = type::eof;
                return faux;
            }
            size += view_comment_close.size();
            page.append(type::comment_begin, utf::pop_front(data, size));
            return true;
        }
        void push(sptr& item_ptr, sptr& nested_ptr)
        {
            auto& nested_name = nested_ptr->name->utf8;
            item_ptr->hive[nested_name].push_back(nested_ptr);
            nested_ptr->boss = item_ptr;
        }
        void read_subsections(sptr& item_ptr, view& data, type& what, type& last, si32& deep)
        {
            do
            {
                auto temp = data;
                utf::trim_front(temp, whitespaces);
                //auto p = std::vector{ std::tuple{ 0, what, last, temp }};
                peek(temp, what, last);
                while (what != type::close_tag && what != type::eof)
                {
                    //p.push_back(std::tuple{ 1, what, last, temp });
                    if (what == type::quoted_text)
                    {
                        page.append_if_nonempty(type::spaces, data - temp);
                        data = temp;
                        // #quoted_text
                        auto delim = data.front();
                        auto delim_view = view(&delim, 1);
                                        page.append(type::quotes, delim_view);
                        auto frag_ptr = page.append(type::raw_quoted, utf::take_quote(data, delim));
                                        page.append(type::quotes, delim_view);
                        item_ptr->body.push_back(frag_ptr);

                        page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
                        temp = data;
                    }
                    else if (what == type::tag_joiner)
                    {
                        page.append_if_nonempty(type::spaces, data - temp);
                        page.append(type::tag_joiner, utf::pop_front(temp, view_tag_joiner.size()));
                        page.append_if_nonempty(type::spaces, utf::pop_front_chars(temp, whitespaces));
                        data = temp;
                        peek(temp, what, last);
                        if (what == type::quoted_text)
                        {
                            continue;
                        }
                        auto is_reference = what == type::raw_text && netxs::onlydigits.find(temp.front()) == text::npos; // Only literal raw text is allowed as a reference name.
                        if (is_reference)
                        {
                            what = type::tag_reference;
                            // #reference
                            auto frag_ptr = page.append(type::raw_reference, utf::take_front(data, reference_delims));
                            item_ptr->body.push_back(frag_ptr);
                            page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
                            temp = data;
                        }
                        else
                        {
                            fail(last, what);
                            break;
                        }
                    }
                    else if (what == type::raw_text)
                    {
                        auto iter = utf::find_char_except_skips(temp, '<', view_lua_op_shl, view_lua_op_less, view_lua_op_less_eq);
                        if (iter != temp.end())
                        {
                            auto spaces_len = data.size() - temp.size();
                            auto rest_text_len = iter - temp.begin();
                            auto size = spaces_len + rest_text_len;
                            // #raw_text
                            auto frag_ptr = page.append(type::raw_text, utf::pop_front(data, size));
                            item_ptr->body.push_back(frag_ptr);
                            temp = data;
                        }
                        else // Unexpected end of data.
                        {
                            auto frag_ptr = page.append(type::unknown, data);
                            item_ptr->body.push_back(frag_ptr);
                            data = {};
                            last = what;
                            what = type::eof;
                            break;
                        }
                    }
                    else if (what == type::begin_tag && deep < 30)
                    {
                        page.append_if_nonempty(type::spaces, data - temp);
                        data = temp;
                        auto nested_ptr = ptr::shared<elem>();
                        what = read_node(nested_ptr, data, deep + 1);
                        push(item_ptr, nested_ptr);
                        temp = data;
                        utf::trim_front(temp, whitespaces);
                    }
                    else if (what == type::comment_begin) // Proceed '<!--'.
                    {
                        page.append_if_nonempty(type::spaces, data - temp);
                        data = temp;
                        auto size = data.find(view_comment_close);
                        if (size != view::npos)
                        {
                            size += view_comment_close.size();
                            page.append(type::comment_begin, utf::pop_front(data, size));
                            temp = data;
                            utf::trim_front(temp, whitespaces);
                        }
                        else // Unexpected end of data.
                        {
                            page.append(type::unknown, data);
                            data = {};
                            last = what;
                            what = type::eof;
                            break;
                        }
                    }
                    else // Unknown/unexpected data.
                    {
                        last = type::unknown;
                        fail(last, what);
                        skip(temp, what);
                        page.append(type::unknown, data - temp);
                        data = temp;
                    }
                    //p.push_back(std::tuple{ 2, what, last, temp });
                    peek(temp, what, last);
                }
                if (what == type::close_tag) // Proceed '</token>'.
                {
                    auto spaces_before_close_tag = page.append(type::spaces, data - temp);
                    auto close_tag = utf::pop_front(temp, view_close_tag.size());
                    auto trim_frag = utf::pop_front_chars(temp, whitespaces);
                    peek(temp, what, last);
                    if (what == type::token)
                    {
                        auto item_name = utf::take_front(temp, token_delims);
                        if (item_name == item_ptr->name->utf8)
                        {
                            item_ptr->insB = spaces_before_close_tag;
                            page.append(            type::close_tag, close_tag);
                            page.append_if_nonempty(type::spaces,    trim_frag);
                            page.append(            type::end_token, item_ptr->name->utf8);
                            page.append_if_nonempty(type::spaces, utf::pop_front_chars(temp, whitespaces));
                            page.append(            type::close_inline, utf::take_front_including<faux>(temp, view_close_inline));
                            data = temp;
                            break;
                        }
                        else
                        {
                            what = type::unknown;
                            page.append(            what, close_tag);
                            page.append_if_nonempty(what, trim_frag);
                            page.append(            what, item_name);
                            page.append(            what, utf::take_front<faux>(temp, view_close_inline));
                            data = temp;
                            fail(ansi::add("Unexpected closing tag name '", item_name, "', expected: '", item_ptr->name->utf8, "'"));
                            continue; // Repeat until eof or success.
                        }
                    }
                    else // Unexpected data.
                    {
                        page.append(type::unknown, data - temp);
                        data = temp;
                        fail(last, what);
                        continue; // Repeat until eof or success.
                    }
                }
                else if (what == type::eof)
                {
                    item_ptr->insB = page.append(type::spaces, utf::pop_front_chars(data, whitespaces));
                    if (deep != 0)
                    {
                        fail("Unexpected {EOF}");
                    }
                }
            }
            while (data.size());
        }
        auto read_node(sptr& item_ptr, view& data, si32 deep = {}) -> document::type
        {
            auto what = type::na;
            auto last = type::na;
            auto fire = faux;
            open(item_ptr);
            peek(data, what, last);
            if (what == type::begin_tag)
            {
                page.append(type::begin_tag, utf::pop_front(data, view_begin_tag.size()));
                // No spaces allowed between type::begin_tag and type::token
                peek(data, what, last);
                if (what == type::token)
                {
                    take_pair(item_ptr, data, what, last, type::top_token);
                    while (what == type::compact)
                    {
                        page.append(what, utf::pop_front(data, view_compact.size()));
                        item_ptr->mode = elem::form::pact;
                        auto next_ptr = ptr::shared<elem>();
                        open(next_ptr);
                        page.append(type::begin_tag); // Add begin_tag placeholder.
                        peek(data, what, last);
                        take_pair(next_ptr, data, what, last, type::top_token);
                        auto& sub_name = next_ptr->name->utf8;
                        item_ptr->hive[sub_name].push_back(next_ptr);
                        next_ptr->boss = item_ptr;
                        compacted.push_back(item_ptr);
                        item_ptr = next_ptr;
                    }
                    auto prev_last = last;
                    peek(data, what, last);
                    if (what == type::token)
                    {
                        do // Proceed inlined subs.
                        {
                            auto next_ptr = ptr::shared<elem>();
                            next_ptr->mode = elem::form::attr;
                            open(next_ptr);
                            take_pair(next_ptr, data, what, last, type::token);
                            seal(next_ptr);
                            push(item_ptr, next_ptr);
                            page.append_if_nonempty(type::spaces, utf::pop_front_chars(data, whitespaces));
                            peek(data, what, last);
                        }
                        while (what == type::token);
                    }
                    if (what == type::empty_tag) // Proceed '/>'.
                    {
                        item_ptr->mode = elem::form::flat;
                        item_ptr->insA = last == type::spaces ? page.back
                                                              : page.append(type::spaces);
                        last = type::spaces;
                        page.append(type::empty_tag, utf::pop_front(data, view_empty_tag.size()));
                        while (true) // Pull inline comments if it is: .../>  <!-- comments --> ... <!-- comments -->
                        {
                            auto temp = data;
                            auto idle = utf::pop_front_chars(temp, whitespaces);
                            auto w = what;
                            auto l = last;
                            peek(temp, w, l);
                            if (idle.find('\n') == text::npos && w == type::comment_begin)
                            {
                                data = temp;
                                what = w;
                                last = l;
                                page.append(type::spaces, idle);
                                if (note(data, what, last)) continue;
                            }
                            break;
                        }
                    }
                    else if (compacted.empty() && what == type::close_inline) // Proceed '>' nested subs.
                    {
                        item_ptr->insA = last == type::spaces ? page.back
                                                              : page.append(type::spaces);
                        page.append(type::close_inline, utf::pop_front(data, view_close_inline.size()));
                        read_subsections(item_ptr, data, what, last, deep);
                    }
                    else
                    {
                        fire = true;
                        last = prev_last;
                    }
                }
                else
                {
                    fire = true;
                }
            }
            else
            {
                fire = true;
            }
            if (!item_ptr->name)
            {
                auto head = page.back;
                while (true) // Reverse find a broken open tag and mark all after it as an unknown data.
                {
                    auto kind = head->kind;
                    head->kind = type::unknown;
                    if (head == page.data || kind == type::begin_tag) break;
                    head = head->prev.lock();
                }
                item_ptr->name = page.append(type::tag_value);
                fail("Empty tag name");
            }
            if (fire)
            {
                fail(last, what);
            }
            if (what == type::eof)
            {
                page.append(type::eof);
            }
            seal(item_ptr);
            while (!compacted.empty()) // Close compact nodes.
            {
                item_ptr = compacted.back();
                seal(item_ptr);
                compacted.pop_back();
            }
            return what;
        }
        void read(view& data)
        {
            auto what = type::na;
            auto last = type::na;
            auto deep = 0;
            open(root);
            root->mode = elem::form::node;
            root->name = page.append(type::na);
            root->insB = page.append(type::spaces);
            read_subsections(root, data, what, last, deep);
            seal(root);
            if (page.fail) log("%%Inconsistent xml data from %file%:\n%config%\n", prompt::xml, page.file.empty() ? "memory"sv : page.file, page.show());
        }
    };

    struct settings
    {
        using vect = xml::document::vect;
        using sptr = xml::document::sptr;
        using list = std::list<xml::document::sptr>;

        netxs::sptr<xml::document> document; // settings: XML document.
        vect tmpbuff; // settings: Temp buffer.
        list context; // settings: Current working context stack (reference context).

        settings() = default;
        settings(settings const&) = default;
        settings(view utf8_xml)
            : document{ ptr::shared<xml::document>(utf8_xml, "") }
        { }
        settings(xml::document& other)
            : document{ ptr::shared<xml::document>(std::move(other)) }
        { }

        sptr get_context()
        {
            auto context_path = context.size() ? context.back() : document->root;
            return context_path;
        }
        // settings: Push document context by name.
        auto push_context(sptr new_context_ptr)
        {
            struct pop_ctx
            {
                list&          context;
                list::iterator iterator;

                pop_ctx(list& context, list::iterator iterator)
                    : context{ context },
                      iterator{ iterator }
                { }
                pop_ctx(pop_ctx&& ctx)
                    : context{ ctx.context },
                      iterator{ std::exchange(ctx.iterator, context.end()) }
                { }
                void operator = (pop_ctx&& ctx)
                {
                    assert(&context == &ctx.context);
                    iterator = std::exchange(ctx.iterator, context.end());
                }
                ~pop_ctx()
                {
                    if (iterator != context.end())
                    {
                        context.erase(iterator);
                    }
                }
            };
            auto iterator = context.emplace(context.end(), new_context_ptr);
            return pop_ctx(context, iterator);
        }
        auto push_context(qiew context_path)
        {
            auto new_context_ptr = settings::_find_name<true>(context_path);
            return settings::push_context(new_context_ptr);
        }
        friend auto& operator << (std::ostream& s, settings const& p)
        {
            return s << p.document->page.show();
        }
        // settings: Lookup document context for item_ptr by its reference name path.
        void _find_namepath(view reference_namepath, sptr& item_ptr)
        {
            auto item_ptr_list = document->take_ptr_list<true>(reference_namepath);
            if (item_ptr_list.size())
            {
                item_ptr = item_ptr_list.back();
            }
        }
        // settings: Lookup document context for item_ptr by its reference name.
        template<bool WithTemplate = faux>
        sptr _find_name(view reference_path)
        {
            auto item_ptr = sptr{};
            auto namepath = text{};
            if (reference_path.empty() || reference_path.front() != '/') // Relative reference. Iterate over nested contexts.
            {
                auto item_ptr_list = vect{};
                auto context_ptr = settings::get_context();
                while (context_ptr)
                {
                    settings::_take_ptr_list_of(context_ptr, reference_path, item_ptr_list);
                    if (item_ptr_list.size() && item_ptr_list.front())
                    {
                        item_ptr = item_ptr_list.front();
                        break;
                    }
                    context_ptr = context_ptr->get_parent_ptr();
                }
                if (!context_ptr)
                {
                    log("%%Settings reference '%ref%' not found", prompt::xml, reference_path);
                }
            }
            else // Absolute reference.
            {
                settings::_find_namepath(reference_path, item_ptr);
            }
            return item_ptr;
        }
        void _take_value(sptr item_ptr, text& value)
        {
            for (auto& value_placeholder : item_ptr->body)
            {
                auto kind = value_placeholder->kind;
                if (kind == document::type::tag_reference
                 || kind == document::type::raw_reference)
                {
                    auto& reference_name = value_placeholder->utf8;
                    if (!value_placeholder->busy)
                    {
                        value_placeholder->busy = true;
                        if (auto base_item_ptr = settings::_find_name(reference_name))
                        {
                            settings::_take_value(base_item_ptr, value);
                        }
                        else
                        {
                            log("%%%red%Reference name '%ref%' not found%nil%", prompt::xml, ansi::fgc(redlt), reference_name, ansi::nil());
                        }
                        value_placeholder->busy = faux;
                    }
                    else
                    {
                        log("%%%red%Reference loop detected for '%ref%'%nil%", prompt::xml, ansi::fgc(redlt), reference_name, ansi::nil());
                    }
                }
                else // if (value_placeholder->kind != document::type::tag_joiner)
                {
                    value += value_placeholder->utf8;
                }
            }
        }
        text take_value(sptr item_ptr)
        {
            auto value = text{};
            settings::_take_value(item_ptr, value);
            utf::unescape(value);
            return value;
        }
        void _take_ptr_list_of(sptr subsection_ptr, view attribute, vect& item_ptr_list)
        {
            // Recursively take all base lists.
            for (auto& value_placeholder : subsection_ptr->body)
            {
                auto kind = value_placeholder->kind;
                if (kind == document::type::tag_reference
                 || kind == document::type::raw_reference)
                {
                    auto& reference_name = value_placeholder->utf8;
                    if (!value_placeholder->busy) // Silently ignore reference loops.
                    {
                        value_placeholder->busy = true;
                        if (auto base_ptr = settings::_find_name(reference_name)) // Lookup outside.
                        {
                            settings::_take_ptr_list_of(base_ptr, attribute, item_ptr_list);
                        }
                        value_placeholder->busy = faux;
                    }
                }
            }
            // Take native attribute list.
            subsection_ptr->get_list3(attribute, item_ptr_list);
        }
        auto take_ptr_list_of(sptr subsection_ptr, view attribute)
        {
            auto item_ptr_list = document::vect{};
            settings::_take_ptr_list_of(subsection_ptr, attribute, item_ptr_list);
            return item_ptr_list;
        }
        // settings: Get item_ptr list from the current context.
        auto take_ptr_list_for_name(view attribute)
        {
            auto item_ptr_list = document::vect{};
            auto absolute = attribute.size() && attribute.front() == '/';
            if (auto context_ptr = absolute ? settings::_find_name<true>(attribute) : settings::get_context())
            {
                settings::_take_ptr_list_of(context_ptr, attribute, item_ptr_list);
            }
            return item_ptr_list;
        }
        auto take_value_list_of(sptr subsection_ptr, view attribute)
        {
            auto strings = txts{};
            settings::_take_ptr_list_of(subsection_ptr, attribute, tmpbuff);
            strings.reserve(tmpbuff.size());
            for (auto attr_ptr : tmpbuff)
            {
                strings.emplace_back(settings::take_value(attr_ptr));
            }
            tmpbuff.clear();
            return strings;
        }
        template<bool Quiet = true, class T = si32>
        auto take_value_from(sptr subsection_ptr, view attribute, T defval = {})
        {
            auto crop = text{};
            settings::_take_ptr_list_of(subsection_ptr, attribute, tmpbuff);
            if (tmpbuff.size())
            {
                crop = settings::take_value(tmpbuff.back());
            }
            else
            {
                if constexpr (!Quiet)
                {
                    log("%%%red% xml path not found: %nil%%path%", prompt::xml, ansi::fgc(redlt), ansi::nil(), attribute);
                }
                return defval;
            }
            tmpbuff.clear();
            if constexpr (std::is_same_v<std::decay_t<T>, text>)
            {
                return crop;
            }
            else
            {
                if (auto result = xml::take<T>(crop))
                {
                    return result.value();
                }
                else
                {
                    return defval;
                }
            }
        }
        template<bool Quiet = true, class T = si32>
        auto take(text frompath, T defval = {})
        {
            auto absolute = frompath.size() && frompath.front() == '/';
            auto context_ptr = sptr{};
            if (absolute)
            {
                auto [parent_path, branch_path] = utf::split_back(utf::get_trimmed_back(frompath, '/'), '/');
                context_ptr = parent_path ? settings::_find_name<true>(parent_path) : settings::get_context();
                frompath = branch_path;
            }
            if (!context_ptr)
            {
                context_ptr = settings::get_context();
            }
            if (context_ptr)
            {
                auto ctx = settings::push_context(context_ptr);
                auto item_ptr_list = vect{};
                context_ptr->get_list3(frompath, item_ptr_list);
                //if (auto item_ptr = settings::_find_name(frompath))
                if (auto item_ptr = item_ptr_list.size() ? item_ptr_list.back() : sptr{})
                {
                    auto crop = settings::take_value(item_ptr);
                    if constexpr (std::is_same_v<std::decay_t<T>, text>)
                    {
                        return crop;
                    }
                    else
                    {
                        if (auto result = xml::take<T>(crop))
                        {
                            return result.value();
                        }
                        else
                        {
                            return defval;
                        }
                    }
                }
            }
            if constexpr (!Quiet)
            {
                log("%%%red% xml path not found: %nil%%path%", prompt::xml, ansi::fgc(redlt), ansi::nil(), frompath);
            }
            return defval;
        }
        template<bool Quiet = true, class T>
        auto take_value_from(sptr subsection_ptr, view attribute, T defval, utf::unordered_map<text, T> const& dict)
        {
            if (subsection_ptr)
            {
                auto crop = settings::take_value_from<Quiet>(subsection_ptr, attribute, ""s);
                if (crop.empty())
                {
                    if constexpr (!Quiet)
                    {
                        log("%%%red% xml path not found: %nil%%path%", prompt::xml, ansi::fgc(redlt), ansi::nil(), attribute);
                    }
                }
                else
                {
                    auto iter = dict.find(crop);
                    if (iter != dict.end())
                    {
                        return iter->second;
                    }
                }
            }
            return defval;
        }
        template<class T>
        auto take(text frompath, T defval, utf::unordered_map<text, T> const& dict)
        {
            if (frompath.size())
            {
                auto crop = settings::take(frompath, ""s);
                if (crop.empty())
                {
                    log("%%%red% xml path not found: %nil%%path%", prompt::xml, ansi::fgc(redlt), ansi::nil(), frompath);
                }
                else
                {
                    auto iter = dict.find(crop);
                    if (iter != dict.end())
                    {
                        return iter->second;
                    }
                }
            }
            return defval;
        }
        auto take(text frompath, cell defval)
        {
            if (frompath.empty()) return defval;
            auto fgc_path = frompath + '/' + "fgc";
            auto bgc_path = frompath + '/' + "bgc";
            auto itc_path = frompath + '/' + "itc";
            auto bld_path = frompath + '/' + "bld";
            auto und_path = frompath + '/' + "und";
            auto inv_path = frompath + '/' + "inv";
            auto ovr_path = frompath + '/' + "ovr";
            auto blk_path = frompath + '/' + "blk";
            auto txt_path = frompath + '/' + "txt";
            auto fba_path = frompath + '/' + "alpha";
            auto crop = cell{ defval.txt() };
            crop.fgc(settings::take(fgc_path, defval.fgc()));
            crop.bgc(settings::take(bgc_path, defval.bgc()));
            crop.itc(settings::take(itc_path, defval.itc()));
            crop.bld(settings::take(bld_path, defval.bld()));
            crop.und(settings::take(und_path, defval.und()));
            crop.inv(settings::take(inv_path, defval.inv()));
            crop.ovr(settings::take(ovr_path, defval.ovr()));
            crop.blk(settings::take(blk_path, defval.blk()));
            auto t = settings::take(txt_path, ""s);
            auto a = settings::take(fba_path, -1);
            if (t.size()) crop.txt(t);
            if (a != -1)  crop.alpha((byte)std::clamp(a, 0, 255));
            return crop;
        }
        auto utf8()
        {
            return document->page.utf8();
        }
        template<bool Print = faux>
        auto fuse(view utf8_xml, view filepath = {})
        {
            if (utf8_xml.empty()) return;
            if (filepath.size()) document->page.file = filepath;
            context.clear();
            auto tmp_config = xml::document{ utf8_xml, filepath };
            if constexpr (Print)
            {
                log("%%Settings from %file%:\n%config%", prompt::xml, filepath.empty() ? "memory"sv : filepath, tmp_config.page.show());
            }
            document->overlay(tmp_config.root);
        }
    };
    namespace options
    {
        static auto format = utf::unordered_map<text, si32>
           {{ "none",      mime::disabled },
            { "text",      mime::textonly },
            { "ansi",      mime::ansitext },
            { "rich",      mime::richtext },
            { "html",      mime::htmltext },
            { "protected", mime::safetext }};

        static auto cursor = utf::unordered_map<text, si32>
           {{ "underline",  text_cursor::underline },
            { "block",      text_cursor::block     },
            { "bar",        text_cursor::I_bar     },
            { "I_bar",      text_cursor::I_bar     }};

        static auto align = utf::unordered_map<text, bias>
           {{ "left",   bias::left   },
            { "right",  bias::right  },
            { "center", bias::center }};
    }
}
namespace netxs
{
    using settings = xml::settings;
}