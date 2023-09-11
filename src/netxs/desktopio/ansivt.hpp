// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "canvas.hpp"
#include "quartz.hpp"

#include <mutex>
#include <array>
#include <list>
#include <functional>

namespace netxs::ansi
{
    using ctrl = utf::ctrl;

    static const auto esc_csi     = '['; // ESC [ ...
    static const auto esc_ocs     = ']'; // ESC ] ...
    static const auto esc_dsc     = 'P'; // ESC P ... BELL/ST
    static const auto esc_sos     = 'X'; // ESC X ... BELL/ST
    static const auto esc_pm      = '^'; // ESC ^ ... BELL/ST
    static const auto esc_apc     = '_'; // ESC _ ... BELL/ST
    static const auto esc_g0set   = '('; // ESC ( c
    static const auto esc_g1set   = ')'; // ESC ) c
    static const auto esc_g2set   = '*'; // ESC * c
    static const auto esc_g3set   = '+'; // ESC + c
    static const auto esc_g1xset  = '-'; // ESC - c
    static const auto esc_g2xset  = '.'; // ESC . c
    static const auto esc_g3xset  = '/'; // ESC / c
    static const auto esc_ctrl    = ' '; // ESC sp F, ESC sp G, ESC sp L, ESC sp M, ESC sp N
    static const auto esc_decdhl  = '#'; // ESC # 3, ESC # 4, ESC # 5, ESC # 6, ESC # 8
    static const auto esc_chrset  = '%'; // ESC % @, ESC % G  G: Select UTF-8, @: Select default.
    static const auto esc_st      ='\\'; // ESC backslash
    static const auto esc_delim   = ';'; // ESC ;
    static const auto esc_key_a   = '='; // ESC =  Application keypad.
    static const auto esc_key_n   = '>'; // ESC >  Normal      keypad.
    static const auto esc_decbi   = '6'; // ESC 6  Back index,    DECBI.
    static const auto esc_decfi   = '9'; // ESC 9  Forward index, DECFI.
    static const auto esc_sc      = '7'; // ESC 7  Save    caret position and rendition state.
    static const auto esc_rc      = '8'; // ESC 8  Restore caret position and rendition state.
    static const auto esc_hts     = 'H'; // ESC H  Set tabstop at the current caret position.
    static const auto esc_nel     = 'E'; // ESC E  Move caret down and CR.
    static const auto esc_clb     = 'F'; // ESC F  Move caret to lower leftmost position.
    static const auto esc_ind     = 'D'; // ESC D  Caret down.
    static const auto esc_ir      = 'M'; // ESC M  Caret up.
    static const auto esc_ris     = 'c'; // ESC c  Reset terminal to initial state.
    static const auto esc_memlk   = 'l'; // ESC l  Memory lock.
    static const auto esc_munlk   = 'm'; // ESC m  Memory unlock.
    static const auto esc_ls2     = 'n'; // ESC n  LS2.
    static const auto esc_ls3     = 'o'; // ESC o  LS3.
    static const auto esc_ls1r    = '~'; // ESC ~  LS1R.
    static const auto esc_ls2r    = '}'; // ESC }  LS2R.
    static const auto esc_ls3r    = '|'; // ESC |  LS3R.
    static const auto esc_ss3     = 'O'; // ESC O  SS3.
    static const auto esc_ss2     = 'N'; // ESC N  SS2.
    static const auto esc_spa     = 'V'; // ESC V  SPA.
    static const auto esc_epa     = 'W'; // ESC W  EPA.
    static const auto esc_rid     = 'Z'; // ESC Z  Return ID.
    static const auto csi_spc_slc = '@'; // CSI n SP   @  — Shift left n columns(s).
    static const auto csi_spc_src = 'A'; // CSI n SP   A  — Shift right n columns(s).
    static const auto csi_spc_cst = 'q'; // CSI n SP   q  — Set caret style (DECSCUSR).
    static const auto csi_hsh_scp = 'P'; // CSI n #    P  — Push current palette colors onto stack. n default is 0.
    static const auto csi_hsh_rcp = 'Q'; // CSI n #    Q  — Pop current palette colors from stack. n default is 0.
    static const auto csi_hsh_sva = 'p'; // CSI   #    p  — Push video attributes from stack (XTPUSHSGR).
    static const auto csi_hsh_rva = 'q'; // CSI   #    q  — Pop  video attributes from stack (XTPOPSGR).
    static const auto csi_hsh_psh = '{'; // CSI # {  — Push SGR attributes onto stack (XTPUSHSGR).
    static const auto csi_hsh_pop = '}'; // CSI # }  — Pop  SGR attributes from stack (XTPOPSGR).
    static const auto csi_dqt_scp = 'q'; // CSI n "    q  — Select character protection attribute.
    static const auto csi_exl_rst = 'p'; // CSI   !    p  — Reset terminal to initial state.
    static const auto csi_qst_rtb = 'W'; // CSI   ?    W  — Reset tabstops to the defaults.
    static const auto csi_dlr_fra = 'x'; // CSI Char ; Top ; Left ; Bottom ; Right $ x  — Fill rectangular area (DECFRA).
    static const auto csi_cuu     = 'A'; // CSI n      A  — Caret Up.
    static const auto csi_cud     = 'B'; // CSI n      B  — Caret Down.
    static const auto csi_cud2    = 'e'; // CSI n      e  — Caret Down.
    static const auto csi_cuf     = 'C'; // CSI n      C  — Caret Forward.
    static const auto csi_cub     = 'D'; // CSI n      D  — Caret Back.
    static const auto csi_cnl     = 'E'; // CSI n      E  — Caret Next Line.     Move n lines down and to the leftmost column.
    static const auto csi_cpl     = 'F'; // CSI n      F  — Caret Previous Line. Move n lines up   and to the leftmost column.
    static const auto csi_chx     = 'G'; // CSI n      G  — Caret Horizontal Absolute.
    static const auto csi_chy     = 'd'; // CSI n      d  — Caret Vertical Absolute.
    static const auto csi_hvp     = 'f'; // CSI n ; m  f  — Horizontal and Vertical Position.
    static const auto csi_cup     = 'H'; // CSI n ; m  H  — Caret Position.
    static const auto csi_cht     = 'I'; // CSI n      I  — Caret forward  n tab stops (default = 1).
    static const auto csi_cbt     = 'Z'; // CSI n      Z  — Caret backward n tab stops (default = 1).
    static const auto csi_rep     = 'b'; // CSI n      b  — Repeat the preceding character n times.
    static const auto csi_tbc     = 'g'; // CSI n      g  — Reset tabstop value.
    static const auto csi_sgr     = 'm'; // CSI n [;k] m  — Select Graphic Rendition.
    static const auto csi_dsr     = 'n'; // CSI n      n  — Device Status Report (DSR). n==5 -> "OK"; n==6 -> CSI r ; c R
    static const auto csi_scp     = 's'; // CSI        s  — Save caret Position.
    static const auto csi_rcp     = 'u'; // CSI        u  — Restore caret Position.
    static const auto csi__el     = 'K'; // CSI n      K  — Erase 0: from caret to end, 1: from begin to caret, 2: all line.
    static const auto csi__il     = 'L'; // CSI n      L  — Insert n blank lines.
    static const auto csi__ed     = 'J'; // CSI n      J  — Erase 0: from caret to end of screen, 1: from begin to caret, 2: all screen.
    static const auto csi__dl     = 'M'; // CSI n      M  — Delete n lines.
    static const auto csi_dch     = 'P'; // CSI n      P  — Delete n character(s).
    static const auto csi_led     = 'q'; // CSI n      q  — Load keyboard LEDs.
    static const auto csi__sd     = 'T'; // CSI n      T  — Scroll down by n lines, scrolled out lines are lost.
    static const auto csi__su     = 'S'; // CSI n      S  — Scroll   up by n lines, scrolled out lines are lost.
    static const auto csi_win     = 't'; // CSI n;m;k  t  — XTWINOPS, Terminal window props.
    static const auto csi_ech     = 'X'; // CSI n      X  — Erase n character(s) ? difference with delete ?
    static const auto csi_ich     = '@'; // CSI n      @  — Insert/wedge n character(s).
    static const auto csi_pda     = 'c'; // CSI n      c  — Send device attributes (Primary DA).
    static const auto csi_hrm     = 'h'; // CSI n      h  — Reset mode (always Replace mode n=4).
    static const auto csi_lrm     = 'l'; // CSI n      l  — Reset mode (always Replace mode n=4).
    static const auto csi_ccc     = 'p'; // CSI n [; x1; x2; ...; xn ] p — Private vt command subset.
    static const auto decstbm     = 'r'; // CSI t ; b  r  — Set scrolling region (t/b: top + bottom).
    static const auto dec_set     = 'h'; // CSI ? n    h  — DECSET.
    static const auto dec_rst     = 'l'; // CSI ? n    l  — DECRST.
    static const auto w32_inp     = '_'; // CSI EVENT_TYPEn [; x1; x2; ...; xn ] _ — win32-input-mode.
    static const auto c0_nul = '\x00'; // Null                - Originally used to allow gaps to be left on paper tape for edits. Later used for padding after a code that might take a terminal some time to process (e.g. a carriage return or line feed on a printing terminal). Now often used as a string terminator, especially in the programming language C.
    static const auto c0_soh = '\x01'; // Start of Heading    - First character of a message header. In Hadoop, it is often used as a field separator.
    static const auto c0_stx = '\x02'; // Start of Text       - First character of message text, and may be used to terminate the message heading.
    static const auto c0_etx = '\x03'; // End of Text         - Often used as a "break" character (Ctrl-C) to interrupt or terminate a program or process.
    static const auto c0_eot = '\x04'; // End of Transmssn    - Often used on Unix to indicate end-of-file on a terminal.
    static const auto c0_enq = '\x05'; // Enquiry             - Signal intended to trigger a response at the receiving end, to see if it is still present.
    static const auto c0_ack = '\x06'; // Acknowledge         - Response to an ENQ, or an indication of successful receipt of a message.
    static const auto c0_bel = '\x07'; // Bell, Alert     \a  - Originally used to sound a bell on the terminal. Later used for a beep on systems that didn't have a physical bell. May also quickly turn on and off inverse video (a visual bell).
    static const auto c0_bs  = '\x08'; // Backspace       \b  - Move the caret one position leftwards. On input, this may delete the character to the left of the caret. On output, where in early computer technology a character once printed could not be erased, the backspace was sometimes used to generate accented characters in ASCII. For example, à could be produced using the three character sequence a BS ` (or, using the characters’ hex values, 0x61 0x08 0x60). This usage is now deprecated and generally not supported. To provide disambiguation between the two potential uses of backspace, the cancel character control code was made part of the standard C1 control set.
    static const auto c0_ht  = '\x09'; // Character       \t  - Tabulation, Horizontal Tabulation	\t	Position to the next character tab stop.
    static const auto c0_lf  = '\x0A'; // Line Feed       \n  - On typewriters, printers, and some terminal emulators, moves the caret down one row without affecting its column position. On Unix, used to mark end-of-line. In DOS, Windows, and various network standards, LF is used following CR as part of the end-of-line mark.
    static const auto c0_vt  = '\x0B'; // Line Tab,VTab   \v  - Position the form at the next line tab stop.
    static const auto c0_ff  = '\x0C'; // Form Feed       \f  - On printers, load the next page. Treated as whitespace in many programming languages, and may be used to separate logical divisions in code. In some terminal emulators, it clears the screen. It still appears in some common plain text files as a page break character, such as the RFCs published by IETF.
    static const auto c0_cr  = '\x0D'; // Carriage Return \r  - Originally used to move the caret to column zero while staying on the same line. On classic Mac OS (pre-Mac OS X), as well as in earlier systems such as the Apple II and Commodore 64, used to mark end-of-line. In DOS, Windows, and various network standards, it is used preceding LF as part of the end-of-line mark. The Enter or Return key on a keyboard will send this character, but it may be converted to a different end-of-line sequence by a terminal program.
    static const auto c0_so  = '\x0E'; // Shift Out           - Switch to an alternative character set.
    static const auto c0_si  = '\x0F'; // Shift In            - Return to regular character set after Shift Out.
    static const auto c0_dle = '\x10'; // Data Link Escape    - Cause the following octets to be interpreted as raw data, not as control codes or graphic characters. Returning to normal usage would be implementation dependent.
    static const auto c0_dc1 = '\x11'; // Device Control One (XON)    - These four control codes are reserved for device control, with the interpretation dependent upon the device to which they were connected.
    static const auto c0_dc2 = '\x12'; // Device Control Two          > DC1 and DC2 were intended primarily to indicate activating a device while DC3 and DC4 were intended primarily to indicate pausing or turning off a device.
    static const auto c0_dc3 = '\x13'; // Device Control Three (XOFF) > DC1 and DC3 (known also as XON and XOFF respectively in this usage) originated as the "start and stop remote paper-tape-reader" functions in ASCII Telex networks.
    static const auto c0_dc4 = '\x14'; // Device Control Four         > This teleprinter usage became the de facto standard for software flow control.[6]
    static const auto c0_nak = '\x15'; // Negative Acknowldg  - Sent by a station as a negative response to the station with which the connection has been set up. In binary synchronous communication protocol, the NAK is used to indicate that an error was detected in the previously received block and that the receiver is ready to accept retransmission of that block. In multipoint systems, the NAK is used as the not-ready reply to a poll.
    static const auto c0_syn = '\x16'; // Synchronous Idle    - Used in synchronous transmission systems to provide a signal from which synchronous correction may be achieved between data terminal equipment, particularly when no other character is being transmitted.
    static const auto c0_etb = '\x17'; // End of Transmission Block  - Indicates the end of a transmission block of data when data are divided into such blocks for transmission purposes.
    static const auto c0_can = '\x18'; // Cancel              - Indicates that the data preceding it are in error or are to be disregarded.
    static const auto c0_em  = '\x19'; // End of medium       - Intended as means of indicating on paper or magnetic tapes that the end of the usable portion of the tape had been reached.
    static const auto c0_sub = '\x1A'; // Substitute          - Originally intended for use as a transmission control character to indicate that garbled or invalid characters had been received. It has often been put to use for other purposes when the in-band signaling of errors it provides is unneeded, especially where robust methods of error detection and correction are used, or where errors are expected to be rare enough to make using the character for other purposes advisable. In DOS, Windows and other CP/M derivatives, it is used to indicate the end of file, both when typing on the terminal, and sometimes in text files stored on disk.
    static const auto c0_esc = '\x1B'; // Escape          \e  - The Esc key on the keyboard will cause this character to be sent on most systems. It can be used in software user interfaces to exit from a screen, menu, or mode, or in device-control protocols (e.g., printers and terminals) to signal that what follows is a special command sequence rather than normal text. In systems based on ISO/IEC 2022, even if another set of C0 control codes are used, this octet is required to always represent the escape character.
    static const auto c0_fs  = '\x1C'; // File Separator      - Can be used as delimiters to mark fields of data structures. If used for hierarchical levels, US is the lowest level (dividing plain-text data items), while RS, GS, and FS are of increasing level to divide groups made up of items of the level beneath it.
    static const auto c0_gs  = '\x1D'; // Group Separator.
    static const auto c0_rs  = '\x1E'; // Record Separator.
    static const auto c0_us  = '\x1F'; // Unit Separator.
    static const auto c0_del = '\x7F'; // Delete cell backward.

