// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "directvt.hpp"

namespace netxs::ui
{
    using ansi::writ;
    using ansi::deco;

    class poly
    {
        cell basis;
        cell grade[256];

    public:
        poly() = default;
        poly(cell const& c)
            : basis{ c }
        {
            recalc(c);
        }

        void recalc(cell const& c)
        {
            for (auto k = 0; k < 256; k++)
            {
                auto& b = grade[k];
                b = c;
                b.bga((byte)(b.bga() * k >> 8));
                b.fga((byte)(b.fga() * k >> 8));
            }
        }

        operator auto& () const { return basis; }
        auto& operator [] (byte k) const
        {
            return grade[k];
        }
    };

    // richtext: The text feeder.
    class flow
        : protected ansi::runtime
    {
    protected:
        rect textline{ }; // flow: Textline placeholder.
        si32 textsize{ }; // flow: Full textline length (1D).
        rect boundary{ }; // flow: Affected area by the text output.
        si32 curpoint{ }; // flow: Current substring start position.
        si32 caret_mx{ }; // flow: Maximum x-coor value on the visible area.
        twod caretpos{ }; // flow: Current virtual (w/o style applied) cursor position.
        twod caretsav{ }; // flow: Cursor pos saver.
        rect cliprect{ }; // flow: Client area inside page margins.
        rect pagerect{ }; // flow: Client full area. Used as a nested areas (coords++) accumulator.
        rect pagecopy{ }; // flow: Client full area saver.
        deco selfcopy{ }; // flow: Flow state storage.
        deco runstyle{ }; // flow: Flow state.
        si32 highness{1}; // flow: Last processed line height.

        si32 const& size_x;
        si32 const& size_y;

        using hndl = void (*)(flow&, si32);

        // flow: Command list.
        static void exec_dx(flow& f, si32 a) { f.dx(a); }
        static void exec_dy(flow& f, si32 a) { f.dy(a); }
        static void exec_ax(flow& f, si32 a) { f.ax(a); }
        static void exec_ay(flow& f, si32 a) { f.ay(a); }
        static void exec_ox(flow& f, si32 a) { f.ox(a); }
        static void exec_oy(flow& f, si32 a) { f.oy(a); }
        static void exec_px(flow& f, si32 a) { f.px(a); }
        static void exec_py(flow& f, si32 a) { f.py(a); }
        static void exec_tb(flow& f, si32 a) { f.tb(a); }
        static void exec_nl(flow& f, si32 a) { f.nl(a); }
        static void exec_sc(flow& f, si32  ) { f.sc( ); }
        static void exec_rc(flow& f, si32  ) { f.rc( ); }
        static void exec_zz(flow& f, si32  ) { f.zz( ); }

        // flow: Draw commands (customizable).
        template<ansi::fn Cmd>
        static void exec_dc(flow& f, si32 a) { if (f.custom) f.custom(Cmd, a); }
        //static void exec_dc(flow& f, si32 a) { f.custom(Cmd, a); }

        // flow: Abstract handler
        //       ansi::fn::ed
        //       ansi::fn::el
        //virtual void custom(ansi::fn cmd, si32 arg) = 0;

        static constexpr auto exec = std::array<hndl, ansi::fn_count>
        {   // Order does matter, see definition of ansi::fn.
            exec_dx, // horizontal delta
            exec_dy, // vertical delta
            exec_ax, // x absolute
            exec_ay, // y absolute
            exec_ox, // old format x absolute (1-based)
            exec_oy, // old format y absolute (1-based)
            exec_px, // x percent
            exec_py, // y percent
            exec_tb, // horizontal tab
            exec_nl, // next line and reset x to west (carriage return)

            exec_sc, // save cursor position
            exec_rc, // load cursor position
            exec_zz, // all params reset to zero

            exec_dc<ansi::fn::ed>, // CSI Ps J  Erase in Display (ED)
            exec_dc<ansi::fn::el>, // CSI Ps K  Erase in Line    (EL)
        };

        // flow: Main cursor forwarding proc.
        template<bool Split, bool Wrap, bool RtoL, bool ReLF, class T, class P>
        void output(T const& block, P print)
        {
            textline.coor = caretpos;

            auto printout = rect{};
            auto outwidth = si32{};
            if constexpr (Wrap)
            {
                printout = textline.trunc(cliprect.size);
                outwidth = printout.coor.x + printout.size.x - textline.coor.x;

                if constexpr (!Split)
                if (printout.size.x > 1 && textline.size.x > printout.size.x)
                {
                    auto n = printout.size.x - 1;
                    auto p = curpoint + n;
                    while (n)
                    {
                        auto& c = block.at(p);
                        if (c.isspc() || c.matrix_end() || c.txt().ends_with(utf::utf8view<0x200B>)) break;
                        n--;
                        p--;
                    }
                    if (n > 0) // Cut by whitespace.
                    {
                        printout.size.x = n + (RtoL ? 0 : 1);
                    }
                    else // Try to cut on a matrix boundary (CJK/Emoji).
                    {
                        auto q = curpoint + printout.size.x - 1;
                        auto& c = block.at(q);
                        auto [w, h, x, y] = c.whxy();
                        if (w != 1 && h == 1 && x == 1 && y == 1)
                        {
                            --printout.size.x;
                        }
                    }
                }
            }
            else
            {
                printout = textline;
                outwidth = textline.size.x;
            }

            //flow::up();
            flow::dx(outwidth);
            //flow::up();

            auto startpos = curpoint;
            curpoint += std::max(printout.size.x, 1);
            textline.size.x = textsize - curpoint;

            if (RtoL) printout.coor.x = cliprect.size.x - (printout.coor.x + printout.size.x);
            if (ReLF) printout.coor.y = cliprect.size.y - (printout.coor.y + printout.size.y);
            else      printout.coor.y = textline.coor.y; //do not truncate y-axis?
            //todo revise: It is actually only for the coor.y that is negative.

            printout.coor += cliprect.coor;
            minmax(printout);

            if constexpr (!std::is_same_v<P, noop>)
            {
                //todo margins don't apply to unwrapped text
                //auto imprint = Wrap ? printout
                //                    : cliprect.clip(printout);
                if (printout)
                {
                    auto& coord = printout.coor;
                    auto& width = printout.size.x;
                    auto& start = straight ? startpos
                                           : textline.size.x;
                    print(coord, block.substr(start, width), runtime::isr_to_l);
                }
            }
            highness = textline.size.y;
        }

        auto middle() { return (cliprect.size.x >> 1) - (textline.size.x >> 1); }
        void autocr() { if (caretpos.x >= caret_mx) flow::nl(highness); }

        void cut_leading_spaces(auto const& block)
        {
            while (textline.size.x > 0 && block.at(curpoint).isspc())
            {
                textline.size.x--;
                curpoint++;
            }
        }
        template<bool Split, bool RtoL, bool ReLF, class T, class P>
        void centred(T const& block, P print)
        {
            while (textline.size.x > 0)
            {
                autocr();
                auto axis = textline.size.x >= caret_mx ? 0 : middle();
                flow::ax(axis);
                output<Split, true, RtoL, ReLF>(block, print);
                if constexpr (!Split) // Cut all leading spaces on wrapping.
                {
                    cut_leading_spaces(block);
                }
            }
        }
        template<bool Split, bool RtoL, bool ReLF, class T, class P>
        void wrapped(T const& block, P print)
        {
            while (textline.size.x > 0)
            {
                autocr();
                output<Split, true, RtoL, ReLF>(block, print);
                if constexpr (!Split) // Cut all leading spaces on wrapping.
                {
                    cut_leading_spaces(block);
                }
            }
        }
        template<bool Split, bool RtoL, bool ReLF, class T, class P>
        void trimmed(T const& block, P print)
        {
            if (textline.size.x > 0)
            {
                if (centered) flow::ax(middle());
                output<Split, faux, RtoL, ReLF>(block, print);
            }
        }
        template<bool Split, bool RtoL, bool ReLF, class T, class P>
        void proceed(T const& block, P print)
        {
            if (iswrapln) if (centered) centred<Split, RtoL, ReLF>(block, print);
                          else          wrapped<Split, RtoL, ReLF>(block, print);
            else                        trimmed<Split, RtoL, ReLF>(block, print);
        }

        std::function<void(ansi::fn cmd, si32 arg)> custom; // flow: Draw commands (customizable).

    public:
        flow(si32 const& size_x, si32 const& size_y)
            : size_x{ size_x },
              size_y{ size_y }
        { }
        flow(twod const& size )
            : flow{ size.x, size.y }
        { }
        flow()
            : flow{ pagerect.size }
        { }

        void   vsize(si32 height) { pagerect.size.y = height;      } // flow: Set client full height.
        void    size(twod size)   { pagerect.size = size;          } // flow: Set client full size.
        void    full(rect area)   { pagerect = area;               } // flow: Set client full rect.
        auto&   full() const      { return pagerect;               } // flow: Get client full rect reference.
        auto& minmax() const      { return boundary;               } // flow: Return the output range.
        void  minmax(twod p)      { boundary |= rect{ p, dot_11 }; } // flow: Register twod (cursor).
        void  minmax(rect r)      { boundary |= r;                 } // flow: Register rect.

        // flow: Sync paragraph style.
        template<class T>
        auto sync(T const& block)
        {
            combine(runstyle, block.style);
        }
        // flow: Split specified textblock on the substrings
        //       and place it to the form by the specified proc.
        template<bool Split, class T, class P = noop>
        void compose(T const& block, P print = {})
        {
            auto block_size = block.size();
            textsize = getlen(block_size);
            if (textsize)
            {
                textline = getvol(block_size);
                curpoint = 0;
                cliprect = textpads.area(size_x, size_y);
                cliprect.coor += pagerect.coor;
                caret_mx = cliprect.size.x;

                // Move cursor down if next line is lower than previous.
                if (highness > textline.size.y)
                //if (highness != textline.size.y)
                {
                    dy(highness - textline.size.y);
                    highness = textline.size.y;
                }

                if (arighted)
                {
                    if (isrlfeed) proceed<Split, true, true>(block, print);
                    else          proceed<Split, true, faux>(block, print);
                }
                else
                {
                    if (isrlfeed) proceed<Split, faux, true>(block, print);
                    else          proceed<Split, faux, faux>(block, print);
                }
            }
        }
        // flow: Execute specified locus instruction list.
        auto forward(writ const& cmds)
        {
            //flow::up();
            for (auto [cmd, arg] : cmds)
            {
                flow::exec[cmd](*this, arg);
            }
            return flow::up();
        }
        // flow: Execute specified locus instruction list (with EL: CSI 0 K).
        template<class P = noop>
        auto forward2(auto& block, core& canvas, P printfx)
        {
            auto& cmds = static_cast<writ const&>(block);
            for (auto [cmd, arg] : cmds)
            {
                if (cmd == ansi::fn::el && arg == 0)
                {
                    auto coor = flow::cp();
                    auto mark = cell{ block.brush() }.txt(' ');
                    auto line = arighted ? rect{{ textpads.l, coor.y }, { coor.x, 1 }}
                                         : rect{ coor, { caret_mx - coor.x + 1, 1 }};
                    line.coor.x += pagerect.coor.x;
                    if constexpr (std::is_same_v<P, noop>) netxs::onrect2(canvas, line, cell::shaders::fusefull(mark));
                    else                                   netxs::onrect2(canvas, line, printfx(mark));
                    flow::ax(caret_mx);
                }
                else flow::exec[cmd](*this, arg);
            }
            flow::up();
        }
        template<bool Split = true, class T>
        void go(T const& block)
        {
            compose<Split>(block);
        }
        template<bool Split = true, class T, class P = noop>
        void go(T const& block, core& canvas, P printfx = {})
        {
            compose<Split>(block, [&](auto const& coord, auto const& subblock, auto isr_to_l)
                                  {
                                      canvas.text(coord, subblock, isr_to_l, printfx);
                                  });
        }
        template<bool UseLocus = true, bool Split = faux, class T, class P = noop>
        auto print(T const& block, core& canvas, P printfx = {})
        {
            sync(block);
            auto coor = std::invoke_result_t<decltype(&flow::cp), flow>{};

            if constexpr (UseLocus) coor = forward(block);
            else                    coor = flow::cp();

            go<Split>(block, canvas, printfx);
            return coor;
        }
        template<bool Split = faux, class T, class P = noop>
        auto print2(T const& block, core& canvas, P printfx = {})
        {
            sync(block);
            forward2(block, canvas, printfx);
            go<Split>(block, canvas, printfx);
        }
        template<bool UseLocus = true, bool Split = faux, class T>
        auto print(T const& block)
        {
            sync(block);
            auto coor = std::invoke_result_t<decltype(&flow::cp), flow>{};

            if constexpr (UseLocus) coor = forward(block);
            else                    coor = flow::cp();

            go<Split>(block);
            return coor;
        }

        void ax(si32 x) { caretpos.x  = x;                 }
        void ay(si32 y) { caretpos.y  = y;                 }
        void ac(twod c) { ax(c.x); ay(c.y);                }
        void ox(si32 x) { caretpos.x  = x - 1;             }
        void oy(si32 y) { caretpos.y  = y - 1;             }
        void oc(twod c) { ox(c.x); oy(c.y);                }
        void dx(si32 n) { caretpos.x += n;                 }
        void dy(si32 n) { caretpos.y += n;                 }
        void nl(si32 n) { ax(0); dy(n);                    }
        void px(si32 x) { ax(textpads.h_ratio(x, size_x)); }
        void py(si32 y) { ay(textpads.v_ratio(y, size_y)); }
        void pc(twod c) { px(c.x); py(c.y);                }
        void tb(si32 n)
        {
            if (n)
            {
                dx(tabwidth - netxs::grid_mod(caretpos.x, tabwidth));
                if (n > 0 ? --n : ++n) dx(tabwidth * n);
            }
        }
        twod cp () const // flow: Return absolute cursor position.
        {
            auto coor = twod{ caretpos };
            if (arighted) coor.x = textpads.width (size_x) - 1 - coor.x;
            if (isrlfeed) coor.y = textpads.height(size_y) - 1 - coor.y;
            coor += textpads.corner();
            return coor;
        }
        twod up () // flow: Register cursor position.
        {
            auto cp = flow::cp();
            minmax(cp); /* |= cursor*/;
            return cp;
        }
        void zz (twod offset = dot_00)
        {
            runstyle.reset();
            caretpos = dot_00;
            pagerect.coor = offset;
        }
        void sc () // flow: Save current state.
        {
            selfcopy = runstyle;
            caretsav = caretpos;
            pagecopy.coor = pagerect.coor;
        }
        void rc () // flow: Restore state.
        {
            runstyle = selfcopy;
            caretpos = caretsav;
            pagerect.coor = pagecopy.coor;
        }
        auto get_state()
        {
            return std::tuple<deco,twod,twod>{ runstyle, caretpos, pagerect.coor };
        }
        template<class S>
        auto set_state(S const& state)
        {
            runstyle      = std::get<0>(state);
            caretpos      = std::get<1>(state);
            pagerect.coor = std::get<2>(state);
        }
        void reset(twod offset = dot_00) // flow: Reset flow state.
        {
            flow::zz(offset);
            flow::sc();
            boundary = { .coor = caretpos };
        }
        void reset(flow const& canvas) // flow: Reset flow state.
        {
            reset(canvas.pagerect.coor);
        }
        void reset(flow const& canvas, twod offset)
        {
            reset(canvas.pagerect.coor + offset);
        }
        void set_style(deco const& new_style)
        {
            runstyle = new_style;
        }
        auto get_style()
        {
            return runstyle;
        }
    };

