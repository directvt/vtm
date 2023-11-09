// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "canvas.hpp"
#include "quartz.hpp"

namespace netxs::xml
{
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
    auto take(qiew utf8) -> std::optional<T>
    {
        if (utf8.starts_with("0x"))
        {
            utf8.remove_prefix(2);
            return utf::to_int<T, 16>(utf8);
        }
        else return utf::to_int<T, 10>(utf8);
    }
    template<>
    auto take<text>(qiew utf8) -> std::optional<text>
    {
        return utf8.str();
    }
    template<>
    auto take<bool>(qiew utf8) -> std::optional<bool>
    {
        auto value = utf::to_low(utf8.str());
        return value.empty() || value.starts_with("1")  // 1 - true
                             || value.starts_with("on") // on
                             || value.starts_with("y")  // yes
                             || value.starts_with("t"); // true
    }
    template<>
    auto take<twod>(qiew utf8) -> std::optional<twod>
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
    auto take<span>(qiew utf8) -> std::optional<span>
    {
        using namespace std::chrono;
        utf::trim_front(utf8, " ({[\"\'");
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
    auto take<rgba>(qiew utf8) -> std::optional<rgba>
    {
        auto tobyte = [](auto c)
        {
                 if (c >= '0' && c <= '9') return c - '0';
            else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            else                           return 0;
        };

        auto value = utf::to_low(utf8.str());
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
            else log("%%Unknown hex rgba format: { %value% }, expected #rrggbbaa or #rrggbb rgba hex value", prompt::xml, value);
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
            else log("%%Unknown hex rgba format: { %value% }, expected 0xaabbggrr or 0xbbggrr rgba hex value", prompt::xml, value);
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
            log("%%Unknown hex rgba format: { %value% }, expected 000,000,000,000 decimal rgba value", prompt::xml, value);
        }
        else if (auto c = utf::to_int(shadow)) // Single ANSI color value
        {
            if (c.value() >=0 && c.value() <=255)
            {
                result = rgba::vt256[c.value()];
                return result;
            }
            else log("%%Unknown ANSI 256-color value format: { %value% }, expected 0-255 decimal value", prompt::xml, value);
        }
        return std::nullopt;
    }

    struct document
    {
        enum type
        {
            na,            // start of file
            eof,           // end of file
            eol,           // end of line
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
            //compact,       // '/[^>]' ex: compact syntax: <name/nested_block1/nested_block2=value param=value />
            include,       // ':'     ex: <name:...=value param=value />
            localpath,     //         ex: <name:/path/path=value param=value />
            filepath,      //         ex: <name:"/filepath/filepath"=value param=value />
            spaces,        // ' '     ex: \s\t\r\n...
            unknown,       //
            tag_value,     //
            error,         // Inline error message.
        };

        struct literal;
        using frag = netxs::sptr<literal>;

        struct literal
        {
            using wptr = netxs::wptr<literal>;

            wptr prev; // literal: Pointer to the prev.
            frag next; // literal: Pointer to the next.
            type kind; // literal: Content type.
            text utf8; // literal: Content data.

            template<class ...Args>
            literal(type kind, Args&&... args)
                : kind{ kind },
                  utf8{ std::forward<Args>(args)... }
            { }
        };

        struct suit
        {
            frag data; // suit: Linked list start.
            bool fail; // suit: Broken format.
            text file; // suit: Data source name.
            frag back; // suit: Linked list end.

            suit(suit&&) = default;
            suit(view file = {})
                : data{ ptr::shared<literal>(type::na) },
                  fail{ faux },
                  file{ file },
                  back{ data }
            { }

