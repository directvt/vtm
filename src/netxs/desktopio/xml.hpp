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
        enum type
        {
            na,            // Start of file
            eof,           // End of file
            eol,           // End of line
            top_token,     // Opening tag name
            end_token,     // Closing tag name
            token,         // Tag name
            raw_text,      //         ex: raw text
            quotes,        // '"'     ex: " or '
            quoted_text,   // '"'     ex: " text "
            begin_tag,     // '<'     ex: <name ...
            close_tag,     // '</'    ex: ... </name>
            comment,       // '<!--'  ex: ... <!-- ...
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
            insA,          // ''      ex: Attribute form insertion point.
            insB,          // ''      ex: Block form insertion point.
            unknown,       //
            tag_value,     // Quoted value.               ex: object="value"
            tag_numvalue,  // Value begins with digit.    ex: object=123ms
            tag_reference, // Non-quoted value.           ex: object=reference/to/value
            tag_joiner,    // Value joiner.               ex: object="value" | reference
            raw_reference, // Reference from outside.     ex: <object> "" | reference </object>
            raw_quoted,    // Quoted text from outside.   ex: <object> "quoted text" | reference </object>
            error,         // Inline error message.
        };

        // whitespaces ws     = [ \t\r\n\v\f]*                                                             // " \t\r\n\v\f"
        // asterisk           = [*]                                                                        // ASCII 0x2A '*'
        // equal              = [=]                                                                        // ASCII 0x3D '='
        // digits             = [0-9]                                                                      // ASCII decimal digits
        // markup             = [[:whitespaces:]!"#$%&'()*+,/;<=>?@\[\\\]^`{|}~]                           // the characters forbidden for name
        // name               = [^-.[:digits:][:markup:]][^[:markup:]]*                                    // alphanumeric literal begining with non-digit, '-'(minus) and '.'(period)
        // numeric_value      = [[:digits:]][^[:markup:]]*                                                 // alphanumeric literal begining with digit
        // double_quoted      = "(\\.|[^\\"])*"                                                            // "quoted'text"
        // single_quoted      = '(\\.|[^\\'])*'                                                            // 'quoted"text'
        // quoted_text        = double_quoted | single_quoted                                              // "quoted'text" or 'quoted"text'
        // reference          = /?([:name:])(/[:name:])*                                                   // path to the element, relative or absolute
        // single_value       = reference | quoted_text | numeric_value                                    // one of
        // union              = (whitespaces)* [|] (whitespaces)*                                          // element union operator
        // value              = single_value([:union:] single_value)*                                      // combined values
        // comment            = <!--.+?-->                                                                 // commentary block
        // comments           = ((whitespaces)* comment)*$                                                 // comments at the end of line excluding the LF character
        // name_value_pair    = (whitespaces)* name(asterisk)?((equal) value)?                             // name=value pair
        // raw_text           = (?<=[>])(.+?)*(?= <[:name:] | </[:name:] | <!--)                           // an arbitrary text between the closing and opening tags or comment block
        // content            = (ws "quoted_text" (union value)* | raw_text)*                              // element content
        // subelement         = name_value_pair                                                            // element in an attribute form
        // element_inline     = whitespaces <name_value_pair (subelement)* /> comments                     // element in an inline form
        // element_compact    = whitespaces <name(/name)*/name_value_pair (subelement)* /> comments        // nested elements in a compact form
        // element_block      = whitespaces <name_value_pair (subelement)* > comments                      // element in a block form
        //                         ((content* | element*) comments)*                                       // nested content or elements with following comments
        //                      whitespaces </name> comments                                               // closing tag
        // element            = element_inline | element_compact | element_block
        // document           = comments (element)*

        struct literal
        {
            text utf8; // literal: Content data.
            si32 kind; // literal: Content type.
            si32 busy; // literal: Reference loop detector mark.

            literal(literal&&) = default;
            literal(literal const&) = default;
            literal(si32 kind = type::na, view utf8 = {})
                : utf8{ utf8 },
                  kind{ kind },
                  busy{      }
            { }
        };

        using list = std::list<literal>;
        using f_it = list::iterator;
        using heap = std::vector<f_it>;

        struct suit
        {
            list frag_list; // suit: Fragment list.
            bool fail; // suit: Broken format.
            text file; // suit: Data source name.

            suit(suit&&) = default;
            suit(view file = {})
                : frag_list{ literal{ type::na } },
                  fail{ faux },
                  file{ file }
            { }

            void init(view filename = {})
            {
                frag_list.clear();
                frag_list.push_back(literal{ type::na });
                fail = faux;
                file = filename;
            }
            auto lines() const
            {
                auto count = 0_sz;
                for (auto& frag : frag_list)
                {
                    auto& utf8 = frag.utf8;
                    count += std::count(utf8.begin(), utf8.end(), '\n');
                }
                return std::max(1_sz, count);
            }
            auto utf8() const
            {
                auto crop = text{};
                auto size = arch{};
                for (auto& frag : frag_list)
                {
                    size += frag.utf8.size();
                }
                crop.reserve(size);
                for (auto& frag : frag_list)
                {
                    crop += frag.utf8;
                }
                return crop;
            }
            auto show() const
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
                //static constexpr auto control_fg   = argb{ 0xFF'00'00'00 };
                //static constexpr auto control_bg   = argb{ 0xFF'f0'f0'20 };

                //test
                //auto tmp = frag_list.front().upto;
                //auto clr = 0;

                auto yield = ansi::escx{};
                for (auto& frag : frag_list)
                {
                    auto& utf8 = frag.utf8;
                    auto& kind = frag.kind;
                    auto fgc = argb{};
                    auto bgc = argb{};
                    auto und = faux;
                    //auto ctrl = faux;
                    //test
                    //if (frag.upto == frag_list.end() || tmp != frag.upto)
                    //{
                    //    clr++;
                    //    tmp = frag.upto;
                    //}

                    switch (kind)
                    {
                        //case type::insB:
                        //case type::insA:          ctrl = true;
                        //                          fgc = control_fg;
                        //                          bgc = control_bg;   break;
                        case type::eof:           fgc = redlt;        break;
                        case type::top_token:     fgc = top_token_fg; break;
                        case type::end_token:     fgc = end_token_fg; break;
                        case type::compact:       fgc = end_token_fg; break;
                        case type::token:         fgc = token_fg;     break;
                        case type::comment:
                        case type::comment_begin:
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
                    //else
                    //{
                    //    if (ctrl)
                    //    {
                    //        yield.fgc(fgc).bgc(bgc).add(kind == type::insB ? "insB" : "insA").nil();
                    //    }
                    //}
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

            list& frag_list; // elem: Document fragment list.
            f_it from; // elem: Pointer to the beginning of the semantic block.
            f_it upto; // elem: Pointer to the end of the semantic block.
            f_it name; // elem: Pointer to the Tag name.
            f_it insA; // elem: Insertion point for inline subelements.
            f_it insB; // elem: Insertion point for nested subelements.
            std::vector<std::pair<f_it, f_it>> value_segments; // elem: List of the value fragment segments stored as pairs of iterators for begin (an empty opening frag) and end (an empty closing frag). The first segment is the value segment right after the equal sign.
            heap body; // elem: List of pointers to value fragments.
            subs hive; // elem: Map of Subelement lists.
            bool base; // elem: Merge overwrite priority (clear dest list on overlaying if true).
            form mode; // elem: Element storage form.
            wptr parent_wptr; // elem: Weak reference to the parent element.

            elem(list& frag_list)
                : frag_list{ frag_list },
                  from{ frag_list.end() },
                  upto{ frag_list.end() },
                  name{ frag_list.end() },
                  insA{ frag_list.end() },
                  insB{ frag_list.end() },
                  base{ faux },
                  mode{ node }
            { }

            auto open()
            {
                from = std::prev(frag_list.end());
            }
            auto seal()
            {
                upto = std::prev(frag_list.end());
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
                    if (value_segments.empty()) // It is possible the target is in a compact form.
                    {
                        // Don't restructurize the element instead of error reporting. The structure of the overlay configuration must be compatible with the destination configuration.
                        log("%%%err%There are no placeholders to store the value; it is possible the target is in a compact form%nil%", prompt::xml, ansi::err(), ansi::nil());
                        for (auto& frag_iter : item.body)
                        {
                            log("\tvalue: %value%", ansi::hi(frag_iter->utf8));
                        }
                        return;
                    }
                    for (auto& [dst_vbeg, dst_vend] : value_segments) // Clear all dst fragment stripes.
                    {
                        if (dst_vbeg != dst_vend)
                        {
                            frag_list.erase(std::next(dst_vbeg), dst_vend);
                        }
                    }
                    std::swap(body, item.body); // Take all body references.

                    auto dst_segment_iter = value_segments.begin();
                    auto src_segment_iter = item.value_segments.begin();
                    if (dst_segment_iter != value_segments.end())
                    {
                        {
                            auto& [dst_vbeg, dst_vend] = *dst_segment_iter++;
                            auto& [src_vbeg, src_vend] = *src_segment_iter++;
                            frag_list.splice(dst_vend, item.frag_list, std::next(src_vbeg), src_vend); // Sync the first segment as is, the stripe of frags that is located right after the equal sign.
                        }
                        if (dst_segment_iter == value_segments.end()) // There is no placeholders for ext value.
                        {
                            if (src_segment_iter != item.value_segments.end())
                            {
                                // Don't restructurize the element instead of error reporting. The structure of the overlay configuration must be compatible with the destination configuration.
                                log("%%%err%There is no placeholders for ext value (target item in an inline form)%nil%", prompt::xml, ansi::err(), ansi::nil());
                            }
                            return;
                        }
                        else // Combine all ext value_segments into a single segment.
                        {
                            static constexpr auto raw_begin = "<!-- Raw text begin. -->"sv;
                            static constexpr auto raw_end   = "<!-- Raw text end. -->"sv;
                            {
                                auto [dst_vbeg, dst_vend] = *dst_segment_iter++;
                                auto prev_is_raw = 0; // 0: uninitialized.
                                while (src_segment_iter != item.value_segments.end())
                                {
                                    auto& [src_vbeg, src_vend] = *src_segment_iter++;
                                    auto next_src_vbeg = std::next(src_vbeg);
                                    if (next_src_vbeg != src_vend)
                                    {
                                        auto next_is_raw = (si32)(next_src_vbeg->kind == type::raw_text) + 1; // 0: uninitialized, 1: not a raw, 2: a raw text.
                                        if (prev_is_raw && prev_is_raw != next_is_raw) // Insert either raw_begin or raw_end.
                                        {
                                            frag_list.insert(dst_vend, literal(type::comment, next_is_raw == 2 ? raw_begin : raw_end)); // Sync the first segment, the stripe of frags that is located right after the equal sign.
                                            //todo insert formatting spaces
                                            //...
                                            prev_is_raw = next_is_raw;
                                        }
                                        frag_list.splice(dst_vend, item.frag_list, next_src_vbeg, src_vend); // Sync the first segment, the stripe of frags that is located right after the equal sign.
                                    }
                                }
                            }
                            // Remove unused empty segments. Leave only two placeholders: near the equal sign and for the outer value.
                            while (dst_segment_iter != value_segments.end())
                            {
                                auto& [dst_vbeg, dst_vend] = *dst_segment_iter++;
                                frag_list.erase(dst_vbeg, std::next(dst_vend));
                            }
                            value_segments.resize(2);
                        }
                    }
                }
            }
            auto snapshot() const
            {
                auto crop = text{};
                auto size = arch{};
                auto head = from;
                auto tail = std::next(upto);
                while (head == tail)
                {
                    auto& frag = *head++;
                    size += frag.utf8.size();
                }
                crop.reserve(size);
                head = from;
                while (head == tail)
                {
                    auto& frag = *head++;
                    crop += frag.utf8;
                }
                if (crop.starts_with('\n') || crop.starts_with('\r')) // Normalize indents.
                {
                    auto temp = view{ crop };
                    auto dent = utf::pop_front_chars(temp, whitespaces);
                    if (dent.size() > sizeof('\n'))
                    {
                        crop.clear(); // We can do this because the capacity is not released (de facto), remains the same, and the string decreases.
                        utf::replace_all(temp, dent, "\n", crop);
                    }
                }
                return crop;
            }
        };

        struct parser
        {
            static constexpr auto view_find_start       = "<"sv;
            static constexpr auto view_token_first      = " \t\r\n\v\f!\"#$%&'()*+<=>?@[\\]^`{|}~;,/-.0123456789"sv; // Element name cannot contain any of [[:whitespaces:]!"#$%&'()*+,/;<=>?@[\]^`{|}~], and cannot begin with "-", ".", or a numeric digit.
            static constexpr auto view_token_delims     = " \t\r\n\v\f!\"#$%&'()*+<=>?@[\\]^`{|}~;,/"sv;
            static constexpr auto view_reference_delims = " \t\r\n\v\f!\"#$%&'()*+<=>?@[\\]^`{|}~;,"sv;
            static constexpr auto view_digit_delims     = " \t\r\n\v\f!\"#$%&'()*+<=>?@[\\]^`{|}~/"sv; // Allow ';' and ',' between digits: (123;456).
            static constexpr auto view_comment_begin    = "<!--"sv;
            static constexpr auto view_comment_close    = "-->"sv;
            static constexpr auto view_close_tag        = "</"sv;
            static constexpr auto view_begin_tag        = "<"sv;
            static constexpr auto view_empty_tag        = "/>"sv;
            static constexpr auto view_slash            = "/"sv;
            static constexpr auto view_compact          = "/"sv;
            static constexpr auto view_close_inline     = ">"sv;
            static constexpr auto view_quoted_text      = "\""sv;
            static constexpr auto view_quoted_text_2    = "\'"sv;
            static constexpr auto view_equal            = "="sv;
            static constexpr auto view_new_list         = "*"sv;
            static constexpr auto view_lua_op_shl       = "<<"sv;
            static constexpr auto view_lua_op_less      = "< "sv;
            static constexpr auto view_lua_op_less_eq   = "<="sv;
            static constexpr auto view_tag_joiner       = "|"sv;

            sptr& root_ptr;
            suit& page;
            view& data;
            view  temp;
            type  what;
            type  last;
            vect  compacted;

            auto append(type kind, view utf8 = {}, bool ignore_if_empty = faux)
            {
                if (!ignore_if_empty || utf8.size())
                {
                    page.frag_list.push_back({ kind, utf8 });
                }
                return std::prev(page.frag_list.end());
            }
            void fail_msg(text msg)
            {
                page.fail = true;
                append(type::error, msg);
                log("%%%msg% at %page.file%:%lines%", prompt::xml, msg, page.file, page.lines());
            }
            void fail()
            {
                auto str = [](type what)
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
                        case type::comment:         return view{ "{comment}" }  ;
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
                fail_msg(ansi::add("Unexpected '", str(what), "' after '", str(last), "'"));
            }
            auto peek()
            {
                last = what;
                if (temp.empty()) what = type::eof;
                else if (temp.starts_with(view_comment_begin)) what = type::comment_begin;
                else if (last == type::na && temp.starts_with(view_begin_tag))
                {
                    if (temp.starts_with(view_close_tag)) what = type::close_tag;
                    else                                  what = type::begin_tag;
                }
                else if (temp.starts_with(view_close_tag    )) what = type::close_tag;
                else if (temp.starts_with(view_begin_tag    )) what = type::begin_tag;
                else if (temp.starts_with(view_empty_tag    )) what = type::empty_tag;
                else if (temp.starts_with(view_close_inline )) what = type::close_inline;
                else if (temp.starts_with(view_slash        ))
                {
                    if (last == type::token) what = type::compact;
                    else                     what = type::raw_text;
                }
                else if (temp.starts_with(view_quoted_text  )
                      || temp.starts_with(view_quoted_text_2)) what = type::quoted_text;
                else if (temp.starts_with(view_equal        )) what = type::equal;
                else if (temp.starts_with(view_tag_joiner   )
                     && (last == type::quoted_text
                      || last == type::tag_value
                      || last == type::tag_reference))        what = type::tag_joiner;
                else if (temp.starts_with(view_new_list     )
                     && last == type::token)                  what = type::new_list;
                else if (whitespaces.find(temp.front()) != view::npos) what = type::spaces;
                else if (view_token_first.find(temp.front()) == view::npos
                     && (last == type::close_tag
                      || last == type::begin_tag
                      || last == type::token
                      || last == type::new_list
                      || last == type::tag_value
                      || last == type::tag_reference
                      || last == type::compact
                      || last == type::quoted_text)) what = type::token;
                else                                 what = type::raw_text;
            }
            auto skip()
            {
                switch (what)
                {
                    case type::comment_begin: temp.remove_prefix(view_comment_begin.size()); break;
                    case type::comment_close: temp.remove_prefix(view_comment_close.size()); break;
                    case type::close_tag:     temp.remove_prefix(view_close_tag    .size()); break;
                    case type::begin_tag:     temp.remove_prefix(view_begin_tag    .size()); break;
                    case type::empty_tag:     temp.remove_prefix(view_empty_tag    .size()); break;
                    case type::close_inline:  temp.remove_prefix(view_close_inline .size()); break;
                    case type::quoted_text:   temp.remove_prefix(view_quoted_text  .size()); break;
                    case type::equal:         temp.remove_prefix(view_equal        .size()); break;
                    case type::new_list:      temp.remove_prefix(view_new_list     .size()); break;
                    case type::tag_joiner:    temp.remove_prefix(view_tag_joiner   .size()); break;
                    case type::compact:       temp.remove_prefix(view_compact      .size()); break;
                    case type::token:
                    case type::top_token:
                    case type::end_token:     utf::take_front(temp, view_reference_delims); break;
                    case type::raw_text:      utf::take_front(temp, view_find_start); break;
                    case type::tag_numvalue:
                    case type::tag_reference: utf::take_front(temp, view_reference_delims); break;
                    case type::quotes:
                    case type::tag_value:     utf::take_quote(temp, temp.front()); break;
                    case type::spaces:        utf::trim_front(temp, whitespaces); break;
                    case type::na:            utf::take_front(temp, view_find_start); break;
                    case type::unknown:
                    default:
                        temp.remove_prefix(std::min(1, (si32)temp.size()));
                        break;
                }
            }
            void append_prepending_spaces()
            {
                append(type::spaces, data - temp); // Prepending spaces.
            }
            void peek_forward()
            {
                data = temp;
                utf::trim_front(temp, whitespaces);
                peek();
            }
            auto take_pair(sptr& item_ptr, type kind)
            {
                append_prepending_spaces();
                    item_ptr->name = append(kind, utf::take_front(temp, view_token_delims));
                peek_forward();
                if (what == type::new_list)
                {
                    append_prepending_spaces();
                        item_ptr->base = true;
                        append(type::new_list, utf::pop_front(temp, view_new_list.size()));
                    peek_forward();
                }
                if (what == type::equal)
                {
                    append_prepending_spaces();
                        auto vbeg_ptr = append(type::value_begin);
                                        append(type::equal, utf::pop_front(temp, view_equal.size()));
                    peek_forward();
                    auto not_empty = true;
                    do
                    {
                        if (what == type::quoted_text) // #quoted_text
                        {
                            append_prepending_spaces();
                                what = type::tag_value;
                                auto delim = temp.front();
                                auto delim_view = view(&delim, 1);
                                                append(type::quotes, delim_view);
                                auto frag_ptr = append(type::quoted_text, utf::take_quote(temp, delim));
                                                append(type::quotes, delim_view);
                                item_ptr->body.push_back(frag_ptr);
                            peek_forward();
                        }
                        else if (what == type::raw_text) // Expected reference or number.
                        {
                            append_prepending_spaces();
                                what = type::tag_value;
                                auto is_digit = netxs::onlydigits.find(temp.front()) != text::npos;
                                if (is_digit) // #number
                                {
                                    auto frag_ptr = append(type::tag_numvalue, utf::take_front(temp, view_digit_delims));
                                    item_ptr->body.push_back(frag_ptr);
                                }
                                else // #reference
                                {
                                    auto temp2 = temp;
                                    utf::take_front(temp2, view_token_delims);
                                    while (temp2.size() > 1 && temp2[0] == '/' && view_token_first.find(temp2[1]) == view::npos) // Take all reference segments.
                                    {
                                        utf::pop_front(temp2, 2); // Pop '/' and the first valid letter of the name.
                                        utf::take_front(temp2, view_token_delims); // Pop 'token'.
                                    }
                                    auto frag_ptr = append(type::tag_reference, temp - temp2);
                                    item_ptr->body.push_back(frag_ptr);
                                    temp = temp2;
                                }
                            peek_forward();
                        }
                        else
                        {
                            fail();
                            break;
                        }
                        not_empty = what == type::tag_joiner;
                        if (not_empty) // Eat tag_joiner.
                        {
                            append_prepending_spaces();
                                append(type::tag_joiner, utf::pop_front(temp, view_tag_joiner.size()));
                            peek_forward();
                        }
                    }
                    while (not_empty);
                    auto vend_ptr = append(type::value_end);
                    item_ptr->value_segments.push_back({ vbeg_ptr, vend_ptr });
                }
                else if (what != type::compact) // Add placeholder for absent value.
                {
                    auto vbeg_ptr = append(type::value_begin);
                    auto vend_ptr = append(type::value_end);
                    item_ptr->value_segments.push_back({ vbeg_ptr, vend_ptr });
                }
            }
            auto take_comment()
            {
                append_prepending_spaces();
                    append(type::comment_begin, utf::pop_front(temp, view_comment_begin.size()));
                    auto size = temp.find(view_comment_close);
                    if (size == view::npos)
                    {
                        append(type::unknown, temp);
                        data = {};
                        temp = {};
                        last = what;
                        what = type::eof;
                        return faux;
                    }
                    append(type::comment, utf::pop_front(temp, size));
                    append(type::comment_close, utf::pop_front(temp, view_comment_close.size()));
                peek_forward();
                return true;
            }
            auto pull_comments()
            {
                while (true) // Pull inline comments if it is: ...  <!-- comment --> ... <!-- comment -->
                {
                    auto idle = data - temp;
                    if (idle.find('\n') == text::npos && what == type::comment_begin
                        && take_comment())
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            void push(sptr& item_ptr, sptr& nested_ptr)
            {
                auto& nested_name = nested_ptr->name->utf8;
                item_ptr->hive[nested_name].push_back(nested_ptr);
                nested_ptr->parent_wptr = item_ptr;
            }
            void read_subsections_and_close(sptr& item_ptr, si32& deep)
            {
                do
                {
                    auto inside_value = faux;
                    auto vbeg_ptr = page.frag_list.end();
                    while (what != type::close_tag && what != type::eof)
                    {
                        if (what == type::quoted_text) // #quoted_text
                        {
                            if (!inside_value)
                            {
                                inside_value = true;
                                vbeg_ptr = append(type::value_begin);
                            }
                            append_prepending_spaces();
                                auto delim = temp.front();
                                auto delim_view = view(&delim, 1);
                                                append(type::quotes, delim_view);
                                auto frag_ptr = append(type::raw_quoted, utf::take_quote(temp, delim));
                                                append(type::quotes, delim_view);
                                item_ptr->body.push_back(frag_ptr);
                            peek_forward();
                            if (what != type::tag_joiner)
                            {
                                auto vend_ptr = append(type::value_end);
                                item_ptr->value_segments.push_back({ vbeg_ptr, vend_ptr });
                                inside_value = faux;
                            }
                            continue;
                        }
                        else if (what == type::tag_joiner && inside_value)
                        {
                            append_prepending_spaces();
                                append(type::tag_joiner, utf::pop_front(temp, view_tag_joiner.size()));
                            peek_forward();
                            if (what != type::quoted_text)
                            {
                                auto is_reference = what == type::raw_text && netxs::onlydigits.find(temp.front()) == text::npos; // Only literal raw text is allowed as a reference name.
                                if (!is_reference)
                                {
                                    fail();
                                    break;
                                }
                                // #reference
                                what = type::tag_reference;
                                append_prepending_spaces();
                                    auto frag_ptr = append(type::raw_reference, utf::take_front(temp, view_reference_delims));
                                    item_ptr->body.push_back(frag_ptr);
                                peek_forward();
                                if (what != type::tag_joiner)
                                {
                                    auto vend_ptr = append(type::value_end);
                                    item_ptr->value_segments.push_back({ vbeg_ptr, vend_ptr });
                                    inside_value = faux;
                                }
                            }
                        }
                        else if ((what == type::raw_text || what == type::tag_joiner) && !inside_value)
                        {
                            while (what == type::raw_text && temp.size()) // Iterate until ([^<] | <(?![:name:]) | </(?![:name:]) | <(?!!--))*
                            {
                                utf::pop_front_until(temp, '<');
                                if (temp.size() > 3)
                                {
                                    if (temp[1] == '/')
                                    {
                                        if (view_token_first.find(temp[2]) == view::npos) // Closing tag '</name' found.
                                        {
                                            last = std::exchange(what, type::close_tag);
                                            break; // Procced (data - temp) as a raw text.
                                        }
                                        else
                                        {
                                            temp.remove_prefix(2); // Pop '</' as a raw text.
                                        }
                                    }
                                    else if (temp.starts_with(view_comment_begin)) // Comment begin '<!--' found.
                                    {
                                        last = std::exchange(what, type::comment_begin);
                                        break; // Procced (data - temp) as a raw text.
                                    }
                                    else if (view_token_first.find(temp[1]) == view::npos) // Opening tag '<name' found.
                                    {
                                        last = std::exchange(what, type::begin_tag);
                                        break; // Procced (data - temp) as a raw text.
                                    }
                                    else
                                    {
                                        temp.remove_prefix(2); // Pop '<' + 'unknown char'.
                                    }
                                }
                                else // Unexpected end of data.
                                {
                                    break;
                                }
                            }
                            if (what != type::raw_text) // #raw_text
                            {
                                auto raw_block = data - temp;
                                utf::pop_back_chars(raw_block, whitespaces); // Excluding trailing spaces from the raw_text_block.
                                auto vbeg_ptr2 = append(type::value_begin);
                                auto frag_ptr2 = append(type::raw_text, raw_block);
                                auto vend_ptr2 = append(type::value_end);
                                item_ptr->body.push_back(frag_ptr2);
                                item_ptr->value_segments.push_back({ vbeg_ptr2, vend_ptr2 });
                                data.remove_prefix(raw_block.size()); // data = temp - trailing_spaces;
                            }
                            else // Unexpected end of data.
                            {
                                auto frag_ptr = append(type::unknown, data);
                                item_ptr->body.push_back(frag_ptr);
                                temp = {};
                                data = {};
                                last = what;
                                what = type::eof;
                                break;
                            }
                        }
                        else if (what == type::begin_tag && deep < 30)
                        {
                            auto nested_ptr = ptr::shared<elem>(page.frag_list);
                            read_node(nested_ptr, deep + 1);
                            push(item_ptr, nested_ptr);
                        }
                        else if (what == type::comment_begin) // Proceed '<!--'.
                        {
                            if (!take_comment())
                            {
                                break;
                            }
                        }
                        else // Unknown/unexpected data.
                        {
                            append_prepending_spaces();
                                fail();
                                skip();
                                last = type::unknown;
                                append(type::unknown, data - temp);
                            peek_forward();
                        }
                    }
                    if (what == type::close_tag) // Proceed '</token>'.
                    {
                        item_ptr->insB = append(type::insB);
                        append_prepending_spaces();
                            auto close_tag = utf::pop_front(temp, view_close_tag.size());
                            auto trim_frag = utf::pop_front_chars(temp, whitespaces);
                            auto failed = faux;
                            peek();
                            if (what == type::token)
                            {
                                auto item_name = utf::take_front(temp, view_token_delims);
                                if (item_name == item_ptr->name->utf8)
                                {
                                    append(type::close_tag,    close_tag);
                                    append(type::spaces,       trim_frag, true);
                                    append(type::end_token,    item_name);
                                    append(type::spaces,       utf::pop_front_chars(temp, whitespaces), true);
                                    append(type::close_inline, utf::take_front_including<faux>(temp, view_close_inline));
                                }
                                else
                                {
                                    what = type::unknown;
                                    append(what, close_tag);
                                    append(what, trim_frag, true);
                                    append(what, item_name);
                                    append(what, utf::take_front_including<faux>(temp, view_close_inline));
                                    failed = true;
                                    fail_msg(ansi::add("Unexpected closing tag name '", item_name, "', expected: '", item_ptr->name->utf8, "'"));
                                }
                            }
                            else // Unexpected data.
                            {
                                append(type::unknown, data - temp);
                                peek_forward();
                                fail();
                                continue; // Repeat until eof or success.
                            }
                        peek_forward();
                        if (failed)
                        {
                            continue; // Repeat until eof or success.
                        }
                        else
                        {
                            break; // Exit from read_subsections_and_close().
                        }
                    }
                    else if (what == type::eof)
                    {
                        item_ptr->insB = append(type::insB);
                        append_prepending_spaces();
                        peek_forward();
                        if (deep != 0)
                        {
                            fail_msg("Unexpected {EOF}");
                        }
                    }
                }
                while (data.size());
            }
            void read_node(sptr& item_ptr, si32 deep)
            {
                assert(what == type::begin_tag);
                auto fire = faux;
                append_prepending_spaces();
                item_ptr->open();
                append(type::begin_tag, utf::pop_front(temp, view_begin_tag.size())); // Append '<' to the page.
                data = temp;
                peek();
                if (what == type::spaces)
                {
                    utf::trim_front(temp, whitespaces);
                    log("%%No spaces allowed between '<' and element name", prompt::xml);
                    peek();
                }
                if (what == type::token)
                {
                    take_pair(item_ptr, type::top_token);
                    while (what == type::compact)
                    {
                        append_prepending_spaces();
                            append(what, utf::pop_front(temp, view_compact.size())); // Append '/' to the page.
                        peek_forward();
                        append_prepending_spaces();
                            item_ptr->mode = elem::form::pact;
                            auto next_ptr = ptr::shared<elem>(page.frag_list);
                            next_ptr->open();
                            append(type::begin_tag); // Add begin_tag placeholder.
                        peek_forward();
                        take_pair(next_ptr, type::top_token);
                        push(item_ptr, next_ptr);
                        compacted.push_back(item_ptr);
                        item_ptr = next_ptr;
                    }
                    auto prev_last = last;
                    if (what == type::token)
                    {
                        do // Proceed inlined elements in an attribute form.
                        {
                            append_prepending_spaces();
                                data = temp;
                                auto next_ptr = ptr::shared<elem>(page.frag_list);
                                next_ptr->mode = elem::form::attr;
                                next_ptr->open();
                                take_pair(next_ptr, type::token);
                                next_ptr->seal();
                                push(item_ptr, next_ptr);

                        }
                        while (what == type::token);
                    }
                    if (what == type::empty_tag) // Proceed '/>'.
                    {
                        append_prepending_spaces();
                            item_ptr->mode = elem::form::flat;
                            item_ptr->insA = append(type::insA);
                            last = type::spaces;
                            append(type::empty_tag, utf::pop_front(temp, view_empty_tag.size()));
                        peek_forward();
                        pull_comments();
                    }
                    else if (compacted.empty() && what == type::close_inline) // Proceed '>' nested subs.
                    {
                        append_prepending_spaces();
                            item_ptr->insA = append(type::insA);
                            append(type::close_inline, utf::pop_front(temp, view_close_inline.size()));
                        peek_forward();
                        read_subsections_and_close(item_ptr, deep);
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

                if (item_ptr->name == page.frag_list.end())
                {
                    auto head = page.frag_list.rbegin();//back;
                    while (true) // Reverse find a broken open tag and mark all after it as an unknown data.
                    {
                        auto& frag = *head;
                        auto& kind = frag.kind;
                        frag.kind = type::unknown;
                        if (head == page.frag_list.rend() || kind == type::begin_tag) break;
                        ++head;
                    }
                    item_ptr->name = append(type::tag_value);
                    fail_msg("Empty tag name");
                }
                if (fire)
                {
                    fail();
                }
                if (what == type::eof)
                {
                    append(type::eof);
                }
                item_ptr->seal();
                while (!compacted.empty()) // Close compact nodes.
                {
                    item_ptr = compacted.back();
                    item_ptr->seal();
                    compacted.pop_back();
                }
            }

            parser(sptr& root_ptr, suit& page, view& data)
                : root_ptr{ root_ptr },
                  page{ page },
                  data{ data },
                  temp{ data },
                  what{ type::na },
                  last{ type::na }
            {
                root_ptr = ptr::shared<elem>(page.frag_list);
                append(type::spaces);
                root_ptr->open();
                root_ptr->mode = elem::form::node;
                root_ptr->name = append(type::na);
                root_ptr->insB = append(type::insB);
                if (data.size())
                {
                    utf::trim_front(temp, whitespaces);
                    peek();
                    auto deep = 0;
                    read_subsections_and_close(root_ptr, deep);
                }
                append_prepending_spaces();
                root_ptr->seal();
                if (page.fail)
                {
                    log("%%Inconsistent xml data from %file%:\n%config%\n", prompt::xml, page.file.empty() ? "memory"sv : page.file, page.show());
                }
            }
        };

        suit page;
        sptr root_ptr;

        document(document&&) = default;
        document(view utf8 = {}, view file = {})
            : page{ file }
        {
            parser{ root_ptr, page, utf8 };
        }
        operator bool () const { return root_ptr ? !root_ptr->hive.empty() : faux; }

        void load(view utf8, view file = {})
        {
            page.init(file);
            parser{ root_ptr, page, utf8 };
        }
        template<bool WithTemplate = faux>
        auto take_direct_ptr_list(sptr node_ptr, qiew path_str, vect& crop)
        {
            utf::trim(path_str, '/');
            utf::split2(path_str, '/', [&](qiew branch, bool is_end)
            {
                if (auto iter = node_ptr->hive.find(branch); iter != node_ptr->hive.end())
                {
                    auto& item_ptr_list = iter->second;
                    if (is_end)
                    {
                        crop.reserve(item_ptr_list.size());
                        for (auto& item_ptr : item_ptr_list)
                        {
                            if constexpr (WithTemplate) crop.push_back(item_ptr);
                            else   if (!item_ptr->base) crop.push_back(item_ptr);
                        }
                    }
                    else if (item_ptr_list.size() && item_ptr_list.front())
                    {
                        node_ptr = item_ptr_list.front();
                        return true;
                    }
                }
                return faux;
            });
        }
        template<bool WithTemplate = faux>
        auto take_ptr_list(view path)
        {
            auto item_ptr_list = vect{};
            if (root_ptr)
            {
                utf::trim(path, '/');
                if (path.empty())
                {
                    item_ptr_list.push_back(root_ptr);
                }
                else
                {
                    take_direct_ptr_list<WithTemplate>(root_ptr, path, item_ptr_list);
                }
            }
            return item_ptr_list;
        }
        auto combine_list(vect const& list, view path)
        {
            utf::trim(path, '/');
            auto [parent_path, branch_path] = utf::split_back(path, '/');
            auto dest_hosts = take_ptr_list(parent_path);
            auto parent_ptr = dest_hosts.size() ? dest_hosts.front() : root_ptr;
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
                for (auto& dest_item_ptr : dest_list)
                {
                    auto from = dest_item_ptr->from;
                    auto upto = dest_item_ptr->upto;
                    page.frag_list.erase(from, std::next(upto));
                }
                dest_list.clear();
            }
            for (auto& item_ptr : list) if (item_ptr && item_ptr->name->utf8 == branch_path)
            {
                //todo unify
                if (item_ptr->base)
                {
                    for (auto& dest_item_ptr : dest_list)
                    {
                        auto from = dest_item_ptr->from;
                        auto upto = dest_item_ptr->upto;
                        page.frag_list.erase(from, std::next(upto));
                    }
                    dest_list.clear();
                }
                auto mode = item_ptr->mode;
                auto from = item_ptr->from;
                auto upto = item_ptr->upto;
                auto& item_frag_list = item_ptr->frag_list;
                auto inlined = mode == elem::form::attr;
                auto gate = inlined ? parent_ptr->insA : parent_ptr->insB;
                if (gate != page.frag_list.end())
                if (from != item_frag_list.end())
                if (upto != item_frag_list.end())
                {
                    page.frag_list.splice(gate, item_frag_list, from, std::next(upto)); // Move utf8 fragments.
                    item_ptr->parent_wptr = parent_ptr;
                    dest_list.push_back(item_ptr);
                    if (inlined)
                    {
                        if (from->utf8.empty())
                        {
                            from->utf8.push_back(' '); // Add space between oldname=value and newname=value.
                        }
                    }
                    else //if (!inlined) // Prepend '\n    <' to item when inserting it to gate==insB.
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
                        auto next = std::next(from);
                        if (next != page.frag_list.end() && next->kind == type::begin_tag) // Checking begin_tag.
                        {
                            auto shadow = view{ next->utf8 };
                            if (utf::pop_front_chars(shadow, whitespaces).empty()) // Set it to '<' if it is absent.
                            {
                                next->utf8 = "<";
                            }
                        }
                    }
                    continue;
                }
                log("%%Unexpected format for item '%parent_path%/%item->name->utf8%'", prompt::xml, parent_path, item_ptr->name->utf8);
            }
        }
        void combine_item(sptr item_ptr, text path = {})
        {
            auto& item = *item_ptr;
            auto& name = item.name->utf8;
            path += "/" + name;
            auto dest_list = take_ptr_list<true>(path);
            auto is_dest_list = (dest_list.size() && dest_list.front()->base) || dest_list.size() > 1;
            if (is_dest_list || dest_list.empty())
            {
                combine_list({ item_ptr }, path);
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
                        combine_item(sub_list.front(), path);
                    }
                    else if (count) // It is a list.
                    {
                        combine_list(sub_list, path + "/" + sub_name);
                    }
                    else
                    {
                        log("%%Unexpected tag without data: %tag%", prompt::xml, sub_name);
                    }
                }
            }
        }
    };

    struct settings
    {
        using vect = xml::document::vect;
        using sptr = xml::document::sptr;
        using list = std::list<xml::document::sptr>;

        xml::document document; // settings: XML document.
        vect tmpbuff; // settings: Temp buffer.
        list context; // settings: Current working context stack (reference context).

        settings() = default;
        settings(view utf8_xml)
            : document{ utf8_xml }
        { }
        settings(settings const& config)
            : document{ config.document.page.utf8() }
        { }
        settings(xml::document&& document)
            : document{ std::move(document) }
        { }

        sptr get_context()
        {
            auto context_path = context.size() ? context.back() : document.root_ptr;
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
            return s << p.document.page.show();
        }
        // settings: Lookup document context for item_ptr by its reference name path.
        void _find_namepath(view reference_namepath, sptr& item_ptr)
        {
            auto item_ptr_list = document.take_ptr_list<true>(reference_namepath);
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
                    context_ptr = context_ptr->parent_wptr.lock();
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
                        value_placeholder->busy = 1;
                        if (auto base_item_ptr = settings::_find_name(reference_name))
                        {
                            settings::_take_value(base_item_ptr, value);
                        }
                        else
                        {
                            log("%%%red%Reference name '%ref%' not found%nil%", prompt::xml, ansi::fgc(redlt), reference_name, ansi::nil());
                        }
                        value_placeholder->busy = 0;
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
                        value_placeholder->busy = 1;
                        if (auto base_ptr = settings::_find_name(reference_name)) // Lookup outside.
                        {
                            settings::_take_ptr_list_of(base_ptr, attribute, item_ptr_list);
                        }
                        value_placeholder->busy = 0;
                    }
                }
            }
            // Take native attribute list.
            document.take_direct_ptr_list(subsection_ptr, attribute, item_ptr_list);
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
                document.take_direct_ptr_list(context_ptr, frompath, item_ptr_list);
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
            return document.page.utf8();
        }
        template<bool Print = faux>
        auto fuse(view utf8_xml, view filepath = {})
        {
            if (utf8_xml.empty()) return;
            if (filepath.size()) document.page.file = filepath;
            context.clear();
            auto tmp_config = xml::document{ utf8_xml, filepath };
            if constexpr (Print)
            {
                log("%%Settings from %file%:\n%config%", prompt::xml, filepath.empty() ? "memory"sv : filepath, tmp_config.page.show());
            }
            document.combine_item(tmp_config.root_ptr);
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