    // richtext: The shadow of the para.
    class shot
    {
        core const& body;
        si32        skip;
        si32        used;

    public:
        shot(shot const&) = default;

        constexpr
        shot(core const& body, si32 start, si32 length)
            : body{ body },
              skip{ std::max(0, start) },
              used{ std::min(std::max(0, length), body.size().x - skip) }
        {
            if (body.size().x <= skip)
            {
                skip = 0;
                used = 0;
            }
        }

        constexpr
        shot(core const& body)
            : body{ body },
              skip{ 0    },
              used{ body.size().x }
        { }

        constexpr
        auto substr(si32 start, si32 length = si32max) const
        {
            auto w = body.size().x;
            auto a = skip + std::max(0, start);
            return a < w ? shot{ body, a, std::min(std::max(0, length), w - a) }
                         : shot{ body, 0, 0 };
        }
                  auto&  mark() const { return  body.mark();         }
                  auto  begin() const { return  body.begin() + skip; }
        constexpr auto   size() const { return  body.size();         }
        constexpr auto  empty() const { return !used;                }
        constexpr auto length() const { return  used;                }
        // shot: Compare content.
        template<class P>
        auto same(shot const& s, P compare) const
        {
            if (used != s.used) return faux;
            auto dest = s.body.begin();
            auto head =   body.begin();
            auto tail = head + used;
            while (head != tail)
            {
                if (!compare(*head++, *dest++)) return faux;
            }
            return true;
        }
        auto operator == (shot const& s) const { return same(s, [](auto const& a, auto const& b){ return a == b;        }); }
        auto  same       (shot const& s) const { return same(s, [](auto const& a, auto const& b){ return a.same_txt(b); }); }

        template<bool RtoL, class P = noop>
        auto output(core& canvas, twod coord, P print = {}) const  // shot: Print the source content using the specified print proc.
        {
            //todo place is wrong if RtoL==true
            //rect place{ pos, { RtoL ? used, body.size().y } };
            auto place = rect{ coord, { used, body.size().y } };
            auto joint = canvas.clip().trim(place);
            if (joint)
            {
                if constexpr (RtoL)
                {
                    place.coor.x -= joint.coor.x - place.size.x + joint.size.x;
                    place.coor.y  = joint.coor.y - place.coor.y;
                }
                else
                {
                    place.coor = joint.coor - place.coor;
                }
                joint.coor -= canvas.coor();
                place.coor.x += skip;
                if constexpr (std::is_same_v<P, noop>) netxs::inbody<RtoL>(canvas, body, joint, place.coor, cell::shaders::fusefull);
                else                                   netxs::inbody<RtoL>(canvas, body, joint, place.coor, print);
            }
        }
    };