            template<class ...Args>
            auto append(type kind, Args&&... args)
            {
                auto item = ptr::shared<literal>(kind, std::forward<Args>(args)...);
                item->prev = back;
                back->next = item;
                back = item;
                return back;
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
    
                auto yield = ansi::escx{};
                auto next = data;
                while (next)
                {
                    auto& item = *next;
                    auto& data = item.utf8;
                    auto  kind = item.kind;
                    next = next->next;
    
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
                        //case compact:       fgc = end_token_fg; break;
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
                        case error:         fgc = whitelt;
                                            bgc = reddk;
                                            yield += ' ';       break;
                        default: break;
                    }
                    //test
                    //yield.bgc((tint)(clr % 8));

                    if (data.size())                        
                    {
                             if (bgc) yield.fgc(fgc).bgc(bgc).add(data).nil();
                        else if (fgc) yield.fgc(fgc)         .add(data).nil();
                        else          yield                  .add(data);
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
        using heap = std::vector<frag>;
        using vect = std::vector<sptr>;
        using subs = std::unordered_map<text, vect, qiew::hash, qiew::equal>;

        struct elem
        {
            enum form
            {
                node,
                attr,
                flat,
            };

            frag from; // elem: First fragment in document.
            frag name; // elem: Tag name.
            frag insA; // elem: Insertion point for inline subelements.
            frag insB; // elem: Insertion point for nested subelements.
            frag upto; // elem: Pointer to the end of the semantic block.
            heap body; // elem: Value fragments.
            subs hive; // elem: Subelements.
            wptr defs; // elem: Template.
            bool fake; // elem: Is it template.
            bool base; // elem: Merge overwrite priority.
            form mode; // elem: Element storage form.

            elem()
                : fake{ faux },
                  base{ faux },
                  mode{ node }
            { }
           ~elem()
            {
                hive.clear();
                if (auto prev = from->prev.lock())
                {
                    auto next = upto->next;
                              prev->next = next;
                    if (next) next->prev = prev;
                }
            }

            template<bool WithTemplate = faux>
            auto list(qiew path_str)
            {
                path_str = utf::trim(path_str, '/');
                auto root = this;
                auto crop = vect{}; //auto& items = config.root->hive["menu"][0]->hive["item"]...;
                auto temp = text{};
                auto path = utf::divide(path_str, '/');
                if (path.size())
                {
                    auto head = path.begin();
                    auto tail = path.end();
                    while (head != tail)
                    {
                        temp = *head++;
                        if (auto iter = root->hive.find(temp);
                                 iter!= root->hive.end())
                        {
                            auto& i = iter->second;
                            crop.reserve(i.size());
                            if (head == tail)
                            {
                                for (auto& item : i)
                                {
                                    if constexpr (WithTemplate) crop.push_back(item);
                                    else       if (!item->fake) crop.push_back(item);
                                }
                            }
                            else if (i.size() && i.front())
                            {
                                root = &(*(i.front()));
                            }
                            else break;
                        }
                        else break;
                    }
                }
                return crop;
            }
            auto value() -> text
            {
                auto crop = text{};
                for (auto& v : body)
                {
                    crop += xml::unescape(v->utf8);
                }
                return crop;
            }
            void value(text value)
            {
                if (body.size())
                {
                    auto head = body.begin() + 1;
                    auto tail = body.end();
                    while (head != tail)
                    {
                        (*head++)->utf8.clear();
                    }
                    body.resize(1);
                    auto value_placeholder = body.front();
                    if (value_placeholder->kind == type::tag_value) // equal [spaces] quotes tag_value quotes
                    if (auto quote_placeholder = value_placeholder->prev.lock())
                    if (quote_placeholder->kind == type::quotes)
                    if (auto equal_placeholder = quote_placeholder->prev.lock())
                    {
                        if (equal_placeholder->kind != type::equal) // Spaces after equal sign.
                        {
                            equal_placeholder = equal_placeholder->prev.lock();
                        }
                        if (equal_placeholder && equal_placeholder->kind == type::equal)
                        {
                            if (value.size())
                            {
                                equal_placeholder->utf8 = " = ";
                                quote_placeholder->utf8 = "\"";
                                if (value_placeholder->next) value_placeholder->next->utf8 = "\"";
                            }
                            else
                            {
                                equal_placeholder->utf8 = "";
                                quote_placeholder->utf8 = "";
                                if (value_placeholder->next) value_placeholder->next->utf8 = "";
                            }
                        }
                        else log(prompt::xml, "Equal sign not found");
                    }
                    value_placeholder->utf8 = xml::escape(value);
                }
                else log(prompt::xml, "Unexpected assignment to ", name->utf8);
            }
            template<class T>
            auto take(qiew attr, T fallback = {})
            {
                if (auto iter = hive.find(attr); iter != hive.end())
                {
                    auto& item_set = iter->second;
                    if (item_set.size()) // Take the first item only.
                    {
                        auto crop = item_set.front()->value();
                        if (auto result = xml::take<T>(crop)) return result.value();
                        else                                  return fallback;
                    }
                }
                if (auto defs_ptr = defs.lock()) return defs_ptr->take(attr, fallback);
                else                             return fallback;
            }
            template<class T>
            auto take(qiew attr, T defval, std::unordered_map<text, T> const& dict)
            {
                if (attr.empty()) return defval;
                auto crop = take(attr, ""s);
                auto iter = dict.find(crop);
                return iter == dict.end() ? defval
                                          : iter->second;
            }
            auto show(sz_t indent = 0) -> text
            {
                auto data = text{};
                data += text(indent, ' ') + '<' + name->utf8;
                if (fake) data += view_defaults;

                if (body.size())
                {
                    auto val = text{};
                    for (auto& val_ptr : body)
                    {
                        val += val_ptr->utf8;
                    }
                    if (val.size())
                    {
                        if (utf::check_any(val, rawtext_delims)) data += "=\"" + xml::escape(val) + "\"";
                        else                                     data += '='   + xml::escape(val) + ' ';
                    }
                }

                if (hive.empty()) data += "/>\n";
                else
                {
                    data += ">\n";
                    for (auto& [sub_name, sub_list] : hive)
                    {
                        for (auto& item : sub_list)
                        {
                            data += item->show(indent + 4);
                        }
                    }
                    data += text(indent, ' ') + "</" + name->utf8 + ">\n";
                }

                return data;
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
                    auto size = crop.size();
                    auto temp = view{ crop };
                    auto dent = text{ utf::trim_front(temp, whitespaces) };
                    crop = temp;
                    utf::change(crop, dent, "\n");
                }
                return crop;
            }
        };

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

        suit page;
        sptr root;

        document(document&&) = default;
        document(view data, view file = {})
            : page{ file },
              root{ ptr::shared<elem>()}
        {
            read(data);
            if (page.fail) log(prompt::xml, "Inconsistent xml data from ", file.empty() ? "memory"sv : file, ":\n", page.show(), "\n");
        }
        template<bool WithTemplate = faux>
        auto take(view path)
        {
            auto name = root && root->name ? root->name->utf8 : text{};
            path = utf::trim(path, '/');
            if (path.empty()
             || path == name)
            {
                return vect{ root };
            }
            else
            {
                auto temp = utf::cutoff(path, '/');
                if (name == temp)
                {
                    return root->list<WithTemplate>(path.substr(temp.size()));
                }
            }
            return vect{};
        }
        auto join(view path, vect const& list, bool rewrite = faux)
        {
            path = utf::trim(path, '/');
            auto parent_path = utf::cutoff(path, '/', faux);
            auto branch_path = utf::remain(path, '/', faux);
            auto dest_host = take(parent_path);
            if (dest_host.size())
            {
                auto parent = dest_host.front();
                auto& hive = parent->hive;
                auto iter = hive.find(qiew{ branch_path });
                if (iter == hive.end())
                {
                    iter = hive.emplace(branch_path , vect{}).first;
                }
                auto& dest = iter->second;
                if (rewrite) dest.clear();
                for (auto& item : list)
                {
                    auto mode = item->mode;
                    auto from = item->from;
                    auto upto = item->upto;
                    auto next = upto->next;
                    if (auto gate = mode == elem::form::attr ? parent->insA : parent->insB)
                    if (auto prev = gate->prev.lock())
                    if (auto past = from->prev.lock())
                    {
                        from->prev = prev;
                        upto->next = gate;
                        gate->prev = upto;
                        prev->next = from;
                        past->next = next;  // Release an element from the previous list.
                        if (next) next->prev = past;
                        dest.push_back(item);
                        continue;
                    }
                    log("%%Unexpected format for item '%parent_path%/%item->name->utf8%'", prompt::xml, parent_path, item->name->utf8);
                }
            }
            else log(prompt::xml, "Destination path not found ", parent_path);
        }

    private:
        auto fail(text msg)
        {
            page.fail = true;
            page.append(type::error, msg);
            log(prompt::xml, msg, " at ", page.file, ":", page.lines());
        }
        auto fail(type last, type what)
        {
            auto str = [&](type what)
            {
                switch (what)
                {
                    case type::na:            return view{ "{START}" }   ;
                    case type::eof:           return view{ "{EOF}" }     ;
                    case type::eol:           return view{ "{EOL}" }     ;
                    case type::token:         return view{ "{token}" }   ;
                    case type::raw_text:      return view{ "{raw text}" };
                    //case type::compact:       return view{ "{compact}" } ;
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
            fail(ansi::add("Unexpected '", str(what), "' after '", str(last), "'"));
        }
        auto peek(view& data, type& what, type& last)
        {
            last = what;
            if (data.empty()) what = type::eof;
            else if (last == type::na)
            {
                if (!data.starts_with(view_comment_begin)
                 && !data.starts_with(view_close_tag    )
                 &&  data.starts_with(view_begin_tag    )) what = type::begin_tag;
                else return;
            }
            else if (data.starts_with(view_comment_begin)) what = type::comment_begin;
            else if (data.starts_with(view_close_tag    )) what = type::close_tag;
            else if (data.starts_with(view_begin_tag    )) what = type::begin_tag;
            else if (data.starts_with(view_empty_tag    )) what = type::empty_tag;
            else if (data.starts_with(view_slash        )) what = type::unknown;
            //else if (data.starts_with(view_slash        ))
            //{
            //    if (last == type::token) what = type::compact;
            //    else                     what = type::unknown;
            //}
            else if (data.starts_with(view_close_inline )) what = type::close_inline;
            else if (data.starts_with(view_quoted_text  )) what = type::quoted_text;
            else if (data.starts_with(view_equal        )) what = type::equal;
            else if (data.starts_with(view_defaults     )
                  && last == type::token)                  what = type::defaults;
            else if (whitespaces.find(data.front()) != view::npos) what = type::spaces;
            else if (last == type::close_tag
                  || last == type::begin_tag
                  || last == type::token
                  || last == type::defaults
                  || last == type::raw_text
                  || last == type::quoted_text)  what = type::token;
            else                                 what = type::raw_text;
        }
        auto name(view& data)
        {
            auto item = utf::get_tail(data, token_delims).str();
            utf::to_low(item);
            return item;
        }
        auto body(view& data, type kind = type::tag_value)
        {
            auto item_ptr = frag{};
            if (data.size())
            {
                auto delim = data.front();
                if (delim != '\'' && delim != '\"')
                {
                    auto crop = utf::get_tail(data, rawtext_delims);
                               page.append(type::quotes);
                    item_ptr = page.append(kind, crop);
                               page.append(type::quotes);
                }
                else
                {
                    auto delim_view = view(&delim, 1);
                    auto crop = utf::get_quote(data, delim_view);
                               page.append(type::quotes, delim_view);
                    item_ptr = page.append(kind, crop);
                               page.append(type::quotes, delim_view);
                }
            }
            else item_ptr = page.append(kind);
            return item_ptr;
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
                case type::defaults:      data.remove_prefix(view_defaults     .size()); break;
                case type::token:
                case type::top_token:
                case type::end_token:     utf::eat_tail(data, token_delims); break;
                case type::raw_text:
                case type::quotes:
                case type::tag_value:     body(data, type::raw_text);             break;
                case type::spaces:        utf::trim_front(data, whitespaces);     break;
                case type::na:            utf::get_tail<faux>(data, find_start);  break;
                //case type::compact:
                case type::unknown:       if (data.size()) data.remove_prefix(1); break;
                default: break;
            }
            return temp.substr(0, temp.size() - data.size());
        }
        auto trim(view& data)
        {
            auto temp = utf::trim_front(data, whitespaces);
            auto crop = !temp.empty();
            if (crop) page.append(type::spaces, std::move(temp));
            return crop;
        }
        auto diff(view& data, view& temp, type kind = type::spaces)
        {
            auto delta = temp.size() - data.size();
                 if (delta > 0) page.append(kind, temp.substr(0, delta));
            else if (delta < 0) fail("Unexpected data");
        }
        auto pair(sptr& item, view& data, type& what, type& last, type kind)
        {
            //todo
            //include external blocks if name contains ':'s.
            item->name = page.append(kind, name(data));
            auto temp = data;
            utf::trim_front(temp, whitespaces);
            peek(temp, what, last);
            if (what == type::defaults)
            {
                diff(data, temp, type::defaults);
                data = temp;
                item->fake = true;
                auto& last_type = page.back->kind;
                if (last_type == type::top_token || last_type == type::token)
                {
                    last_type = type::defaults;
                }
                page.append(type::defaults, skip(data, what));
                temp = data;
                utf::trim_front(temp, whitespaces);
                peek(temp, what, last);
                item->base = what == type::empty_tag;
            }
            if (what == type::equal)
            {
                diff(temp, data, type::spaces);
                data = temp;
                page.append(type::equal, skip(data, what));
                trim(data);
                peek(data, what, last);
                if (what == type::quoted_text || what == type::raw_text)
                {
                    item->body.push_back(body(data));
                }
                else fail(last, what);
            }
            else // Add placeholder for absent value.
            {
                                     page.append(type::equal);
                                     page.append(type::quotes);
                item->body.push_back(page.append(type::tag_value));
                                     page.append(type::quotes);
            }
        }
        auto open(sptr& item)
        {
            if (!page.data || page.back->kind != type::spaces)
            {
                page.append(type::spaces);
            }
            item->from = page.back;
        }
        auto seal(sptr& item)
        {
            item->upto = page.back;
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
            page.append(type::comment_begin, data.substr(0, size));
            data.remove_prefix(size);
            return true;
        }
        void read(sptr& item, view& data, si32 deep = {})
        {
            auto what = type::na;
            auto last = type::na;
            auto defs = std::unordered_map<text, wptr>{};
            auto fire = faux;
            auto push = [&](sptr& next)
            {
                auto& sub_name = next->name->utf8;
                if (next->fake) defs[sub_name] = next;
                else
                {
                    auto iter = defs.find(sub_name);
                    if (iter != defs.end())
                    {
                        next->defs = iter->second;
                    }
                }
                item->hive[sub_name].push_back(next);
            };
            trim(data);
            open(item);
            peek(data, what, last);
            if (what == type::begin_tag)
            {
                page.append(type::begin_tag, skip(data, what));
                trim(data);
                peek(data, what, last);
                if (what == type::token)
                {
                    pair(item, data, what, last, type::top_token);
                    trim(data);
                    peek(data, what, last);
                    if (what == type::token)
                    {
                        do // Proceed inlined subs.
                        {
                            auto next = ptr::shared<elem>();
                            next->mode = elem::form::attr;
                            open(next);
                            pair(next, data, what, last, type::token);
                            if (last == type::defaults) next->base = true; // Inlined list resetter.
                            seal(next);
                            push(next);
                            trim(data);
                            peek(data, what, last);
                        }
                        while (what == type::token);
                    }
                    if (what == type::empty_tag) // Proceed '/>'.
                    {
                        item->mode = elem::form::flat;
                        item->insA = last == type::spaces ? page.back
                                                          : page.append(type::spaces);
                        last = type::spaces;
                        page.append(type::empty_tag, skip(data, what));
                        while (true) // Pull inline comments: .../>  <!-- comments --> ... <!-- comments -->
                        {
                            auto temp = data;
                            auto idle = utf::trim_front(temp, whitespaces);
                            auto w = what;
                            auto l = last;
                            peek(temp, w, l);
                            if (idle.find('\n') == text::npos && w == type::comment_begin)
                            {
                                data = temp;
                                what = w;
                                last = l;
                                page.append(type::spaces, idle);
                                if (!note(data, what, last)) break;
                            }
                            else break;
                        }
                    }
                    else if (what == type::close_inline) // Proceed nested subs.
                    {
                        item->insA = last == type::spaces ? page.back
                                                          : page.append(type::spaces);
                        page.append(type::close_inline, skip(data, what));
                        do
                        {
                            auto temp = data;
                            utf::trim_front(temp, whitespaces);
                            peek(temp, what, last);
                            do
                            {
                                if (what == type::quoted_text)
                                {
                                    diff(temp, data, type::quoted_text);
                                    data = temp;
                                    item->body.push_back(body(data));
                                    trim(data);
                                    temp = data;
                                }
                                else if (what == type::raw_text)
                                {
                                    auto size = data.find('<');
                                    if (size == view::npos)
                                    {
                                        item->body.push_back(page.append(type::unknown, data));
                                        data = {};
                                        last = what;
                                        what = type::eof;
                                        break;
                                    }
                                    item->body.push_back(page.append(type::raw_text, data.substr(0, size)));
                                    data.remove_prefix(size);
                                    temp = data;
                                }
                                else if (what == type::begin_tag && deep < 30)
                                {
                                    trim(data);
                                    data = temp;
                                    auto next = ptr::shared<elem>();
                                    read(next, data, deep + 1);
                                    push(next);
                                    temp = data;
                                    utf::trim_front(temp, whitespaces);
                                }
                                else if (what == type::comment_begin) // Proceed '<!--'.
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
                                    utf::trim_front(temp, whitespaces);
                                }
                                else if (what != type::close_tag
                                      && what != type::eof)
                                {
                                    fail(last, what);
                                    skip(temp, what);
                                    diff(temp, data, type::unknown);
                                    data = temp;
                                }
                                peek(temp, what, last);
                            }
                            while (what != type::close_tag
                                && what != type::eof);
                            if (what == type::close_tag) // Proceed '</token>'.
                            {
                                auto skip_frag = skip(temp, what);
                                auto trim_frag = utf::trim_front(temp, whitespaces);
                                auto spaced = last == type::spaces;
                                peek(temp, what, last);
                                if (what == type::token)
                                {
                                    auto object = name(temp);
                                    auto spaced = trim(data);
                                    if (object == item->name->utf8)
                                    {
                                        item->insB = spaced ? page.back
                                                            : page.append(type::spaces);
                                                              page.append(type::close_tag, skip_frag);
                                        if (trim_frag.size()) page.append(type::spaces, trim_frag);
                                                              page.append(type::end_token, item->name->utf8);
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
                                                              page.append(what, object);
                                        data = temp;
                                        auto tail = data.find('>');
                                        if (tail != view::npos) data.remove_prefix(tail + 1);
                                        else                    data = {};
                                        diff(data, temp, what);
                                        fail(ansi::add("Unexpected closing tag name '", object, "', expected: '", item->name->utf8, "'"));
                                        continue; // Repeat until eof or success.
                                    }
                                }
                                else
                                {
                                    diff(temp, data, type::unknown);
                                    data = temp;
                                    fail(last, what);
                                    continue; // Repeat until eof or success.
                                }
                            }
                            else if (what == type::eof)
                            {
                                trim(data);
                                if (page.back->kind == type::eof) fail("Unexpected {EOF}");
                            }
                        }
                        while (data.size());
                    }
                    else fire = true;
                }
                else fire = true;
            }
            else fire = true;
            if (!item->name)
            {
                auto head = page.back;
                while (true) // Reverse find a broken open tag and mark all after it as an unknown data.
                {
                    auto kind = head->kind;
                    head->kind = type::unknown;
                    if (head == page.data || kind == type::begin_tag) break;
                    head = head->prev.lock();
                }
                item->name = page.append(type::tag_value);
                fail("Empty tag name");
            }
            if (fire) fail(last, what);
            if (what == type::eof) page.append(what);
            seal(item);
        }
        void read(view& data)
        {
            auto temp = data;
            auto what = type::na;
            auto last = type::na;
            auto deep = si32{};
            auto idle = utf::trim_front(temp, whitespaces);
            peek(temp, what, last);
            while (what != type::begin_tag && what != type::eof) // Skip all non-xml data.
            {
                if (what == type::na) fail(last, type::raw_text);
                else                  fail(last, what);
                page.append(type::unknown, idle);
                page.append(type::unknown, skip(temp, what));
                data = temp;
                idle = utf::trim_front(temp, whitespaces);
                peek(temp, what, last);
            }
            read(root, data);
        }
    };