    static const auto osc_label_title  = "0"   ; // Set icon label and title.
    static const auto osc_label        = "1"   ; // Set icon label.
    static const auto osc_title        = "2"   ; // Set title.
    static const auto osc_xprop        = "3"   ; // Set xprop.
    static const auto osc_linux_color  = "P"   ; // Set 16 colors palette. (Linux console)
    static const auto osc_linux_reset  = "R"   ; // Reset 16/256 colors palette. (Linux console)
    static const auto osc_set_palette  = "4"   ; // Set 256 colors palette.
    static const auto osc_clipboard    = "52"  ; // Set clipboard.
    static const auto osc_set_fgcolor  = "10"  ; // Set fg color.
    static const auto osc_set_bgcolor  = "11"  ; // Set bg color.
    static const auto osc_reset_color  = "104" ; // Reset color N to default palette. Without params all palette reset.
    static const auto osc_reset_fgclr  = "110" ; // Reset fg color to default.
    static const auto osc_reset_bgclr  = "111" ; // Reset bg color to default.
    static const auto osc_title_report = "l"   ; // Get terminal window title.
    static const auto osc_label_report = "L"   ; // Get terminal window icon label.

    static const auto sgr_rst       = 0;
    static const auto sgr_sav       = 10;
    static const auto sgr_bold      = 1;
    static const auto sgr_faint     = 22;
    static const auto sgr_italic    = 3;
    static const auto sgr_nonitalic = 23;
    static const auto sgr_und       = 4;
    static const auto sgr_doubleund = 21;
    static const auto sgr_nound     = 24;
    static const auto sgr_slowblink = 5;
    static const auto sgr_fastblink = 6;
    static const auto sgr_no_blink  = 25;
    static const auto sgr_inv       = 7;
    static const auto sgr_noinv     = 27;
    static const auto sgr_strike    = 9;
    static const auto sgr_nostrike  = 29;
    static const auto sgr_overln    = 53;
    static const auto sgr_nooverln  = 55;
    static const auto sgr_fg_blk    = 30;
    static const auto sgr_fg_red    = 31;
    static const auto sgr_fg_grn    = 32;
    static const auto sgr_fg_ylw    = 33;
    static const auto sgr_fg_blu    = 34;
    static const auto sgr_fg_mgt    = 35;
    static const auto sgr_fg_cyn    = 36;
    static const auto sgr_fg_wht    = 37;
    static const auto sgr_fg_rgb    = 38;
    static const auto sgr_fg        = 39;
    static const auto sgr_bg_blk    = 40;
    static const auto sgr_bg_red    = 41;
    static const auto sgr_bg_grn    = 42;
    static const auto sgr_bg_ylw    = 43;
    static const auto sgr_bg_blu    = 44;
    static const auto sgr_bg_mgt    = 45;
    static const auto sgr_bg_cyn    = 46;
    static const auto sgr_bg_wht    = 47;
    static const auto sgr_bg_rgb    = 48;
    static const auto sgr_bg        = 49;
    static const auto sgr_fg_blk_lt = 90;
    static const auto sgr_fg_red_lt = 91;
    static const auto sgr_fg_grn_lt = 92;
    static const auto sgr_fg_ylw_lt = 93;
    static const auto sgr_fg_blu_lt = 94;
    static const auto sgr_fg_mgt_lt = 95;
    static const auto sgr_fg_cyn_lt = 96;
    static const auto sgr_fg_wht_lt = 97;
    static const auto sgr_bg_blk_lt = 100;
    static const auto sgr_bg_red_lt = 101;
    static const auto sgr_bg_grn_lt = 102;
    static const auto sgr_bg_ylw_lt = 103;
    static const auto sgr_bg_blu_lt = 104;
    static const auto sgr_bg_mgt_lt = 105;
    static const auto sgr_bg_cyn_lt = 106;
    static const auto sgr_bg_wht_lt = 107;

    static const auto ccc_nop    = 0  ; // CSI             p  - no operation.
    static const auto ccc_rst    = 1  ; // CSI 1           p  - reset to zero all params (zz).
    static const auto ccc_cpp    = 2  ; // CSI 2 : x [: y] p  - caret percent position.
    static const auto ccc_cpx    = 3  ; // CSI 3 : x       p  - caret H percent position.
    static const auto ccc_cpy    = 4  ; // CSI 4 : y       p  - caret V percent position.
    static const auto ccc_tbs    = 5  ; // CSI 5 : n       p  - tab step length.
    static const auto ccc_mgn    = 6  ; // CSI 6 : l:r:t:b p  - margin left, right, top, bottom.
    static const auto ccc_mgl    = 7  ; // CSI 7 : n       p  - margin left   ╮
    static const auto ccc_mgr    = 8  ; // CSI 8 : n       p  - margin right  │ positive - native binding.
    static const auto ccc_mgt    = 9  ; // CSI 9 : n       p  - margin top    │ negative - oppisite binding.
    static const auto ccc_mgb    = 10 ; // CSI 10: n       p  - margin bottom ╯
    static const auto ccc_jet    = 11 ; // CSI 11: n       p  - text alignment (bias).
    static const auto ccc_wrp    = 12 ; // CSI 12: n       p  - text wrapping none/on/off.
    static const auto ccc_rtl    = 13 ; // CSI 13: n       p  - text right-to-left none/on/off.
    static const auto ccc_rlf    = 14 ; // CSI 14: n       p  - reverse line feed none/on/off.
    static const auto ccc_jet_or = 15 ; // CSI 15: n       p  - set text alignment (bias) if it is not set.
    static const auto ccc_wrp_or = 16 ; // CSI 16: n       p  - set text wrapping none/on/off if it is not set.
    static const auto ccc_rtl_or = 17 ; // CSI 17: n       p  - set text right-to-left none/on/off if it is not set.
    static const auto ccc_rlf_or = 18 ; // CSI 18: n       p  - set reverse line feed none/on/off if it is not set.
    static const auto ccc_idx    = 19 ; // CSI 19: id      p  - Split the text run and associate the fragment with an id.
    static const auto ccc_cup    = 20 ; // CSI 20: x [: y] p  - caret absolute position 0-based.
    static const auto ccc_chx    = 21 ; // CSI 21: x       p  - caret H absolute position 0-based.
    static const auto ccc_chy    = 22 ; // CSI 22: y       p  - caret V absolute position 0-based.
    static const auto ccc_ref    = 23 ; // CSI 23: id      p  - create the reference to the existing paragraph.
    static const auto ccc_sbs    = 24 ; // CSI 24: n: m: q p  - define scrollback size: n: init size, m: grow_by step, q: max limit.
    static const auto ccc_sms    = 26 ; // CSI 26: b       p  - Should the mouse poiner to be drawn.
    static const auto ccc_sgr    = 28 ; // CSI 28: ...     p  - Set the default SGR attribute for the built-in terminal background (one attribute per command).
    static const auto ccc_sel    = 29 ; // CSI 29: n       p  - Set selection mode for the built-in terminal, n: 0 - off, 1 - plaintext, 2 - ansi-text.
    static const auto ccc_pad    = 30 ; // CSI 30: n       p  - Set left/right padding for the built-in terminal.
    static const auto ccc_lnk    = 31 ; // CSI 31: n       p  - Set object id to the cell owner.
    static const auto ccc_lsr    = 32 ; // CSI 32: n       p  - Enable line style reporting.
    static const auto ccc_stl    = 33 ; // CSI 32: n       p  - Line style report.

    //static const auto ctrl_break = si32{ 0xE046 }; // Pressed Ctrl+Break scancode.
    static const auto ctrl_break = si32{ 0x46 }; // Pressed Ctrl+Break scancode.

    template<class Base>
    class basevt
    {
        char  heap[32];
        char* tail = heap + sizeof(heap);
        Base& block;

        template<class T>
        inline void itos(T data)
        {
            auto cptr = tail;
            auto bake = [&](auto bits)
            {
                do *--cptr = static_cast<char>('0' + bits % 10);
                while (bits /= 10);
            };
            if constexpr (std::is_signed_v<T>)
            {
                if (data < 0)
                {
                    bake(std::make_unsigned_t<T>(-data));
                    *--cptr = '-';
                }
                else bake(data);
            }
            else bake(data);
            auto gain = tail - cptr;
            auto size = block.size();
            block.resize(size + gain);
            ::memcpy(size + block.data(), cptr, gain);
        }
        template<class T>
        inline void fuse(T&& data)
        {
            using D = std::remove_cv_t<std::remove_reference_t<T>>;

            if constexpr (std::is_same_v<D, char>)
            {
                block.push_back(data);
            }
            else if constexpr (std::is_integral_v<D>)
            {
                itos(data);
            }
            else if constexpr (std::is_same_v<D, bias>
                            || std::is_same_v<D, wrap>
                            || std::is_same_v<D, rtol>
                            || std::is_same_v<D, feed>)
            {
                itos(static_cast<si32>(data));
            }
            else if constexpr (std::is_same_v<D, twod>)
            {
                block += "{ "; itos(data.x); block += ", ";
                               itos(data.y); block += " }";
            }
            else if constexpr (std::is_same_v<D, rect>)
            {
                block += "{"; fuse(data.coor); block += ",";
                              fuse(data.size); block += "}";
            }
            else if constexpr (std::is_same_v<D, time>)
            {
                block += "{ time: "s + utf::to_hex_0x(data.time_since_epoch().count()) + " }"s;
            }
            else block += std::forward<T>(data);
        }

    public:
        basevt(Base& block)
            : block{ block }
        { }

        template<class T>
        inline auto& add(T&& data)
        {
            fuse(std::forward<T>(data));
            return block;
        }
        template<class T, class ...Args>
        inline auto& add(T&& data, Args&&... data_list)
        {
            fuse(std::forward<T>(data));
            return add(std::forward<Args>(data_list)...);
        }