    // richtext: Text line.
    class rich
        : public core
    {
    public:
        using core::core;

        rich(core&& s)
            : core{ std::forward<core>(s) }
        { }

        auto length() const                                     { return size().x;                         }
        auto shadow() const                                     { return shot{ *this };                    }
        auto substr(si32 at, si32 width = netxs::si32max) const { return shadow().substr(at, width);       }
        void trimto(si32 max_size)                              { if (length() > max_size) crop(max_size); }
        void resize(si32 oversize)                              { if (oversize > length()) crop(oversize); }
        auto take_piece(si32 at, si32 width = netxs::si32max) const
        {
            if (width == netxs::si32max) width = length() - at;
            return rich{ core::crop(at, width) };
        }
        auto copy_piece(rich& dest, si32 from, si32 width) const
        {
            auto my_size = size();
            if (from >= my_size.x * my_size.y)
            {
                dest.crop(0);
                return;
            }
            auto new_width = from % my_size.x + width;
            if (new_width > my_size.x)
            {
                width = my_size.x - from;
            }
            dest.crop(width);
            auto src = begin() + from;
            auto dst = dest.begin();
            auto end = dest.end();
            while (dst != end)
            {
                *dst++ = *src++;
            }
        }
        auto empty()
        {
            return canvas.empty();
        }
        void shrink(cell const& blank, si32 max_size = 0, si32 min_size = 0)
        {
            assert(min_size <= length());
            auto head = begin();
            auto tail = end();
            auto stop = head + min_size;
            while (stop != tail)
            {
                auto next = tail - 1;
                if (*next != blank) break;
                tail = next;
            }
            auto new_size = (si32)(tail - head);
            if (max_size && max_size < new_size) new_size = max_size;
            if (new_size != length()) crop(new_size);
        }
        template<bool AutoGrow = faux>
        void splice(si32 at, si32 count, cell const& blank)
        {
            if (count <= 0) return;
            auto len = length();
            if constexpr (AutoGrow)
            {
                rich::resize(at + count);
            }
            else
            {
                if (at >= len) return;
                count = std::min(count, len - at);
            }
            auto ptr = begin();
            auto dst = ptr + at;
            auto end = dst + count;
            while (dst != end) *dst++ = blank;
        }
        template<class Span, class Shader>
        void splice(si32 at, Span const& fragment, Shader fuse)
        {
            auto len = fragment.length();
            rich::resize(len + at);
            auto ptr = begin();
            auto dst = ptr + at;
            auto end = dst + len;
            auto src = fragment.begin();
            while (dst != end) fuse(*dst++, *src++);
        }
        template<bool Copy = faux, class SrcIt, class DstIt, class Shader>
        static void forward_fill_proc(SrcIt data, DstIt dest, DstIt tail, Shader fuse)
        {
            if constexpr (Copy)
            {
                while (dest != tail) fuse(*dest++, *data++);
            }
            else
            {
                while (dest < tail)
                {
                    auto c = *data++;
                    auto [w, h, x, y] = c.whxy();
                    if (x == 0 && y == 0)
                    {
                        if (w == 0 && h == 0)
                        {
                            //todo implement controls/commands
                            // winsrv2019's cmd.exe sets title with a zero at the end
                            //*dst++ = cell{ c, whitespace };
                        }
                        else if (h == 1)
                        {
                            if (w == 1)
                            {
                                fuse(*dest++, c.wdt(w, h, 1, 1));
                            }
                            else if (w != 0)
                            {
                                if (c.rtl())
                                {
                                    x = w;
                                    do fuse(*dest++, c.wdt(w, h, x--, 1));
                                    while (x != 0 && dest != tail);
                                }
                                else
                                {
                                    do fuse(*dest++, c.wdt(w, h, ++x, 1));
                                    while (x != w && dest != tail);
                                }
                            }
                        }
                    }
                    else if (x == 0) // x==0; Expand hz cell stripe.
                    {
                        if (w == 1)
                        {
                            fuse(*dest++, c.wdt(w, h, 1, y));
                        }
                        else
                        {
                            if (c.rtl())
                            {
                                x = w;
                                while (x != 0 && dest != tail) fuse(*dest++, c.wdt(w, h, x--, y));
                            }
                            else
                            {
                                while (x != w && dest != tail) fuse(*dest++, c.wdt(w, h, ++x, y));
                            }
                        }
                    }
                    else fuse(*dest++, c);
                }
            }
        }
        template<bool Copy = faux, class SrcIt, class DstIt, class Shader>
        static void unlimit_fill_proc(SrcIt data, si32 size, DstIt dest, DstIt tail, si32 back, Shader fuse)
        {
            if constexpr (Copy)
            {
                while (size-- > 0) fuse(*dest++, *data++);
            }
            else
            {
                auto set = [&](auto const& c)
                {
                    if (dest == tail) dest -= back;
                    fuse(*dest++, c);
                    --size;
                };
                while (size > 0)
                {
                    auto c = *data++;
                    auto [w, h, x, y] = c.whxy();
                    if (x == 0 && y == 0)
                    {
                        if (w == 0 && h == 0)
                        {
                            //todo implement controls/commands
                            // winsrv2019's cmd.exe sets title with a zero at the end
                            //*dst++ = cell{ c, whitespace };
                        }
                        else if (h == 1)
                        {
                            if (w == 1)
                            {
                                set(c.wdt(w, h, 1, 1));
                            }
                            else if (w != 0)
                            {
                                if (c.rtl())
                                {
                                    x = w;
                                    do set(c.wdt(w, h, x--, 1));
                                    while (x != 0 && size != 0);
                                }
                                else
                                {
                                    do set(c.wdt(w, h, ++x, 1));
                                    while (x != w && size != 0);
                                }
                            }
                        }
                    }
                    else if (x == 0) // x==0; Expand hz cell stripe.
                    {
                        if (w == 1)
                        {
                            set(c.wdt(w, h, 1, y));
                        }
                        else
                        {
                            if (c.rtl())
                            {
                                x = w;
                                while (x != 0 && size != 0) set(c.wdt(w, h, x--, y));
                            }
                            else
                            {
                                while (x != w && size != 0) set(c.wdt(w, h, ++x, y));
                            }
                        }
                    }
                    else set(c);
                }
            }
        }
        template<bool Copy = faux, class SrcIt, class DstIt, class Shader>
        static void reverse_fill_proc(SrcIt data, DstIt dest, DstIt tail, Shader fuse)
        {
            if constexpr (Copy)
            {
                while (dest != tail) fuse(*--dest, *--data);
            }
            else
            {
                while (dest > tail)
                {
                    auto c = *--data;
                    auto [w, h, x, y] = c.whxy();
                    if (x == 0 && y == 0)
                    {
                        if (w == 0 && h == 0)
                        {
                            //todo implement controls/commands
                            // winsrv2019's cmd.exe sets title with a zero at the end
                            //*dst++ = cell{ c, whitespace };
                        }
                        else if (h == 1)
                        {
                            if (w == 1)
                            {
                                fuse(*--dest, c.wdt(w, h, 1, 1));
                            }
                            else if (w != 0)
                            {
                                if (c.rtl())
                                {
                                    do fuse(*--dest, c.wdt(w, h, ++x, 1));
                                    while (x != w && dest != tail);
                                }
                                else
                                {
                                    x = w;
                                    do fuse(*--dest, c.wdt(w, h, x--, 1));
                                    while (x != 0 && dest != tail);
                                }
                            }
                        }
                    }
                    else if (x == 0) // x==0; Expand hz cell stripe.
                    {
                        if (w == 1)
                        {
                            fuse(*--dest, c.wdt(w, h, 1, y));
                        }
                        else
                        {
                            if (c.rtl())
                            {
                                while (x != w && dest != tail) fuse(*--dest, c.wdt(w, h, ++x, y));
                            }
                            else
                            {
                                x = w;
                                while (x != 0 && dest != tail) fuse(*--dest, c.wdt(w, h, x--, y));
                            }
                        }
                    }
                    else fuse(*--dest, c);
                }
            }
        }
        void unpack2d(auto const& proto, twod block_size)
        {
            core::size(block_size);
            //todo simplify (use netxs::onrect)
            auto iter = core::begin();
            auto bottom = block_size.x * (block_size.y - 1);
            auto width_check = block_size.x;
            for (auto c : proto)
            {
                auto [w, h, x, y] = c.whxy();
                assert(y == 0);
                if (x != 0) w = 1;
                width_check -= w;
                if (width_check < 0) break;
                auto stride = block_size.x - w;
                auto dx = w;
                auto dy = bottom + w;
                if (x == 0) // Fullsize character.
                {
                    y += 1;
                    netxs::inrect(iter, dx, dy, stride, [&](cell& b){ b = c.xy(++x, y); }, [&]{ x = 0; y++; });
                }
                else // Vertical stripe (char_size = twod{ 1, h }).
                {
                    netxs::inrect(iter, dx, dy, stride, [&](cell& b){ b = c.xy(x, ++y); });
                }
                iter += w;
            }
        }
        // rich: Splice proto with auto grow.
        template<bool Copy = faux, class Span, class Shader>
        void splice(si32 at, si32 count, Span const& proto, Shader fuse)
        {
            if (count <= 0) return;
            rich::resize(at + count);
            auto end = begin() + at;
            auto dst = end + count;
            auto src = proto.end();
            reverse_fill_proc<Copy>(src, dst, end, fuse);
        }
        template<bool Copy = faux, class Span, class Shader>
        void splice(twod at, si32 count, Span const& proto, Shader fuse)
        {
            if (count <= 0) return;
            auto end = begin() + at.x + at.y * size().x;
            auto dst = end + count;
            auto src = proto.end();
            reverse_fill_proc<Copy>(src, dst, end, fuse);
        }
        // rich: Scroll by gap the 2D-block of lines between top and end (exclusive); down: gap > 0; up: gap < 0.
        void scroll(si32 top, si32 end, si32 gap, cell const& clr)
        {
            auto data = core::begin();
            auto size = core::size();
            auto step = size.x;
            assert(top >= 0 && top < end && end <= size.y);
            assert(gap != 0);
            if (gap > 0)
            {
                auto src = top * size.x + data;
                auto cut = end - gap;
                if (cut > top)
                {
                    step *= gap;
                    auto head = data + size.x * cut;
                    auto dest = head + step;
                    auto tail = src;
                    while (head != tail) *--dest = *--head;
                }
                else step *= end - top;

                auto dst = src + step;
                while (dst != src) *src++ = clr;
            }
            else
            {
                auto src = end * size.x + data;
                auto cut = top - gap;
                if (cut < end)
                {
                    step *= gap;
                    auto head = data + size.x * cut;
                    auto dest = head + step;
                    auto tail = src;
                    while (head != tail) *dest++ = *head++;
                }
                else step *= top - end;

                auto dst = src + step;
                while (dst != src) *--src = clr;
            }
        }
        // rich: Shift 1D substring inside the line.
        void scroll(si32 from, si32 size, si32 step)
        {
            if (step == 0 || size == 0) return;
            if (step < 0)
            {
                std::swap(size, step);
                size = -size;
                from -= size;
            }
            auto need = from + size + step;
            if (need > core::size().x) crop(need);

            auto tail = core::begin() + from;
            auto iter = tail + size;
            while (iter != tail)
            {
                auto head = --iter;
                auto stop = head + step;
                while (head != stop)
                {
                    auto& src = *head;
                    auto& dst = *++head;
                    std::swap(src, dst);
                }
            }
        }
        // rich: (current segment) Insert n blanks at the specified position. Autogrow within segment only.
        void insert(si32 at, si32 count, cell const& blank, si32 margin)
        {
            if (count <= 0 || margin == 0) return;
            auto len = length();
            auto pos = at % margin;
            auto vol = std::min(count, margin - pos);
            auto max = std::min(len + vol, at + margin - pos);
            rich::resize(max);
            auto ptr = begin();
            auto dst = ptr + max;
            auto src = dst - vol;
            auto end = ptr + at;
            while (src != end) *--dst = *--src;
            while (dst != end) *--dst = blank;
        }
        // rich: (whole line) Insert n blanks at the specified position. Autogrow.
        void insert_full(si32 at, si32 count, cell const& blank)
        {
            if (count <= 0) return;
            auto len = length();
            crop(std::max(at, len) + count);
            auto ptr = begin();
            auto pos = ptr + at;
            auto end = pos + count;
            auto src = ptr + len;
            if (at < len)
            {
                auto dst = src + count;
                while (dst != end) *--dst = *--src;
                src = pos;
            }
            while (src != end) *src++ = blank;
        }
        // rich: (current segment) Delete n chars and add blanks at the right margin.
        void cutoff(si32 at, si32 count, cell const& blank, si32 margin)
        {
            if (count <= 0 || margin == 0) return;
            auto len = length();
            if (at < len)
            {
                auto pos = at % margin;
                auto rem = std::min(margin - pos, len - at);
                auto vol = std::min(count, rem);
                auto dst = begin() + at;
                auto end = dst + rem;
                auto src = dst + vol;
                while (src != end) *dst++ = *src++;
                while (dst != end) *dst++ = blank;
            }
        }
        // rich: (current segment) Delete n chars.
        void cutoff(si32 at, si32 count)
        {
            if (count <= 0) return;
            auto len = length();
            if (at < len)
            {
                auto rem = len - at;
                auto vol = std::min(count, rem);
                auto dst = begin() + at;
                auto end = dst + rem;
                auto src = dst + vol;
                while (src != end) *dst++ = *src++;
                crop(len - vol);
            }
        }
        // rich: (whole line) Delete n chars and add blanks at the right margin.
        void cutoff_full(si32 at, si32 count, cell const& blank, si32 margin)
        {
            if (count <= 0 || margin == 0) return;
            auto len = length();
            if (at < len)
            {
                margin -= at % margin;
                count = std::min(count, margin);
                if (count >= len - at)
                {
                    auto ptr = begin();
                    auto dst = ptr + at;
                    auto end = ptr + len;
                    while (dst != end) *dst++ = blank;
                }
                else
                {
                    rich::resize(margin + at);
                    auto ptr = begin();
                    auto dst = ptr + at;
                    auto src = dst + count;
                    auto end = dst - count + margin;
                    while (dst != end) *dst++ = *src++;
                    end += count; //end = ptr + right_margin;
                    while (dst != end) *dst++ = blank;
                }
            }
        }
        // rich: Put n blanks on top of the chars and cut them off with the right edge.
        void splice(twod at, si32 count, cell const& blank)
        {
            if (count <= 0) return;
            auto len = size();
            auto vol = std::min(count, len.x - at.x);
            assert(at.x + at.y * len.x + vol <= len.y * len.x);
            auto ptr = begin();
            auto dst = ptr + at.x + at.y * len.x;
            auto end = dst + vol;
            while (dst != end) *dst++ = blank;
        }
        // rich: Put n blanks on top of the chars and wrap them at the right edge.
        void backsp(twod at, si32 count, cell const& blank)
        {
            auto len = size();
            if (at.y >= len.y || (at.y == len.y - 1 && at.x >= len.x)) return;
            auto ps = std::clamp(at, dot_00, len - dot_11);
            auto d1 = at.y * len.x + ps.x;
            auto d2 = ps.y * len.x + ps.x;
            auto dt = d1 - d2;
            count -= dt;
            if (count <= 0) return;
            auto vol = std::min(count, len.x * len.y - d2);
            auto ptr = begin();
            auto dst = ptr + d2;
            auto end = dst + vol;
            while (dst != end) *dst++ = blank;
        }
        // rich: Insert n blanks by shifting chars to the right. Same as delete(twod), but shifts from left to right.
        void insert(twod at, si32 count, cell const& blank)
        {
            if (count <= 0) return;
            auto len = size();
            auto vol = std::min(count, len.x - at.x);
            assert(at.x + at.y * len.x + vol <= len.y * len.x);
            auto ptr = begin();
            auto pos = ptr + at.y * len.x;
            auto dst = pos + len.x;
            auto end = pos + at.x;
            auto src = dst - vol;
            while (src != end) *--dst = *--src;
            while (dst != end) *--dst = blank;
        }
        // rich: Insert fragment with shifting chars to the right.
        void insert(si32 at, rich const& fragment)
        {
            auto add = fragment.length();
            if (add == 0) return;
            if (at < 0) at = 0;
            auto len = length();
            auto max = len + add;
            if (at > len) max += at - len;
            rich::resize(max);
            auto pos = max - add;
            auto dst = begin() + pos;
            auto src = fragment.begin();
            auto end = fragment.end();
            while (src != end) *dst++ = *src++;
            if (at < len) scroll(at, len - at, add);
        }
        // rich: Delete n chars and add blanks at the right. Same as insert(twod), but shifts from right to left.
        void cutoff(twod at, si32 count, cell const& blank)
        {
            if (count <= 0) return;
            auto len = size();
            auto vol = std::min(count, len.x - at.x);
            assert(at.x + at.y * len.x + vol <= len.y * len.x);
            auto ptr = begin();
            auto pos = ptr + at.y * len.x;
            auto dst = pos + at.x;
            auto end = pos + len.x;
            auto src = dst + vol;
            while (src != end) *dst++ = *src++;
            while (dst != end) *dst++ = blank;
        }
        // rich: Clear from the specified coor to the bottom.
        void del_below(twod pos, cell const& blank)
        {
            auto len = size();
            auto ptr = begin();
            auto dst = ptr + std::min<si32>(pos.x + pos.y * len.x,
                                                    len.y * len.x);
            auto end = core::end();
            while (dst != end) *dst++ = blank;
        }
        // rich: Clear from the top to the specified coor.
        void del_above(twod pos, cell const& blank)
        {
            auto len = size();
            auto dst = begin();
            auto end = dst + std::min<si32>(pos.x + pos.y * len.x,
                                                    len.y * len.x);
            while (dst != end) *dst++ = blank;
        }
        //todo make it 2D
        // rich: Pop glyph matrix.
        auto pop_cluster()
        {
            auto cluster = netxs::text{};
            auto size = (si32)core::canvas.size();
            if (size)
            {
                auto& back = canvas.back();
                auto [w, h, x, y] = back.whxy();
                //if constexpr (debugmode) log("\tw=%%, h=%%, x=%%, y=%%", w, h, x, y);
                if (w && x == w && size >= w)
                {
                    auto current_x = w - 1;
                    auto head = canvas.rbegin() + 1;
                    auto tail = head + current_x;
                    while (head != tail)
                    {
                        auto& c = *head;
                        if (!c.same_txt(back) || !c.like(back))
                        {
                            break;
                        }
                        auto [cw, ch, cx, cy] = c.whxy();
                        //if constexpr (debugmode) log("\t\tcurrent_x=%%, cw=%%, ch=%%, cx=%%, cy=%%", current_x, cw, ch, cx, cy);
                        if (cw != w || ch != h || cy != y || cx != current_x)
                        {
                            break;
                        }
                        head++;
                        current_x--;
                    }
                    if (head == tail)
                    {
                        cluster = back.txt();
                        if (cluster.size())
                        {
                            core::crop(size - w);
                        }
                    }
                }
            }
            return cluster;
        }
        //todo unify
        auto& at(si32 p) const
        {
            assert(p >= 0);
            return *(core::begin() + p);
        }
    };