    struct settings
    {
        using vect = xml::document::vect;
        using sptr = netxs::sptr<xml::document>;
        using hist = std::list<std::pair<text, text>>;

        sptr document; // settings: XML document.
        vect tempbuff; // settings: Temp buffer.
        vect homelist; // settings: Current directory item list.
        text homepath; // settings: Current working directory.
        text backpath; // settings: Fallback path.
        hist cwdstack; // settings: Stack for saving current cwd.

        settings() = default;
        settings(settings const&) = default;
        settings(view utf8_xml)
            : document{ ptr::shared<xml::document>(utf8_xml, "") }
        {
            homepath = "/";
            homelist = document->take(homepath);
        }

        auto cd(text gotopath, view fallback = {})
        {
            backpath = utf::trim(fallback, '/');
            if (gotopath.empty()) return faux;
            if (gotopath.front() == '/')
            {
                homepath = "/";
                homepath += utf::trim(gotopath, '/');
                homelist = document->take(homepath);
            }
            else
            {
                auto relative = utf::trim(gotopath, '/');
                if (homelist.size())
                {
                    homelist = homelist.front()->list(relative);
                }
                homepath += "/";
                homepath += relative;
            }
            auto test = !!homelist.size();
            if (!test)
            {
                log("%% %err%xml path not found: %path%%nil%", prompt::xml, ansi::err(), homepath, ansi::nil());
            }
            return test;
        }
        void popd()
        {
            if (cwdstack.empty())
            {
                log(prompt::xml, "CWD stack is empty");
            }
            else
            {
                auto& [gotopath, fallback] = cwdstack.back();
                cd(gotopath, fallback);
                cwdstack.pop_back();
            }
        }
        void pushd(text gotopath, view fallback = {})
        {
            cwdstack.push_back({ homepath, backpath });
            cd(gotopath, fallback);
        }
        template<bool Quiet = faux, class T = si32>
        auto take(text frompath, T defval = {})
        {
            if (frompath.empty()) return defval;
            auto crop = text{};
            if (frompath.front() == '/')
            {
                frompath = utf::trim(frompath, '/');
                tempbuff = document->take(frompath);
            }
            else
            {
                frompath = utf::trim(frompath, '/');
                if (homelist.size()) tempbuff = homelist.front()->list(frompath);
                if (tempbuff.empty() && backpath.size())
                {
                    frompath = backpath + "/" + frompath;
                    tempbuff = document->take(frompath);
                }
                else frompath = homepath + "/" + frompath;
            }
            if (tempbuff.size()) crop = tempbuff.back()->value();
            else
            {
                if constexpr (!Quiet) log("%prompt%%red% xml path not found: %nil%%path%", prompt::xml, ansi::fgc(redlt), ansi::nil(), frompath);
                return defval;
            }
            tempbuff.clear();
            if (auto result = xml::take<T>(crop)) return result.value();
            if (crop.size())                      return take<Quiet>("/config/set/" + crop, defval);
            else                                  return defval;
        }
        template<class T>
        auto take(text frompath, T defval, std::unordered_map<text, T> const& dict)
        {
            if (frompath.empty()) return defval;
            auto crop = take(frompath, ""s);
            auto iter = dict.find(crop);
            return iter == dict.end() ? defval
                                      : iter->second;
        }
        auto take(text frompath, cell defval)
        {
            if (frompath.empty()) return defval;
            auto fgc_path = frompath + '/' + "fgc";
            auto bgc_path = frompath + '/' + "bgc";
            auto txt_path = frompath + '/' + "txt";
            auto fba_path = frompath + '/' + "alpha";
            auto crop = cell{ defval.txt() };
            crop.fgc(take<true>(fgc_path, defval.fgc()));
            crop.bgc(take<true>(bgc_path, defval.bgc()));
            auto t = take<true>(txt_path, ""s);
            auto a = take<true>(fba_path, -1);
            if (t.size()) crop.txt(t);
            if (a != -1)  crop.alpha(std::clamp(a, 0, 255));
            return crop;
        }
        template<bool WithTemplate = faux>
        auto list(view frompath)
        {
            if (frompath.empty())        return homelist;
            if (frompath.front() == '/') return document->take<WithTemplate>(frompath);
            if (homelist.size())         return homelist.front()->list<WithTemplate>(frompath);
            else                         return vect{};
        }
        template<class T>
        void set(view frompath, T&& value)
        {
            auto items = list(frompath);
            if (items.empty())
            {
                //todo add new
            }
            else
            {
                items.front()->value(utf::concat(value));
            }
        }
        auto utf8()
        {
            return document->page.utf8();
        }
        template<bool Print = faux>
        auto fuse(view utf8_xml, view filepath = {})
        {
            if (filepath.size()) document->page.file = filepath;
            if (utf8_xml.empty()) return;
            homepath.clear();
            homelist.clear();
            auto run_config = xml::document{ utf8_xml, filepath };
            if constexpr (Print)
            {
                log(prompt::xml, "Settings from ", filepath.empty() ? "memory"sv : filepath, ":\n", run_config.page.show());
            }
            auto proc = [&](auto node_ptr, auto path, auto proc) -> void
            {
                auto& node = *node_ptr;
                auto& name = node.name->utf8;
                path += "/" + name;
                auto dest_list = list<true>(path);
                auto is_dest_list = (dest_list.size() && dest_list.front()->fake)
                                  || dest_list.size() > 1;
                if (is_dest_list)
                {
                    document->join(path, { node_ptr });
                }
                else
                {
                    auto value = node.value();
                    if (dest_list.size())
                    {
                        auto& dest = dest_list.front();
                        auto dst_value = dest->value();
                        auto src_value = node.value();
                        if (dst_value != src_value)
                        {
                            dest->value(src_value);
                        }
                        for (auto& [sub_name, sub_list] : node.hive) // Proceed subelements.
                        {
                            auto count = sub_list.size();
                            if (count == 1 && sub_list.front()->fake == faux)
                            {
                                proc(sub_list.front(), path, proc);
                            }
                            else if (count) // It is a list.
                            {
                                //todo Clang 13.0.0 don't get it.
                                //auto rewrite = sub_list.end() != std::ranges::find_if(sub_list, [](auto& a){ return a->base; });
                                auto rewrite = sub_list.end() != std::find_if(sub_list.begin(), sub_list.end(), [](auto& a){ return a->base; });
                                document->join(path + "/" + sub_name, sub_list, rewrite);
                            }
                            else log(prompt::xml, "Unexpected tag without data: ", sub_name);
                        }
                    }
                    else
                    {
                        document->join(path, { node_ptr });
                    }
                }
            };
            auto path = text{};
            proc(run_config.root, path, proc);
            homepath = "/";
            homelist = document->take(homepath);
        }
        friend auto& operator << (std::ostream& s, settings const& p)
        {
            return s << p.document->page.show();
        }
    };
    namespace options
    {
        static auto selmod = std::unordered_map<text, si32>
           {{ "none",      mime::disabled },
            { "text",      mime::textonly },
            { "ansi",      mime::ansitext },
            { "rich",      mime::richtext },
            { "html",      mime::htmltext },
            { "protected", mime::safetext }};

        static auto cursor = std::unordered_map<text, bool>
           {{ "underline", faux },
            { "block"    , true }};

        static auto align = std::unordered_map<text, bias>
           {{ "left",   bias::left   },
            { "right",  bias::right  },
            { "center", bias::center }};
    }
}
namespace netxs
{
    using xmls = xml::settings;
}