        auto& bld(bool b)           { return add(b ? "\033[1m" : "\033[22m"         ); } // basevt: SGR 𝗕𝗼𝗹𝗱 attribute.
        auto& und(si32 n)           { return add(n==0 ? "\033[24m" :
                                                 n==2 ? "\033[21m" : "\033[4m"      ); } // basevt: SGR 𝗨𝗻𝗱𝗲𝗿𝗹𝗶𝗻𝗲 attribute.
        auto& blk(bool b)           { return add(b ? "\033[5m" : "\033[25m"         ); } // basevt: SGR Blink attribute.
        auto& inv(bool b)           { return add(b ? "\033[7m" : "\033[27m"         ); } // basevt: SGR 𝗡𝗲𝗴𝗮𝘁𝗶𝘃𝗲 attribute.
        auto& itc(bool b)           { return add(b ? "\033[3m" : "\033[23m"         ); } // basevt: SGR 𝑰𝒕𝒂𝒍𝒊𝒄 attribute.
        auto& stk(bool b)           { return add(b ? "\033[9m" : "\033[29m"         ); } // basevt: SGR Strikethrough attribute.
        auto& ovr(bool b)           { return add(b ? "\033[53m": "\033[55m"         ); } // basevt: SGR Overline attribute.
        auto& sav()                 { return add("\033[10m"                         ); } // basevt: Save SGR attributes.
        auto& nil()                 { return add("\033[m"                           ); } // basevt: Reset SGR attributes to zero.
        auto& fgc()                 { return add("\033[39m"                         ); } // basevt: Set default foreground color.
        auto& bgc()                 { return add("\033[49m"                         ); } // basevt: Set default background color.
        auto& scroll_wipe()         { return add("\033[2J"                          ); } // basevt: Erase scrollback.
        auto& locate(twod const& p) { return add("\033[", p.y + 1, ';', p.x + 1, 'H'); } // basevt: 0-Based caret position.
        auto& cuu(si32 n)           { return add("\033[", n, 'A'                    ); } // basevt: Caret up.
        auto& cud(si32 n)           { return add("\033[", n, 'B'                    ); } // basevt: Caret down.
        auto& cuf(si32 n)           { return add("\033[", n, 'C'                    ); } // basevt: Caret forward.  Negative values can wrap to the prev line.
        auto& cub(si32 n)           { return add("\033[", n, 'D'                    ); } // basevt: Caret backward. Negative values can wrap to the next line.
        auto& cnl(si32 n)           { return add("\033[", n, 'E'                    ); } // basevt: caret next line.
        auto& cpl(si32 n)           { return add("\033[", n, 'F'                    ); } // basevt: Caret previous line.
        auto& ocx(si32 n)           { return add("\033[", n, 'G'                    ); } // basevt: Caret 1-based horizontal absolute.
        auto& ocy(si32 n)           { return add("\033[", n, 'd'                    ); } // basevt: Caret 1-based vertical absolute.
        auto& dch(si32 n)           { return add("\033[", n, 'P'                    ); } // basevt: DCH
        auto& fwd(si32 n)           { return n > 0 ? add("\033[",-n, 'D')
                                           : n < 0 ? add("\033[", n, 'C') : *this;     } // basevt: Move caret n cell in line with wrapping.
        auto& del()                 { return add('\x7F'                             ); } // basevt: Delete cell backwards.
        auto& scp()                 { return add("\033[s"                           ); } // basevt: Save caret position in memory.
        auto& rcp()                 { return add("\033[u"                           ); } // basevt: Restore caret position from memory.
        auto& pushsgr()             { return add("\033[#{"                          ); } // basevt: Push SGR attributes onto stack.
        auto& popsgr()              { return add("\033[#}"                          ); } // basevt: Pop  SGR attributes from stack.
        auto& fcs(bool b)           { return add("\033[", b ? 'I' : 'O'             ); } // basevt: Terminal window focus.
        auto& eol()                 { return add("\n"                               ); } // basevt: EOL.
        auto& edl()                 { return add("\033[K"                           ); } // basevt: EDL.
        auto& del(si32 n)           { return add("\033[", n, "J"                    ); } // basevt: CSI n J  Erase display.
        auto& del_below()           { return add("\033[J"                           ); } // basevt: CSI   J  Erase below cursor.
        auto& fgx(rgba const& c)    { return add("\033[38:2:", c.chan.r, ':',            // basevt: SGR Foreground color. RGB: red, green, blue and alpha.
                                                               c.chan.g, ':',
                                                               c.chan.b, ':',
                                                               c.chan.a, 'm'); }
        auto& bgx(rgba const& c)    { return add("\033[48:2:", c.chan.r, ':',            // basevt: SGR Background color. RGB: red, green, blue and alpha.
                                                               c.chan.g, ':',
                                                               c.chan.b, ':',
                                                               c.chan.a, 'm'); }
        auto& fgc256(si32 c) { return add("\033[38;5;", c, 'm'); } // basevt: SGR Foreground color (256-color mode).
        auto& bgc256(si32 c) { return add("\033[48;5;", c, 'm'); } // basevt: SGR Background color (256-color mode).
        auto& fgc_16(si32 f) // basevt: SGR Foreground color (16-color mode).
        {
            return f < 8 ? add("\033[", f + 30, ";22m") // CSI22m: Linux console unexpectedly sets the high intensity bit when CSI 9x m used.
                         : add("\033[", f + 90 - 8, "m");
        }
        auto& bgc_8(si32 b) // basevt: SGR Background color (8-color mode).
        {
            return add("\033[", b + 40, 'm');
        }
        template<svga Mode = svga::vtrgb>
        auto& fgc(rgba const& c) // basevt: SGR Foreground color. RGB: red, green, blue.
        {
                 if constexpr (Mode == svga::vt16 ) return fgc_16(c.to_vtm16(true));
            else if constexpr (Mode == svga::vt256) return fgc256(c.to_256cube());
            else if constexpr (Mode == svga::vtrgb) return c.chan.a == 0 ? add("\033[39m")
                                                                         : add("\033[38;2;", c.chan.r, ';',
                                                                                             c.chan.g, ';',
                                                                                             c.chan.b, 'm');
            else return block;
        }
        template<svga Mode = svga::vtrgb>
        auto& bgc(rgba const& c) // basevt: SGR Background color. RGB: red, green, blue.
        {
                 if constexpr (Mode == svga::vt16 ) return bgc_8(c.to_vtm8());
            else if constexpr (Mode == svga::vt256) return bgc256(c.to_256cube());
            else if constexpr (Mode == svga::vtrgb) return c.chan.a == 0 ? add("\033[49m")
                                                                         : add("\033[48;2;", c.chan.r, ';',
                                                                                             c.chan.g, ';',
                                                                                             c.chan.b, 'm');
            else return block;
        }
        template<class ...Args>
        auto& clr(rgba c, Args&&... data) { return fgc(c).add(std::forward<Args>(data)...).nil(); } // basevt: Add colored message.
        template<class ...Args>
        auto& hi(Args&&... data) { return inv(true).add(std::forward<Args>(data)...).nil(); } // basevt: Add highlighted message.
        auto& err() { return fgc(redlt); } // basevt: Add error color.
        template<class ...Args>
        auto& err(Args&&... data) { return fgc(redlt).add(std::forward<Args>(data)...).nil(); } // basevt: Add error message.
        // basevt: Ansify/textify content of specified region.
        template<bool UseSGR = true, bool Initial = true, bool Finalize = true>
        auto& s11n(core const& canvas, rect region, cell& state)
        {
            auto badfx = [&]
            {
                add(utf::replacement);
                state.set_gc();
                state.wdt(1);
            };
            auto side_badfx = [&] // Restoring the halves on the side
            {
                add(state.txt());
                state.set_gc();
                state.wdt(1);
            };
            auto allfx = [&](cell const& c)
            {
                auto width = c.wdt();
                if (width < 2) // Narrow character
                {
                    if (state.wdt() == 2) badfx(); // Left part alone
                    c.scan<svga::vtrgb, UseSGR>(state, block);
                }
                else
                {
                    if (width == 2) // Left part
                    {
                        if (state.wdt() == 2) badfx();  // Left part alone
                        c.scan_attr<svga::vtrgb, UseSGR>(state, block);
                        state.set_gc(c); // Save char from c for the next iteration
                    }
                    else if (width == 3) // Right part
                    {
                        if (state.wdt() == 2)
                        {
                            if (state.scan<svga::vtrgb, UseSGR>(c, state, block)) state.set_gc(); // Cleanup used t
                            else
                            {
                                badfx(); // Left part alone
                                c.scan_attr<svga::vtrgb, UseSGR>(state, block);
                                badfx(); // Right part alone
                            }
                        }
                        else
                        {
                            c.scan_attr<svga::vtrgb, UseSGR>(state, block);
                            if (state.wdt() == 0) side_badfx(); // Right part alone at the left side
                            else                  badfx(); // Right part alone
                        }
                    }
                }
            };
            auto eolfx = [&]
            {
                if (state.wdt() == 2) side_badfx();  // Left part alone at the right side
                state.set_gc();
                basevt::eol();
            };

            if (region)
            {
                if constexpr (UseSGR && Initial) basevt::nil();
                netxs::onrect(canvas, region, allfx, eolfx);
                if constexpr (Finalize)
                {
                    if (block.size()) block.pop_back(); // Pop last eol (lf).
                    if constexpr (UseSGR) basevt::nil();
                }
            }
            return block;
        }
        template<bool UseSGR = true, bool Initial = true, bool Finalize = true>
        auto& s11n(core const& canvas, rect region) // basevt: Ansify/textify content of specified region.
        {
            auto state = cell{};
            return s11n<UseSGR, Initial, Finalize>(canvas, region, state);
        }
        template<bool UseSGR = true, bool Initial = true, bool Finalize = true>
        auto& s11n(core const& canvas, cell& state) // basevt: Ansify/textify all content.
        {
            auto region = rect{-dot_mx / 2, dot_mx };
            return s11n<UseSGR, Initial, Finalize>(canvas, region, state);
        }
    };