    // richtext: Text paragraph.
    class para
        : public ansi::parser
    {
        using corx = netxs::sptr<rich>;

    public:
        si32 caret = 0; // para: Cursor position inside lyric.
        ui32 index = 0;
        writ locus;
        corx lyric = ptr::shared<rich>();

        using parser::parser;
        para()                         = default;
        para(para&&)                   = default;
        para(para const&)              = default;
        para& operator = (para&&)      = default;
        para& operator = (para const&) = default;
        para(ui32 newid, deco const& style = {}, ansi::mark const& brush = {})
            : parser{ style, brush },
              index { newid }
        { }
        virtual ~para() = default;

        para(id_t id, auto utf8)      { brush.link(id); ansi::parse(utf8, this);               }
        para(auto utf8)               {                 ansi::parse(utf8, this);               }
        auto& operator  = (auto utf8) { wipe(brush);    ansi::parse(utf8, this); return *this; }
        auto& operator += (auto utf8)
        {
            if (parser::defer && caret && caret == length())
            {
                //if constexpr (debugmode) log("try to reassemble cluster=", lyric->back().txt());
                auto last_cluster = lyric->pop_cluster();
                if (caret != length())
                {
                    caret = length();
                    auto reassembled_cluster = text{};
                    reassembled_cluster.reserve(last_cluster.length() + utf8.length());
                    reassembled_cluster += last_cluster;
                    reassembled_cluster += utf8;
                    ansi::parse(reassembled_cluster, this);
                    //if constexpr (debugmode) log("\treassembled_cluster=", utf::buffer_to_hex(reassembled_cluster, true));
                    return *this;
                }
            }
            ansi::parse(utf8, this);
            return *this;
        }

        operator writ const& () const { return locus; }

        void decouple() { lyric = ptr::shared<rich>(*lyric); } // para: Make canvas isolated copy.
        void  content(rich& r){ *lyric = r; caret = r.length(); } // para: Set paragraph content.
        auto& content() const { return *lyric; } // para: Return paragraph content.
        shot   shadow() const { return *lyric; } // para: Return paragraph shadow.
        shot   substr(si32 start, si32 width) const // para: Return paragraph substring shadow.
        {
            return shadow().substr(start, width);
        }
        bool   bare() const { return locus.bare();    } // para: Does the paragraph have no locator.
        si32 length() const { return lyric->size().x; } // para: Return printable length. //todo Apple clang doesn't get auto return.
        auto  empty() const { return !length();       } // para: Return true if empty.
        auto   size() const { return lyric->size();   } // para: Return 2D volume size.
        auto&  back() const { return brush;           } // para: Return current brush.
        bool   busy() const { return length() || !parser::empty() || brush.busy(); } // para: Is it filled.
        void   link(id_t id) { lyric->each([&](auto& c){ c.link(id); });  } // para: Set object ID for each cell.
        template<bool ResetStyle = faux>
        void wipe(cell c = cell{}) // para: Clear the text and locus, and reset SGR attributes.
        {
            parser::reset<ResetStyle>(c);
            caret = 0;
            locus.kill();
            lyric->kill();
        }
        // para: Add locus command. In case of text presence try to change current target otherwise abort content building.
        void task(ansi::rule const& cmd)
        {
            if (cmd.cmd == ansi::fn::sc) // Save caret position as a command argument.
            {
                parser::flush();
                locus.push({ ansi::fn::sc, caret });
            }
            else if (!busy()) locus.push(cmd);
        }
        // para: Convert into the screen-adapted sequence (unfold, remove zerospace chars, etc.).
        void data(si32 width, si32 /*height*/, core::body const& proto) override
        {
            lyric->splice(caret, width, proto, cell::shaders::full);
            caret += width;
        }
        void id(ui32 newid) { index = newid; }
        auto id() const     { return index;  }

        auto& set(cell const& c) { brush.set(c); return *this; }

        //todo unify
        auto& at(si32 p) const { return *(lyric->begin(p)); } // para: .

        // para: Normalize cursor position.
        void caret_check()
        {
            caret = std::clamp(caret, 0, length());
        }
        // para: Move cursor to the beginning.
        auto move_to_home(bool erase)
        {
            if (erase)
            {
                caret_check();
                auto oldpos = caret;
                auto& line = content();
                line.cutoff(0, oldpos);
            }
            caret = 0;
        }
        // para: Move cursor to the end.
        auto move_to_end(bool erase)
        {
            if (erase)
            {
                caret_check();
                auto& line = content();
                line.crop(caret);
            }
            caret = length();
        }
        // para: Move cursor one cell to the left.
        auto step_by_cell_rev()
        {
            caret_check();
            if (caret > 0)
            {
                caret--;
                return true;
            }
            else return faux;
        }
        // para: Move cursor one cell to the right.
        auto step_by_cell_fwd()
        {
            caret_check();
            if (caret < length())
            {
                caret++;
                return true;
            }
            else return faux;
        }
        // para: Move cursor one grapheme cluster to the left.
        auto step_by_gc_rev()
        {
            caret_check();
            if (caret > 0)
            {
                caret--;
                auto& line = content();
                auto  iter = line.begin() + caret;
                if (caret > 0)
                {
                    auto [w, h, x, y] = iter->whxy();
                    if (w == 2 && x == 2 && caret > 0)
                    {
                        --iter;
                        auto [w2, h2, x2, y2] = iter->whxy();
                        if (w2 == 2 && x2 == 1)
                        {
                            caret--;
                        }
                    }
                }
                return true;
            }
            else return faux;
        }
        // para: Delete one grapheme cluster to the left.
        auto del_gc_rev()
        {
            caret_check();
            auto oldpos = caret;
            if (step_by_gc_rev())
            {
                auto newpos = caret;
                auto& line = content();
                line.cutoff(newpos, oldpos - newpos);
                return true;
            }
            else return faux;
        }
        // para: Move cursor one grapheme cluster to the right.
        auto step_by_gc_fwd()
        {
            caret_check();
            if (caret < length())
            {
                auto& line = content();
                auto  iter = line.begin() + caret;
                caret++;
                if (caret < length())
                {
                    auto [w, h, x, y] = iter->whxy();
                    if (w == 2 && x == 1)
                    {
                        ++iter;
                        auto [w2, h2, x2, y2] = iter->whxy();
                        if (w2 == 2 && x2 == 2)
                        {
                            caret++;
                        }
                    }
                }
                return true;
            }
            else return faux;
        }
        // para: Delete one grapheme cluster to the right.
        auto del_gc_fwd()
        {
            caret_check();
            auto oldpos = caret;
            if (step_by_gc_fwd())
            {
                auto newpos = caret;
                auto& line = content();
                line.cutoff(oldpos, newpos - oldpos);
                caret = oldpos;
                return true;
            }
            else return faux;
        }
        // para: Insert one proto cell before cursor (the proto means that it will be expanded if it is wide - wdt == 2).
        auto insert(cell c, bool inserting = true)
        {
            if (!inserting) del_gc_fwd();
            else            caret_check();
            auto [w, h, x, y] = c.whxy();
            auto& line = content();
            if (w == 2)
            {
                line.insert_full(caret, 2, c.wdt(2, 1, 1, 1));
                caret++;
                line.begin(caret)->wdt(2, 1, 2, 1);
                caret++;
            }
            else line.insert_full(caret++, 1, c.wdt(1, 1, 1, 1));
        }
        // para: Insert text.
        void insert(view utf8, bool inserting)
        {
            caret_check();
            if (caret != length())
            {
                auto& line = content();
                auto left = line.take_piece(0, caret);
                auto right = line.take_piece(caret);
                std::swap(left, line);
                parser::defer = true;
                auto prev_caret = caret;
                operator+=(utf8);
                caret = line.length();
                if (inserting)
                {
                    line.rich::insert(caret, right);
                }
                else
                {
                    auto delta = caret - prev_caret;
                    if (delta < right.length())
                    {
                        auto [w, h, x, y] = right.begin(delta)->whxy();
                        if (w != 1 && x != 1) // Broken cluster.
                        {
                            delta++;
                        }
                        if (delta < right.length())
                        {
                            line.rich::insert(caret, right.take_piece(delta));
                        }
                    }
                }
            }
            else operator+=(utf8);
        }
        // para: Move cursor one word to the left.
        auto step_by_word_rev()
        {
            caret_check();
            if (caret > 0)
            {
                auto& line = content();
                caret = line.word<feed::rev>(caret - 1);
                return true;
            }
            else return faux;
        }
        // para: Delete one word to the left.
        auto del_word_rev()
        {
            caret_check();
            auto oldpos = caret;
            if (step_by_word_rev())
            {
                auto newpos = caret;
                auto& line = content();
                line.cutoff(newpos, oldpos - newpos);
                return true;
            }
            else return faux;
        }
        // para: Move cursor one word to the right.
        auto step_by_word_fwd()
        {
            caret_check();
            if (caret < length())
            {
                auto& line = content();
                caret = line.word<feed::fwd>(caret) + 1;
                caret_check();
                return true;
            }
            else return faux;
        }
        // para: Delete one word to the right.
        auto del_word_fwd()
        {
            caret_check();
            auto oldpos = caret;
            if (step_by_word_fwd())
            {
                auto newpos = caret;
                auto& line = content();
                line.cutoff(oldpos, newpos - oldpos);
                caret = oldpos;
                return true;
            }
            else return faux;
        }
        // para: Move cursor one word(true) or grapheme cluster(faux) to the left.
        auto step_rev(bool by_word)
        {
            return by_word ? step_by_word_rev()
                           : step_by_gc_rev();
        }
        // para: Move cursor one word(true) or grapheme cluster(faux) to the right.
        auto step_fwd(bool by_word, rich const& fallback)
        {
                 if (by_word)           return step_by_word_fwd();
            else if (caret != length()) return step_by_gc_fwd();
            else
            {
                auto& data = content();
                auto iter1 = data.begin();
                auto end_1 = iter1 + length();
                auto iter2 = fallback.begin();
                auto end_2 = iter2 + fallback.length();
                while (iter2 != end_2)
                {
                    if (iter1 == end_1)
                    {
                        insert(*iter2);
                        return true;
                    }
                    auto [w1, h1, x1, y1] = (iter1++)->whxy();
                    if (w1 == 2 && x1 == 1 && iter1 != end_1)
                    {
                        auto [w3, h3, x3, y3] = (iter1++)->whxy();
                        if (w3 != 2 || x3 != 2) log(prompt::para, "Corrupted glyph");
                    }
                    auto [w2, h2, x2, y2] = (iter2++)->whxy();
                    if (w1 == 2 && x2 == 1 && iter2 != end_2)
                    {
                        auto [w3, h3, x3, y3] = (iter2++)->whxy();
                        if (w3 != 2 || x3 != 2) log(prompt::para, "Corrupted glyph");
                    }
                }
            }
            return faux;
        }
        // para: Delete one word(true) or grapheme cluster(faux) to the left.
        auto wipe_rev(bool by_word)
        {
            return by_word ? del_word_rev()
                           : del_gc_rev();
        }
        // para: Delete one word(true) or grapheme cluster(faux) to the right.
        auto wipe_fwd(bool by_word)
        {
            return by_word ? del_word_fwd()
                           : del_gc_fwd();
        }
    };