    // ansi: Escaped sequences accumulator.
    class escx
        : public text,
          public basevt<escx>
    {
    public:
        escx()
            : basevt{ *this }
        { }

        template<class T>
        escx(T&& data)
            : basevt{ *this }
        {
            add(std::forward<T>(data));
        }

        auto& operator = (escx const& other)
        {
            text::clear();
            return add(other);
        }

        auto& shellmouse(bool b) // escx: Mouse shell integration on/off.
        {
            return add(b ? "\033[?1000;1006h"
                         : "\033[?1000;1006l");
        }
        auto& vmouse(bool b) // escx: Focus and Mouse position reporting/tracking.
        {
            return add(b ? "\033[?1002;1003;1004;1006;10060h"
                         : "\033[?1002;1003;1004;1006;10060l");
        }
        auto& setutf(bool b)        { return add(b ? "\033%G"      : "\033%@"        ); } // escx: Select UTF-8 character set (true) or default (faux). Not supported by Apple Terminal on macOS.
        auto& altbuf(bool b)        { return add(b ? "\033[?1049h" : "\033[?1049l"   ); } // escx: Alternative buffer.
        auto& cursor(bool b)        { return add(b ? "\033[?25h"   : "\033[?25l"     ); } // escx: Caret visibility.
        auto& appkey(bool b)        { return add(b ? "\033[?1h"    : "\033[?1l"      ); } // escx: Application(=on)/ANSI(=off) Caret Keys (DECCKM).
        auto& bpmode(bool b)        { return add(b ? "\033[?2004h" : "\033[?2004l"   ); } // escx: Set bracketed paste mode.
        auto& autowr(bool b)        { return add(b ? "\033[?7h"    : "\033[?7l"      ); } // escx: Set autowrap mode.
        auto& report(twod const& p) { return add("\033[", p.y+1, ";", p.x+1, "R"     ); } // escx: Report 1-Based caret position (CPR).
        auto& win_sz(twod const& p) { return add("\033[", p.y, ";", p.x, "t"         ); } // escx: Report viewport size (Reply on CSI 18 t).
        auto& locate_wipe()         { return add("\033[r"                            ); } // escx: Enable scrolling for entire display (clear screen).
        auto& locate_call()         { return add("\033[6n"                           ); } // escx: Report caret position.
        auto& scrn_reset()          { return add("\033[H\033[m\033[2J"               ); } // escx: Reset palette, erase scrollback and reset caret location.
        auto& save_title()          { return add("\033[22;0t"                        ); } // escx: Save terminal window title.
        auto& load_title()          { return add("\033[23;0t"                        ); } // escx: Restore terminal window title.
        auto& osc(view p, view arg) { return add("\033]", p, ';', arg,        c0_bel ); } // escx: OSC report.
        auto& header(view t)        { return add("\033]2;", t,                c0_bel ); } // escx: Window title.
        auto& save_palette()        { return add("\033[#P"                           ); } // escx: Push palette onto stack XTPUSHCOLORS.
        auto& load_palette()        { return add("\033[#Q"                           ); } // escx: Pop  palette from stack XTPOPCOLORS.
        auto& old_palette_reset()   { return add("\033]R"                            ); } // escx: Reset color palette (Linux console).
        auto& clipbuf(twod size, view utf8, si32 form) // escx: Set clipboard buffer.
        {
            return add("\033]52;", mime::meta(size, form), ";", utf::base64(utf8), c0_bel);
        }
        auto& old_palette(si32 i, rgba const& c) // escx: Set color palette (Linux console).
        {
            return add("\033]P", utf::to_hex(i, 1), utf::to_hex(c.chan.r, 2),
                                                    utf::to_hex(c.chan.g, 2),
                                                    utf::to_hex(c.chan.b, 2), '\033');
        }
        auto& osc_palette(si32 i, rgba const& c) // escx: Set color palette. ESC ] 4 ; <i> ; rgb : <r> / <g> / <b> BEL.
        {
            return add("\033]4;", i, ";rgb:", utf::to_hex(c.chan.r), '/',
                                              utf::to_hex(c.chan.g), '/',
                                              utf::to_hex(c.chan.b), c0_bel);
        }
        auto& osc_palette_reset() // escx: Reset color palette.
        {
            for (auto i = 0; i < 16; i++)
            {
                osc_palette(i, rgba::vt256[i]);
            }
            return *this;
        }
        auto& set_palette(bool legacy_color)
        {
            if (legacy_color)
            {
                save_palette();
                rgba::set_vtm16_palette([&](auto ...Args){ escx::old_palette(Args...); });
                rgba::set_vtm16_palette([&](auto ...Args){ escx::osc_palette(Args...); });
            }
            return *this;
        }
        auto& rst_palette(bool legacy_color)
        {
            if (legacy_color)
            {
                old_palette_reset();
                osc_palette_reset();
                load_palette();
            }
            return *this;
        }
        template<class T>
        auto& mouse_sgr(T const& gear, twod const& coor) // escx: Mouse tracking report (SGR).
        {
            using hids = T;
            static constexpr auto left     = si32{ 0  };
            static constexpr auto mddl     = si32{ 1  };
            static constexpr auto rght     = si32{ 2  };
            static constexpr auto btup     = si32{ 3  };
            static constexpr auto idle     = si32{ 32 };
            static constexpr auto wheel_up = si32{ 64 };
            static constexpr auto wheel_dn = si32{ 65 };

            auto ctrl = si32{};
            if (gear.m.ctlstat & hids::anyShift) ctrl |= 0x04;
            if (gear.m.ctlstat & hids::anyAlt  ) ctrl |= 0x08;
            if (gear.m.ctlstat & hids::anyCtrl ) ctrl |= 0x10;

            auto m_bttn = std::bitset<8>{ gear.m.buttons };
            auto s_bttn = std::bitset<8>{ gear.s.buttons };
            auto m_left = m_bttn[hids::left  ];
            auto m_rght = m_bttn[hids::right ];
            auto m_mddl = m_bttn[hids::middle];
            auto s_left = s_bttn[hids::left  ];
            auto s_rght = s_bttn[hids::right ];
            auto s_mddl = s_bttn[hids::middle];
            auto pressed = bool{};

            if (m_left != s_left)
            {
                ctrl |= left;
                pressed = m_left;
            }
            else if (m_rght != s_rght)
            {
                ctrl |= rght;
                pressed = m_rght;
            }
            else if (m_mddl != s_mddl)
            {
                ctrl |= mddl;
                pressed = m_mddl;
            }
            else if (gear.m.wheeled)
            {
                ctrl |= gear.m.wheeldt > 0 ? wheel_up
                                           : wheel_dn;
                pressed = true;
            }
            else if (gear.m.buttons)
            {
                     if (m_left) ctrl |= left;
                else if (m_rght) ctrl |= rght;
                else if (m_mddl) ctrl |= mddl;
                ctrl |= idle;
                pressed = true;
            }
            else
            {
                ctrl |= idle + btup;
                pressed = faux;
            }
            return add("\033[<", ctrl, ';',
                           coor.x + 1, ';',
                           coor.y + 1, pressed ? 'M' : 'm');
        }
        template<class T>
        auto& mouse_x11(T const& gear, twod const& coor, bool utf8) // escx: Mouse tracking report (X11).
        {
            using hids = T;
            static constexpr auto left     = si32{ 0  };
            static constexpr auto mddl     = si32{ 1  };
            static constexpr auto rght     = si32{ 2  };
            static constexpr auto btup     = si32{ 3  };
            static constexpr auto idle     = si32{ 32 };
            static constexpr auto wheel_up = si32{ 64 };
            static constexpr auto wheel_dn = si32{ 65 };

            auto ctrl = si32{};
            if (gear.m.ctlstat & hids::anyShift) ctrl |= 0x04;
            if (gear.m.ctlstat & hids::anyAlt  ) ctrl |= 0x08;
            if (gear.m.ctlstat & hids::anyCtrl ) ctrl |= 0x10;

            auto m_bttn = std::bitset<8>{ gear.m.buttons };
            auto s_bttn = std::bitset<8>{ gear.s.buttons };
            auto m_left = m_bttn[hids::left  ];
            auto m_rght = m_bttn[hids::right ];
            auto m_mddl = m_bttn[hids::middle];
            auto s_left = s_bttn[hids::left  ];
            auto s_rght = s_bttn[hids::right ];
            auto s_mddl = s_bttn[hids::middle];

                 if (m_left != s_left) ctrl |= m_left ? left : btup;
            else if (m_rght != s_rght) ctrl |= m_rght ? rght : btup;
            else if (m_mddl != s_mddl) ctrl |= m_mddl ? mddl : btup;
            else if (gear.m.wheeled  ) ctrl |= gear.m.wheeldt > 0 ? wheel_up
                                                                  : wheel_dn;
            else if (gear.m.buttons)
            {
                     if (m_left) ctrl |= left;
                else if (m_rght) ctrl |= rght;
                else if (m_mddl) ctrl |= mddl;
                ctrl |= idle;
            }
            else ctrl |= idle + btup;

            if (utf8)
            {
                add("\033[M");
                utf::to_utf_from_code(std::clamp(ctrl,       0, si16max - 32) + 32, *this);
                utf::to_utf_from_code(std::clamp(coor.x + 1, 1, si16max - 32) + 32, *this);
                utf::to_utf_from_code(std::clamp(coor.y + 1, 1, si16max - 32) + 32, *this);
            }
            else
            {
                add("\033[M", static_cast<char>(std::clamp(ctrl,       0, 127-32) + 32),
                              static_cast<char>(std::clamp(coor.x + 1, 1, 127-32) + 32),
                              static_cast<char>(std::clamp(coor.y + 1, 1, 127-32) + 32));
            }

            return *this;
        }
        auto& w32keybd(si32 Vk, si32 Sc, si32 Uc, si32 Kd, si32 Cs, si32 Rc) // escx: win32-input-mode sequence (keyboard).
        {
            // \033 [ Vk ; Sc ; Uc ; Kd ; Cs ; Rc _
            return add("\033[", Vk, ';',      // Vk: the value of wVirtualKeyCode - any number. If omitted, defaults to '0'.
                                Sc, ';',      // Sc: the value of wVirtualScanCode - any number. If omitted, defaults to '0'.
                                Uc, ';',      // Uc: the decimal value of UnicodeChar - for example, NUL is "0", LF is "10", the character 'A' is "65". If omitted, defaults to '0'.
                                Kd, ';',      // Kd: the value of bKeyDown - either a '0' or '1'. If omitted, defaults to '0'.
                                Cs, ';',      // Cs: the value of dwControlKeyState - any number. If omitted, defaults to '0'.
                                Rc, w32_inp); // Rc: the value of wRepeatCount - any number. If omitted, defaults to '1'.
        }
        // Private vt command subset.
        //todo use '_' instead of 'p' in csi_ccc
        auto& nop()              { return add("\033["   ,      csi_ccc); } // escx: No operation. Split the text run.
        auto& rst()              { return add("\033[1"  ,      csi_ccc); } // escx: Reset formatting parameters.
        auto& tbs(si32 n)        { return add("\033[5:" , n  , csi_ccc); } // escx: Tabulation step length.
        auto& chx(si32 n)        { return add("\033[21:", n  , csi_ccc); } // escx: Caret 0-based horizontal absolute.
        auto& chy(si32 n)        { return add("\033[22:", n  , csi_ccc); } // escx: Caret 0-based vertical absolute.
        auto& cpx(si32 n)        { return add("\033[3:" , n  , csi_ccc); } // escx: Caret horizontal percent position.
        auto& cpy(si32 n)        { return add("\033[4:" , n  , csi_ccc); } // escx: Caret vertical percent position.
        auto& cup(twod const& p) { return add("\033[20:", p.y, ':',        // escx: 0-Based caret position.
                                                          p.x, csi_ccc); }
        auto& cpp(twod const& p) { return add("\033[2:" , p.x, ':',        // escx: Caret percent position.
                                                          p.y, csi_ccc); }
        auto& mgn(side const& n) { return add("\033[6:" , n.l, ':',        // escx: Margin (left, right, top, bottom).
                                                          n.r, ':',
                                                          n.t, ':',
                                                          n.b, csi_ccc); }
        auto& mgl(si32 n)        { return add("\033[7:" , n  , csi_ccc); } // escx: Left margin. Positive - native binding. Negative - opposite binding.
        auto& mgr(si32 n)        { return add("\033[8:" , n  , csi_ccc); } // escx: Right margin. Positive - native binding. Negative - opposite binding.
        auto& mgt(si32 n)        { return add("\033[9:" , n  , csi_ccc); } // escx: Top margin. Positive - native binding. Negative - opposite binding.
        auto& mgb(si32 n)        { return add("\033[10:", n  , csi_ccc); } // escx: Bottom margin. Positive - native binding. Negative - opposite binding.
        auto& jet(bias n)        { return add("\033[11:", n  , csi_ccc); } // escx: Text alignment.
        auto& wrp(wrap n)        { return add("\033[12:", n  , csi_ccc); } // escx: Text wrapping.
        auto& rtl(rtol n)        { return add("\033[13:", n  , csi_ccc); } // escx: Text right-to-left.
        auto& rlf(feed n)        { return add("\033[14:", n  , csi_ccc); } // escx: Reverse line feed.
        auto& jet_or(bias n)     { return add("\033[15:", n  , csi_ccc); } // escx: Text alignment.
        auto& wrp_or(wrap n)     { return add("\033[16:", n  , csi_ccc); } // escx: Text wrapping.
        auto& rtl_or(rtol n)     { return add("\033[17:", n  , csi_ccc); } // escx: Text right-to-left.
        auto& rlf_or(feed n)     { return add("\033[18:", n  , csi_ccc); } // escx: Reverse line feed.
        auto& idx(si32 i)        { return add("\033[19:", i  , csi_ccc); } // escx: Split the text run and associate the fragment with an id.
        auto& ref(si32 i)        { return add("\033[23:", i  , csi_ccc); } // escx: Create the reference to the existing paragraph.
        auto& show_mouse(si32 b) { return add("\033[26:", b  , csi_ccc); } // escx: Should the mouse poiner to be drawn.
        auto& link(si32 i)       { return add("\033[31:", i  , csi_ccc); } // escx: Set object id link.
        auto& styled(si32 b)     { return add("\033[32:", b  , csi_ccc); } // escx: Enable line style reporting (0/1).
        auto& style(ui32 i)      { return add("\033[33:", i  , csi_ccc); } // escx: Line style response (deco::format: alignment, wrapping, RTL, etc).
    };

    template<bool UseSGR = true, bool Initial = true, bool Finalize = true>
    static auto s11n(core const& canvas, rect region) // ansi: Ansify/textify content of specified region.
    {
        return escx{}.s11n<UseSGR, Initial, Finalize>(canvas, region);
    }
    template<class ...Args>
    static auto clipbuf(Args&&... data) { return escx{}.clipbuf(std::forward<Args>(data)...); } // ansi: Set clipboard.
    template<class ...Args>
    static auto add(Args&&... data)   { return escx{}.add(std::forward<Args>(data)...); } // ansi: Add text.
    template<class ...Args>
    static auto clr(rgba c, Args&&... data) { return escx{}.clr(c, std::forward<Args>(data)...); } // ansi: Add colored message.
    template<class ...Args>
    static auto err(Args&&... data)   { return escx{}.err(std::forward<Args>(data)...); } // ansi: Add error message.
    template<class ...Args>
    static auto hi(Args&&... data)    { return escx{}.hi(std::forward<Args>(data)...); } // ansi: Add highlighted message.
    static auto cup(twod const& n)    { return escx{}.cup(n);        } // ansi: 0-Based caret position.
    static auto cuu(si32 n)           { return escx{}.cuu(n);        } // ansi: Caret up.
    static auto cud(si32 n)           { return escx{}.cud(n);        } // ansi: Caret down.
    static auto cuf(si32 n)           { return escx{}.cuf(n);        } // ansi: Caret forward.
    static auto cub(si32 n)           { return escx{}.cub(n);        } // ansi: Caret backward.
    static auto cnl(si32 n)           { return escx{}.cnl(n);        } // ansi: Caret next line.
    static auto cpl(si32 n)           { return escx{}.cpl(n);        } // ansi: Caret previous line.
    static auto ocx(si32 n)           { return escx{}.ocx(n);        } // ansi: Caret 1-based horizontal absolute.
    static auto ocy(si32 n)           { return escx{}.ocy(n);        } // ansi: Caret 1-based vertical absolute.
    static auto chx(si32 n)           { return escx{}.chx(n);        } // ansi: Caret 0-based horizontal absolute.
    static auto chy(si32 n)           { return escx{}.chy(n);        } // ansi: Caret 0-based vertical absolute.
    static auto fwd(si32 n)           { return escx{}.fwd(n);        } // ansi: Move caret n cell in line.
    static auto dch(si32 n)           { return escx{}.dch(n);        } // ansi: Delete (not Erase) letters under the cursor.
    static auto del()                 { return escx{}.del( );        } // ansi: Delete cell backwards ('\x7F').
    static auto bld(bool b = true)    { return escx{}.bld(b);        } // ansi: SGR 𝗕𝗼𝗹𝗱 attribute.
    static auto und(si32 n = 1   )    { return escx{}.und(n);        } // ansi: SGR 𝗨𝗻𝗱𝗲𝗿𝗹𝗶𝗻𝗲 attribute. 0 - no underline, 1 - single, 2 - double.
    static auto blk(bool b = true)    { return escx{}.blk(b);        } // ansi: SGR Blink attribute.
    static auto inv(bool b = true)    { return escx{}.inv(b);        } // ansi: SGR 𝗡𝗲𝗴𝗮𝘁𝗶𝘃𝗲 attribute.
    static auto itc(bool b = true)    { return escx{}.itc(b);        } // ansi: SGR 𝑰𝒕𝒂𝒍𝒊𝒄 attribute.
    static auto stk(bool b = true)    { return escx{}.stk(b);        } // ansi: SGR Strikethrough attribute.
    static auto ovr(bool b = true)    { return escx{}.ovr(b);        } // ansi: SGR Overline attribute.
    static auto fgc(rgba const& n)    { return escx{}.fgc(n);        } // ansi: SGR Foreground color.
    static auto bgc(rgba const& n)    { return escx{}.bgc(n);        } // ansi: SGR Background color.
    static auto fgx(rgba const& n)    { return escx{}.fgx(n);        } // ansi: SGR Foreground color with alpha.
    static auto bgx(rgba const& n)    { return escx{}.bgx(n);        } // ansi: SGR Background color with alpha.
    static auto fgc()                 { return escx{}.fgc( );        } // ansi: Set default foreground color.
    static auto bgc()                 { return escx{}.bgc( );        } // ansi: Set default background color.
    static auto sav()                 { return escx{}.sav( );        } // ansi: Save SGR attributes.
    static auto nil()                 { return escx{}.nil( );        } // ansi: Reset (restore) SGR attributes.
    static auto nop()                 { return escx{}.nop( );        } // ansi: No operation. Split the text run.
    static auto rst()                 { return escx{}.rst( );        } // ansi: Reset formatting parameters.
    static auto eol()                 { return escx{}.eol( );        } // ansi: EOL.
    static auto edl()                 { return escx{}.edl( );        } // ansi: EDL.
    static auto scp()                 { return escx{}.scp( );        } // ansi: Save caret position in memory.
    static auto rcp()                 { return escx{}.rcp( );        } // ansi: Restore caret position from memory.
    static auto del(si32 n)           { return escx{}.del(n);        } // ansi: CSI n J  Erase display.
    static auto pushsgr()             { return escx{}.pushsgr();     } // ansi: Push SGR attrs onto stack.
    static auto popsgr()              { return escx{}.popsgr();      } // ansi: Pop  SGR attrs from stack.
    static auto cpp(twod const& n)    { return escx{}.cpp(n);        } // ansi: Caret percent position.
    static auto cpx(si32 n)           { return escx{}.cpx(n);        } // ansi: Caret horizontal percent position.
    static auto cpy(si32 n)           { return escx{}.cpy(n);        } // ansi: Caret vertical percent position.
    static auto tbs(si32 n)           { return escx{}.tbs(n);        } // ansi: Tabulation step length.
    static auto mgn(side const& n)    { return escx{}.mgn(n);        } // ansi: Margin (left, right, top, bottom).
    static auto mgl(si32 n)           { return escx{}.mgl(n);        } // ansi: Left margin.
    static auto mgr(si32 n)           { return escx{}.mgr(n);        } // ansi: Right margin.
    static auto mgt(si32 n)           { return escx{}.mgt(n);        } // ansi: Top margin.
    static auto mgb(si32 n)           { return escx{}.mgb(n);        } // ansi: Bottom margin.
    static auto fcs(bool b)           { return escx{}.fcs(b);        } // ansi: Terminal window focus.
    static auto jet(bias n)           { return escx{}.jet(n);        } // ansi: Text alignment.
    static auto wrp(wrap n)           { return escx{}.wrp(n);        } // ansi: Text wrapping.
    static auto rtl(rtol n)           { return escx{}.rtl(n);        } // ansi: Text right-to-left.
    static auto rlf(feed n)           { return escx{}.rlf(n);        } // ansi: Reverse line feed.
    static auto jet_or(bias n)        { return escx{}.jet_or(n);     } // ansi: Set text alignment if it is not set.
    static auto wrp_or(wrap n)        { return escx{}.wrp_or(n);     } // ansi: Set text wrapping if it is not set.
    static auto rtl_or(rtol n)        { return escx{}.rtl_or(n);     } // ansi: Set text right-to-left if it is not set.
    static auto rlf_or(feed n)        { return escx{}.rlf_or(n);     } // ansi: Set reverse line feed if it is not set.
    static auto show_mouse(bool b)    { return escx{}.show_mouse(b); } // ansi: Should the mouse poiner to be drawn.
    static auto shellmouse(bool b)    { return escx{}.shellmouse(b); } // ansi: Mouse shell integration on/off.
    static auto vmouse(bool b)        { return escx{}.vmouse(b);     } // ansi: Mouse position reporting/tracking.
    static auto locate(twod const& n) { return escx{}.locate(n);     } // ansi: 1-Based caret position.
    static auto locate_wipe()         { return escx{}.locate_wipe(); } // ansi: Enable scrolling for entire display (clear screen).
    static auto locate_call()         { return escx{}.locate_call(); } // ansi: Report caret position.
    static auto scrn_reset()          { return escx{}.scrn_reset();  } // ansi: Reset palette, erase scrollback and reset caret location.
    static auto save_title()          { return escx{}.save_title();  } // ansi: Save terminal window title.
    static auto load_title()          { return escx{}.load_title();  } // ansi: Restore terminal window title.
    static auto setutf(bool b)        { return escx{}.setutf(b);     } // ansi: Select UTF-8 character set.
    static auto header(view t)        { return escx{}.header(t);     } // ansi: Window title.
    static auto altbuf(bool b)        { return escx{}.altbuf(b);     } // ansi: Alternative buffer.
    static auto cursor(bool b)        { return escx{}.cursor(b);     } // ansi: Caret visibility.
    static auto appkey(bool b)        { return escx{}.appkey(b);     } // ansi: Application cursor Keys (DECCKM).
    static auto bpmode(bool b)        { return escx{}.bpmode(b);     } // ansi: Set bracketed paste mode.
    static auto styled(si32 b)        { return escx{}.styled(b);     } // ansi: Enable line style reporting.
    static auto style(si32 i)         { return escx{}.style(i);      } // ansi: Line style report.
    static auto link(si32 i)          { return escx{}.link(i);       } // ansi: Set object id link.
    static auto ref(si32 i)           { return escx{}.ref(i);        } // ansi: Create the reference to the existing paragraph. Create new id if it is not existing.
    static auto idx(si32 i)           { return escx{}.idx(i);        } // ansi: Split the text run and associate the fragment with an id.
                                                                      //       All following text is under the IDX until the next command is issued.
                                                                      //       Redefine if the id already exists.
    // ansi: Caret forwarding instructions.
    // The order is important (see the richtext::flow::exec constexpr).
    //todo tie with richtext::flow::exec
    enum fn : si32
    {
        dx, // horizontal delta.
        dy, // vertical delta.
        ax, // x absolute (0-based).
        ay, // y absolute (0-based).

        //todo deprecated
        ox, // old format x absolute (1-based).
        oy, // old format y absolute (1-based).

        px, // x percent.
        py, // y percent.
        //ts, // set tab size.
        tb, // tab forward.
        nl, // next line and reset x to west (carriage return).
        //br, // text wrap mode (DECSET: CSI ? 7 h/l Auto-wrap Mode (DECAWM) or CSI ? 45 h/l reverse wrap around mode).
        //yx, // bidi.
        //hz, // text horizontal alignment.
        //rf, // reverse (line) feed.

        //wl, // set left	horizontal wrapping field.
        //wr, // set right	horizontal wrapping field.
        //wt, // set top		vertical wrapping field.
        //wb, // set bottom	vertical wrapping field.

        sc, // save caret position.
        rc, // load caret position.
        zz, // all params reset to zero.

        //todo revise/deprecated
        // ansi: Paint instructions. The order is important (see the mill).
        // CSI Ps J  Erase in Display (ED), VT100.
        ed, // Ps = 0  ⇒  Erase Below (default).
            // Ps = 1  ⇒  Erase Above.
            // Ps = 2  ⇒  Erase All.
            // Ps = 3  ⇒  Erase Scrollback

        // CSI Ps K  Erase in Line (EL), VT100. Caret position does not change.
        el, // Ps = 0  ⇒  Erase to Right (default).
            // Ps = 1  ⇒  Erase to Left.
            // Ps = 2  ⇒  Erase All.

        fn_count
    };

    // ansi: Caret control sequence: one command with one argument.
    struct rule
    {
        si32 cmd;
        si32 arg;
    };
    struct mark
        : public cell
    {
        cell spare; // mark: Stored  brush.
        cell fresh; // mark: Initial brush.
        mark() = default;
        mark(mark&&) = default;
        mark(mark const&) = default;
        mark& operator = (mark const&) = default;
        mark& operator = (cell const& c)
        {
            cell::operator=(c);
            return *this;
        }
        mark(cell const& brush)
            : cell { brush },
              fresh{ brush },
              spare{ brush }
        { }
        void reset()              { *this = spare = fresh;     }
        void reset(cell const& c) { *this = spare = fresh = c; }
        auto busy() const         { return  fresh != *this;    } // mark: Is the marker modified.
        void  sav()               { spare.set(*this);          } // mark: Save current SGR attributes.
        void  sfg(rgba c)         { spare.fgc(c);              } // mark: Set default foreground color.
        void  sbg(rgba c)         { spare.bgc(c);              } // mark: Set default background color.
        void  nil()               { this->set(spare);          } // mark: Restore saved SGR attributes.
        void  rfg()               { this->fgc(spare.fgc());    } // mark: Reset SGR Foreground color.
        void  rbg()               { this->bgc(spare.bgc());    } // mark: Reset SGR Background color.
    };
    struct deco
    {
        enum type : si32
        {
            leftside, // default
            rghtside,
            centered,
            autowrap,
            count,
        };

        static constexpr auto defwrp = wrap::on;    // deco: Default autowrap behavior.
        static constexpr auto maxtab = si32{ 255 }; // deco: Tab length limit.

        wrap wrapln : 2 = {}; // deco: Autowrap.
        bias adjust : 2 = {}; // deco: Horizontal alignment.
        rtol r_to_l : 2 = {}; // deco: RTL.
        feed rlfeed : 2 = {}; // deco: Reverse line feed.
        byte tablen : 8 = {}; // deco: Tab length.
        dent margin     = {}; // deco: Page margins.

        deco() = default;
        deco(ui32 format)
            : wrapln{ (byte)((format >> 0) & 0x03) },
              adjust{ (byte)((format >> 2) & 0x03) },
              r_to_l{ (byte)((format >> 4) & 0x03) },
              rlfeed{ (byte)((format >> 6) & 0x03) },
              tablen{ (byte)((format >> 8) & 0xFF) }
        { }
        bool operator==(deco const&) const = default;
        // deco: Return serialized deco (wo margins).
        auto format() const
        {
            return (ui32)wrapln << 0 |
                   (ui32)adjust << 2 |
                   (ui32)r_to_l << 4 |
                   (ui32)rlfeed << 6 |
                   (ui32)tablen << 8;
        }
        auto  wrp   () const  { return wrapln;                                      } // deco: Return Auto wrapping.
        auto  jet   () const  { return adjust;                                      } // deco: Return Paragraph adjustment.
        auto  rtl   () const  { return r_to_l;                                      } // deco: Return RTL.
        auto  rlf   () const  { return rlfeed;                                      } // deco: Return Reverse line feed.
        auto  tbs   () const  { return tablen;                                      } // deco: Return Reverse line feed.
        auto& mgn   () const  { return margin;                                      } // deco: Return margins.
        auto& wrp   (bool  b) { wrapln = b ? wrap::on  : wrap::off;   return *this; } // deco: Set auto wrapping.
        auto& rtl   (bool  b) { r_to_l = b ? rtol::rtl : rtol::ltr;   return *this; } // deco: Set RTL.
        auto& rlf   (bool  b) { rlfeed = b ? feed::rev : feed::fwd;   return *this; } // deco: Set revverse line feed.
        auto& wrp   (wrap  n) { wrapln = n;                           return *this; } // deco: Auto wrapping.
        auto& jet   (bias  n) { adjust = n;                           return *this; } // deco: Paragraph adjustment.
        auto& rtl   (rtol  n) { r_to_l = n;                           return *this; } // deco: RTL.
        auto& rlf   (feed  n) { rlfeed = n;                           return *this; } // deco: Reverse line feed.
        auto& wrp_or(wrap  n) { if (wrapln == wrap::none) wrapln = n; return *this; } // deco: Auto wrapping.
        auto& jet_or(bias  n) { if (adjust == bias::none) adjust = n; return *this; } // deco: Paragraph adjustment.
        auto& rtl_or(rtol  n) { if (r_to_l == rtol::none) r_to_l = n; return *this; } // deco: RTL.
        auto& rlf_or(feed  n) { if (rlfeed == feed::none) rlfeed = n; return *this; } // deco: Reverse line feed.
        auto& tbs   (si32  n) { tablen = std::min(n, maxtab);         return *this; } // deco: fx_ccc_tbs.
        auto& mgl   (si32  n) { margin.west.step = n;                 return *this; } // deco: fx_ccc_mgl.
        auto& mgr   (si32  n) { margin.east.step = n;                 return *this; } // deco: fx_ccc_mgr.
        auto& mgt   (si32  n) { margin.head.step = n;                 return *this; } // deco: fx_ccc_mgt.
        auto& mgb   (si32  n) { margin.foot.step = n;                 return *this; } // deco: fx_ccc_mgb.
        auto& mgn   (fifo& q) { margin.set(q);                        return *this; } // deco: fx_ccc_mgn.
        auto& rst   ()        { *this = {};                           return *this; } // deco: Reset.
        // deco: Reset to global default.
        constexpr auto& reset()
        {
            wrapln = deco::defwrp;
            adjust = bias::left;
            r_to_l = rtol::ltr;
            rlfeed = feed::fwd;
            tablen = 8;
            margin = {};
            return *this;
        }
        auto get_kind() const
        {
            return wrapln == wrap::on    ? type::autowrap :
                   adjust == bias::left  ? type::leftside :
                   adjust == bias::right ? type::rghtside :
                                           type::centered ;
        }
    };