    // richtext: Cascade of the identical paragraphs.
    class rope
    {
        using iter = std::list<netxs::sptr<para>>::const_iterator;
        iter source; // rope: First segment.
        si32 prefix; // rope: .
        iter finish; // rope: Last segment (inclusive).
        si32 suffix; // rope: .
        twod volume; // Rope must consist of text lines of the same height.

        rope(iter& source, si32 prefix, iter& finish, si32 suffix, twod volume)
            : source{ source },
              prefix{ prefix },
              finish{ finish },
              suffix{ suffix },
              volume{ volume },
              style{(**source).style}
        { }

    public:
        deco style;
        twod coord;

        rope(iter head, iter tail, twod size)
            : source{ head },
              prefix{ 0    },
              finish{ tail },
              suffix{ 0    },
              volume{ size },
              style{ (**head).style }
        { }

        operator writ const& () const { return **source; }

        // rope: Return a substring rope the source content.
        //       ! No checking of boundaries !
        rope substr(si32 start, si32 width) const
        {
            auto first = source;
            auto piece = (**first).size().x;

            while (piece <= start)
            {
                start -= piece;
                piece = (**++first).size().x;
            }

            auto end = first;
            piece -= start;
            while (piece < width)
            {
                piece += (**++end).size().x;
            }
            piece -= width;

            return rope{ first, start, end, piece, { width, volume.y } };
        }
        // rope: Print the source content using the specified print proc,
        //       which returns the number of characters printed.
        template<bool RtoL, class P = noop>
        void output(core& canvas, twod coord, P print = {}) const
        {
            auto total = volume.x;

            auto draw = [&](auto& item, auto start, auto width)
            {
                auto line = item.substr(start, width);
                line.template output<RtoL>(canvas, coord, print);
                auto size = line.length();
                coord.x += size;
                return size;
            };

            if constexpr (RtoL)
            {
                auto crop = [](auto piece, auto total, auto& start, auto& width)
                {
                    if (piece > total)
                    {
                        start = piece - total;
                        width = total;
                    }
                    else
                    {
                        start = 0;
                        width = piece;
                    }
                };

                auto refer = finish;
                auto& item = **refer;
                auto piece = item.size().x - suffix;
                auto start = 0;
                auto width = 0;
                auto yield = 0;
                crop(piece, total, start, width);
                yield = draw(item, start, width);

                while (total -= yield)
                {
                    auto& next = **--refer;
                    piece = next.size().x;

                    crop(piece, total, start, width);
                    yield = draw(next, start, width);
                }
            }
            else
            {
                auto refer = source;
                auto& item = **refer;
                auto yield = draw(item, prefix, total);

                while (total -= yield)
                {
                    auto& next = **++refer;
                    yield = draw(next, 0, total);
                }
            }
        }
        constexpr
        auto   size() const { return volume;                } // rope: Return volume of the source content.
        auto length() const { return volume.x;              } // rope: Return the length of the source content.
        auto     id() const { return (**source).id();       } // rope: Return paragraph id.
        auto& front() const { return (**source).at(prefix); } // rope: Return first cell.
        auto& brush() const { return (**source).brush;      } // rope: Return source brush.

        //todo unify
        auto& at(si32 p) const // rope: .
        {
            auto shadow = substr(p, 1);
            return shadow.front();
        }
    };