    static constexpr auto def_style = deco{}.reset();

    struct runtime
    {
        bool iswrapln;
        bool isr_to_l;
        bool isrlfeed;
        bool straight; // runtime: Text substring retrieving direction.
        bool centered;
        bool arighted;
        si32 tabwidth;
        dent textpads;

        void combine(deco const& global, deco const& custom)
        {
            // Custom settings take precedence over global.
            auto s_wrp = custom.wrp(); iswrapln = s_wrp != wrap::none ? s_wrp == wrap::on  : global.wrp() == wrap::on;
            auto s_rtl = custom.rtl(); isr_to_l = s_rtl != rtol::none ? s_rtl == rtol::rtl : global.rtl() == rtol::rtl;
            auto s_rlf = custom.rlf(); isrlfeed = s_rlf != feed::none ? s_rlf == feed::rev : global.rlf() == feed::rev;
            auto s_tbs = custom.tbs(); tabwidth = s_tbs != 0          ? s_tbs              : global.tbs();
            auto s_jet = custom.jet();
            if (s_jet != bias::none)
            {
                arighted = s_jet == bias::right;
                centered = s_jet == bias::center;
            }
            else
            {
                auto g_jet = global.jet();
                arighted = g_jet == bias::right;
                centered = g_jet == bias::center;
            }
            straight = iswrapln || isr_to_l == arighted;
            // Combine local and global margins.
            textpads = global.mgn();
            textpads+= custom.mgn();
        }
    };

    // Parse these controls as a C0-like,
    // split paragraphs when flow direction changes, for example.
    struct marker
    {
        using changer = std::array<void (*)(cell&), ctrl::count>;
        changer	setter = {};
        marker()
        {
            setter[ctrl::alm                 ] = [](cell& p) { p.rtl(true);    };
            setter[ctrl::rlm                 ] = [](cell& p) { p.rtl(true);    };
            setter[ctrl::lrm                 ] = [](cell& p) { p.rtl(faux);    };
            setter[ctrl::shy                 ] = [](cell& p) { p.hyphen(true); };
            setter[ctrl::function_application] = [](cell& p) { p.fnappl(true); };
            setter[ctrl::invisible_times     ] = [](cell& p) { p.itimes(true); };
            setter[ctrl::invisible_separator ] = [](cell& p) { p.isepar(true); };
            setter[ctrl::invisible_plus      ] = [](cell& p) { p.inplus(true); };
            setter[ctrl::zwnbsp              ] = [](cell& p) { p.zwnbsp(true); };
        }
    };

    template<class Q, class C>
    using func = generics::tree<Q, C*, std::function<void(Q&, C*&)>>;

    template<class T, bool NoMultiArg = faux>
    struct csi_t
    {
        using tree = func<fifo, T>;

        tree table         ;
        tree table_quest   ;
        tree table_excl    ;
        tree table_gt      ;
        tree table_lt      ;
        tree table_equals  ;
        tree table_hash    ;
        tree table_dollarsn;
        tree table_space   ;
        tree table_dblqoute;
        tree table_sglqoute;
        tree table_asterisk;

        csi_t()
        {
           /* Contract for client p
            * Unicode
            * - void task(ansi::rule const& cmd);          // Proceed curses command.
            * - void meta(deco& old, deco& new);           // Proceed new style.
            * - void data(si32 count, grid const& proto);  // Proceed new cells.
            * SGR:
            * - void nil();                          // Reset all SGR to default.
            * - void sav();                          // Set current SGR as default.
            * - void rfg();                          // Reset foreground color to default.
            * - void rbg();                          // Reset background color to default.
            * - void fgc(rgba const& c);             // Set foreground color.
            * - void bgc(rgba const& c);             // Set background color.
            * - void bld(bool b);                    // Set bold attribute.
            * - void itc(bool b);                    // Set italic attribute.
            * - void inv(bool b);                    // Set inverse attribute.
            * - void stk(bool b);                    // Set strikethgh attribute.
            * - void und(si32 b);                    // Set underline attribute. 1 - single, 2 - double.
            * - void blk(bool b);                    // Set blink attribute.
            * - void ovr(bool b);                    // Set overline attribute.
            * - void wrp(bool b);                    // Set auto wrap.
            * - void jet(si32 b);                    // Set adjustment.
            * - void rtl(bool b);                    // Set reverse line feed.
            * - void link(id_t i);                   // Set object id link.
            */

            #define V [](auto& q, auto& p)
            #define F(t, q) p->task(rule{ fn::t, q })

            table_quest   .resize(0x100);
                table_quest[dec_set] = nullptr;
                table_quest[dec_rst] = nullptr;

            table_excl    .resize(0x100);
                table_excl[csi_exl_rst] = nullptr;

            table_gt      .resize(0x100);
            table_lt      .resize(0x100);
            table_equals  .resize(0x100);
            table_hash    .resize(0x100);
            table_dollarsn.resize(0x100);
            table_space   .resize(0x100);
            table_dblqoute.resize(0x100);
            table_sglqoute.resize(0x100);
            table_asterisk.resize(0x100);

            table         .resize(0x100);
                table[csi_cuu] = V{ F(dy,-q(1)); };              // fx_cuu
                table[csi_cud] = V{ F(dy, q(1)); };              // fx_cud
                table[csi_cuf] = V{ F(dx, q(1)); };              // fx_cuf
                table[csi_cub] = V{ F(dx,-q(1)); };              // fx_cub
                table[csi_cnl] = V{ F(nl, q(1)); };              // fx_cnl
                table[csi_cpl] = V{ F(nl,-q(1)); };              // fx_cpl
                table[csi_chx] = V{ F(ox, q(1)); };              // fx_ocx
                table[csi_chy] = V{ F(oy, q(1)); };              // fx_ocy
                table[csi_scp] = V{ F(sc,   0 ); };              // fx_scp
                table[csi_rcp] = V{ F(rc,   0 ); };              // fx_rcp
                table[csi_cup] = V{ F(oy, q(1)); F(ox, q(1)); }; // fx_ocp
                table[csi_hvp] = V{ F(oy, q(1)); F(ox, q(1)); }; // fx_ocp
                table[csi_hrm] = V{ /*Nothing, Replace mode*/ }; // fx_irm
                table[csi_lrm] = V{ /*Nothing, Replace mode*/ }; // fx_irm
                table[csi__ed] = nullptr;
                table[csi__el] = nullptr;
                table[csi_dch] = nullptr;
                table[csi_ech] = nullptr;
                table[csi_ich] = nullptr;
                table[csi__dl] = nullptr;
                table[decstbm] = nullptr;
                table[csi__sd] = nullptr;
                table[csi__su] = nullptr;
                table[csi_win] = nullptr;
                table[csi_dsr] = nullptr;

                auto& ccc = table[csi_ccc].resize(0x100);
                    ccc.template enable_multi_arg<NoMultiArg>();
                    ccc[ccc_cup] = V{ F(ay, q(0)); F(ax, q(0)); }; // fx_ccc_cup
                    ccc[ccc_cpp] = V{ F(py, q(0)); F(px, q(0)); }; // fx_ccc_cpp
                    ccc[ccc_chx] = V{ F(ax, q(0)); }; // fx_ccc_chx
                    ccc[ccc_chy] = V{ F(ay, q(0)); }; // fx_ccc_chy
                    ccc[ccc_cpx] = V{ F(px, q(0)); }; // fx_ccc_cpx
                    ccc[ccc_cpy] = V{ F(py, q(0)); }; // fx_ccc_cpy
                    ccc[ccc_rst] = V{ F(zz,   0 ); }; // fx_ccc_rst

                    ccc[ccc_mgn   ] = V{ p->style.mgn   (q   ); }; // fx_ccc_mgn
                    ccc[ccc_mgl   ] = V{ p->style.mgl   (q(0)); }; // fx_ccc_mgl
                    ccc[ccc_mgr   ] = V{ p->style.mgr   (q(0)); }; // fx_ccc_mgr
                    ccc[ccc_mgt   ] = V{ p->style.mgt   (q(0)); }; // fx_ccc_mgt
                    ccc[ccc_mgb   ] = V{ p->style.mgb   (q(0)); }; // fx_ccc_mgb
                    ccc[ccc_tbs   ] = V{ p->style.tbs   (q(0)); }; // fx_ccc_tbs
                    ccc[ccc_jet   ] = V{ p->style.jet   (static_cast<bias>(q(0))); }; // fx_ccc_jet
                    ccc[ccc_wrp   ] = V{ p->style.wrp   (static_cast<wrap>(q(0))); }; // fx_ccc_wrp
                    ccc[ccc_rtl   ] = V{ p->style.rtl   (static_cast<rtol>(q(0))); }; // fx_ccc_rtl
                    ccc[ccc_rlf   ] = V{ p->style.rlf   (static_cast<feed>(q(0))); }; // fx_ccc_rlf
                    ccc[ccc_jet_or] = V{ p->style.jet_or(static_cast<bias>(q(0))); }; // fx_ccc_or_jet
                    ccc[ccc_wrp_or] = V{ p->style.wrp_or(static_cast<wrap>(q(0))); }; // fx_ccc_or_wrp
                    ccc[ccc_rtl_or] = V{ p->style.rtl_or(static_cast<rtol>(q(0))); }; // fx_ccc_or_rtl
                    ccc[ccc_rlf_or] = V{ p->style.rlf_or(static_cast<feed>(q(0))); }; // fx_ccc_or_rlf

                    ccc[ccc_lnk   ] = V{ p->brush.link  (static_cast<id_t>(q(0))); }; // fx_ccc_lnk

                    ccc[ccc_nop] = nullptr;
                    ccc[ccc_idx] = nullptr;
                    ccc[ccc_ref] = nullptr;
                    ccc[ccc_sbs] = nullptr;
                    ccc[ccc_sms] = nullptr;
                    ccc[ccc_sgr] = nullptr;
                    ccc[ccc_sel] = nullptr;
                    ccc[ccc_pad] = nullptr;

                auto& sgr = table[csi_sgr].resize(0x100);
                    sgr.template enable_multi_arg<NoMultiArg>();
                    sgr[sgr_rst      ] = V{ p->brush.nil( );    }; // fx_sgr_rst      ;
                    sgr[sgr_sav      ] = V{ p->brush.sav( );    }; // fx_sgr_sav      ;
                    sgr[sgr_fg       ] = V{ p->brush.rfg( );    }; // fx_sgr_fg_def   ;
                    sgr[sgr_bg       ] = V{ p->brush.rbg( );    }; // fx_sgr_bg_def   ;
                    sgr[sgr_bold     ] = V{ p->brush.bld(true); }; // fx_sgr_bld<true>;
                    sgr[sgr_faint    ] = V{ p->brush.bld(faux); }; // fx_sgr_bld<faux>;
                    sgr[sgr_italic   ] = V{ p->brush.itc(true); }; // fx_sgr_itc<true>;
                    sgr[sgr_nonitalic] = V{ p->brush.itc(faux); }; // fx_sgr_itc<faux>;
                    sgr[sgr_inv      ] = V{ p->brush.inv(true); }; // fx_sgr_inv<true>;
                    sgr[sgr_noinv    ] = V{ p->brush.inv(faux); }; // fx_sgr_inv<faux>;
                    sgr[sgr_und      ] = V{ p->brush.und(   1); }; // fx_sgr_und;
                    sgr[sgr_doubleund] = V{ p->brush.und(   2); }; // fx_sgr_dnl;
                    sgr[sgr_nound    ] = V{ p->brush.und(faux); }; // fx_sgr_und;
                    sgr[sgr_slowblink] = V{ p->brush.blk(true); }; // fx_sgr_blk;
                    sgr[sgr_fastblink] = V{ p->brush.blk(true); }; // fx_sgr_blk;
                    sgr[sgr_no_blink ] = V{ p->brush.blk(faux); }; // fx_sgr_blk;
                    sgr[sgr_strike   ] = V{ p->brush.stk(true); }; // fx_sgr_stk<true>;
                    sgr[sgr_nostrike ] = V{ p->brush.stk(faux); }; // fx_sgr_stk<faux>;
                    sgr[sgr_overln   ] = V{ p->brush.ovr(true); }; // fx_sgr_ovr<faux>;
                    sgr[sgr_nooverln ] = V{ p->brush.ovr(faux); }; // fx_sgr_ovr<faux>;
                    sgr[sgr_fg_rgb   ] = V{ p->brush.fgc(q);    }; // fx_sgr_fg_rgb   ;
                    sgr[sgr_bg_rgb   ] = V{ p->brush.bgc(q);    }; // fx_sgr_bg_rgb   ;
                    sgr[sgr_fg_blk   ] = V{ p->brush.fgc(tint::blackdk  ); }; // fx_sgr_fg_16<tint::blackdk>  ;
                    sgr[sgr_fg_red   ] = V{ p->brush.fgc(tint::reddk    ); }; // fx_sgr_fg_16<tint::reddk>    ;
                    sgr[sgr_fg_grn   ] = V{ p->brush.fgc(tint::greendk  ); }; // fx_sgr_fg_16<tint::greendk>  ;
                    sgr[sgr_fg_ylw   ] = V{ p->brush.fgc(tint::yellowdk ); }; // fx_sgr_fg_16<tint::yellowdk> ;
                    sgr[sgr_fg_blu   ] = V{ p->brush.fgc(tint::bluedk   ); }; // fx_sgr_fg_16<tint::bluedk>   ;
                    sgr[sgr_fg_mgt   ] = V{ p->brush.fgc(tint::magentadk); }; // fx_sgr_fg_16<tint::magentadk>;
                    sgr[sgr_fg_cyn   ] = V{ p->brush.fgc(tint::cyandk   ); }; // fx_sgr_fg_16<tint::cyandk>   ;
                    sgr[sgr_fg_wht   ] = V{ p->brush.fgc(tint::whitedk  ); }; // fx_sgr_fg_16<tint::whitedk>  ;
                    sgr[sgr_fg_blk_lt] = V{ p->brush.fgc(tint::blacklt  ); }; // fx_sgr_fg_16<tint::blacklt>  ;
                    sgr[sgr_fg_red_lt] = V{ p->brush.fgc(tint::redlt    ); }; // fx_sgr_fg_16<tint::redlt>    ;
                    sgr[sgr_fg_grn_lt] = V{ p->brush.fgc(tint::greenlt  ); }; // fx_sgr_fg_16<tint::greenlt>  ;
                    sgr[sgr_fg_ylw_lt] = V{ p->brush.fgc(tint::yellowlt ); }; // fx_sgr_fg_16<tint::yellowlt> ;
                    sgr[sgr_fg_blu_lt] = V{ p->brush.fgc(tint::bluelt   ); }; // fx_sgr_fg_16<tint::bluelt>   ;
                    sgr[sgr_fg_mgt_lt] = V{ p->brush.fgc(tint::magentalt); }; // fx_sgr_fg_16<tint::magentalt>;
                    sgr[sgr_fg_cyn_lt] = V{ p->brush.fgc(tint::cyanlt   ); }; // fx_sgr_fg_16<tint::cyanlt>   ;
                    sgr[sgr_fg_wht_lt] = V{ p->brush.fgc(tint::whitelt  ); }; // fx_sgr_fg_16<tint::whitelt>  ;
                    sgr[sgr_bg_blk   ] = V{ p->brush.bgc(tint::blackdk  ); }; // fx_sgr_bg_16<tint::blackdk>  ;
                    sgr[sgr_bg_red   ] = V{ p->brush.bgc(tint::reddk    ); }; // fx_sgr_bg_16<tint::reddk>    ;
                    sgr[sgr_bg_grn   ] = V{ p->brush.bgc(tint::greendk  ); }; // fx_sgr_bg_16<tint::greendk>  ;
                    sgr[sgr_bg_ylw   ] = V{ p->brush.bgc(tint::yellowdk ); }; // fx_sgr_bg_16<tint::yellowdk> ;
                    sgr[sgr_bg_blu   ] = V{ p->brush.bgc(tint::bluedk   ); }; // fx_sgr_bg_16<tint::bluedk>   ;
                    sgr[sgr_bg_mgt   ] = V{ p->brush.bgc(tint::magentadk); }; // fx_sgr_bg_16<tint::magentadk>;
                    sgr[sgr_bg_cyn   ] = V{ p->brush.bgc(tint::cyandk   ); }; // fx_sgr_bg_16<tint::cyandk>   ;
                    sgr[sgr_bg_wht   ] = V{ p->brush.bgc(tint::whitedk  ); }; // fx_sgr_bg_16<tint::whitedk>  ;
                    sgr[sgr_bg_blk_lt] = V{ p->brush.bgc(tint::blacklt  ); }; // fx_sgr_bg_16<tint::blacklt>  ;
                    sgr[sgr_bg_red_lt] = V{ p->brush.bgc(tint::redlt    ); }; // fx_sgr_bg_16<tint::redlt>    ;
                    sgr[sgr_bg_grn_lt] = V{ p->brush.bgc(tint::greenlt  ); }; // fx_sgr_bg_16<tint::greenlt>  ;
                    sgr[sgr_bg_ylw_lt] = V{ p->brush.bgc(tint::yellowlt ); }; // fx_sgr_bg_16<tint::yellowlt> ;
                    sgr[sgr_bg_blu_lt] = V{ p->brush.bgc(tint::bluelt   ); }; // fx_sgr_bg_16<tint::bluelt>   ;
                    sgr[sgr_bg_mgt_lt] = V{ p->brush.bgc(tint::magentalt); }; // fx_sgr_bg_16<tint::magentalt>;
                    sgr[sgr_bg_cyn_lt] = V{ p->brush.bgc(tint::cyanlt   ); }; // fx_sgr_bg_16<tint::cyanlt>   ;
                    sgr[sgr_bg_wht_lt] = V{ p->brush.bgc(tint::whitelt  ); }; // fx_sgr_bg_16<tint::whitelt>  ;

            #undef F
            #undef V
        }