    // richtext: Text page.
    class page
        : public ansi::parser
    {
        using list = std::list<netxs::sptr<para>>;
        using iter = list::iterator;
        using pmap = std::map<si32, netxs::wptr<para>>;
        using redo = std::list<std::pair<deco, ansi::mark>>;

    public:
        ui32 index{};              // page: Current paragraph id.
        list batch{ ptr::shared<para>(index) }; // page: Paragraph source list.
        pmap parts{};              // page: Paragraph index.
        redo stack{};              // page: Style state stack.
        iter layer{ batch.begin() };   // page: Current paragraph.
        std::vector<rope> ropes;   // page: Printable paragraphs.

        using ring = generics::ring<std::vector<para>>;
        struct buff : public ring
        {
            using ring::ring;
            //void undock_base_front(para& p) override { }
            //void undock_base_back (para& p) override { }
        };

        template<class T>
        static void parser_config(T& vt)
        {
            using namespace netxs::ansi;

            #define V []([[maybe_unused]] auto& q, [[maybe_unused]] auto& p)
            //vt.intro[ctrl::nul]             = V{ p->post(utf::frag{ emptyspace, utf::prop{ 0, 1 } }); };
            vt.intro[ctrl::cr ]              = V{ q.pop_if(ctrl::eol); p->task({ fn::nl,1 }); };
            vt.intro[ctrl::tab]              = V{ p->task({ fn::tb, q.pop_all(ctrl::tab) }); };
            vt.intro[ctrl::eol]              = V{ p->task({ fn::nl, q.pop_all(ctrl::eol) }); };
            vt.csier.table[csi__ed]          = V{ p->task({ fn::ed, q(0) }); }; // CSI Ps J
            vt.csier.table[csi__el]          = V{ p->task({ fn::el, q(0) }); }; // CSI Ps K
            vt.csier.table[csi_ccc][ccc_nop] = V{ p->fork(); };
            vt.csier.table[csi_ccc][ccc_idx] = V{ p->fork(q.subarg(0)); };
            vt.csier.table[csi_ccc][ccc_ref] = V{ p->bind(q.subarg(0)); };
            vt.csier.table_hash[csi_hsh_psh] = V{ p->pushsgr(); }; // CSI # {  Push current SGR attributes and style onto stack.
            vt.csier.table_hash[csi_hsh_pop] = V{ p->popsgr();  }; // CSI # }  Pop  current SGR attributes and style from stack.
            #undef V
        }

        auto& operator  = (view utf8) { clear(); ansi::parse(utf8, this); reindex(); return *this; }
        auto& operator += (view utf8) {          ansi::parse(utf8, this); reindex(); return *this; }
        page(view utf8)               {          ansi::parse(utf8, this); reindex();               }
        page(view utf8, cell c)
        {
            parser::brush.reset(c);
            batch.front()->parser::brush.reset(c);
            ansi::parse(utf8, this);
            reindex();
        }
        page() = default;
        page(page&& p)
            : parser{        },
              index{ p.index },
              batch{ std::move(p.batch) },
              parts{ std::move(p.parts) },
              stack{ std::move(p.stack) },
              layer{ std::prev(batch.end()) }
        {
            reindex();
        }
        page(page const& p)
            : parser{        },
              index{ p.index },
              batch{ p.batch },
              parts{ p.parts },
              stack{ p.stack },
              layer{ std::prev(batch.end()) }
        {
            reindex();
        }
        page& operator = (page const& p)
        {
            index = p.index;
            batch = p.batch;
            parts = p.parts;
            stack = p.stack;
            layer = std::prev(batch.end());
            reindex();
            return *this;
        }
        auto& operator += (page const& p)
        {
            parts.insert(p.parts.begin(), p.parts.end()); // Part id should be unique across pages
            //batch.splice(std::next(layer), p.batch);
            for (auto& a : p.batch)
            {
                batch.push_back(a);
                batch.back()->id(++index);
            }
            layer = std::prev(batch.end());
            return *this;
        }
        // page: Acquire para by id.
        auto& operator [] (si32 id)
        {
            if (netxs::on_key(parts, id))
            {
                if (auto item = parts[id].lock()) return *item;
            }
            fork(id);
            parts.emplace(id, *layer);
            return **layer;
        }
        // page: CSI # {  Push SGR attributes.
        void pushsgr()
        {
            parser::flush();
            stack.push_back({ parser::style, parser::brush });
            if (stack.size() == 10) stack.pop_front();
        }
        // page: CSI # }  Pop SGR attributes.
        void popsgr()
        {
            parser::flush();
            if (stack.size())
            {
                auto& [s, b] = stack.back();
                parser::style = s;
                parser::brush = b;
                parser::flush_style();
                stack.pop_back();
            }
        }
        // page: Clear the list of paragraphs.
        page& clear(bool preserve_state = faux)
        {
            if (!preserve_state) parser::reset();
            parts.clear();
            batch.resize(1);
            layer = batch.begin();
            index = 0;
            auto& item = **layer;
            item.id(index);
            item.wipe<true>(parser::brush);
            reindex();
            return *this;
        }
        // page: Disintegrate the page content into atomic contiguous pieces - ropes.
        //       Call publish(rope{first, last, length}):
        //       a range of [ first,last ] is the uniform consecutive paragraphs set.
        //       Length is the sum of the lengths of the paragraphs.
        template<class P>
        void stream(P publish)
        {
            for (auto& p : ropes)
            {
                publish(p);
            }
        }
        // page: Print to.
        template<class P = noop>
        void print(flow& printer, core& canvas, P printfx = {})
        {
            auto publish = [&](auto& combo)
            {
                combo.coord = printer.flow::print(combo, canvas, printfx);
            };
            stream(publish);
        }
        // page: Split the text run.
        template<bool Flush = true>
        void fork()
        {
            if constexpr (Flush) parser::flush();
            layer = batch.insert(std::next(layer), ptr::shared<para>(parser::style, parser::brush));
            (**layer).id(++index);
        }
        // page: Split the text run and associate the next paragraph with id.
        void fork(si32 id)
        {
            fork();
            parts[id] = *layer;
        }
        void test()
        {
            parser::flush();
            if ((**layer).busy()) fork<faux>();
        }
        // page: Make a shared copy of lyric of existing paragraph.
        void bind(si32 id)
        {
            test();
            auto it = parts.find(id);
            if (it != parts.end())
            {
                if (auto item = it->second.lock())
                {
                    (**layer).lyric = item->lyric;
                    return;
                }
            }
            parts.emplace(id, *layer);
        }
        // page: Add a locus command. In case of text presence change current target.
        void task(ansi::rule cmd)
        {
            test();
            auto& item = **layer;
            item.locus.push(cmd);
        }
        // page: .
        void meta(deco const& /*old_style*/) override
        {
            auto& item = **layer;
            item.style = parser::style;
        }
        // page: .
        void data(si32 width, si32 /*height*/, core::body const& proto) override
        {
            auto& item = **layer;
            item.lyric->splice(item.caret, width, proto, cell::shaders::full);
            item.caret += width;
        }
        auto& current()       { return **layer; } // page: Access to the current paragraph.
        auto& current() const { return **layer; } // page: RO access to the current paragraph.
        auto  size()    const { return (si32)batch.size(); }
        // page: Estimated page size calculation (use fake printing for accurate calc).
        auto  limits() const
        {
            auto size = twod{};
            auto head = batch.begin();
            auto tail = batch.end();
            while (head != tail)
            {
                auto s = (**head++).size();
                size.x = std::max(size.x, s.x);
                size.y += s.y;
            }
            return size;
        }
        // page: Re-glue paragraphs.
        void reindex()
        {
            ropes.clear();
            ropes.reserve(batch.size());
            auto next = dot_00;
            auto last = batch.begin();
            auto tail = batch.end();
            while (last != tail)
            {
                auto r_to_l = (**last).style.r_to_l;
                auto adjust = (**last).style.adjust;
                auto size = (**last).size();
                auto head = last;
                while (++last != tail
                   && (**last).bare()
                   && size.y == (next = (**last).size()).y
                   && r_to_l == (**last).style.r_to_l
                   && adjust == (**last).style.adjust)
                {
                    size.x += next.x;
                }
                ropes.emplace_back(head, std::prev(last), size);
            }
        }
        auto lookup(twod anker)
        {
            struct entry
            {
                ui32 id;
                twod coor;
            };
            auto bound = [](auto& r){ return r.coord.y; };
            auto found = std::ranges::lower_bound(ropes, anker.y, {}, bound);
            if (found != ropes.end()) return entry{ found->id(), found->coord };
            else                      return entry{ 0,     twod{ 0, si32max } };
        }

        struct rtf_dest_t
        {
            using cmap = std::unordered_map<ui32, size_t>;

            static constexpr auto fg_1 = "\\cf"sv;
            static constexpr auto fg_2 = "\\chcfpat"sv;
            static constexpr auto bg_1 = "\\cb"sv;
            static constexpr auto bg_2 = "\\chcbpat"sv;

            wide buff;
            text data;
            cmap clrs;
            cell base;

            auto operator += (qiew utf8)
            {
                buff.clear();
                utf::to_utf(utf8, buff);
                for (auto c : buff)
                {
                         if (c =='\\') { data.push_back('\\'); data.push_back('\\'); }
                    else if (c == '{') { data.push_back('\\'); data.push_back('{' ); }
                    else if (c == '}') { data.push_back('\\'); data.push_back('}' ); }
                    else if (c < 0x80) { data.push_back((char)c); }
                    else
                    {
                        data.push_back('\\'); data.push_back('u');
                        data += std::to_string((si16)c);
                        data.push_back('?');
                    }
                }
            }
            auto clr(argb c, view tag1, view tag2)
            {
                auto size = clrs.size();
                auto iter = clrs.try_emplace(c.token, size).first;
                auto istr = std::to_string(iter->second + 1) + ' ';
                data += tag1;
                data += istr;
                data += tag2;
                data += istr;
            }
            template<svga Mode = svga::vtrgb>
            auto fgc(argb c)
            {
                base.inv() ? clr(c, bg_1, bg_2)
                           : clr(c, fg_1, fg_2);
            }
            template<svga Mode = svga::vtrgb>
            auto bgc(argb c)
            {
                base.inv() ? clr(c, fg_1, fg_2)
                           : clr(c, bg_1, bg_2);
            }
            auto bld(bool b)
            {
                static constexpr auto set = "\\b "sv;
                static constexpr auto off = "\\b0 "sv;
                data += b ? set : off;
            }
            auto itc(bool b)
            {
                static constexpr auto set = "\\i "sv;
                static constexpr auto off = "\\i0 "sv;
                data += b ? set : off;
            }
            auto unc(argb ) { }
            auto dim(si32 ) { }
            auto und(si32 unline)
            {
                static constexpr auto off = "\\ul0 "sv;
                static constexpr auto sgl = "\\ul "sv;
                static constexpr auto dbl = "\\uldb "sv;
                static constexpr auto wavy = "\\ulwave "sv;
                static constexpr auto dotted = "\\uld "sv;
                static constexpr auto dashed = "\\uldash "sv;
                     if (unline == unln::line  ) data += sgl;
                else if (unline == unln::biline) data += dbl;
                else if (unline == unln::wavy  ) data += wavy;
                else if (unline == unln::dotted) data += dotted;
                else if (unline == unln::dashed) data += dashed;
                else                             data += off;
            }
            auto inv(bool b)
            {
                base.inv(b);
                fgc(base.fgc());
                bgc(base.bgc());
            }
            auto stk(bool b)
            {
                static constexpr auto set = "\\strike "sv;
                static constexpr auto off = "\\strike0 "sv;
                data += b ? set : off;
            }
            auto ovr(bool) { } // not supported
            auto blk(bool) { } // not supported
            auto hid(bool) { } // not supported
        };

        auto to_rich(text font = {}) const
        {
            // Reference https://www.biblioscape.com/rtf15_spec.htm
            static const auto deffnt = "Courier"s;
            static const auto red    = "\\red"s;
            static const auto green  = "\\green"s;
            static const auto blue   = "\\blue"s;
            static const auto nline  = "\\line "s;
            static const auto nnbsp  = "\\u8239 "" "s;  // U+202F   NARROW NO-BREAK SPACE (NNBSP)
            static const auto intro  = "{\\rtf1\\ansi\\deff0\\fcharset1"
                                       "\\chshdng0"  // Character shading. The N argument is a value representing the shading of the text in hundredths of a percent.
                                       "\\fs28{\\fonttbl{\\f0\\fmodern "s;
            static const auto colors = ";}}{\\colortbl;"s;
            auto crop = intro + (font.empty() ? deffnt : font) + colors;
            auto dest = rtf_dest_t{};
            for (auto& line_ptr : batch)
            {
                auto& curln = *line_ptr;
                auto  space = true;
                for (auto c : curln.locus)
                {
                    if (c.cmd == ansi::fn::nl)
                    {
                        if (dest.data.size() && dest.data.back() == ' ' && space)
                        {
                            dest.data.pop_back();
                            dest.data += nnbsp;
                            space = faux;
                        }
                        while (c.arg--) dest.data += nline;
                    }
                }
                curln.lyric->each([&](cell& c)
                {
                    if (c.isspc()) c.txt(whitespace);
                    auto [w, h, x, y] = c.whxy();
                    if (x == 1) // Capture the first cell only.
                    {
                        c.scan(dest.base, dest);
                    }
                });
            }
            auto vect = std::vector<argb>(dest.clrs.size());
            for (auto& [key, val] : dest.clrs)
            {
                vect[val].token = key;
            }
            for (auto& c : vect)
            {
                crop += red   + std::to_string(c.chan.r)
                      + green + std::to_string(c.chan.g)
                      + blue  + std::to_string(c.chan.b) + ';';
            }
            crop += "}\\f0 ";
                //"\\par"             // New paragraph.
                //"\\pard"            // Reset paragraph style to defaults.
                //"\\f0"              // Select font from fonttable.
                //"\\sl20\\slmult0 "; // \slN - Absolute(if negative N) or at least(if positive N) line spacing in pt * 20 (14pt = -280); \slmult0 - 0 means exactly or (at least if negative \sl used). Doesn't work on copy/paste.
            crop += dest.data + '}';
            return crop;
        }

        struct html_dest_t
        {
            static constexpr auto bclr = "<span style=\"background-color:#"sv;
            static constexpr auto fclr = ";color:#"sv;
            static constexpr auto line   = ";text-decoration:underline"sv;
            static constexpr auto biline = ";text-decoration:double"sv;
            static constexpr auto wavy   = ";text-decoration:wavy"sv;
            static constexpr auto dotted = ";text-decoration:dotted"sv;
            static constexpr auto dashed = ";text-decoration:dashed"sv;
            static constexpr auto itlc = ";font-style:italic"sv;
            static constexpr auto bold = ";font-weight:bold"sv;
            static constexpr auto strk = ";text-decoration:line-through"sv;
            static constexpr auto ovln = ";text-decoration:overline"sv;
            static constexpr auto stop = ";\">"sv;
            static constexpr auto done = "</span>"sv;
            static constexpr auto amp  = "&amp;"sv;
            static constexpr auto lt   = "&lt;"sv;
            static constexpr auto gt   = "&gt;"sv;

            text data;
            cell prev;
            cell base;

            auto operator += (qiew utf8)
            {
                if (utf8)
                {
                    if (prev != base)
                    {
                        prev = base;
                        if (data.size()) data += done;
                        auto [bg, fg] = base.inv() ? std::pair{ base.fgc(), base.bgc() }
                                                   : std::pair{ base.bgc(), base.fgc() };
                        data += bclr;
                        utf::to_hex(bg.chan.r, data);
                        utf::to_hex(bg.chan.g, data);
                        utf::to_hex(bg.chan.b, data);
                        data += fclr;
                        utf::to_hex(fg.chan.r, data);
                        utf::to_hex(fg.chan.g, data);
                        utf::to_hex(fg.chan.b, data);
                        if (base.itc()) data += itlc;
                        if (base.bld()) data += bold;
                        if (base.stk()) data += strk;
                        if (base.ovr()) data += ovln;
                             if (base.und() == unln::line  ) data += line;
                        else if (base.und() == unln::biline) data += biline;
                        else if (base.und() == unln::wavy  ) data += wavy;
                        else if (base.und() == unln::dotted) data += dotted;
                        else if (base.und() == unln::dashed) data += dashed;
                        data += stop;
                    }
                    for (auto c : utf8)
                    {
                             if (c == '&') data += amp;
                        else if (c == '<') data += lt;
                        else if (c == '>') data += gt;
                        else               data.push_back(c);
                    }
                }
            }
            template<svga Mode>
            auto fgc(argb ) { }
            template<svga Mode>
            auto bgc(argb ) { }
            auto bld(bool ) { }
            auto itc(bool ) { }
            auto dim(si32 ) { }
            auto und(si32 ) { }
            auto unc(argb ) { }
            auto inv(bool ) { }
            auto stk(bool ) { }
            auto ovr(bool ) { }
            auto blk(bool ) { }
            auto hid(bool ) { }
            auto cursor0(si32 ) { }
        };

        auto to_html(text font = {}) const
        {
            // Reference https://learn.microsoft.com/en-us/windows/win32/dataxchg/html-clipboard-format
            static const auto deffnt = "Courier"s;
            static const auto head = "Version:0.9\nStartHTML:-1\nEndHTML:-1\nStartFragment:"s;
            static const auto frag = "EndFragment:"s;

            auto crop = "<pre style=\"display:inline-block;"s;
            crop += "font-size:14pt;font-family:'" + (font.empty() ? deffnt : font) + "',monospace;line-height:1.0;\">\n";
            auto dest = html_dest_t{};
            for (auto& line_ptr : batch)
            {
                auto& curln = *line_ptr;
                for (auto c : curln.locus)
                {
                    if (c.cmd == ansi::fn::nl)
                    {
                        while (c.arg--) dest.data += "\n";
                    }
                }
                curln.lyric->each([&](cell c)
                {
                    if (c.isspc()) c.txt(whitespace);
                    auto [w, h, x, y] = c.whxy();
                    if (x == 1) // Capture the first cell only.
                    {
                        c.scan(dest.base, dest);
                    }
                });
            }
            if (dest.data.size()) dest.data += "</span>";
            crop += dest.data + "</pre>";

            auto xval = head.size();
            auto yval = xval + crop.size();
            auto xstr = std::to_string(xval);
            auto ystr = std::to_string(yval);
            auto xlen = xstr.size();
            auto ylen = ystr.size();
            do
            {
                xval = head.size() + xlen + 1
                     + frag.size() + ylen + 1;
                yval = xval + crop.size();
                xstr = std::to_string(xval);
                ystr = std::to_string(yval);
                xlen = xstr.size();
                ylen = ystr.size();
            }
            while (xval != head.size() + xlen + 1
                         + frag.size() + ylen + 1);
            auto clip = head + xstr + '\n'
                      + frag + ystr + '\n'
                      + crop;
            return std::pair{ clip, crop };
        }

        struct utf8_dest_t
        {
            text data;
            cell base;

            auto operator += (qiew utf8)
            {
                data += utf8;
            }
            template<svga Mode>
            auto fgc(argb ) { }
            template<svga Mode>
            auto bgc(argb ) { }
            auto bld(bool ) { }
            auto itc(bool ) { }
            auto und(si32 ) { }
            auto dim(si32 ) { }
            auto unc(argb ) { }
            auto inv(bool ) { }
            auto stk(bool ) { }
            auto ovr(bool ) { }
            auto blk(bool ) { }
            auto hid(bool ) { }
            auto cursor0(si32 ) { }
        };

        template<bool UseSGR = true>
        auto to_utf8() const
        {
            auto dest = utf8_dest_t{};
            for (auto& line_ptr : batch)
            {
                auto& curln = *line_ptr;
                for (auto c : curln.locus)
                {
                    if (c.cmd == ansi::fn::nl)
                    {
                        while (c.arg--) dest.data += "\n";
                    }
                }
                curln.lyric->each([&](cell c)
                {
                    if (c.isspc()) c.txt(whitespace);
                    auto [w, h, x, y] = c.whxy();
                    if (x == 1) // Capture the first cell only.
                    {
                        c.scan<svga::vt_2D, UseSGR>(dest.base, dest);
                    }
                });
            }
            return dest.data;
        }
    };

    // richtext: Textographical canvas.
    class face
        : public rich, public flow, public std::enable_shared_from_this<face>
    {
    protected:
        twod anker;     // face: The position of the nearest visible paragraph.
        id_t piece = 1; // face: The nearest to top paragraph.

        // face: Is the c inside the viewport?
        bool inside(twod c)
        {
            return c.y >= 0 && c.y < region.size.y;
            //todo X-axis
        }

    public:
        //todo revise
        bool caret = faux; // face: Cursor visibility.
        bool moved = faux; // face: Is reflow required.
        bool decoy = true; // face: Is the cursor inside the viewport.
        svga cmode = svga::vtrgb; // face: Color mode.

        // face: Print proxy something else at the specified coor.
        template<bool Split = true, class T, class P>
        void output_proxy(T const& block, twod coord, P proxy)
        {
            flow::sync(block);
            flow::ac(coord);
            flow::compose<Split>(block, proxy);
        }
        // face: Print something else at the specified coor.
        template<bool Split = true, class T, class P = noop>
        void output(T const& block, twod coord, P printfx = {})
        {
            flow::sync(block);
            flow::ac(coord);
            flow::go<Split>(block, *this, printfx);
        }
        // face: Print something else.
        template<bool UseFWD = faux, bool Split = faux, class T, class P = noop>
        void output(T const& block, P printfx = {})
        {
            //todo unify
            flow::print<UseFWD, Split>(block, *this, printfx);
        }
        // face: Print paragraph.
        void output(para const& block)
        {
            flow::print(block, *this);
        }
        // face: Print page.
        template<bool Split = faux, class P = noop>
        void output(page& textpage, P printfx = {})
        {
            auto publish = [&](auto& combo)
            {
                combo.coord = flow::print<true, Split>(combo, *this, printfx);
            };
            textpage.stream(publish);
        }
        // face: Print page with holding top visible paragraph on its own place.
        void output(page& textpage, bool reset)
        {
            //todo if the cursor is visible then bind to the cursor position,
            //     otherwise bind to the first visible text line.

            auto done = faux;
            // Take the vertical position of the paragraph closest to the top.
            auto gain = [&](auto& combo)
            {
                combo.coord = flow::print(combo, *this);
                auto pred = combo.coord;
                auto post = flow::cp();
                if (!done)
                {
                    if (pred.y <= 0 && post.y >= 0)
                    {
                        anker.y = pred.y;
                        piece = combo.id();
                        done = true;
                    }
                    else
                    {
                        if (std::abs(anker.y) > std::abs(pred.y))
                        {
                            anker.y = pred.y;
                            piece = combo.id();
                        }
                    }
                }
            };
            // Take the vertical position of the specified paragraph.
            auto find = [&](auto& combo)
            {
                combo.coord = flow::print(combo);
                if (combo.id() == piece) anker = combo.coord;
            };

            if (reset)
            {
                anker.y = si32max;
                textpage.stream(gain);
                decoy = caret && inside(flow::cp());
            }
            else
            {
                textpage.stream(find);
            }
        }
        auto get_page_size(page& object, twod& size, bool update_all)
        {
            auto cp = dot_00;
            flow::reset();
            flow::size(size);
            auto publish = [&](auto const& combo)
            {
                cp = flow::print(combo);
            };
            object.stream(publish);
            auto& cover = flow::minmax();
            if (update_all)
            {
                size = cover.size;
            }
            else
            {
                size.y = cover.size.y;
            }
            return cp;
        }
        // face: Reflow text page on the canvas and hold position
        //       of the top visible paragraph while resizing.
        void reflow_deprecated(page& textpage)
        {
            if (moved)
            {
                flow::zz(); //flow::sc();

                auto delta = anker;
                output(textpage, faux);
                std::swap(delta, anker);

                auto cover = flow::minmax();
                //auto& basis = flow::origin;
                auto basis = dot_00;// flow::origin;
                basis.y += anker.y - delta.y;

                if (decoy)
                {
                    // Don't tie the first line if it's the only one. Make one step forward.
                    if (anker.y == 0
                     && anker.y == flow::cp().y
                     && cover.size.y > 1)
                    {
                        //todo? the increment is removed bcos it shifts mc one row down on Ctrl+O and back
                        //basis.y++;
                    }

                    auto newcp = flow::cp();
                    if (!inside(newcp))
                    {
                        if (newcp.y < 0) basis.y -= newcp.y;
                        else             basis.y -= newcp.y - region.size.y + 1;
                    }
                }
                else
                {
                    basis.y = std::clamp(basis.y, -(cover.coor.y + cover.size.y - 1), region.size.y - cover.coor.y - 1);
                }

                moved = faux;
            }

            wipe();
        }

        // face: Forward call to the core and reset cursor.
        template<class ...Args>
        void wipe(Args&&... args) // Optional args.
        {
            core::wipe(args...);
            flow::reset();
        }
        // face: Change current 2D context. Return old 2D context.
        auto bump(dent delta, bool bump_clip = true)
        {
            auto old_full = flow::full();
            auto old_clip = core::clip();
            if (bump_clip)
            {
                auto new_clip = delta.bump(old_clip).trimby(core::area());
                core::clip(new_clip);
            }
            auto new_full = delta.bump(old_full);
            flow::full(new_full);
            return std::pair{ old_full, old_clip };
        }
        // face: Restore previously saved 2D context.
        void bump(std::pair<rect, rect> ctx)
        {
            flow::full(ctx.first);
            core::clip(ctx.second);
        }
        // face: Dive into object 2D context.
        template<bool Forced = faux>
        auto change_basis(rect object_area, bool trim = true)
        {
            struct ctx
            {
                face& canvas;
                rect canvas_full;
                rect canvas_clip;
                twod canvas_coor;
                bool nested_clip;

                operator bool () { return Forced || nested_clip; };

                ctx(face& canvas, rect canvas_full = {}, rect canvas_clip = {}, twod canvas_coor = {}, bool nested_clip = {})
                    :      canvas{ canvas      },
                      canvas_full{ canvas_full },
                      canvas_clip{ canvas_clip },
                      canvas_coor{ canvas_coor },
                      nested_clip{ nested_clip }
                { }
                ctx(ctx&& c)
                    :      canvas{ c.canvas      },
                      canvas_full{ c.canvas_full },
                      canvas_clip{ c.canvas_clip },
                      canvas_coor{ c.canvas_coor },
                      nested_clip{ c.nested_clip }
                {
                    c.nested_clip = faux;
                }
               ~ctx()
                {
                    if (nested_clip)
                    {
                        canvas.flow::full(canvas_full);
                        canvas.core::clip(canvas_clip);
                        canvas.core::move(canvas_coor);
                    }
                }
            };
            auto nested_clip = trim ? core::clip().trim(object_area) : core::clip();
            auto proceed = Forced || nested_clip;
            if (proceed)
            {
                auto context2D = ctx{ *this, flow::full(), core::clip(), core::coor(), true };
                core::step(                       - object_area.coor);
                core::clip({     nested_clip.coor - object_area.coor,   nested_clip.size });
                flow::full({{ }/*object_area.coor - object_area.coor*/, object_area.size });
                return context2D;
            }
            return ctx{ *this };
        }
        auto move_basis(twod new_coor)
        {
            region.coor = new_coor;
            client.coor = new_coor;
            pagerect.coor = new_coor;
        }
        // Use a two letter function if we don't need to return *this
        face& cup(twod p)     { flow::ac( p); return *this; } // face: Cursor 0-based absolute position.
        face& chx(si32 x)     { flow::ax( x); return *this; } // face: Cursor 0-based horizontal absolute.
        face& chy(si32 y)     { flow::ay( y); return *this; } // face: Cursor 0-based vertical absolute.
        face& cpp(twod p)     { flow::pc( p); return *this; } // face: Cursor percent position.
        face& cpx(si32 x)     { flow::px( x); return *this; } // face: Cursor H percent position.
        face& cpy(si32 y)     { flow::py( y); return *this; } // face: Cursor V percent position.
        face& cuu(si32 n = 1) { flow::dy(-n); return *this; } // face: cursor up.
        face& cud(si32 n = 1) { flow::dy( n); return *this; } // face: Cursor down.
        face& cuf(si32 n = 1) { flow::dx( n); return *this; } // face: Cursor forward.
        face& cub(si32 n = 1) { flow::dx(-n); return *this; } // face: Cursor backward.
        face& cnl(si32 n = 1) { flow::dy( n); return *this; } // face: Cursor next line.
        face& cpl(si32 n = 1) { flow::dy(-n); return *this; } // face: Cursor previous line.
        // face: Set margins.
        face& mgn(dent n)
        {
            auto area = core::area();
            auto cropped_area = rect{ area.coor + std::max(dot_00, n.corner()), area.size - n };
            flow::full(cropped_area);
            return *this;
        }

        face& ocp(twod p) { flow::oc( p); return *this; } // face: Cursor 1-based absolute position.
        face& ocx(si32 x) { flow::ox( x); return *this; } // face: Cursor 1-based horizontal absolute.
        face& ocy(si32 y) { flow::oy( y); return *this; } // face: Cursor 1-based vertical absolute.

        face& scp()              { flow::sc(  ); return *this; } // face: Save cursor position.
        face& rcp()              { flow::rc(  ); return *this; } // face: Restore cursor position.
        face& rst()  { flow::zz(); flow::sc(  ); return *this; } // face: Reset to zero all cursor params.

        face& tab(si32 n = 1)    { flow::tb( n); return *this; } // face: Proceed the \t .
        face& eol(si32 n = 1)    { flow::nl( n); return *this; } // face: Proceed the \r || \n || \r\n .

        auto& area() const // face: Return core::region/area.
        {
            return core::area();
        }
        void area(rect new_area) // face: Change the area of the face/core.
        {
            core::area(new_area);
            flow::full(new_area);
        }
        void size(twod new_size) // face: Change the size of the face/core.
        {
            core::size(new_size);
            flow::size(new_size);
        }
        auto resize(twod new_size) // face: Change the size of the face/core.
        {
            auto changed = new_size != core::size();
            if (changed)
            {
                core::size(new_size);
                flow::size(new_size);
            }
            return changed;
        }
        auto size() // face: Return size of the face/core.
        {
            return core::size();
        }
        template<bool BottomAnchored = faux>
        void crop(twod new_size, cell const& c) // face: Resize while saving the bitmap.
        {
            core::crop<BottomAnchored>(new_size, c);
            flow::size(new_size);
        }
        template<bool BottomAnchored = faux>
        void crop(twod new_size) // face: Resize while saving the bitmap.
        {
            core::crop<BottomAnchored>(new_size, core::mark());
            flow::size(new_size);
        }
        // face: Double boxblur the face background.
        template<bool InnerGlow = faux, class T = vrgb, class P = noop>
        void blur(si32 r, T&& cache = {}, P shade = {}) // face: .
        {
            using irgb = vrgb::value_type;

            auto area = core::area();
            auto clip = core::clip();

            auto w = std::max(0, clip.size.x);
            auto h = std::max(0, clip.size.y);
            auto s = w * h;

            if (cache.size() < (size_t)s)
            {
                cache.resize(s);
            }

            auto s_ptr = core::begin(clip.coor - area.coor);
            auto d_ptr = cache.begin();

            auto s_width = area.size.x;
            auto d_width = clip.size.x;

            auto s_point = [](auto c)->auto& { return c->bgc(); };
            auto d_point = [](auto c)->auto& { return *c;       };

            for (auto _(2); _--;) // Emulate Gaussian blur.
            netxs::boxblur<irgb, InnerGlow>(s_ptr,
                                            d_ptr, w,
                                                   h, r, s_width,
                                                         d_width, 2, s_point,
                                                                     d_point, shade);
        }
    };