        void proceed(si32 cmd, T*& client) { table.execute(cmd, client); }
        void proceed           (fifo& q, T*& p) { table         .execute(q, p); }
        void proceed_quest     (fifo& q, T*& p) { table_quest   .execute(q, p); }
        void proceed_gt        (fifo& q, T*& p) { table_gt      .execute(q, p); }
        void proceed_lt        (fifo& q, T*& p) { table_lt      .execute(q, p); }
        void proceed_hash      (fifo& q, T*& p) { table_hash    .execute(q, p); }
        void proceed_equals    (fifo& q, T*& p) { table_equals  .execute(q, p); }
        void proceed_excl      (fifo& q, T*& p) { table_excl    .execute(q, p); }
        void proceed_dollarsn  (fifo& q, T*& p) { table_dollarsn.execute(q, p); }
        void proceed_space     (fifo& q, T*& p) { table_space   .execute(q, p); }
        void proceed_dblqoute  (fifo& q, T*& p) { table_dblqoute.execute(q, p); }
        void proceed_sglqoute  (fifo& q, T*& p) { table_sglqoute.execute(q, p); }
        void proceed_asterisk  (fifo& q, T*& p) { table_asterisk.execute(q, p); }
    };

    template<class T> struct _glb { static typename T::template vt_parser<T>          vt_parser; };
    template<class T>                      typename T::template vt_parser<T> _glb<T>::vt_parser;

    template<class T> inline void parse(view utf8, T*&  dest) { _glb<T>::vt_parser.parse(utf8, dest); }
    template<class T> inline void parse(view utf8, T*&& dest) { T* dptr = dest;    parse(utf8, dptr); }
    template<class T> inline auto& get_parser()               { return _glb<T>::vt_parser; }

    template<class T> using esc_t = func<qiew, T>;
    template<class T> using osc_h = std::function<void(view&, T*&)>;
    template<class T> using osc_t = std::map<text, osc_h<T>>;

    template<class T>
    struct vt_parser
    {
        ansi::esc_t<T> intro; // vt_parser:  C0 table.
        ansi::csi_t<T> csier; // vt_parser: CSI table.
        ansi::osc_t<T> oscer; // vt_parser: OSC table.
        si32           decsg; // vt_parser: Enable DEC Special Graphics Mode (if non zero).

        vt_parser()
            : decsg{ 0 }
        {
            intro.resize(ctrl::non_control);
            //intro[ctrl::bs ] = backspace;
            //intro[ctrl::del] = backspace;
            //intro[ctrl::cr ] = crlf;
            //intro[ctrl::eol] = exec <fn::nl, 1>;

            auto& esc = intro[ctrl::esc].resize(0x100);
                esc[esc_csi   ] = xcsi;
                esc[esc_ocs   ] = xosc;
                esc[esc_key_a ] = keym;
                esc[esc_key_n ] = keym;
                esc[esc_g0set ] = g0__;
                //esc[esc_sc] = ;
                //esc[esc_rc] = ;
                //esc['M'  ] = __ri;
        }

        // vt_parser: Static UTF-8/ANSI parser proc.
        void parse(view utf8, T*& client)
        {
            auto s = [&](auto& traits, auto& utf8)
            {
                qiew queue{ utf8 };
                intro.execute(traits.control, queue, client); // Make one iteration using firstcmd and return.
                return queue;
            };
            auto y = [&](auto const& cluster) { client->post(cluster); };

            utf::decode(s, y, utf8, decsg);
            client->flush();
        }
        // vt_parser: Static UTF-8/ANSI parser proc.
        void parse(view utf8, T*&& client)
        {
            T* p = client;
            parse(utf8, p);
        }

    private:
        // vt_parser: Control Sequence Introducer (CSI) parser.
        static void xcsi(qiew& ascii, T*& client)
        {
            // Take the control sequence from the string until CSI (cmd >= 0x40 && cmd <= 0x7E) command occured
            // ESC [ n1 ; n2:p1:p2:...pi ; ... nx CSICMD
            //      [-----------------------]

            static constexpr auto maxarg = 32_sz; // ansi: Maximal number of the parameters in one escaped sequence.
            using fifo = generics::bank<si32, maxarg>;

            if (ascii.length())
            {
                auto b = '\0';
                auto ints = []  (unsigned char cmd) { return cmd >= 0x20 && cmd <= 0x2f; }; // "intermediate bytes" in the range 0x20–0x2F
                auto pars = []  (unsigned char cmd) { return cmd >= 0x3C && cmd <= 0x3f; }; // "parameter bytes" in the range 0x30–0x3F
                auto cmds = []  (unsigned char cmd) { return cmd >= 0x40 && cmd <= 0x7E; };
                auto isC0 = []  (unsigned char cmd) { return cmd <= 0x1F; };
                auto trap = [&] (auto& c) // Catch and execute C0.
                {
                    if (isC0(c))
                    {
                        auto& intro = _glb<T>::vt_parser.intro;
                        auto  empty = qiew{};
                        do
                        {
                            intro.execute(c, empty, client); // Make one iteration using firstcmd and return.
                            ascii.pop_front();
                            if (ascii.empty()) break;
                            c = ascii.front();
                        }
                        while (isC0(c));
                        return true;
                    }
                    return faux;
                };
                auto fill = [&] (auto& queue)
                {
                    auto a = ';';
                    auto push = [&](auto num) // Parse subparameters divided by colon ':' (max arg value<int32_t> is 1,073,741,823)
                    {
                        if (a == ':') queue.template push<true>(num);
                        else          queue.template push<faux>(num);
                    };

                    while (ascii.length())
                    {
                        if (auto param = utf::to_int(ascii))
                        {
                            push(param.value());
                            if (ascii.empty()) break;
                            a = ascii.front(); // Delimiter or cmd after number.
                            trap(a);
                            if (ascii.empty()) break;
                        }
                        else
                        {
                            auto c = ascii.front();
                            if (trap(c)) continue;
                            push(fifo::skip); // Default parameter expressed by standalone delimiter/semicolon.
                            a = c; // Delimiter or cmd after number.
                        }
                        ascii.pop_front();
                        if (cmds(a))
                        {
                            queue.settop(a);
                            break;
                        }
                        else if (ints(a)) b = a; // Intermediate byte and parameter byte never appear at the same time, so consider they as a single group.
                    }
                };

                auto& csier = _glb<T>::vt_parser.csier;
                auto c = ascii.front();
                if (cmds(c))
                {
                    ascii.pop_front();
                    csier.proceed(c, client);
                }
                else
                {
                    auto queue = fifo{ ccc_nop }; // Reserve for the command type.
                    if (pars(c))
                    {
                        ascii.pop_front();
                        fill(queue);
                             if (c == '?' ) csier.proceed_quest   (queue, client);
                        else if (c == '>' ) csier.proceed_gt      (queue, client);
                        else if (c == '<' ) csier.proceed_lt      (queue, client);
                        else if (c == '=' ) csier.proceed_equals  (queue, client);
                    }
                    else
                    {
                        fill(queue);
                             if (b == '\0') csier.proceed         (queue, client);
                        else if (b == '!' ) csier.proceed_excl    (queue, client);
                        else if (b == '#' ) csier.proceed_hash    (queue, client);
                        else if (b == '$' ) csier.proceed_dollarsn(queue, client);
                        else if (b == ' ' ) csier.proceed_space   (queue, client);
                        else if (b == '\"') csier.proceed_dblqoute(queue, client);
                        else if (b == '\'') csier.proceed_sglqoute(queue, client);
                        else if (b == '*' ) csier.proceed_asterisk(queue, client);
                    }
                }
            }
        }

        // vt_parser: Operating System Command (OSC) parser.
        static void xosc(qiew& ascii, T*& client)
        {
            // Take the string until ST (='\e\\'='ESC\' aka String Terminator) or BEL (='\x07')
            // n: si32
            // ST: ESC \  (0x9C, ST = String Terminator)
            // BEL: 0x07
            //
            // ESC ] n ; _text_ BEL
            //      [--------------]
            // or
            // ESC ] n ; _text_ ST
            // ESC ] n ; _text_ ESC \
            //      [--------------]
            //
            // ESC ] I ; _text_ ST  Set icon to file.
            // ESC ] l ; _text_ ST  Set window title.
            // ESC ] L ; _text_ ST  Set window icon label.
            //
            // ESC ] P Nrrggbb  Set N (hex) of 16color palette to rrggbb (hex).

            // Find ST and ';', if no ST or no ';' when drop
            if (ascii)
            {
                auto& oscer = _glb<T>::vt_parser.oscer;
                auto c = ascii.front();
                if (c == 'P') // OSC_LINUX_COLOR  Set linux console 16 colors palette.
                {
                    assert(ascii.length() >= 8);
                    auto cmd = text{ osc_linux_color };
                    if (auto it = oscer.find(cmd); it != oscer.end())
                    {
                        auto size = 7; // Nrrggbb
                        auto data = ascii.substr(1, size);
                        auto proc = (*it).second;
                        proc(data, client);
                    }
                    ascii.remove_prefix(8); // PNrrggbb
                    return;
                }
                else if (c == 'R') // OSC_LINUX_RESET  Reset linux console 16/256 colors palette.
                {
                    auto cmd = text{ osc_linux_reset };
                    if (auto it = oscer.find(cmd); it != oscer.end())
                    {
                        auto data = view{};
                        auto proc = (*it).second;
                        proc(data, client);
                    }
                    ascii.remove_prefix(1); // R
                    return;
                }

                auto base = ascii.data();
                auto head = base;
                auto tail = head + ascii.length();
                auto delm = tail; // Semicolon ';' position
                auto exec = [&](auto pad)
                {
                    auto cmd = text(base, delm);
                    ++delm;
                    auto size = head - delm;
                    if (auto it = oscer.find(cmd); it != oscer.end())
                    {
                        auto data = view(delm, size);
                        auto proc = (*it).second;
                        proc(data, client);
                    }
                    ascii.remove_prefix(head - base + pad); // Take the text and BEL or ST too.
                };

                while (head != tail)
                {
                    auto c = *head;
                    if (c == ';')
                    {
                        delm = head++;
                        while (head != tail)
                        {
                            unsigned char c = *head;
                            if (c <= c0_esc) // To avoid double comparing.
                            {
                                if (c == c0_bel)
                                {
                                    exec(1);
                                    return;
                                }
                                else if (c == c0_esc)
                                {
                                    auto next = std::next(head);
                                    if (next != tail && *next == '\\')
                                    {
                                        exec(2);
                                        return;
                                    }
                                }
                            }
                            ++head;
                        }
                        return; // Drop bcuz no ST in the sequence.
                    }
                    else if (c == c0_bel) return; // Drop bcuz no ';' in the sequence.
                    else if (c == c0_esc)
                    {
                        auto next = std::next(head);
                        if (next != tail && *next == '\\')
                        {
                            return; // Drop bcuz no ';' in the sequence.
                        }
                    }
                    ++head;
                }
            }
        }

        // vt_parser: Set keypad mode.
        static void keym(qiew& ascii, T*& p)
        {
            // Keypad mode	Application ESC =
            // Keypad mode	Numeric     ESC >

            //if (ascii)
            //{
            //	ascii.pop_front(); // Take mode specifier =/>
            //	//todo implement
            //}
        }

        // vt_parser: Designate G0 Character Set.
        static void g0__(qiew& ascii, T*& p)
        {
            // ESC ( C
            //      [-]
            if (ascii)
            {
                auto& parser = ansi::get_parser<T>();
                parser.decsg = ascii.front() == '0' ? 1 : 0; // '0' - DEC Special Graphics mode.
                ascii.pop_front(); // Take Final character C for designating 94-character sets.
            }
        }
    };

    struct parser
    {
        deco style{}; // parser: Parser style.
        deco state{}; // parser: Parser style last state.
        grid proto{}; // parser: Proto lyric.
        si32 count{}; // parser: Proto lyric length.
        mark brush{}; // parser: Parser brush.
        //text debug{};

        parser() = default;
        parser(deco style)
            : style{ style },
              state{ style }
        { };

        template<class T>
        struct vt_parser : public ansi::vt_parser<T>
        {
            using vt = ansi::vt_parser<T>;
            vt_parser() : vt()
            {
                if constexpr (requires{ T::parser_config(*this); })
                {
                    T::parser_config(*this);
                }
            }
        };

        void data(core& cooked)
        {
            if (auto count = cooked.size().x)
            {
                cooked.each([&](cell& c) { c.meta(brush); });
                data(count, cooked.pick());
            }
        }
        void post(utf::frag const& cluster)
        {
            static auto marker = ansi::marker{};

            auto& utf8 = cluster.text;
            auto& attr = cluster.attr;
            if (auto w = attr.ucwidth)
            {
                count += w;
                brush.set_gc(utf8, w);
                proto.push_back(brush);
                //debug += (debug.size() ? "_"s : ""s) + text(utf8);
            }
            else
            {
                if (auto set_prop = marker.setter[attr.control])
                {
                    if (proto.size())
                    {
                        set_prop(proto.back());
                    }
                    else
                    {
                        auto empty = brush;
                        empty.txt(whitespace).wdt(w);
                        set_prop(empty);
                        proto.push_back(empty);
                    }
                }
                else
                {
                    brush.set_gc(utf8, w);
                    proto.push_back(brush);
                }
                //auto i = utf::to_hex((size_t)attr.control, 5, true);
                //debug += (debug.size() ? "_<fn:"s : "<fn:"s) + i + ">"s;
            }
        }
        inline void flush_style()
        {
            if (state != style)
            {
                meta(state);
                state = style;
            }
        }
        inline void flush_data()
        {
            if (count)
            {
                data(count, proto);
                proto.clear();
                count = 0;
            }
        }
        inline void flush()
        {
            flush_style();
            flush_data();
        }
        virtual void meta(deco const& old_style)         { };
        virtual void data(si32 count, grid const& proto) { };
    };

    // ansi: Caret manipulation command list.
    struct writ
        : public std::list<ansi::rule>
    {
        using list = std::list<ansi::rule>;

        inline void  push(rule const& cmd)   { list::push_back(cmd); } // Append single command to the locus.
        inline void   pop()                  { list::pop_back();     } // Append single command to the locus.
        inline bool  bare()    const         { return list::empty(); } // Is it empty the list of commands?
        inline writ& kill()    { list::clear();        return *this; } // Clear command list.

        writ& rst()           { push({ fn::zz, 0   }); return *this; } // Reset formatting parameters. Do not clear the command list.
        writ& cpp(twod p)     { push({ fn::px, p.x });                 // Caret percent position.
                                push({ fn::py, p.y }); return *this; }
        writ& cpx(si32 x)     { push({ fn::px, x   }); return *this; } // Caret horizontal percent position.
        writ& cpy(si32 y)     { push({ fn::py, y   }); return *this; } // Caret vertical percent position.
        writ& cup(twod p)     { push({ fn::ay, p.y });                 // 0-Based caret position.
                                push({ fn::ax, p.x }); return *this; }
        writ& cuu(si32 n = 1) { push({ fn::dy,-n   }); return *this; } // Caret up.
        writ& cud(si32 n = 1) { push({ fn::dy, n   }); return *this; } // Caret down.
        writ& cuf(si32 n = 1) { push({ fn::dx, n   }); return *this; } // Caret forward.
        writ& cub(si32 n = 1) { push({ fn::dx,-n   }); return *this; } // Caret backward.
        writ& cnl(si32 n = 1) { push({ fn::nl, n   }); return *this; } // Caret next line.
        writ& cpl(si32 n = 1) { push({ fn::nl,-n   }); return *this; } // Caret previous line.
        writ& chx(si32 x)     { push({ fn::ax, x   }); return *this; } // Caret o-based horizontal absolute.
        writ& chy(si32 y)     { push({ fn::ay, y   }); return *this; } // Caret o-based vertical absolute.
        writ& scp()           { push({ fn::sc, 0   }); return *this; } // Save caret position in memory.
        writ& rcp()           { push({ fn::rc, 0   }); return *this; } // Restore caret position from memory.
    };

    // ansi: Checking ANSI/UTF-8 integrity and return a valid view.
    auto purify(qiew utf8)
    {
        if (utf8.size())
        {
            auto head = utf8.begin();
            auto tail = utf8.end();
            auto prev = tail;
            auto find = faux;
            do   find = *--prev == 0x1b; // find ESC
            while (head != prev && !find);

            if (find)
            {
                auto next = prev;
                if (++next != tail) // test bytes after ESC
                {
                    auto c = *next;
                    if (c == '[') // test CSI: ESC [ pn;...;pn cmd
                    {
                        while (++next != tail) // find CSI command: cmd >= 0x40 && cmd <= 0x7E
                        {
                            auto cmd = *next;
                            if (cmd >= 0x40 && cmd <= 0x7E) break;
                        }
                        if (next == tail)
                        {
                            //utf8 = { head, prev }; //todo Clang 13.0.0 doesn't get it // preserve ESC at the end
                            utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                            return utf8;
                        }
                    }
                    else if (c == ']') // test OSC: ESC ] ... BEL
                    {
                        // test OSC: ESC ] P Nrrggbb
                        auto step = next;
                        if (++step != tail)
                        {
                            auto c = *step;
                            if (c == 'P') // Set linux console palette.
                            {
                                if (tail - step < 8)
                                {
                                    //utf8 = { head, prev }; //todo Clang 13.0.0 doesn't get it // preserve ESC at the end
                                    utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                                }
                                else
                                {
                                    utf::purify(utf8);
                                }
                                return utf8;
                            }
                            else if (c == 'R') // Reset linux console palette.
                            {
                                utf::purify(utf8);
                                return utf8;
                            }
                        }
                        while (++next != tail) // find BEL
                        {
                            auto cmd = *next;
                            if (cmd == 0x07) break;
                        }
                        if (next == tail)
                        {
                            //utf8 = { head, prev }; //todo Clang 13.0.0 doesn't get it // preserve ESC at the end
                            utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                            return utf8;
                        }
                    }
                    else if (c == '\\') // test ST: ESC \ ...
                    {
                        if (++next == tail)
                        {
                            return utf8;
                        }
                    }
                    // test Message/Command:
                    else if (c == 'P'  // DSC ESC P ... BEL
                          || c == 'X'  // SOS ESC X ... BEL
                          || c == '^'  // PM  ESC ^ ... BEL
                          || c == '_') // APC ESC _ ... BEL
                    {
                        while (++next != tail) // find BEL
                        {
                            auto cmd = *next;
                            if (cmd == 0x07) break;
                        }
                        if (next == tail)
                        {
                            //utf8 = { head, prev }; //todo Clang 13.0.0 doesn't get it // preserve ESC at the end
                            utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                            return utf8;
                        }
                    }
                    // test Esc + byte + rest:
                    else if (c == '('  // G0SET VT100  ESC ( c  94 characters
                          || c == ')'  // G1SET VT100  ESC ) c  94 characters
                          || c == '*'  // G2SET VT220  ESC * c  94 characters
                          || c == '+'  // G3SET VT220  ESC + c  94 characters
                          || c == '-'  // G1SET VT300  ESC - c  96 characters
                          || c == '.'  // G2SET VT300  ESC . c  96 characters
                          || c == '/'  // G3SET VT300  ESC / c  96 characters
                          || c == ' '  // ESC sp F, ESC sp G, ESC sp L, ESC sp M, ESC sp N
                          || c == '#'  // ESC # 3, ESC # 4, ESC # 5, ESC # 6, ESC # 8
                          || c == '%') // ESC % @, ESC % G  G: Select UTF-8, @: Select default
                    {
                        if (++next == tail)
                        {
                            //utf8 = { head, prev }; //todo Clang 13.0.0 doesn't get it // preserve ESC at the end
                            utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                        }
                    }
                    // test Esc + byte: ESC 7 8 D E H M ...
                    else if (c == '6'  // Back index, DECBI
                          || c == '7'  // Save    cursor coor and rendition state
                          || c == '8'  // Restore cursor coor and rendition state
                          || c == '9'  // Forward index, DECFI
                          || c == 'c'  // Full reset, RIS
                          || c == 'D'  // Caret down
                          || c == 'M'  // Caret up
                          || c == 'E'  // Next line
                          || c == 'F'  // Set cursor to lower leftmost coor
                          || c == 'H'  // Tabstop set
                          || c == '='  // Application keypad
                          || c == '>'  // Normal      keypad
                          || c == 'l'  // Memory lock
                          || c == 'm'  // Memory unlock
                          || c == 'n'  // LS2
                          || c == 'o'  // LS3
                          || c == '~'  // LS1R
                          || c == '}'  // LS2R
                          || c == '|'  // LS3R
                          || c == 'O'  // SS3
                          || c == 'N'  // SS2
                          || c == 'V'  // SPA
                          || c == 'W'  // EPA
                          || c == 'Z') // Return ID
                    {
                        if (++next == tail)
                        {
                            return utf8;
                        }
                    }
                }
                else
                {
                    //utf8 = { head, prev }; //todo Clang 13.0.0 doesn't get it // preserve ESC at the end
                    utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                    return utf8;
                }
            }
        }

        utf::purify(utf8);
        return utf8;
    }
}