    // Derivative vt-parser example.
    struct derived_from_page
        : public page
    {
        template<class T>
        static void parser_config(T& vt)
        {
            page::parser_config(vt); // Inherit base vt-functionality.

            // Override vt-functionality.
            using namespace netxs::ansi;
            #define V []([[maybe_unused]] auto& q, [[maybe_unused]] auto& p)
            vt.intro[ctrl::tab] = V{ p->tabs(q.pop_all(ctrl::tab)); };
            #undef V
        }

        derived_from_page (view utf8) {          ansi::parse(utf8, this);               }
        auto& operator  = (view utf8) { clear(); ansi::parse(utf8, this); return *this; }
        auto& operator += (view utf8) {          ansi::parse(utf8, this); return *this; }

        void tabs(si32) { if constexpr (debugmode) log(prompt::page, "Tabs not supported"); }
    };

    struct tone
    {
        #define prop_list                         \
        X(window_clr , "Window base color")       \
        X(winfocus   , "Focused item color")      \
        X(brighter   , "Highlighter modificator") \
        X(shadower   , "Darklighter modificator") \
        X(placeholder, "Placeholder overlay")     \
        X(warning    , "Warning color")           \
        X(danger     , "Danger color")            \
        X(action     , "Action color")            \
        X(selected   , "Selected item color")     \
        X(active     , "Active item color")       \
        X(focused    , "Focused item color")      \
        X(label      , "Static label color")      \
        X(inactive   , "Inactive label color")

        enum prop
        {
            #define X(a, b) a,
            prop_list
            #undef X
        };
        #undef prop_list

        prop dynamic = prop::brighter;
        prop passive = prop::shadower;
    };
}