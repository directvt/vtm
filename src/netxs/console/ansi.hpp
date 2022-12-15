// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_ANSI_HPP
#define NETXS_ANSI_HPP

#include "../ui/layout.hpp"
#include "../abstract/tree.hpp"
#include "../datetime/quartz.hpp"

#include <mutex>
#include <array>
#include <list>
#include <functional>

#define VT_PROC [](auto& q, auto& p)

namespace netxs::ansi
{
    using namespace netxs::ui::atoms;
    using namespace netxs::utf;

    static const auto ESCCSI = "\033[";
    static const auto ESCOCS = "\033]";

    static const auto ESC_CSI    = '['; // ESC [ ...
    static const auto ESC_OCS    = ']'; // ESC ] ...
    static const auto ESC_DSC    = 'P'; // ESC P ... BELL/ST
    static const auto ESC_SOS    = 'X'; // ESC X ... BELL/ST
    static const auto ESC_PM     = '^'; // ESC ^ ... BELL/ST
    static const auto ESC_APC    = '_'; // ESC _ ... BELL/ST

    static const auto ESC_G0SET  = '('; // ESC ( c
    static const auto ESC_G1SET  = ')'; // ESC ) c
    static const auto ESC_G2SET  = '*'; // ESC * c
    static const auto ESC_G3SET  = '+'; // ESC + c
    static const auto ESC_G1xSET = '-'; // ESC - c
    static const auto ESC_G2xSET = '.'; // ESC . c
    static const auto ESC_G3xSET = '/'; // ESC / c

    static const auto ESC_CTRL   = ' '; // ESC sp F, ESC sp G, ESC sp L, ESC sp M, ESC sp N
    static const auto ESC_DECDHL = '#'; // ESC # 3, ESC # 4, ESC # 5, ESC # 6, ESC # 8
    static const auto ESC_CHRSET = '%'; // ESC % @, ESC % G  G: Select UTF-8, @: Select default.

    static const auto ESC_ST     ='\\'; // ESC backslash
    static const auto ESC_DELIM  = ';'; // ESC ;
    static const auto ESC_KEY_A  = '='; // ESC =  Application keypad.
    static const auto ESC_KEY_N  = '>'; // ESC >  Normal      keypad.
    static const auto ESC_DECBI  = '6'; // ESC 6  Back index,    DECBI.
    static const auto ESC_DECFI  = '9'; // ESC 9  Forward index, DECFI.
    static const auto ESC_SC     = '7'; // ESC 7  Save    caret position and rendition state.
    static const auto ESC_RC     = '8'; // ESC 8  Restore caret position and rendition state.
    static const auto ESC_HTS    = 'H'; // ESC H  Set tabstop at the current caret position.
    static const auto ESC_NEL    = 'E'; // ESC E  Move caret down and CR.
    static const auto ESC_CLB    = 'F'; // ESC F  Move caret to lower leftmost position.
    static const auto ESC_IND    = 'D'; // ESC D  Caret down.
    static const auto ESC_IR     = 'M'; // ESC M  Caret up.
    static const auto ESC_RIS    = 'c'; // ESC c  Reset terminal to initial state.
    static const auto ESC_MEMLK  = 'l'; // ESC l  Memory lock.
    static const auto ESC_MUNLK  = 'm'; // ESC m  Memory unlock.
    static const auto ESC_LS2    = 'n'; // ESC n  LS2.
    static const auto ESC_LS3    = 'o'; // ESC o  LS3.
    static const auto ESC_LS1R   = '~'; // ESC ~  LS1R.
    static const auto ESC_LS2R   = '}'; // ESC }  LS2R.
    static const auto ESC_LS3R   = '|'; // ESC |  LS3R.
    static const auto ESC_SS3    = 'O'; // ESC O  SS3.
    static const auto ESC_SS2    = 'N'; // ESC N  SS2.
    static const auto ESC_SPA    = 'V'; // ESC V  SPA.
    static const auto ESC_EPA    = 'W'; // ESC W  EPA.
    static const auto ESC_RID    = 'Z'; // ESC Z  Return ID.

    static const auto CSI_SPC_SLC = '@'; // CSI n SP   @  — Shift left n columns(s).
    static const auto CSI_SPC_SRC = 'A'; // CSI n SP   A  — Shift right n columns(s).
    static const auto CSI_SPC_CST = 'q'; // CSI n SP   q  — Set caret style (DECSCUSR).

    static const auto CSI_HSH_SCP = 'P'; // CSI n #    P  — Push current palette colors onto stack. n default is 0.
    static const auto CSI_HSH_RCP = 'Q'; // CSI n #    Q  — Pop current palette colors from stack. n default is 0.
    static const auto CSI_HSH_SVA = 'p'; // CSI   #    p  — Push video attributes from stack (XTPUSHSGR).
    static const auto CSI_HSH_RVA = 'q'; // CSI   #    q  — Pop  video attributes from stack (XTPOPSGR).
    static const auto CSI_HSH_PUSH_SGR = '{'; // CSI # {  — Push SGR attributes onto stack (XTPUSHSGR).
    static const auto CSI_HSH_POP_SGR  = '}'; // CSI # }  — Pop  SGR attributes from stack (XTPOPSGR).

    static const auto CSI_DQT_SCP = 'q'; // CSI n "    q  — Select character protection attribute.

    static const auto CSI_EXL_RST = 'p'; // CSI   !    p  — Reset terminal to initial state.

    static const auto CSI_QST_RTB = 'W'; // CSI   ?    W  — Reset tabstops to the defaults.

    static const auto CSI_CUU = 'A';     // CSI n      A  — Caret Up.
    static const auto CSI_CUD = 'B';     // CSI n      B  — Caret Down.
    static const auto CSI_CUD2= 'e';     // CSI n      e  — Caret Down.
    static const auto CSI_CUF = 'C';     // CSI n      C  — Caret Forward.
    static const auto CSI_CUB = 'D';     // CSI n      D  — Caret Back.
    static const auto CSI_CNL = 'E';     // CSI n      E  — Caret Next Line.     Move n lines down and to the leftmost column.
    static const auto CSI_CPL = 'F';     // CSI n      F  — Caret Previous Line. Move n lines up   and to the leftmost column.
    static const auto CSI_CHX = 'G';     // CSI n      G  — Caret Horizontal Absolute.
    static const auto CSI_CHY = 'd';     // CSI n      d  — Caret Vertical Absolute.
    static const auto CSI_HVP = 'f';     // CSI n ; m  f  — Horizontal and Vertical Position.
    static const auto CSI_CUP = 'H';     // CSI n ; m  H  — Caret Position.
    static const auto CSI_CHT = 'I';     // CSI n      I  — Caret forward  n tab stops (default = 1).
    static const auto CSI_CBT = 'Z';     // CSI n      Z  — Caret backward n tab stops (default = 1).
    static const auto CSI_TBC = 'g';     // CSI n      g  — Reset tabstop value.
    static const auto CSI_SGR = 'm';     // CSI n [;k] m  — Select Graphic Rendition.
    static const auto CSI_DSR = 'n';     // CSI n      n  — Device Status Report (DSR). n==5 -> "OK"; n==6 -> CSI r ; c R
    static const auto DECSTBM = 'r';     // CSI t ; b  r  — Set scrolling region (t/b: top + bottom).
    static const auto CSI_SCP = 's';     // CSI        s  — Save caret Position.
    static const auto CSI_RCP = 'u';     // CSI        u  — Restore caret Position.
    static const auto CSI__EL = 'K';     // CSI n      K  — Erase 0: from caret to end, 1: from begin to caret, 2: all line.
    static const auto CSI__IL = 'L';     // CSI n      L  — Insert n blank lines.
    static const auto CSI__ED = 'J';     // CSI n      J  — Erase 0: from caret to end of screen, 1: from begin to caret, 2: all screen.
    static const auto CSI__DL = 'M';     // CSI n      M  — Delete n lines.
    static const auto CSI_DCH = 'P';     // CSI n      P  — Delete n character(s).
    static const auto CSI_LED = 'q';     // CSI n      q  — Load keyboard LEDs.
    static const auto CSI__SD = 'T';     // CSI n      T  — Scroll down by n lines, scrolled out lines are lost.
    static const auto CSI__SU = 'S';     // CSI n      S  — Scroll   up by n lines, scrolled out lines are lost.
    static const auto CSI_WIN = 't';     // CSI n;m;k  t  — XTWINOPS, Terminal window props.
    static const auto CSI_ECH = 'X';     // CSI n      X  — Erase n character(s) ? difference with delete ?
    static const auto CSI_ICH = '@';     // CSI n      @  — Insert/wedge n character(s).
    static const auto CSI_PDA = 'c';     // CSI n      c  — Send device attributes (Primary DA).
    static const auto DECSET  = 'h';     // CSI ? n    h  — DECSET.
    static const auto DECRST  = 'l';     // CSI ? n    l  — DECRST.
    static const auto CSI_hRM = 'h';     // CSI n      h  — Reset mode (always Replace mode n=4).
    static const auto CSI_lRM = 'l';     // CSI n      l  — Reset mode (always Replace mode n=4).
    static const auto CSI_CCC = 'p';     // CSI n [; x1; x2; ...; xn ] p — Private vt command subset.
    static const auto W32_INP = '_';     // CSI EVENT_TYPEn [; x1; x2; ...; xn ] _ — win32-input-mode.

    static const auto C0_NUL = '\x00'; // Null                - Originally used to allow gaps to be left on paper tape for edits. Later used for padding after a code that might take a terminal some time to process (e.g. a carriage return or line feed on a printing terminal). Now often used as a string terminator, especially in the programming language C.
    static const auto C0_SOH = '\x01'; // Start of Heading    - First character of a message header. In Hadoop, it is often used as a field separator.
    static const auto C0_STX = '\x02'; // Start of Text       - First character of message text, and may be used to terminate the message heading.
    static const auto C0_ETX = '\x03'; // End of Text         - Often used as a "break" character (Ctrl-C) to interrupt or terminate a program or process.
    static const auto C0_EOT = '\x04'; // End of Transmssn    - Often used on Unix to indicate end-of-file on a terminal.
    static const auto C0_ENQ = '\x05'; // Enquiry             - Signal intended to trigger a response at the receiving end, to see if it is still present.
    static const auto C0_ACK = '\x06'; // Acknowledge         - Response to an ENQ, or an indication of successful receipt of a message.
    static const auto C0_BEL = '\x07'; // Bell, Alert     \a  - Originally used to sound a bell on the terminal. Later used for a beep on systems that didn't have a physical bell. May also quickly turn on and off inverse video (a visual bell).
    static const auto C0_BS  = '\x08'; // Backspace       \b  - Move the caret one position leftwards. On input, this may delete the character to the left of the caret. On output, where in early computer technology a character once printed could not be erased, the backspace was sometimes used to generate accented characters in ASCII. For example, à could be produced using the three character sequence a BS ` (or, using the characters’ hex values, 0x61 0x08 0x60). This usage is now deprecated and generally not supported. To provide disambiguation between the two potential uses of backspace, the cancel character control code was made part of the standard C1 control set.
    static const auto C0_HT  = '\x09'; // Character       \t  - Tabulation, Horizontal Tabulation	\t	Position to the next character tab stop.
    static const auto C0_LF  = '\x0A'; // Line Feed       \n  - On typewriters, printers, and some terminal emulators, moves the caret down one row without affecting its column position. On Unix, used to mark end-of-line. In DOS, Windows, and various network standards, LF is used following CR as part of the end-of-line mark.
    static const auto C0_VT  = '\x0B'; // Line Tab,VTab   \v  - Position the form at the next line tab stop.
    static const auto C0_FF  = '\x0C'; // Form Feed       \f  - On printers, load the next page. Treated as whitespace in many programming languages, and may be used to separate logical divisions in code. In some terminal emulators, it clears the screen. It still appears in some common plain text files as a page break character, such as the RFCs published by IETF.
    static const auto C0_CR  = '\x0D'; // Carriage Return \r  - Originally used to move the caret to column zero while staying on the same line. On classic Mac OS (pre-Mac OS X), as well as in earlier systems such as the Apple II and Commodore 64, used to mark end-of-line. In DOS, Windows, and various network standards, it is used preceding LF as part of the end-of-line mark. The Enter or Return key on a keyboard will send this character, but it may be converted to a different end-of-line sequence by a terminal program.
    static const auto C0_SO  = '\x0E'; // Shift Out           - Switch to an alternative character set.
    static const auto C0_SI  = '\x0F'; // Shift In            - Return to regular character set after Shift Out.
    static const auto C0_DLE = '\x10'; // Data Link Escape    - Cause the following octets to be interpreted as raw data, not as control codes or graphic characters. Returning to normal usage would be implementation dependent.
    static const auto C0_DC1 = '\x11'; // Device Control One (XON)    - These four control codes are reserved for device control, with the interpretation dependent upon the device to which they were connected.
    static const auto C0_DC2 = '\x12'; // Device Control Two          > DC1 and DC2 were intended primarily to indicate activating a device while DC3 and DC4 were intended primarily to indicate pausing or turning off a device.
    static const auto C0_DC3 = '\x13'; // Device Control Three (XOFF) > DC1 and DC3 (known also as XON and XOFF respectively in this usage) originated as the "start and stop remote paper-tape-reader" functions in ASCII Telex networks.
    static const auto C0_DC4 = '\x14'; // Device Control Four         > This teleprinter usage became the de facto standard for software flow control.[6]
    static const auto C0_NAK = '\x15'; // Negative Acknowldg  - Sent by a station as a negative response to the station with which the connection has been set up. In binary synchronous communication protocol, the NAK is used to indicate that an error was detected in the previously received block and that the receiver is ready to accept retransmission of that block. In multipoint systems, the NAK is used as the not-ready reply to a poll.
    static const auto C0_SYN = '\x16'; // Synchronous Idle    - Used in synchronous transmission systems to provide a signal from which synchronous correction may be achieved between data terminal equipment, particularly when no other character is being transmitted.
    static const auto C0_ETB = '\x17'; // End of Transmission Block  - Indicates the end of a transmission block of data when data are divided into such blocks for transmission purposes.
    static const auto C0_CAN = '\x18'; // Cancel              - Indicates that the data preceding it are in error or are to be disregarded.
    static const auto C0_EM  = '\x19'; // End of medium       - Intended as means of indicating on paper or magnetic tapes that the end of the usable portion of the tape had been reached.
    static const auto C0_SUB = '\x1A'; // Substitute          - Originally intended for use as a transmission control character to indicate that garbled or invalid characters had been received. It has often been put to use for other purposes when the in-band signaling of errors it provides is unneeded, especially where robust methods of error detection and correction are used, or where errors are expected to be rare enough to make using the character for other purposes advisable. In DOS, Windows and other CP/M derivatives, it is used to indicate the end of file, both when typing on the terminal, and sometimes in text files stored on disk.
    static const auto C0_ESC = '\x1B'; // Escape          \e  - The Esc key on the keyboard will cause this character to be sent on most systems. It can be used in software user interfaces to exit from a screen, menu, or mode, or in device-control protocols (e.g., printers and terminals) to signal that what follows is a special command sequence rather than normal text. In systems based on ISO/IEC 2022, even if another set of C0 control codes are used, this octet is required to always represent the escape character.
    static const auto C0_FS  = '\x1C'; // File Separator      - Can be used as delimiters to mark fields of data structures. If used for hierarchical levels, US is the lowest level (dividing plain-text data items), while RS, GS, and FS are of increasing level to divide groups made up of items of the level beneath it.
    static const auto C0_GS  = '\x1D'; // Group Separator.
    static const auto C0_RS  = '\x1E'; // Record Separator.
    static const auto C0_US  = '\x1F'; // Unit Separator.
    static const auto C0_DEL = '\x7F'; // Delete cell backward.

    static const auto ctrl_break = si32{ 0xE046 }; // Pressed Ctrl+Break scancode.

    static const auto OSC_LABEL_TITLE  = "0"   ; // Set icon label and title.
    static const auto OSC_LABEL        = "1"   ; // Set icon label.
    static const auto OSC_TITLE        = "2"   ; // Set title.
    static const auto OSC_XPROP        = "3"   ; // Set xprop.
    static const auto OSC_LINUX_COLOR  = "P"   ; // Set 16 colors palette. (Linux console)
    static const auto OSC_LINUX_RESET  = "R"   ; // Reset 16/256 colors palette. (Linux console)
    static const auto OSC_SET_PALETTE  = "4"   ; // Set 256 colors palette.
    static const auto OSC_CLIPBOARD    = "52"  ; // Copy printed text into clipboard.
    static const auto OSC_SET_FGCOLOR  = "10"  ; // Set fg color.
    static const auto OSC_SET_BGCOLOR  = "11"  ; // Set bg color.
    static const auto OSC_RESET_COLOR  = "104" ; // Reset color N to default palette. Without params all palette reset.
    static const auto OSC_RESET_FGCLR  = "110" ; // Reset fg color to default.
    static const auto OSC_RESET_BGCLR  = "111" ; // Reset bg color to default.
    static const auto OSC_CLIPBRD      = "52"  ; // Set clipboard.
    static const auto OSC_TITLE_REPORT = "l"   ; // Get terminal window title.
    static const auto OSC_LABEL_REPORT = "L"   ; // Get terminal window icon label.

    static const auto SGR_RST       = 0;
    static const auto SGR_SAV       = 10;
    static const auto SGR_BOLD      = 1;
    static const auto SGR_FAINT     = 22;
    static const auto SGR_ITALIC    = 3;
    static const auto SGR_NONITALIC = 23;
    static const auto SGR_UND       = 4;
    static const auto SGR_DOUBLEUND = 21;
    static const auto SGR_NOUND     = 24;
    static const auto SGR_SLOWBLINK = 5;
    static const auto SGR_FASTBLINK = 6;
    static const auto SGR_NO_BLINK  = 25;
    static const auto SGR_INV       = 7;
    static const auto SGR_NOINV     = 27;
    static const auto SGR_STRIKE    = 9;
    static const auto SGR_NOSTRIKE  = 29;
    static const auto SGR_OVERLN    = 53;
    static const auto SGR_NOOVERLN  = 55;
    static const auto SGR_FG_BLK    = 30;
    static const auto SGR_FG_RED    = 31;
    static const auto SGR_FG_GRN    = 32;
    static const auto SGR_FG_YLW    = 33;
    static const auto SGR_FG_BLU    = 34;
    static const auto SGR_FG_MGT    = 35;
    static const auto SGR_FG_CYN    = 36;
    static const auto SGR_FG_WHT    = 37;
    static const auto SGR_FG_RGB    = 38;
    static const auto SGR_FG        = 39;
    static const auto SGR_BG_BLK    = 40;
    static const auto SGR_BG_RED    = 41;
    static const auto SGR_BG_GRN    = 42;
    static const auto SGR_BG_YLW    = 43;
    static const auto SGR_BG_BLU    = 44;
    static const auto SGR_BG_MGT    = 45;
    static const auto SGR_BG_CYN    = 46;
    static const auto SGR_BG_WHT    = 47;
    static const auto SGR_BG_RGB    = 48;
    static const auto SGR_BG        = 49;
    static const auto SGR_FG_BLK_LT = 90;
    static const auto SGR_FG_RED_LT = 91;
    static const auto SGR_FG_GRN_LT = 92;
    static const auto SGR_FG_YLW_LT = 93;
    static const auto SGR_FG_BLU_LT = 94;
    static const auto SGR_FG_MGT_LT = 95;
    static const auto SGR_FG_CYN_LT = 96;
    static const auto SGR_FG_WHT_LT = 97;
    static const auto SGR_BG_BLK_LT = 100;
    static const auto SGR_BG_RED_LT = 101;
    static const auto SGR_BG_GRN_LT = 102;
    static const auto SGR_BG_YLW_LT = 103;
    static const auto SGR_BG_BLU_LT = 104;
    static const auto SGR_BG_MGT_LT = 105;
    static const auto SGR_BG_CYN_LT = 106;
    static const auto SGR_BG_WHT_LT = 107;

    static const auto CCC_NOP    = 0  ; // CSI             p  - no operation.
    static const auto CCC_RST    = 1  ; // CSI 1           p  - reset to zero all params (zz).
    static const auto CCC_CPP    = 2  ; // CSI 2 : x [: y] p  - caret percent position.
    static const auto CCC_CPX    = 3  ; // CSI 3 : x       p  - caret H percent position.
    static const auto CCC_CPY    = 4  ; // CSI 4 : y       p  - caret V percent position.
    static const auto CCC_TBS    = 5  ; // CSI 5 : n       p  - tab step length.
    static const auto CCC_MGN    = 6  ; // CSI 6 : l:r:t:b p  - margin left, right, top, bottom.
    static const auto CCC_MGL    = 7  ; // CSI 7 : n       p  - margin left   ╮
    static const auto CCC_MGR    = 8  ; // CSI 8 : n       p  - margin right  │ positive - native binding.
    static const auto CCC_MGT    = 9  ; // CSI 9 : n       p  - margin top    │ negative - oppisite binding.
    static const auto CCC_MGB    = 10 ; // CSI 10: n       p  - margin bottom ╯

    static const auto CCC_JET    = 11 ; // CSI 11: n       p  - text alignment (bias).
    static const auto CCC_WRP    = 12 ; // CSI 12: n       p  - text wrapping none/on/off.
    static const auto CCC_RTL    = 13 ; // CSI 13: n       p  - text right-to-left none/on/off.
    static const auto CCC_RLF    = 14 ; // CSI 14: n       p  - reverse line feed none/on/off.

    static const auto CCC_JET_or = 15 ; // CSI 15: n       p  - set text alignment (bias) if it is not set.
    static const auto CCC_WRP_or = 16 ; // CSI 16: n       p  - set text wrapping none/on/off if it is not set.
    static const auto CCC_RTL_or = 17 ; // CSI 17: n       p  - set text right-to-left none/on/off if it is not set.
    static const auto CCC_RLF_or = 18 ; // CSI 18: n       p  - set reverse line feed none/on/off if it is not set.

    static const auto CCC_IDX    = 19 ; // CSI 19: id      p  - Split the text run and associate the fragment with an id.
    static const auto CCC_CUP    = 20 ; // CSI 20: x [: y] p  - caret absolute position 0-based.
    static const auto CCC_CHX    = 21 ; // CSI 21: x       p  - caret H absolute position 0-based.
    static const auto CCC_CHY    = 22 ; // CSI 22: y       p  - caret V absolute position 0-based.
    static const auto CCC_REF    = 23 ; // CSI 23: id      p  - create the reference to the existing paragraph.
    static const auto CCC_SBS    = 24 ; // CSI 24: n: m    p  - define scrollback size: n: max size, m: grow_by step.
    static const auto CCC_SMS    = 26 ; // CSI 26: b       p  - Should the mouse poiner to be drawn.

    static const auto CCC_SGR    = 28 ; // CSI 28: ...     p  - Set the default SGR attribute for the built-in terminal background (one attribute per command).
    static const auto CCC_SEL    = 29 ; // CSI 29: n       p  - Set selection mode for the built-in terminal, n: 0 - off, 1 - plaintext, 2 - ansi-text.
    static const auto CCC_PAD    = 30 ; // CSI 30: n       p  - Set left/right padding for the built-in terminal.

    static const auto mimetext = "text/plain"sv;
    static const auto mimeansi = "text/xterm"sv;
    static const auto mimehtml = "text/html"sv;
    static const auto mimerich = "text/rtf"sv;
    static const auto mimesafe = "text/protected"sv;

    struct clip
    {
        enum mime : si32
        {
            disabled,
            textonly,
            ansitext,
            richtext,
            htmltext,
            safetext, // mime: Sensitive textonly data.
            count,
        };

        twod size;
        text utf8;
        mime kind;

        clip()
            : kind{ mime::ansitext }
        { }
        clip(twod const& size, view utf8, mime kind)
            : size{ size },
              utf8{ utf8 },
              kind{ kind }
        { }
        void set(clip const& data)
        {
            size = dot_00;
            auto rawdata = view{ data.utf8 };
            if (data.kind == mime::disabled)
            {
                auto valid = true;
                kind = ansi::clip::textonly;
                // rawdata=mime/size_x/size_y;data
                     if (rawdata.starts_with(ansi::mimeansi)) { rawdata.remove_prefix(ansi::mimeansi.length()); kind = mime::ansitext; }
                else if (rawdata.starts_with(ansi::mimetext)) { rawdata.remove_prefix(ansi::mimetext.length()); kind = mime::textonly; }
                else if (rawdata.starts_with(ansi::mimerich)) { rawdata.remove_prefix(ansi::mimerich.length()); kind = mime::richtext; }
                else if (rawdata.starts_with(ansi::mimehtml)) { rawdata.remove_prefix(ansi::mimehtml.length()); kind = mime::htmltext; }
                else if (rawdata.starts_with(ansi::mimesafe)) { rawdata.remove_prefix(ansi::mimesafe.length()); kind = mime::safetext; }
                else
                {
                    valid = faux;
                    kind = mime::textonly;
                    auto pos = rawdata.find(';');
                    if (pos != text::npos)
                    {
                        rawdata = rawdata.substr(pos + 1);
                    }
                    else rawdata = {};
                }
                if (valid && rawdata.size())
                {
                    if (rawdata.front() == '/') // Proceed preview size if present.
                    {
                        rawdata.remove_prefix(1);
                        if (auto v = utf::to_int(rawdata))
                        {
                            static constexpr auto max_value = twod{ 2000, 1000 }; //todo unify
                            size.x = v.value();
                            if (rawdata.size())
                            {
                                rawdata.remove_prefix(1);
                                if (auto v = utf::to_int(rawdata))
                                {
                                    size.y = v.value();
                                }
                                else size.x = 0;
                            }
                            size = std::clamp(size, dot_00, max_value);
                        }
                    }
                    if (rawdata.size() && rawdata.front() == ';')
                    {
                        rawdata.remove_prefix(1);
                    }
                    else // Unknown format.
                    {
                        size = {};
                        rawdata = {};
                    }
                }
            }
            else kind = data.kind;
            utf8 = rawdata;
            size = rawdata.empty() ? dot_00
                 : size            ? size
                 : data.size       ? data.size
                                   : twod{ 80,25 }; //todo make it configurable
        }
        void clear()
        {
            utf8.clear();
            kind = mime::ansitext;
            size = dot_00;
        }
    };

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

        auto& bld(bool b)           { return add(b ? "\033[1m" : "\033[22m"         ); } // esc: SGR 𝗕𝗼𝗹𝗱 attribute.
        auto& und(si32 n)           { return add(n==0 ? "\033[24m" :
                                                 n==2 ? "\033[21m" : "\033[4m"      ); } // esc: SGR 𝗨𝗻𝗱𝗲𝗿𝗹𝗶𝗻𝗲 attribute.
        auto& blk(bool b)           { return add(b ? "\033[5m" : "\033[25m"         ); } // esc: SGR Blink attribute.
        auto& inv(bool b)           { return add(b ? "\033[7m" : "\033[27m"         ); } // esc: SGR 𝗡𝗲𝗴𝗮𝘁𝗶𝘃𝗲 attribute.
        auto& itc(bool b)           { return add(b ? "\033[3m" : "\033[23m"         ); } // esc: SGR 𝑰𝒕𝒂𝒍𝒊𝒄 attribute.
        auto& stk(bool b)           { return add(b ? "\033[9m" : "\033[29m"         ); } // esc: SGR Strikethrough attribute.
        auto& ovr(bool b)           { return add(b ? "\033[53m": "\033[55m"         ); } // esc: SGR Overline attribute.
        auto& sav()                 { return add("\033[10m"                         ); } // esc: Save SGR attributes.
        auto& nil()                 { return add("\033[m"                           ); } // esc: Reset SGR attributes to zero.
        auto& fgc()                 { return add("\033[39m"                         ); } // esc: Set default foreground color.
        auto& bgc()                 { return add("\033[49m"                         ); } // esc: Set default background color.
        auto& scroll_wipe()         { return add("\033[2J"                          ); } // esc: Erase scrollback.
        auto& locate(twod const& p) { return add("\033[", p.y + 1, ';', p.x + 1, 'H'); } // esc: 0-Based caret position.
        auto& cuu(si32 n)           { return add("\033[", n, 'A'                    ); } // esc: Caret up.
        auto& cud(si32 n)           { return add("\033[", n, 'B'                    ); } // esc: Caret down.
        auto& cuf(si32 n)           { return add("\033[", n, 'C'                    ); } // esc: Caret forward.  Negative values can wrap to the prev line.
        auto& cub(si32 n)           { return add("\033[", n, 'D'                    ); } // esc: Caret backward. Negative values can wrap to the next line.
        auto& cnl(si32 n)           { return add("\033[", n, 'E'                    ); } // esc: caret next line.
        auto& cpl(si32 n)           { return add("\033[", n, 'F'                    ); } // esc: Caret previous line.
        auto& ocx(si32 n)           { return add("\033[", n, 'G'                    ); } // esc: Caret 1-based horizontal absolute.
        auto& ocy(si32 n)           { return add("\033[", n, 'd'                    ); } // esc: Caret 1-based vertical absolute.
        auto& dch(si32 n)           { return add("\033[", n, 'P'                    ); } // esc: DCH
        auto& fwd(si32 n)           { return n > 0 ? add("\033[",-n, 'D')
                                           : n < 0 ? add("\033[", n, 'C') : *this;     } // esc: Move caret n cell in line with wrapping.
        auto& del()                 { return add('\x7F'                             ); } // esc: Delete cell backwards.
        auto& scp()                 { return add("\033[s"                           ); } // esc: Save caret position in memory.
        auto& rcp()                 { return add("\033[u"                           ); } // esc: Restore caret position from memory.
        auto& pushsgr()             { return add("\033[#{"                          ); } // esc: Push SGR attributes onto stack.
        auto& popsgr()              { return add("\033[#}"                          ); } // esc: Pop  SGR attributes from stack.
        auto& fcs(bool b)           { return add("\033[", b ? 'I' : 'O'             ); } // esc: Terminal window focus.
        auto& eol()                 { return add("\n"                               ); } // esc: EOL.
        auto& edl()                 { return add("\033[K"                           ); } // esc: EDL.
        auto& fgx(rgba const& c)    { return add("\033[38:2:", c.chan.r, ':',            // esc: SGR Foreground color. RGB: red, green, blue and alpha.
                                                               c.chan.g, ':',
                                                               c.chan.b, ':',
                                                               c.chan.a, 'm'); }
        auto& bgx(rgba const& c)    { return add("\033[48:2:", c.chan.r, ':',            // esc: SGR Background color. RGB: red, green, blue and alpha.
                                                               c.chan.g, ':',
                                                               c.chan.b, ':',
                                                               c.chan.a, 'm'); }
        auto& fgc256(rgba const& c) { return add("\033[38;5;", c.to256cube(), 'm'   ); } // esc: SGR Foreground color (256-color mode).
        auto& bgc256(rgba const& c) { return add("\033[48;5;", c.to256cube(), 'm'   ); } // esc: SGR Background color (256-color mode).
        auto& fgc_16(rgba const& c) // esc: SGR Foreground color (16-color mode).
        {
            auto clr = si32{ 30 };
            switch (c.token)
            {
                case 0xFF000000: clr += 0; return add("\033[22;", clr, 'm');
                case 0xFFffffff: clr += 5; return add("\033[22;", clr, 'm');
                case 0xFF00ff00:
                case rgba{ rgba::color256[tint::greenlt  ] }.token: clr += 60 + 0; break;
                case 0xFF00ffff:
                case rgba{ rgba::color256[tint::yellowlt ] }.token: clr += 60 + 1; break;
                case 0xFFff00ff:
                case rgba{ rgba::color256[tint::magentalt] }.token: clr += 60 + 2; break;
                case rgba{ rgba::color256[tint::reddk    ] }.token: clr += 60 + 3; break;
                case rgba{ rgba::color256[tint::bluedk   ] }.token: clr += 60 + 4; break;
                case rgba{ rgba::color256[tint::greendk  ] }.token: clr += 60 + 5; break;
                case rgba{ rgba::color256[tint::yellowdk ] }.token: clr += 60 + 6; break;
                case 0xFFffff00:
                case rgba{ rgba::color256[tint::cyanlt   ] }.token: clr += 60 + 7; break;
                case 0xFF0000ff:
                case rgba{ rgba::color256[tint::redlt    ] }.token: clr += 6; return add("\033[22;", clr, 'm');
                case rgba{ rgba::color256[tint::blacklt  ] }.token: clr += 4; return add("\033[22;", clr, 'm');
                case 0xFFff0000:
                case rgba{ rgba::color256[tint::bluelt   ] }.token: clr += 7; return add("\033[22;", clr, 'm');
                default: auto l = c.luma(); // grayscale
                          if (l < 42)  clr += 1;
                     else if (l < 90)  clr += 2;
                     else if (l < 170) clr += 3;
                     else if (l < 240) clr += 4;
                     else              clr += 5;
                     return add("\033[22;", clr, 'm');
            }
            return add("\033[", clr, 'm');
        }
        auto& bgc_16(rgba const& c) // esc: SGR Background color (16-color mode).
        {
            auto clr = si32{ 40 };
            switch (c.token)
            {
                case 0xFF000000: clr += 0; break;
                case 0xFFffffff: clr += 5; break;
                case 0xFF0000ff:
                case rgba{ rgba::color256[tint::reddk ] }.token: clr += 6; break;
                case rgba{ rgba::color256[tint::redlt ] }.token: clr += 6; break;
                case 0xFFff0000:
                case rgba{ rgba::color256[tint::bluelt] }.token: clr += 7; break;
                default:
                    if (c.chan.b > 0xE0
                     && c.chan.r > 0x30 && c.chan.r < 0x50
                     && c.chan.g > 0x70 && c.chan.g < 0xd0)
                    {
                        clr += 7;
                    }
                    else // grayscale
                    {
                        auto l = c.luma();
                             if (l < 42)  clr += 1;
                        else if (l < 90)  clr += 2;
                        else if (l < 170) clr += 3;
                        else if (l < 240) clr += 4;
                        else              clr += 5;
                    }
            }
            return add("\033[", clr, 'm');
        }
        template<svga VGAMODE = svga::truecolor>
        auto& fgc(rgba const& c) // esc: SGR Foreground color. RGB: red, green, blue.
        {
                 if constexpr (VGAMODE == svga::vga16    ) return fgc_16(c);
            else if constexpr (VGAMODE == svga::vga256   ) return fgc256(c);
            else if constexpr (VGAMODE == svga::truecolor) return c.chan.a == 0 ? add("\033[39m")
                                                                                : add("\033[38;2;", c.chan.r, ';',
                                                                                                    c.chan.g, ';',
                                                                                                    c.chan.b, 'm');
            else return block;
        }
        template<svga VGAMODE = svga::truecolor>
        auto& bgc(rgba const& c) // esc: SGR Background color. RGB: red, green, blue.
        {
                 if constexpr (VGAMODE == svga::vga16    ) return bgc_16(c);
            else if constexpr (VGAMODE == svga::vga256   ) return bgc256(c);
            else if constexpr (VGAMODE == svga::truecolor) return c.chan.a == 0 ? add("\033[49m")
                                                                                : add("\033[48;2;", c.chan.r, ';',
                                                                                                    c.chan.g, ';',
                                                                                                    c.chan.b, 'm');
            else return block;
        }
        // basevt: Ansify/textify content of specified region.
        template<bool USESGR = true, bool INITIAL = true, bool FINALISE = true>
        auto& s11n(core const& canvas, rect region, cell& state)
        {
            auto badfx = [&]
            {
                add(utf::REPLACEMENT_CHARACTER_UTF8_VIEW);
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
                    c.scan<svga::truecolor, USESGR>(state, block);
                }
                else
                {
                    if (width == 2) // Left part
                    {
                        if (state.wdt() == 2) badfx();  // Left part alone
                        c.scan_attr<svga::truecolor, USESGR>(state, block);
                        state.set_gc(c); // Save char from c for the next iteration
                    }
                    else if (width == 3) // Right part
                    {
                        if (state.wdt() == 2)
                        {
                            if (state.scan<svga::truecolor, USESGR>(c, state, block)) state.set_gc(); // Cleanup used t
                            else
                            {
                                badfx(); // Left part alone
                                c.scan_attr<svga::truecolor, USESGR>(state, block);
                                badfx(); // Right part alone
                            }
                        }
                        else
                        {
                            c.scan_attr<svga::truecolor, USESGR>(state, block);
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
                if constexpr (USESGR && INITIAL) basevt::nil();
                netxs::onrect(canvas, region, allfx, eolfx);
                if constexpr (FINALISE)
                {
                    block.pop_back(); // Pop last eol (lf).
                    if constexpr (USESGR) basevt::nil();
                }
            }
            return *this;
        }
        template<bool USESGR = true, bool INITIAL = true, bool FINALISE = true>
        auto& s11n(core const& canvas, rect region) // basevt: Ansify/textify content of specified region.
        {
            auto state = cell{};
            return s11n<USESGR, INITIAL, FINALISE>(canvas, region, state);
        }
        template<bool USESGR = true, bool INITIAL = true, bool FINALISE = true>
        auto& s11n(core const& canvas, cell& state) // basevt: Ansify/textify all content.
        {
            auto region = rect{-dot_mx / 2, dot_mx };
            return s11n<USESGR, INITIAL, FINALISE>(canvas, region, state);
        }
    };

    // ansi: Escaped sequences accumulator.
    class esc
        : public text,
          public basevt<esc>
    {
    public:
        esc()
            : basevt{ *this }
        { }

        template<class T>
        esc(T&& data)
            : basevt{ *this }
        {
            add(std::forward<T>(data));
        }

        auto& operator = (esc const& other)
        {
            text::clear();
            return add(other);
        }

        auto& vmouse(bool b) // esc: Focus and Mouse position reporting/tracking.
        {
            return add(b ? "\033[?1002;1003;1004;1006;10060h"
                         : "\033[?1002;1003;1004;1006;10060l");
        }
        auto& setutf(bool b)        { return add(b ? "\033%G"      : "\033%@"        ); } // esc: Select UTF-8 character set (true) or default (faux).
        auto& altbuf(bool b)        { return add(b ? "\033[?1049h" : "\033[?1049l"   ); } // esc: Alternative buffer.
        auto& cursor(bool b)        { return add(b ? "\033[?25h"   : "\033[?25l"     ); } // esc: Caret visibility.
        auto& appkey(bool b)        { return add(b ? "\033[?1h"    : "\033[?1l"      ); } // esc: Application(=on)/ANSI(=off) Caret Keys (DECCKM).
        auto& bpmode(bool b)        { return add(b ? "\033[?2004h" : "\033[?2004l"   ); } // esc: Set bracketed paste mode.
        auto& autowr(bool b)        { return add(b ? "\033[?7h"    : "\033[?7l"      ); } // esc: Set autowrap mode.
        auto& report(twod const& p) { return add("\033[", p.y+1, ";", p.x+1, "R"     ); } // esc: Report 1-Based caret position (CPR).
        auto& locate_wipe()         { return add("\033[r"                            ); } // esc: Enable scrolling for entire display (clear screen).
        auto& locate_call()         { return add("\033[6n"                           ); } // esc: Report caret position.
        auto& scrn_reset()          { return add("\033[H\033[m\033[2J"               ); } // esc: Reset palette, erase scrollback and reset caret location.
        auto& save_title()          { return add("\033[22;0t"                        ); } // esc: Save terminal window title.
        auto& load_title()          { return add("\033[23;0t"                        ); } // esc: Restore terminal window title.
        auto& osc(view p, view arg) { return add("\033]", p, ';', arg,        C0_BEL ); } // esc: OSC report.
        auto& header(view t)        { return add("\033]2;", t,                C0_BEL ); } // esc: Window title.
        auto& save_palette()        { return add("\033[#P"                           ); } // esc: Push palette onto stack XTPUSHCOLORS.
        auto& load_palette()        { return add("\033[#Q"                           ); } // esc: Pop  palette from stack XTPOPCOLORS.
        auto& old_palette_reset()   { return add("\033]R"                            ); } // esc: Reset color palette (Linux console).
        auto& clipbuf(twod size, view utf8, clip::mime kind) // esc: Set clipboard buffer.
        {
            return add("\033]52;", kind == clip::htmltext ? mimehtml
                                 : kind == clip::richtext ? mimerich
                                 : kind == clip::ansitext ? mimeansi
                                 : kind == clip::safetext ? mimesafe
                                                          : mimetext, "/", size.x, "/", size.y, ";", utf::base64(utf8), C0_BEL);
        }
        auto& old_palette(si32 i, rgba const& c) // esc: Set color palette (Linux console).
        {
            return add("\033]P", utf::to_hex(i, 1), utf::to_hex(c.chan.r, 2),
                                                    utf::to_hex(c.chan.g, 2),
                                                    utf::to_hex(c.chan.b, 2), '\033');
        }
        auto& osc_palette(si32 i, rgba const& c) // esc: Set color palette. ESC ] 4 ; <i> ; rgb : <r> / <g> / <b> BEL.
        {
            return add("\033]4;", i, ";rgb:", utf::to_hex(c.chan.r), '/',
                                              utf::to_hex(c.chan.g), '/',
                                              utf::to_hex(c.chan.b), C0_BEL);
        }
        auto& osc_palette_reset() // esc: Reset color palette.
        {
            osc_palette(0,  rgba::color256[tint::blackdk  ]);
            osc_palette(1,  rgba::color256[tint::reddk    ]);
            osc_palette(2,  rgba::color256[tint::greendk  ]);
            osc_palette(3,  rgba::color256[tint::yellowdk ]);
            osc_palette(4,  rgba::color256[tint::bluedk   ]);
            osc_palette(5,  rgba::color256[tint::magentadk]);
            osc_palette(6,  rgba::color256[tint::cyandk   ]);
            osc_palette(7,  rgba::color256[tint::whitedk  ]);
            osc_palette(8,  rgba::color256[tint::blacklt  ]);
            osc_palette(9,  rgba::color256[tint::redlt    ]);
            osc_palette(10, rgba::color256[tint::greenlt  ]);
            osc_palette(11, rgba::color256[tint::yellowlt ]);
            osc_palette(12, rgba::color256[tint::bluelt   ]);
            osc_palette(13, rgba::color256[tint::magentalt]);
            osc_palette(14, rgba::color256[tint::cyanlt   ]);
            osc_palette(15, rgba::color256[tint::whitelt  ]);
            return *this;
        }
        template<class T, class S>
        auto& mouse_sgr(T const& gear, S const& cached, twod const& coor) // esc: Mouse tracking report (SGR).
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
            auto s_bttn = std::bitset<8>{ cached.buttons };
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
        template<class T, class S>
        auto& mouse_x11(T const& gear, S const& cached, twod const& coor) // esc: Mouse tracking report (X11).
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
            auto s_bttn = std::bitset<8>{ cached.buttons };
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
            return add("\033[M", static_cast<char>(std::clamp(ctrl,       0, 255-32) + 32),
                                 static_cast<char>(std::clamp(coor.x + 1, 1, 255-32) + 32),
                                 static_cast<char>(std::clamp(coor.y + 1, 1, 255-32) + 32));
        }
        auto& w32keybd(si32 Vk, si32 Sc, si32 Uc, si32 Kd, si32 Cs, si32 Rc) // esc: win32-input-mode sequence (keyboard).
        {
            // \033 [ Vk ; Sc ; Uc ; Kd ; Cs ; Rc _
            return add("\033[", Vk, ';',      // Vk: the value of wVirtualKeyCode - any number. If omitted, defaults to '0'.
                                Sc, ';',      // Sc: the value of wVirtualScanCode - any number. If omitted, defaults to '0'.
                                Uc, ';',      // Uc: the decimal value of UnicodeChar - for example, NUL is "0", LF is "10", the character 'A' is "65". If omitted, defaults to '0'.
                                Kd, ';',      // Kd: the value of bKeyDown - either a '0' or '1'. If omitted, defaults to '0'.
                                Cs, ';',      // Cs: the value of dwControlKeyState - any number. If omitted, defaults to '0'.
                                Rc, W32_INP); // Rc: the value of wRepeatCount - any number. If omitted, defaults to '1'.
        }
        // Private vt command subset.
        //todo use '_' instead of 'p' in CSI_CCC
        auto& nop()              { return add("\033["   ,      CSI_CCC); } // esc: No operation. Split the text run.
        auto& rst()              { return add("\033[1"  ,      CSI_CCC); } // esc: Reset formatting parameters.
        auto& tbs(si32 n)        { return add("\033[5:" , n  , CSI_CCC); } // esc: Tabulation step length.
        auto& chx(si32 n)        { return add("\033[21:", n  , CSI_CCC); } // esc: Caret 0-based horizontal absolute.
        auto& chy(si32 n)        { return add("\033[22:", n  , CSI_CCC); } // esc: Caret 0-based vertical absolute.
        auto& cpx(si32 n)        { return add("\033[3:" , n  , CSI_CCC); } // esc: Caret horizontal percent position.
        auto& cpy(si32 n)        { return add("\033[4:" , n  , CSI_CCC); } // esc: Caret vertical percent position.
        auto& cup(twod const& p) { return add("\033[20:", p.y, ':',        // esc: 0-Based caret position.
                                                          p.x, CSI_CCC); }
        auto& cpp(twod const& p) { return add("\033[2:" , p.x, ':',        // esc: Caret percent position.
                                                          p.y, CSI_CCC); }
        auto& mgn(side const& n) { return add("\033[6:" , n.l, ':',        // esc: Margin (left, right, top, bottom).
                                                          n.r, ':',
                                                          n.t, ':',
                                                          n.b, CSI_CCC); }
        auto& mgl(si32 n)        { return add("\033[7:" , n  , CSI_CCC); } // esc: Left margin. Positive - native binding. Negative - opposite binding.
        auto& mgr(si32 n)        { return add("\033[8:" , n  , CSI_CCC); } // esc: Right margin. Positive - native binding. Negative - opposite binding.
        auto& mgt(si32 n)        { return add("\033[9:" , n  , CSI_CCC); } // esc: Top margin. Positive - native binding. Negative - opposite binding.
        auto& mgb(si32 n)        { return add("\033[10:", n  , CSI_CCC); } // esc: Bottom margin. Positive - native binding. Negative - opposite binding.
        auto& jet(bias n)        { return add("\033[11:", n  , CSI_CCC); } // esc: Text alignment.
        auto& wrp(wrap n)        { return add("\033[12:", n  , CSI_CCC); } // esc: Text wrapping.
        auto& rtl(rtol n)        { return add("\033[13:", n  , CSI_CCC); } // esc: Text right-to-left.
        auto& rlf(feed n)        { return add("\033[14:", n  , CSI_CCC); } // esc: Reverse line feed.
        auto& jet_or(bias n)     { return add("\033[15:", n  , CSI_CCC); } // esc: Text alignment.
        auto& wrp_or(wrap n)     { return add("\033[16:", n  , CSI_CCC); } // esc: Text wrapping.
        auto& rtl_or(rtol n)     { return add("\033[17:", n  , CSI_CCC); } // esc: Text right-to-left.
        auto& rlf_or(feed n)     { return add("\033[18:", n  , CSI_CCC); } // esc: Reverse line feed.
        auto& idx(si32 i)        { return add("\033[19:", i  , CSI_CCC); } // esc: Split the text run and associate the fragment with an id.
        auto& ref(si32 i)        { return add("\033[23:", i  , CSI_CCC); } // esc: Create the reference to the existing paragraph.
        auto& show_mouse(si32 b) { return add("\033[26:", b  , CSI_CCC); } // esc: Should the mouse poiner to be drawn.
    };

    template<class ...Args>
    static auto clipbuf(Args&&... data) { return esc{}.clipbuf(std::forward<Args>(data)...); } // ansi: Set clipboard.
    template<class ...Args>
    static auto add(Args&&... data)   { return esc{}.add(std::forward<Args>(data)...); } // ansi: Add text.
    static auto cup(twod const& n)    { return esc{}.cup(n);        } // ansi: 0-Based caret position.
    static auto cuu(si32 n)           { return esc{}.cuu(n);        } // ansi: Caret up.
    static auto cud(si32 n)           { return esc{}.cud(n);        } // ansi: Caret down.
    static auto cuf(si32 n)           { return esc{}.cuf(n);        } // ansi: Caret forward.
    static auto cub(si32 n)           { return esc{}.cub(n);        } // ansi: Caret backward.
    static auto cnl(si32 n)           { return esc{}.cnl(n);        } // ansi: Caret next line.
    static auto cpl(si32 n)           { return esc{}.cpl(n);        } // ansi: Caret previous line.
    static auto ocx(si32 n)           { return esc{}.ocx(n);        } // ansi: Caret 1-based horizontal absolute.
    static auto ocy(si32 n)           { return esc{}.ocy(n);        } // ansi: Caret 1-based vertical absolute.
    static auto chx(si32 n)           { return esc{}.chx(n);        } // ansi: Caret 0-based horizontal absolute.
    static auto chy(si32 n)           { return esc{}.chy(n);        } // ansi: Caret 0-based vertical absolute.
    static auto fwd(si32 n)           { return esc{}.fwd(n);        } // ansi: Move caret n cell in line.
    static auto dch(si32 n)           { return esc{}.dch(n);        } // ansi: Delete (not Erase) letters under the cursor.
    static auto del()                 { return esc{}.del( );        } // ansi: Delete cell backwards ('\x7F').
    static auto bld(bool b = true)    { return esc{}.bld(b);        } // ansi: SGR 𝗕𝗼𝗹𝗱 attribute.
    static auto und(si32 n = 1   )    { return esc{}.und(n);        } // ansi: SGR 𝗨𝗻𝗱𝗲𝗿𝗹𝗶𝗻𝗲 attribute. 0 - no underline, 1 - single, 2 - double.
    static auto blk(bool b = true)    { return esc{}.blk(b);        } // ansi: SGR Blink attribute.
    static auto inv(bool b = true)    { return esc{}.inv(b);        } // ansi: SGR 𝗡𝗲𝗴𝗮𝘁𝗶𝘃𝗲 attribute.
    static auto itc(bool b = true)    { return esc{}.itc(b);        } // ansi: SGR 𝑰𝒕𝒂𝒍𝒊𝒄 attribute.
    static auto stk(bool b = true)    { return esc{}.stk(b);        } // ansi: SGR Strikethrough attribute.
    static auto ovr(bool b = true)    { return esc{}.ovr(b);        } // ansi: SGR Overline attribute.
    static auto fgc(rgba const& n)    { return esc{}.fgc(n);        } // ansi: SGR Foreground color.
    static auto bgc(rgba const& n)    { return esc{}.bgc(n);        } // ansi: SGR Background color.
    static auto fgx(rgba const& n)    { return esc{}.fgx(n);        } // ansi: SGR Foreground color with alpha.
    static auto bgx(rgba const& n)    { return esc{}.bgx(n);        } // ansi: SGR Background color with alpha.
    static auto fgc()                 { return esc{}.fgc( );        } // ansi: Set default foreground color.
    static auto bgc()                 { return esc{}.bgc( );        } // ansi: Set default background color.
    static auto sav()                 { return esc{}.sav( );        } // ansi: Save SGR attributes.
    static auto nil()                 { return esc{}.nil( );        } // ansi: Reset (restore) SGR attributes.
    static auto nop()                 { return esc{}.nop( );        } // ansi: No operation. Split the text run.
    static auto rst()                 { return esc{}.rst( );        } // ansi: Reset formatting parameters.
    static auto eol()                 { return esc{}.eol( );        } // ansi: EOL.
    static auto edl()                 { return esc{}.edl( );        } // ansi: EDL.
    static auto scp()                 { return esc{}.scp( );        } // ansi: Save caret position in memory.
    static auto rcp()                 { return esc{}.rcp( );        } // ansi: Restore caret position from memory.
    static auto pushsgr()             { return esc{}.pushsgr();     } // ansi: Push SGR attrs onto stack.
    static auto popsgr()              { return esc{}.popsgr();      } // ansi: Pop  SGR attrs from stack.
    static auto cpp(twod const& n)    { return esc{}.cpp(n);        } // ansi: Caret percent position.
    static auto cpx(si32 n)           { return esc{}.cpx(n);        } // ansi: Caret horizontal percent position.
    static auto cpy(si32 n)           { return esc{}.cpy(n);        } // ansi: Caret vertical percent position.
    static auto tbs(si32 n)           { return esc{}.tbs(n);        } // ansi: Tabulation step length.
    static auto mgn(side const& n)    { return esc{}.mgn(n);        } // ansi: Margin (left, right, top, bottom).
    static auto mgl(si32 n)           { return esc{}.mgl(n);        } // ansi: Left margin.
    static auto mgr(si32 n)           { return esc{}.mgr(n);        } // ansi: Right margin.
    static auto mgt(si32 n)           { return esc{}.mgt(n);        } // ansi: Top margin.
    static auto mgb(si32 n)           { return esc{}.mgb(n);        } // ansi: Bottom margin.
    static auto fcs(bool b)           { return esc{}.fcs(b);        } // ansi: Terminal window focus.
    static auto jet(bias n)           { return esc{}.jet(n);        } // ansi: Text alignment.
    static auto wrp(wrap n)           { return esc{}.wrp(n);        } // ansi: Text wrapping.
    static auto rtl(rtol n)           { return esc{}.rtl(n);        } // ansi: Text right-to-left.
    static auto rlf(feed n)           { return esc{}.rlf(n);        } // ansi: Reverse line feed.
    static auto jet_or(bias n)        { return esc{}.jet_or(n);     } // ansi: Set text alignment if it is not set.
    static auto wrp_or(wrap n)        { return esc{}.wrp_or(n);     } // ansi: Set text wrapping if it is not set.
    static auto rtl_or(rtol n)        { return esc{}.rtl_or(n);     } // ansi: Set text right-to-left if it is not set.
    static auto rlf_or(feed n)        { return esc{}.rlf_or(n);     } // ansi: Set reverse line feed if it is not set.
    static auto show_mouse (bool b)   { return esc{}.show_mouse(b); } // ansi: Should the mouse poiner to be drawn.
    static auto vmouse(bool b)        { return esc{}.vmouse(b);     } // ansi: Mouse position reporting/tracking.
    static auto locate(twod const& n) { return esc{}.locate(n);     } // ansi: 1-Based caret position.
    static auto locate_wipe()         { return esc{}.locate_wipe(); } // ansi: Enable scrolling for entire display (clear screen).
    static auto locate_call()         { return esc{}.locate_call(); } // ansi: Report caret position.
    static auto setutf(bool b)        { return esc{}.setutf(b);     } // ansi: Select UTF-8 character set.
    static auto header(view t)        { return esc{}.header(t);     } // ansi: Window title.
    static auto altbuf(bool b)        { return esc{}.altbuf(b);     } // ansi: Alternative buffer.
    static auto cursor(bool b)        { return esc{}.cursor(b);     } // ansi: Caret visibility.
    static auto appkey(bool b)        { return esc{}.appkey(b);     } // ansi: Application cursor Keys (DECCKM).
    static auto ref(si32 i)           { return esc{}.ref(i);        } // ansi: Create the reference to the existing paragraph. Create new id if it is not existing.
    static auto idx(si32 i)           { return esc{}.idx(i);        } // ansi: Split the text run and associate the fragment with an id.
                                                                      //       All following text is under the IDX until the next command is issued.
                                                                      //       Redefine if the id already exists.
    // ansi: Caret forwarding instructions.
    // The order is important (see the richtext::flow::exec constexpr).
    // todo tie with richtext::flow::exec
    enum fn : si32
    {
        dx, // horizontal delta.
        dy, // vertical delta.
        ax, // x absolute (0-based).
        ay, // y absolute (0-based).
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
        mark(cell const& brush)
            : cell { brush },
              fresh{ brush },
              spare{ brush }
        { }
        void reset()              { *this = fresh;          }
        void reset(cell const& c) { *this = fresh = c;      }
        auto busy() const         { return  fresh != *this; } // mark: Is the marker modified.
        void  sav()               { spare.set(*this);       } // mark: Save current SGR attributes.
        void  sfg(rgba c)         { spare.fgc(c);           } // mark: Set default foreground color.
        void  sbg(rgba c)         { spare.bgc(c);           } // mark: Set default background color.
        void  nil()               { this->set(spare);       } // mark: Restore saved SGR attributes.
        void  rfg()               { this->fgc(spare.fgc()); } // mark: Reset SGR Foreground color.
        void  rbg()               { this->bgc(spare.bgc()); } // mark: Reset SGR Background color.
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
        bool operator==(const deco&) const = default;

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
        constexpr auto& glb()
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

    static constexpr auto def_style = deco{}.glb();

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

    //todo should we parse these controls as a C0-like?
    //     split paragraphs when flow direction changes, for example.
    struct marker
    {
        using changer = std::array<void (*)(cell&), ctrl::COUNT>;
        changer	setter = {};
        marker()
        {
            setter[ctrl::ALM                 ] = [](cell& p) { p.rtl(true);    };
            setter[ctrl::RLM                 ] = [](cell& p) { p.rtl(true);    };
            setter[ctrl::LRM                 ] = [](cell& p) { p.rtl(faux);    };
            setter[ctrl::SHY                 ] = [](cell& p) { p.hyphen(true); };
            setter[ctrl::FUNCTION_APPLICATION] = [](cell& p) { p.fnappl(true); };
            setter[ctrl::INVISIBLE_TIMES     ] = [](cell& p) { p.itimes(true); };
            setter[ctrl::INVISIBLE_SEPARATOR ] = [](cell& p) { p.isepar(true); };
            setter[ctrl::INVISIBLE_PLUS      ] = [](cell& p) { p.inplus(true); };
            setter[ctrl::ZWNBSP              ] = [](cell& p) { p.zwnbsp(true); };
        }
    };

    template<class Q, class C>
    using func = netxs::generics::tree<Q, C*, std::function<void(Q&, C*&)>>;

    template<class T, bool NOMULTIARG = faux>
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
            */
            #define F(t, q) p->task(rule{ fn::t, q })

            table_quest   .resize(0x100);
                table_quest[DECSET] = nullptr; // decset
                table_quest[DECRST] = nullptr; // decrst

            table_excl    .resize(0x100);
                table_excl[CSI_EXL_RST] = nullptr; // decstr

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
                table[CSI_CUU] = VT_PROC{ F(dy,-q(1)); };              // fx_cuu
                table[CSI_CUD] = VT_PROC{ F(dy, q(1)); };              // fx_cud
                table[CSI_CUF] = VT_PROC{ F(dx, q(1)); };              // fx_cuf
                table[CSI_CUB] = VT_PROC{ F(dx,-q(1)); };              // fx_cub
                table[CSI_CNL] = VT_PROC{ F(nl, q(1)); };              // fx_cnl
                table[CSI_CPL] = VT_PROC{ F(nl,-q(1)); };              // fx_cpl
                table[CSI_CHX] = VT_PROC{ F(ox, q(1)); };              // fx_ocx
                table[CSI_CHY] = VT_PROC{ F(oy, q(1)); };              // fx_ocy
                table[CSI_SCP] = VT_PROC{ F(sc,   0 ); };              // fx_scp
                table[CSI_RCP] = VT_PROC{ F(rc,   0 ); };              // fx_rcp
                table[CSI_CUP] = VT_PROC{ F(oy, q(1)); F(ox, q(1)); }; // fx_ocp
                table[CSI_HVP] = VT_PROC{ F(oy, q(1)); F(ox, q(1)); }; // fx_ocp
                table[CSI_hRM] = VT_PROC{ /*Nothing, Replace mode*/ }; // fx_irm
                table[CSI_lRM] = VT_PROC{ /*Nothing, Replace mode*/ }; // fx_irm
                table[CSI__ED] = nullptr;
                table[CSI__EL] = nullptr;
                table[CSI_DCH] = nullptr;
                table[CSI_ECH] = nullptr;
                table[CSI_ICH] = nullptr;
                table[CSI__DL] = nullptr;
                table[DECSTBM] = nullptr;
                table[CSI__SD] = nullptr;
                table[CSI__SU] = nullptr;
                table[CSI_WIN] = nullptr;
                table[CSI_DSR] = nullptr;

                auto& csi_ccc = table[CSI_CCC].resize(0x100);
                csi_ccc.template enable_multi_arg<NOMULTIARG>();
                    csi_ccc[CCC_CUP] = VT_PROC{ F(ay, q(0)); F(ax, q(0)); }; // fx_ccc_cup
                    csi_ccc[CCC_CPP] = VT_PROC{ F(py, q(0)); F(px, q(0)); }; // fx_ccc_cpp
                    csi_ccc[CCC_CHX] = VT_PROC{ F(ax, q(0)); }; // fx_ccc_chx
                    csi_ccc[CCC_CHY] = VT_PROC{ F(ay, q(0)); }; // fx_ccc_chy
                    csi_ccc[CCC_CPX] = VT_PROC{ F(px, q(0)); }; // fx_ccc_cpx
                    csi_ccc[CCC_CPY] = VT_PROC{ F(py, q(0)); }; // fx_ccc_cpy
                    csi_ccc[CCC_RST] = VT_PROC{ F(zz,   0 ); }; // fx_ccc_rst

                    csi_ccc[CCC_MGN   ] = VT_PROC{ p->style.mgn   (q   ); }; // fx_ccc_mgn
                    csi_ccc[CCC_MGL   ] = VT_PROC{ p->style.mgl   (q(0)); }; // fx_ccc_mgl
                    csi_ccc[CCC_MGR   ] = VT_PROC{ p->style.mgr   (q(0)); }; // fx_ccc_mgr
                    csi_ccc[CCC_MGT   ] = VT_PROC{ p->style.mgt   (q(0)); }; // fx_ccc_mgt
                    csi_ccc[CCC_MGB   ] = VT_PROC{ p->style.mgb   (q(0)); }; // fx_ccc_mgb
                    csi_ccc[CCC_TBS   ] = VT_PROC{ p->style.tbs   (q(0)); }; // fx_ccc_tbs
                    csi_ccc[CCC_JET   ] = VT_PROC{ p->style.jet   (static_cast<bias>(q(0))); }; // fx_ccc_jet
                    csi_ccc[CCC_WRP   ] = VT_PROC{ p->style.wrp   (static_cast<wrap>(q(0))); }; // fx_ccc_wrp
                    csi_ccc[CCC_RTL   ] = VT_PROC{ p->style.rtl   (static_cast<rtol>(q(0))); }; // fx_ccc_rtl
                    csi_ccc[CCC_RLF   ] = VT_PROC{ p->style.rlf   (static_cast<feed>(q(0))); }; // fx_ccc_rlf
                    csi_ccc[CCC_JET_or] = VT_PROC{ p->style.jet_or(static_cast<bias>(q(0))); }; // fx_ccc_or_jet
                    csi_ccc[CCC_WRP_or] = VT_PROC{ p->style.wrp_or(static_cast<wrap>(q(0))); }; // fx_ccc_or_wrp
                    csi_ccc[CCC_RTL_or] = VT_PROC{ p->style.rtl_or(static_cast<rtol>(q(0))); }; // fx_ccc_or_rtl
                    csi_ccc[CCC_RLF_or] = VT_PROC{ p->style.rlf_or(static_cast<feed>(q(0))); }; // fx_ccc_or_rlf

                    csi_ccc[CCC_NOP] = nullptr;
                    csi_ccc[CCC_IDX] = nullptr;
                    csi_ccc[CCC_REF] = nullptr;
                    csi_ccc[CCC_SBS] = nullptr;
                    csi_ccc[CCC_SMS] = nullptr;
                    csi_ccc[CCC_SGR] = nullptr;
                    csi_ccc[CCC_SEL] = nullptr;
                    csi_ccc[CCC_PAD] = nullptr;

                auto& csi_sgr = table[CSI_SGR].resize(0x100);
                csi_sgr.template enable_multi_arg<NOMULTIARG>();
                    csi_sgr[SGR_RST      ] = VT_PROC{ p->brush.nil( );    }; // fx_sgr_rst      ;
                    csi_sgr[SGR_SAV      ] = VT_PROC{ p->brush.sav( );    }; // fx_sgr_sav      ;
                    csi_sgr[SGR_FG       ] = VT_PROC{ p->brush.rfg( );    }; // fx_sgr_fg_def   ;
                    csi_sgr[SGR_BG       ] = VT_PROC{ p->brush.rbg( );    }; // fx_sgr_bg_def   ;
                    csi_sgr[SGR_BOLD     ] = VT_PROC{ p->brush.bld(true); }; // fx_sgr_bld<true>;
                    csi_sgr[SGR_FAINT    ] = VT_PROC{ p->brush.bld(faux); }; // fx_sgr_bld<faux>;
                    csi_sgr[SGR_ITALIC   ] = VT_PROC{ p->brush.itc(true); }; // fx_sgr_itc<true>;
                    csi_sgr[SGR_NONITALIC] = VT_PROC{ p->brush.itc(faux); }; // fx_sgr_itc<faux>;
                    csi_sgr[SGR_INV      ] = VT_PROC{ p->brush.inv(true); }; // fx_sgr_inv<true>;
                    csi_sgr[SGR_NOINV    ] = VT_PROC{ p->brush.inv(faux); }; // fx_sgr_inv<faux>;
                    csi_sgr[SGR_UND      ] = VT_PROC{ p->brush.und(   1); }; // fx_sgr_und;
                    csi_sgr[SGR_DOUBLEUND] = VT_PROC{ p->brush.und(   2); }; // fx_sgr_dnl;
                    csi_sgr[SGR_NOUND    ] = VT_PROC{ p->brush.und(faux); }; // fx_sgr_und;
                    csi_sgr[SGR_SLOWBLINK] = VT_PROC{ p->brush.blk(true); }; // fx_sgr_blk;
                    csi_sgr[SGR_FASTBLINK] = VT_PROC{ p->brush.blk(true); }; // fx_sgr_blk;
                    csi_sgr[SGR_NO_BLINK ] = VT_PROC{ p->brush.blk(faux); }; // fx_sgr_blk;
                    csi_sgr[SGR_STRIKE   ] = VT_PROC{ p->brush.stk(true); }; // fx_sgr_stk<true>;
                    csi_sgr[SGR_NOSTRIKE ] = VT_PROC{ p->brush.stk(faux); }; // fx_sgr_stk<faux>;
                    csi_sgr[SGR_OVERLN   ] = VT_PROC{ p->brush.ovr(true); }; // fx_sgr_ovr<faux>;
                    csi_sgr[SGR_NOOVERLN ] = VT_PROC{ p->brush.ovr(faux); }; // fx_sgr_ovr<faux>;
                    csi_sgr[SGR_FG_RGB   ] = VT_PROC{ p->brush.fgc(q);    }; // fx_sgr_fg_rgb   ;
                    csi_sgr[SGR_BG_RGB   ] = VT_PROC{ p->brush.bgc(q);    }; // fx_sgr_bg_rgb   ;
                    csi_sgr[SGR_FG_BLK   ] = VT_PROC{ p->brush.fgc(tint::blackdk  ); }; // fx_sgr_fg_16<tint::blackdk>  ;
                    csi_sgr[SGR_FG_RED   ] = VT_PROC{ p->brush.fgc(tint::reddk    ); }; // fx_sgr_fg_16<tint::reddk>    ;
                    csi_sgr[SGR_FG_GRN   ] = VT_PROC{ p->brush.fgc(tint::greendk  ); }; // fx_sgr_fg_16<tint::greendk>  ;
                    csi_sgr[SGR_FG_YLW   ] = VT_PROC{ p->brush.fgc(tint::yellowdk ); }; // fx_sgr_fg_16<tint::yellowdk> ;
                    csi_sgr[SGR_FG_BLU   ] = VT_PROC{ p->brush.fgc(tint::bluedk   ); }; // fx_sgr_fg_16<tint::bluedk>   ;
                    csi_sgr[SGR_FG_MGT   ] = VT_PROC{ p->brush.fgc(tint::magentadk); }; // fx_sgr_fg_16<tint::magentadk>;
                    csi_sgr[SGR_FG_CYN   ] = VT_PROC{ p->brush.fgc(tint::cyandk   ); }; // fx_sgr_fg_16<tint::cyandk>   ;
                    csi_sgr[SGR_FG_WHT   ] = VT_PROC{ p->brush.fgc(tint::whitedk  ); }; // fx_sgr_fg_16<tint::whitedk>  ;
                    csi_sgr[SGR_FG_BLK_LT] = VT_PROC{ p->brush.fgc(tint::blacklt  ); }; // fx_sgr_fg_16<tint::blacklt>  ;
                    csi_sgr[SGR_FG_RED_LT] = VT_PROC{ p->brush.fgc(tint::redlt    ); }; // fx_sgr_fg_16<tint::redlt>    ;
                    csi_sgr[SGR_FG_GRN_LT] = VT_PROC{ p->brush.fgc(tint::greenlt  ); }; // fx_sgr_fg_16<tint::greenlt>  ;
                    csi_sgr[SGR_FG_YLW_LT] = VT_PROC{ p->brush.fgc(tint::yellowlt ); }; // fx_sgr_fg_16<tint::yellowlt> ;
                    csi_sgr[SGR_FG_BLU_LT] = VT_PROC{ p->brush.fgc(tint::bluelt   ); }; // fx_sgr_fg_16<tint::bluelt>   ;
                    csi_sgr[SGR_FG_MGT_LT] = VT_PROC{ p->brush.fgc(tint::magentalt); }; // fx_sgr_fg_16<tint::magentalt>;
                    csi_sgr[SGR_FG_CYN_LT] = VT_PROC{ p->brush.fgc(tint::cyanlt   ); }; // fx_sgr_fg_16<tint::cyanlt>   ;
                    csi_sgr[SGR_FG_WHT_LT] = VT_PROC{ p->brush.fgc(tint::whitelt  ); }; // fx_sgr_fg_16<tint::whitelt>  ;
                    csi_sgr[SGR_BG_BLK   ] = VT_PROC{ p->brush.bgc(tint::blackdk  ); }; // fx_sgr_bg_16<tint::blackdk>  ;
                    csi_sgr[SGR_BG_RED   ] = VT_PROC{ p->brush.bgc(tint::reddk    ); }; // fx_sgr_bg_16<tint::reddk>    ;
                    csi_sgr[SGR_BG_GRN   ] = VT_PROC{ p->brush.bgc(tint::greendk  ); }; // fx_sgr_bg_16<tint::greendk>  ;
                    csi_sgr[SGR_BG_YLW   ] = VT_PROC{ p->brush.bgc(tint::yellowdk ); }; // fx_sgr_bg_16<tint::yellowdk> ;
                    csi_sgr[SGR_BG_BLU   ] = VT_PROC{ p->brush.bgc(tint::bluedk   ); }; // fx_sgr_bg_16<tint::bluedk>   ;
                    csi_sgr[SGR_BG_MGT   ] = VT_PROC{ p->brush.bgc(tint::magentadk); }; // fx_sgr_bg_16<tint::magentadk>;
                    csi_sgr[SGR_BG_CYN   ] = VT_PROC{ p->brush.bgc(tint::cyandk   ); }; // fx_sgr_bg_16<tint::cyandk>   ;
                    csi_sgr[SGR_BG_WHT   ] = VT_PROC{ p->brush.bgc(tint::whitedk  ); }; // fx_sgr_bg_16<tint::whitedk>  ;
                    csi_sgr[SGR_BG_BLK_LT] = VT_PROC{ p->brush.bgc(tint::blacklt  ); }; // fx_sgr_bg_16<tint::blacklt>  ;
                    csi_sgr[SGR_BG_RED_LT] = VT_PROC{ p->brush.bgc(tint::redlt    ); }; // fx_sgr_bg_16<tint::redlt>    ;
                    csi_sgr[SGR_BG_GRN_LT] = VT_PROC{ p->brush.bgc(tint::greenlt  ); }; // fx_sgr_bg_16<tint::greenlt>  ;
                    csi_sgr[SGR_BG_YLW_LT] = VT_PROC{ p->brush.bgc(tint::yellowlt ); }; // fx_sgr_bg_16<tint::yellowlt> ;
                    csi_sgr[SGR_BG_BLU_LT] = VT_PROC{ p->brush.bgc(tint::bluelt   ); }; // fx_sgr_bg_16<tint::bluelt>   ;
                    csi_sgr[SGR_BG_MGT_LT] = VT_PROC{ p->brush.bgc(tint::magentalt); }; // fx_sgr_bg_16<tint::magentalt>;
                    csi_sgr[SGR_BG_CYN_LT] = VT_PROC{ p->brush.bgc(tint::cyanlt   ); }; // fx_sgr_bg_16<tint::cyanlt>   ;
                    csi_sgr[SGR_BG_WHT_LT] = VT_PROC{ p->brush.bgc(tint::whitelt  ); }; // fx_sgr_bg_16<tint::whitelt>  ;

            #undef F
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

        vt_parser()
        {
            intro.resize(ctrl::NON_CONTROL);
            //intro[ctrl::BS ] = backspace;
            //intro[ctrl::DEL] = backspace;
            //intro[ctrl::CR ] = crlf;
            //intro[ctrl::EOL] = exec <fn::nl, 1>;

            auto& esc = intro[ctrl::ESC].resize(0x100);
                esc[ESC_CSI   ] = xcsi;
                esc[ESC_OCS   ] = xosc;
                esc[ESC_KEY_A ] = keym;
                esc[ESC_KEY_N ] = keym;
                esc[ESC_G0SET ] = g0__;
                //esc[ESC_SC] = ;
                //esc[ESC_RC] = ;
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
            auto y = [&](auto& cluster) { client->post(cluster); };

            utf::decode(s, y, utf8);
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
            using fifo = netxs::generics::bank<si32, maxarg>;

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
                            push(0); // Default zero parameter expressed by standalone delimiter/semicolon.
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
                    auto queue = fifo{ CCC_NOP }; // Reserve for the command type.
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
                    auto cmd = text{ OSC_LINUX_COLOR };
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
                    auto cmd = text{ OSC_LINUX_RESET };
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
                            if (c <= C0_ESC) // To avoid double comparing.
                            {
                                if (c == C0_BEL)
                                {
                                    exec(1);
                                    return;
                                }
                                else if (c == C0_ESC)
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
                    else if (c == C0_BEL) return; // Drop bcuz no ';' in the sequence.
                    else if (c == C0_ESC)
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
                ascii.pop_front(); // Take Final character C for designating 94-character sets.
                //todo implement
            }
        }
    };

    class parser
    {
    public:
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
                            //utf8 = { head, prev }; //todo apple clang doesn't get it // preserve ESC at the end
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
                                    //utf8 = { head, prev }; //todo apple clang doesn't get it // preserve ESC at the end
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
                            //utf8 = { head, prev }; //todo apple clang doesn't get it // preserve ESC at the end
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
                            //utf8 = { head, prev }; //todo apple clang doesn't get it // preserve ESC at the end
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
                            //utf8 = { head, prev }; //todo apple clang doesn't get it // preserve ESC at the end
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
                    //utf8 = { head, prev }; //todo apple clang doesn't get it // preserve ESC at the end
                    utf8 = view{ &(*head), (size_t)(prev - head) }; // preserve ESC at the end
                    return utf8;
                }
            }
        }

        utf::purify(utf8);
        return utf8;
    }

    namespace dtvt
    {
        using hint = netxs::events::type;

        namespace binary
        {
            using type = byte;
            static constexpr auto is_list = type{ 1 << (sizeof(type) * 8 - 1) };

            //todo unify
            struct frag : public view
            { };

            #pragma pack(push,1)
            static constexpr auto initial = char{ '\xFF' };
            union marker
            {
                struct mask
                {
                    using twod = le_t<netxs::twod>;
                    using sz_t = le_t<size_t>;
                    char mark_FF;
                    twod winsize;
                    sz_t cfgsize;
                    char mark_FE;
                };

                static constexpr auto size = sizeof(mask);
                char data[size];
                mask pack;

                marker()
                { }
                marker(twod const& winsize, size_t cfgsize)
                {
                    pack.mark_FF = initial;
                    pack.winsize.set(winsize);
                    pack.cfgsize.set(cfgsize);
                    pack.mark_FE = initial - 1;
                }

                auto get_sz(twod& winsize, size_t& cfgsize)
                {
                    if (pack.mark_FF == initial
                     && pack.mark_FE == initial - 1)
                    {
                        winsize = pack.winsize.get();
                        cfgsize = pack.cfgsize.get();
                        return true;
                    }
                    else return faux;
                }
            };
            #pragma pack(pop)

            struct stream
            {
                //todo revise
                type next{};

            protected:
                text block;
                sz_t basis;
                sz_t start;

                // stream: .
                template<class T>
                inline void fuse(T&& data)
                {
                    using D = std::remove_cv_t<std::remove_reference_t<T>>;
                    if constexpr (std::is_same_v<D, char>
                               || std::is_same_v<D, byte>
                               || std::is_same_v<D, type>)
                    {
                        block.text::push_back(static_cast<char>(data));
                    }
                    else if constexpr (std::is_integral_v<D>
                                    || std::is_same_v<D, twod>
                                    || std::is_same_v<D, dent>
                                    || std::is_same_v<D, rect>)
                    {
                        auto le_data = netxs::letoh(data);
                        auto v = view{ reinterpret_cast<char const*>(&le_data), sizeof(le_data) };
                        block += v;
                    }
                    else if constexpr (std::is_same_v<D, rgba>)
                    {
                        auto v = view{ reinterpret_cast<char const*>(&data), sizeof(data) };
                        block += v;
                    }
                    else if constexpr (std::is_same_v<D, view>
                                    || std::is_same_v<D, qiew>
                                    || std::is_same_v<D, text>)
                    {
                        auto length = static_cast<sz_t>(data.length());
                        auto le_data = netxs::letoh(length);
                        auto size = view{ reinterpret_cast<char const*>(&le_data), sizeof(le_data) };
                        block += size;
                        block += data;
                    }
                    else if constexpr (std::is_same_v<D, std::unordered_map<text, text>>
                                    || std::is_same_v<D, std::map<text, text>>
                                    || std::is_same_v<D, imap<text, text>>)
                    {
                        //todo implement
                    }
                    else if constexpr (std::is_same_v<D, noop>)
                    {
                        // Noop.
                    }
                    else log("dtvt: unsupported data type");
                }
                // stream: Replace bytes at specified position.
                template<class T>
                inline auto& add_at(sz_t at, T&& data)
                {
                    auto le_data = netxs::letoh(std::forward<T>(data));
                    ::memcpy(block.text::data() + at, reinterpret_cast<void const*>(&le_data), sizeof(le_data));
                    return *this;
                }
                // stream: .
                template<class T, bool PeekOnly = faux>
                static auto _take_item(view& data)
                {
                    using D = std::remove_cv_t<std::remove_reference_t<T>>;
                    if constexpr (std::is_same_v<D, view>
                               || std::is_same_v<D, text>)
                    {
                        if (data.size() < sizeof(sz_t))
                        {
                            log("dtvt: corrupted view frame");
                            return view{};
                        }
                        auto size = netxs::letoh(*reinterpret_cast<sz_t const*>(data.data()));
                        if (data.size() < size + sizeof(size))
                        {
                            log("dtvt: corrupted data");
                            return view{};
                        }
                        auto crop = view{ data.data() + sizeof(sz_t), (size_t)size };
                        if constexpr (!PeekOnly)
                        {
                            data.remove_prefix(sizeof(sz_t) + size);
                        }
                        return crop;
                    }
                    else if constexpr (std::is_same_v<D, frag>)
                    {
                        auto crop = frag{ data };
                        if constexpr (!PeekOnly)
                        {
                            data.remove_prefix(data.size());
                        }
                        return crop;
                    }
                    else if constexpr (std::is_same_v<D, noop>)
                    {
                        return 0;
                    }
                    else
                    {
                        if (data.size() < sizeof(D))
                        {
                            log("dtvt: corrupted integer data");
                            return D{};
                        }
                        auto crop = netxs::letoh(*reinterpret_cast<D const*>(data.data()));
                        if constexpr (!PeekOnly)
                        {
                            data.remove_prefix(sizeof(D));
                        }
                        return crop;
                    }
                }
                // stream: .
                template<class ...Fields>
                static auto _take(std::tuple<Fields...>, view& data)
                {
                    return std::tuple{ _take_item<Fields>(data)..., };
                }

            public:
                // stream: .
                template<class T>
                inline auto& add(T&& data)
                {
                    fuse(std::forward<T>(data));
                    return *this;
                }
                // stream: .
                template<class Char, class Size_t>
                inline auto& add(Char* data, Size_t size)
                {
                    block += view{ data, size };
                    return *this;
                }
                // stream: .
                template<class T, class ...Args>
                inline auto& add(T&& data, Args&&... data_list)
                {
                    fuse(std::forward<T>(data));
                    return add(std::forward<Args>(data_list)...);
                }
                // stream: .
                template<class T>
                static void peek(T&& dest, view& data)
                {
                    dest = _take_item<T, true>(data);
                }
                // stream: .
                template<class ...Fields>
                static auto take(view& data)
                {
                    return std::tuple{ _take_item<Fields>(data)..., };
                }
                // stream: .
                template<class T>
                static void take(T&& dest, view& data)
                {
                    dest = _take_item<T>(data);
                }
                // stream: .
                static void take(void *dest, size_t size, view& data)
                {
                    ::memcpy(dest, reinterpret_cast<void const*>(data.data()), size);
                    data.remove_prefix(size);
                }
                // stream: .
                static auto take_substr(size_t size, view& data)
                {
                    if (size > data.size())
                    {
                        log("dtvt: wrong data size");
                        return view{};
                    }
                    auto crop = data.substr(0, size);
                    data.remove_prefix(size);
                    return crop;
                }
                // stream: Check DirectVT frame integrity.
                static auto purify(view flow)
                {
                    auto size = flow.size();
                    auto head = flow.data();
                    auto iter = head;
                    while (size >= sizeof(sz_t))
                    {
                        auto step = *reinterpret_cast<sz_t const*>(iter);
                        if (step < sizeof(sz_t))
                        {
                            log("dtvt: stream corrupted, frame size: ", step);
                            break;
                        }
                        if (size < step) break;
                        size -= step;
                        iter += step;
                    }
                    auto crop = qiew(head, iter - head);
                    return crop;
                }
                // stream: .
                template<class T, class P, bool Plain = std::is_same_v<void, std::invoke_result_t<P, qiew>>>
                static void reading_loop(T& link, P&& proc)
                {
                    auto flow = text{};
                    while (link)
                    {
                        auto shot = link.recv();
                        if (shot && link)
                        {
                            flow += shot;
                            if (auto crop = purify(flow))
                            {
                                if constexpr (Plain) proc(crop);
                                else            if (!proc(crop)) break;
                                flow.erase(0, crop.size()); // Delete processed data.
                            }
                        }
                        else break;
                    }
                }

                // stream: .
                auto length() const
                {
                    return static_cast<sz_t>(block.length());
                }
                // stream: .
                auto reset()
                {
                    block.resize(basis);
                    return sz_t{ 0 };
                }
                // stream: .
                template<class ...Args>
                void reinit(Args&&... args)
                {
                    reset();
                    add(std::forward<Args>(args)...);
                    start = length();
                }
                // stream: .
                auto commit(bool discard_empty = faux)
                {
                    auto size = length();
                    if (discard_empty && size == start)
                    {
                        return reset();
                    }
                    else
                    {
                        stream::add_at(0, size);
                        return size;
                    }
                }
                // stream: .
                template<bool Discard_empty = faux, class T>
                void sendby(T&& sender)
                {
                    if (stream::commit(Discard_empty))
                    {
                        sender.output(block);
                        stream::reset();
                    }
                }
                // stream: .
                template<type Kind, class ...Args>
                void set(Args&&... args)
                {
                    add(Kind, std::forward<Args>(args)...);
                }
                // stream: .
                void emplace(stream& other)
                {
                    other.commit();
                    block += other.block;
                    other.reset();
                }

                stream(type kind)
                    : basis{ sizeof(basis) + sizeof(kind) },
                      start{ basis }
                {
                    add(basis, kind);
                }
            };

            template<class Base>
            class wrapper
            {
                using utex = std::mutex;
                using cond = std::condition_variable;
                using Lock = std::unique_lock<utex>;

                utex mutex; // wrapper: Accesss mutex.
                cond synch; // wrapper: Accesss notificator.
                Base thing; // wrapper: Protected object.

            public:
                static constexpr auto kind = Base::kind;

                struct access
                {
                    Lock  guard; // access: .
                    Base& thing; // access: .
                    cond& synch; // access: .

                    access(utex& mutex, cond& synch, Base& thing)
                        : guard{ mutex },
                          thing{ thing },
                          synch{ synch }
                    { }
                    access(access&& other)
                        : guard{ std::move(other.guard) },
                          thing{ other.thing },
                          synch{ other.synch }
                    { }
                   ~access()
                    {
                        if (guard) synch.notify_all();
                    }
                    template<class Period>
                    auto wait_for(Period&& maxoff)
                    {
                        return synch.wait_for(guard, std::forward<Period>(maxoff));
                    }
                };

                // wrapper .
                auto freeze()
                {
                    return access{ mutex, synch, thing };
                }
                // wrapper .
                template<bool Discard_empty = faux, class T, class ...Args>
                void send(T&& sender, Args&&... args)
                {
                    auto lock = freeze();
                    if constexpr (!!sizeof...(args))
                    {
                        thing.set(std::forward<Args>(args)...);
                    }
                    thing.template sendby<Discard_empty>(sender);
                }
                // wrapper .
                template<class ...Args>
                auto operator () (Args&&... args)
                {
                    auto lock = freeze();
                    thing.set(std::forward<Args>(args)...);
                    return lock;
                }
                // wrapper .
                auto sync(view& data)
                {
                    auto lock = freeze();
                    thing.get(data);
                    return lock;
                }
            };

            template<class Base, class Type>
            struct list
                : stream
            {
                static constexpr auto kind = type{ Type::kind | binary::is_list };

                Base copy;
                Type item;

                struct iter
                {
                    view  rest;
                    view  crop;
                    bool  stop;
                    Type& item;

                    iter(view data, Type& item)
                        : rest{ data },
                          stop{ faux },
                          item{ item }
                    {
                        operator++();
                    }

                    template<class A>
                    bool  operator == (A&&) const { return stop;                 }
                    auto& operator  * ()    const { item.get(crop); return item; }
                    auto& operator  * ()          { item.get(crop); return item; }
                    auto  operator ++ ()
                    {
                        static constexpr auto head = sizeof(sz_t) + sizeof(type);
                        stop = rest.size() < head;
                        if (stop) crop = {};
                        else
                        {
                            auto size = sz_t{};
                            std::tie(size, item.next) = stream::template take<sz_t, type>(rest);
                            stop = size > rest.size() + head;
                            if (stop) log("dtvt: corrupted data");
                            else
                            {
                                crop = rest.substr(0, size - head);
                                rest.remove_prefix(size - head);
                            }
                        }
                    }
                };

            public:
                list(list const&) = default;
                list(list&&)      = default;
                list()
                    : stream{ kind }
                { }

                auto begin() { return iter{ copy, item }; }
                auto   end() { return text::npos; }

                // list: .
                template<class ...Args>
                void push(Args&&... args)
                {
                    item.set(std::forward<Args>(args)...);
                    stream::emplace(item);
                }
                // list: .
                template<class ...Args>
                void set(Args&&... args)
                {
                    item.set(std::forward<Args>(args)...);
                }
                // list: .
                void get(view& data)
                {
                    copy = data;
                    data = {};
                }
            };

            static constexpr auto _counter_base = __COUNTER__;

            // Definition of plain objects.
            #define MACROGEN_DEF
            #include "../abstract/macrogen.hpp"

            #define STRUCT(struct_name, struct_members)                               \
                struct CAT(struct_name, _t) : public stream                           \
                {                                                                     \
                    static constexpr auto kind = type{ __COUNTER__ - _counter_base }; \
                    SEQ_ATTR(WRAP(struct_members))                                    \
                    CAT(struct_name, _t)()                                            \
                        : stream{ kind }                                              \
                    { }                                                               \
                    void set(SEQ_SIGN(WRAP(struct_members)) int _tmp = {})            \
                    {                                                                 \
                        SEQ_INIT(WRAP(struct_members))                                \
                        stream::reset();                                              \
                        stream::add(SEQ_NAME(WRAP(struct_members)) noop{});           \
                    }                                                                 \
                    template<class T>                                                 \
                    void set(T&& source)                                              \
                    {                                                                 \
                        SEQ_TEMP(WRAP(struct_members))                                \
                        stream::reset();                                              \
                        stream::add(SEQ_NAME(WRAP(struct_members)) noop{});           \
                    }                                                                 \
                    void get(view& _data)                                             \
                    {                                                                 \
                        int _tmp;                                                     \
                        std::tie(SEQ_NAME(WRAP(struct_members)) _tmp) =               \
                            stream::take<SEQ_TYPE(WRAP(struct_members)) noop>(_data); \
                    }                                                                 \
                    void wipe()                                                       \
                    {                                                                 \
                        SEQ_WIPE(WRAP(struct_members))                                \
                        stream::reset();                                              \
                    }                                                                 \
                                                                                      \
                    friend std::ostream& operator << (std::ostream& s,                \
                                                        CAT(struct_name, _t) const& o)\
                    {                                                                 \
                        s << #struct_name " {";                                       \
                        SEQ_LOGS(WRAP(struct_members))                                \
                        s << " }";                                                    \
                        return s;                                                     \
                    }                                                                 \
                };                                                                    \
                using struct_name = wrapper<CAT(struct_name, _t)>;
            //todo use C++20 __VA_OPT__ (MSVC not ready yet)
            #define STRUCT_LITE(struct_name)                                          \
                struct CAT(struct_name, _t) : public stream                           \
                {                                                                     \
                    static constexpr auto kind = type{ __COUNTER__ - _counter_base }; \
                    CAT(struct_name, _t)()                                            \
                        : stream{ kind }                                              \
                    { }                                                               \
                    void set() {}                                                     \
                    void get(view& data) {}                                           \
                                                                                      \
                    friend std::ostream& operator << (std::ostream& s,                \
                                                        CAT(struct_name, _t) const& o)\
                    {                                                                 \
                        return s << #struct_name " { }";                              \
                    }                                                                 \
                };                                                                    \
                using struct_name = wrapper<CAT(struct_name, _t)>;

            using imap = netxs::imap<text, text>;
            //todo unify
            static auto& operator << (std::ostream& s, imap const& o) { return s << "{...}"; }
            static auto& operator << (std::ostream& s, wchr const& o) { return s << "0x" << utf::to_hex(o); }

            // Output stream.
            STRUCT(frame_element,     (frag, data))
            STRUCT(jgc_element,       (ui64, token) (text, cluster))
            STRUCT(tooltip_element,   (id_t, gear_id) (text, tip_text))
            STRUCT(mouse_event,       (id_t, gear_id) (hint, cause) (twod, coord) (twod, delta) (ui32, buttons))
            STRUCT(set_clipboard,     (id_t, gear_id) (twod, clip_prev_size) (text, clipdata) (si32, mimetype))
            STRUCT(request_clipboard, (id_t, gear_id))
            STRUCT(set_focus,         (id_t, gear_id) (bool, combine_focus) (bool, force_group_focus))
            STRUCT(off_focus,         (id_t, gear_id))
            STRUCT(maximize,          (id_t, gear_id))
            STRUCT(form_header,       (id_t, window_id) (text, new_header))
            STRUCT(form_footer,       (id_t, window_id) (text, new_footer))
            STRUCT(warping,           (id_t, window_id) (dent, warpdata))
            STRUCT(vt_command,        (text, command))
            STRUCT(configuration,     (imap, confug))
            STRUCT_LITE(expose)
            STRUCT_LITE(request_debug)

            // Input stream.
            STRUCT(sysfocus,          (id_t, gear_id) (bool, enabled) (bool, combine_focus) (bool, force_group_focus))
            STRUCT(syskeybd,          (id_t, gear_id) (ui32, ctlstat) (ui32, winctrl) (ui32, virtcod) (ui32, scancod) (bool, pressed) (ui32, imitate) (text, cluster) (wchr, winchar))
            STRUCT(sysmouse,          (id_t, gear_id)  // sysmouse: Devide id.
                                      (ui32, enabled)  // sysmouse: Mouse device health status.
                                      (ui32, ctlstat)  // sysmouse: Keybd modifiers state.
                                      (ui32, winctrl)  // sysmouse: Windows specific keybd modifier state.
                                      (ui32, buttons)  // sysmouse: Buttons bit state.
                                      (bool, doubled)  // sysmouse: Double click.
                                      (bool, wheeled)  // sysmouse: Vertical scroll wheel.
                                      (bool, hzwheel)  // sysmouse: Horizontal scroll wheel.
                                      (si32, wheeldt)  // sysmouse: Scroll delta.
                                      (twod, coordxy)  // sysmouse: Cursor coordinates.
                                      (ui32, changed)) // sysmouse: Update stamp.
            STRUCT(mouse_show,        (bool, mode)) // CCC_SMS/* 26:1p */
            STRUCT(winsz,             (id_t, gear_id) (twod, winsize))
            STRUCT(clipdata,          (id_t, gear_id) (text, data) (si32, mimetype))
            STRUCT(osclipdata,        (id_t, gear_id) (text, data) (si32, mimetype))
            STRUCT(plain,             (id_t, gear_id) (text, utf8txt))
            STRUCT(ctrls,             (id_t, gear_id) (ui32, ctlstat))
            STRUCT(unknown_gc,        (ui64, token))
            STRUCT(fps,               (si32, frame_rate))
            STRUCT(bgc,               (rgba, color))
            STRUCT(fgc,               (rgba, color))
            STRUCT(slimmenu,          (bool, menusize))
            STRUCT(debugdata,         (text, data))
            STRUCT(debuglogs,         (text, data))
            STRUCT(debugtext,         (text, data))

            #undef STRUCT
            #undef STRUCT_LITE
            #define MACROGEN_UNDEF
            #include "../abstract/macrogen.hpp"

            // Definition of complex objects.
            class bitmap_t
                : public stream
            {
            public:
                static constexpr auto kind = type{ __COUNTER__ - _counter_base };

                bitmap_t()
                    : stream{ kind }
                { }

                cell                           state; // bitmap: .
                core                           image; // bitmap: .
                std::unordered_map<ui64, text> newgc; // bitmap: Unknown grapheme cluster list.

                struct subtype
                {
                    static constexpr auto nop = byte{ 0x00 }; // Apply current brush. nop = dif - refer.
                    static constexpr auto dif = byte{ 0x20 }; // Cell dif.
                    static constexpr auto mov = byte{ 0xFE }; // Set insertion point. sz_t: offset.
                    static constexpr auto rep = byte{ 0xFF }; // Repeat current brush ui32 times. sz_t: N.
                };

                enum : byte
                {
                    refer = 1 << 0, // 1 - Diff with our canvas cell, 0 - diff with current brush (state).
                    bgclr = 1 << 1,
                    fgclr = 1 << 2,
                    style = 1 << 3,
                    glyph = 1 << 4,
                };

                void set(id_t winid, twod const& coord, core& cache, bool& abort, sz_t& delta)
                {
                    //todo multiple windows
                    stream::reinit(winid, rect{ coord, cache.size() });
                    auto pen = state;
                    auto src = cache.iter();
                    auto end = cache.iend();
                    auto csz = cache.size();
                    auto fsz = image.size();
                    auto dst = image.iter();
                    auto dtx = fsz.x - csz.x;
                    auto min = std::min(csz, fsz);
                    auto beg = src + 1;
                    auto mid = src + csz.x * min.y;
                    bool bad = true;
                    auto sum = sz_t{ 0 };
                    auto rep = [&]
                    {
                        if (sum < sizeof(subtype::rep) + sizeof(sum))
                        {
                            do add(subtype::nop);
                            while (--sum);
                        }
                        else
                        {
                            add(subtype::rep, sum);
                            sum = 0;
                        }
                    };
                    auto tax = [](cell const& c1, cell const& c2)
                    {
                        auto meaning = 0;
                        auto cluster = byte{ 0 };
                        auto changes = byte{ 0 };
                        if (c1.bgc() != c2.bgc()) { meaning += sizeof(c1.bgc()); changes |= bgclr; }
                        if (c1.fgc() != c2.fgc()) { meaning += sizeof(c1.fgc()); changes |= fgclr; }
                        if (c1.stl() != c2.stl()) { meaning += sizeof(c1.stl()); changes |= style; }
                        if (c1.egc() != c2.egc())
                        {
                            cluster = c1.egc().state.jumbo ? 8
                                                           : c1.egc().state.count + 1;
                            meaning += cluster + 1;
                            changes |= glyph;
                        }
                        return std::tuple{ meaning, changes, cluster };
                    };
                    auto dif = [&](byte changes, byte cluster, cell const& cache)
                    {
                        add(changes);
                        if (changes & bgclr) add(cache.bgc());
                        if (changes & fgclr) add(cache.fgc());
                        if (changes & style) add(cache.stl());
                        if (changes & glyph) add(cluster, cache.egc().glyph, cluster);
                        state = cache;
                    };
                    auto map = [&](auto const& cache, auto const& front)
                    {
                        if (cache != front)
                        {
                            if (bad)
                            {
                                if (sum) rep();
                                auto offset = static_cast<sz_t>(src - beg);
                                add(subtype::mov, offset);
                                bad = faux;
                            }
                            if (cache == state) ++sum;
                            else
                            {
                                if (sum) rep();
                                auto [s_meaning, s_changes, s_cluster] = tax(cache, state);
                                auto [f_meaning, f_changes, f_cluster] = tax(cache, front);
                                if (s_meaning < f_meaning) dif(s_changes,         s_cluster, cache);
                                else                       dif(f_changes | refer, f_cluster, cache);
                            }
                        }
                        else bad = true;
                    };
                    while (src != mid && !abort)
                    {
                        auto end = src + min.x;
                        while (src != end) map(*src++, *dst++);

                        if (dtx >= 0) dst += dtx;
                        else
                        {
                            end += -dtx;
                            while (src != end) map(*src++, pen);
                        }
                    }
                    if (csz.y > fsz.y)
                    {
                        while (src != end && !abort) map(*src++, pen);
                    }
                    if (sum) rep();
                    if (abort)
                    {
                        std::swap(state, pen);
                        sum = reset();
                    }
                    else
                    {
                        auto discard_empty = fsz == csz;
                        std::swap(image, cache);
                        sum = commit(discard_empty);
                    }
                    delta = sum;
                }
                void get(view& data)
                {
                    auto [myid, area] = stream::take<id_t, rect>(data);
                    //todo head.myid
                    if (image.size() != area.size)
                    {
                        image.crop(area.size);
                    }
                    auto mark = image.mark();
                    auto head = image.iter();
                    auto tail = image.iend();
                    auto iter = head;
                    auto size = tail - head;
                    auto take = [&](auto what, cell& c)
                    {
                        if (what & bgclr) stream::take(c.bgc(), data);
                        if (what & fgclr) stream::take(c.fgc(), data);
                        if (what & style) stream::take(c.stl(), data);
                        if (what & glyph)
                        {
                            auto [size] = stream::take<byte>(data);
                            stream::take(c.egc().glyph, size, data);
                            if (c.jgc() == faux) newgc[c.tkn()];
                        }
                        return c;
                    };
                    //auto frame_len = data.size();
                    //auto nop_count = 0;
                    //auto rep_count = 0;
                    //auto mov_count = 0;
                    //auto dif_count = 0;
                    while (data.size() > 0)
                    {
                        auto [what] = stream::take<byte>(data);
                        if (what == subtype::nop)
                        {
                            //nop_count++;
                            *iter++ = mark;
                        }
                        else if (what < subtype::dif)
                        {
                            //dif_count++;
                            if (what & refer) mark = take(what, *iter++);
                            else           *iter++ = take(what, mark);
                        }
                        else if (what == subtype::rep)
                        {
                            //rep_count++;
                            auto [count] = stream::take<sz_t>(data);
                            if (count > tail - iter)
                            {
                                log("dtvt: bitmap: corrupted data, subtype: ", what);
                                break;
                            }
                            auto from = iter;
                            std::fill(from, iter += count, mark);
                        }
                        else if (what == subtype::mov)
                        {
                            //mov_count++;
                            auto [offset] = stream::take<sz_t>(data);
                            if (offset >= size)
                            {
                                log("dtvt: bitmap: corrupted data, subtype: ", what);
                                break;
                            }
                            iter = head + offset;
                        }
                        else // Unknown subtype.
                        {
                            log("dtvt: bitmap: unknown data, subtype: ", what);
                            break;
                        }
                    }
                    image.mark(mark);
                    //log("dtvt: frame len: ", frame_len);
                    //log("dtvt: nop count: ", nop_count);
                    //log("dtvt: rep count: ", rep_count);
                    //log("dtvt: dif count: ", dif_count);
                    //log("----------------------------");
                }
            };
            using bitmap = wrapper<bitmap_t>;

            using frames_t     = list<view,   frame_element_t>;
            using jgc_list_t   = list<text,     jgc_element_t>;
            using tooltips_t   = list<text, tooltip_element_t>;
            using request_gc_t = list<text, unknown_gc_t>;

            using frames     = wrapper<frames_t  >;
            using jgc_list   = wrapper<jgc_list_t>;
            using tooltips   = wrapper<tooltips_t>;
            using request_gc = wrapper<request_gc_t>;

            struct s11n
            {
                #define OBJECT_LIST \
                /* Output stream                                                      */\
                X(bitmap           ) /* Canvas data.                                  */\
                X(mouse_event      ) /* Mouse events.                                 */\
                X(tooltips         ) /* Tooltip list.                                 */\
                X(jgc_list         ) /* List of jumbo GC.                             */\
                X(set_clipboard    ) /* Set main clipboard using following data.      */\
                X(request_clipboard) /* Request main clipboard data.                  */\
                X(off_focus        ) /* Request to remove focus.                      */\
                X(set_focus        ) /* Request to set focus.                         */\
                X(maximize         ) /* Request to maximize/restore                   */\
                X(form_header      ) /* Set window title.                             */\
                X(form_footer      ) /* Set window footer.                            */\
                X(warping          ) /* Warp resize.                                  */\
                X(expose           ) /* Bring the form to the front.                  */\
                X(vt_command       ) /* Parse following vt-sequences in UTF-8 format. */\
                X(configuration    ) /* Initial application configuration.            */\
                X(frames           ) /* Received frames.                              */\
                X(tooltip_element  ) /* Tooltip text.                                 */\
                X(jgc_element      ) /* jumbo GC: gc.token + gc.view.                 */\
                X(request_debug    ) /* Request debug output redirection to stdin.    */\
                /* Input stream                                                       */\
                X(sysfocus         ) /* System focus state.                           */\
                X(syskeybd         ) /* System keybd device.                          */\
                X(sysmouse         ) /* System mouse device.                          */\
                X(mouse_show       ) /* Show mouse cursor.                            */\
                X(winsz            ) /* Window resize.                                */\
                X(clipdata         ) /* Clipboard raw data.                           */\
                X(osclipdata       ) /* OS clipboard data.                            */\
                X(plain            ) /* Raw text input.                               */\
                X(ctrls            ) /* Keyboard modifiers state.                     */\
                X(request_gc       ) /* Unknown gc token list.                        */\
                X(unknown_gc       ) /* Unknown gc token.                             */\
                X(fps              ) /* Set frame rate.                               */\
                X(bgc              ) /* Set background color.                         */\
                X(fgc              ) /* Set foreground color.                         */\
                X(slimmenu         ) /* Set window menu size.                         */\
                X(debugdata        ) /* Debug data.                                   */\
                X(debuglogs        ) /* Debug logs.                                   */\
                X(debugtext        ) /* Debug forwarding.                             */

                struct xs
                {
                    #define X(_object) using _object = binary::_object::access;
                    OBJECT_LIST
                    #undef X
                };

                #define X(_object) binary::_object _object;
                OBJECT_LIST
                #undef X

                std::unordered_map<type, std::function<void(view&)>> exec; // s11n: .

                void sync(view& data)
                {
                    auto lock = frames.sync(data);
                    for(auto& frame : lock.thing)
                    {
                        auto iter = exec.find(frame.next);
                        if (iter != exec.end())
                        {
                            iter->second(frame.data);
                        }
                        else log("s11n: unsupported frame type: ", (int)frame.next, "\n", utf::debase(frame.data));
                    }
                }

                s11n() = default;
                template<class T>
                s11n(T& boss, id_t boss_id = {})
                {
                    #define X(_object) \
                        if constexpr (requires(view data) { boss.handle(_object.sync(data)); }) \
                            exec[binary::_object::kind] = [&](auto& data) { boss.handle(_object.sync(data)); };
                    OBJECT_LIST
                    #undef X

                    auto lock = bitmap.freeze();
                    lock.thing.image.link(boss_id);
                }

                #undef OBJECT_LIST
            };
        }

        namespace ascii
        {
            template<svga VGAMODE = svga::truecolor>
            class bitmap
                : public basevt<text>
            {
                text block; // ascii::bitmap: .
                cell state; // ascii::bitmap: .
                core image; // ascii::bitmap: .

            public:
                // ascii::bitmap: .
                template<class T>
                void operator += (T&& str) { block += std::forward<T>(str); }

                bitmap()
                    : basevt{ block }
                { }

                // ascii::bitmap: .
                auto length() const
                {
                    return static_cast<sz_t>(block.length());
                }
                // ascii::bitmap: .
                auto reset()
                {
                    block.clear();
                    return length();
                }
                // ascii::bitmap: .
                template<class T>
                void sendby(T&& sender)
                {
                    if (commit())
                    {
                        sender.output(block);
                        block.clear();
                    }
                }
                // ascii::bitmap: .
                auto commit()
                {
                    return length();
                }

                // ascii::bitmap: .
                void set(id_t winid, twod const& winxy, core& cache, bool& abort, sz_t& delta)
                {
                    auto coord = dot_00;
                    auto saved = state;
                    auto field = cache.size();

                    auto mov = [&](auto x)
                    {
                        coord.x = static_cast<decltype(coord.x)>(x);
                        basevt::locate(coord);
                    };
                    auto put = [&](cell const& cache)
                    {
                        //todo
                        cache.scan<VGAMODE>(state, *this);
                    };
                    auto dif = [&](cell const& cache, cell const& front)
                    {
                        //todo
                        return !cache.scan<VGAMODE>(front, state, *this);
                    };
                    auto left_half = [&](cell const& cache)
                    {
                        auto temp = cache;
                        temp.txt(cache.get_c0_left());
                        put(temp);
                    };
                    auto right_half = [&](cell const& cache)
                    {
                        auto temp = cache;
                        temp.txt(cache.get_c0_right());
                        put(temp);
                    };
                    auto tie = [&](cell const& fore, cell const& next)
                    {
                        if (dif(fore, next))
                        {
                             left_half(fore);
                            right_half(next);
                        }
                    };

                    if (image.hash() != cache.hash())
                    {
                        basevt::scroll_wipe();
                        auto src = cache.iter();
                        while (coord.y < field.y)
                        {
                            if (abort)
                            {
                                delta = reset();
                                state = saved;
                                break;
                            }
                            basevt::locate(coord);
                            auto end = src + field.x;
                            while (src != end)
                            {
                                auto& c = *src++;
                                if (c.wdt() < 2) put(c);
                                else
                                {
                                    if (c.wdt() == 2)
                                    {
                                        if (src != end)
                                        {
                                            auto& next = *src;
                                            if (next.wdt() < 3) left_half(c);
                                            else
                                            {
                                                if (dif(c, next)) left_half(c);
                                                else              ++src;
                                            }
                                        }
                                        else left_half(c);
                                    }
                                    else right_half(c);
                                }
                            }
                            ++coord.y;
                        }
                        std::swap(image, cache);
                        delta = commit();
                    }
                    else
                    {
                        auto src = cache.iter();
                        auto dst = image.iter();
                        while (coord.y < field.y)
                        {
                            if (abort)
                            {
                                delta = reset();
                                state = saved;
                                break;
                            }
                            auto beg = src + 1;
                            auto end = src + field.x;
                            while (src != end)
                            {
                                auto& fore = *src++;
                                auto& back = *dst++;
                                auto w = fore.wdt();
                                if (w < 2)
                                {
                                    if (back != fore)
                                    {
                                        mov(src - beg);
                                        put(fore);
                                        while (src != end)
                                        {
                                            auto& fore = *src++;
                                            auto& back = *dst++;
                                            auto w = fore.wdt();
                                            if (w < 2)
                                            {
                                                if (back == fore) break;
                                                else              put(fore);
                                            }
                                            else if (w == 2) // Check left part.
                                            {
                                                if (src != end)
                                                {
                                                    auto& next = *src;
                                                    if (back == fore && next == *dst)
                                                    {
                                                        ++src;
                                                        ++dst;
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        if (next.wdt() < 3) left_half(fore);
                                                        else // next.wdt() == 3
                                                        {
                                                            tie(fore, next);
                                                            ++src;
                                                            ++dst;
                                                        }
                                                    }
                                                }
                                                else left_half(fore);
                                            }
                                            else right_half(fore); // w == 3
                                        }
                                    }
                                }
                                else
                                {
                                    if (w == 2) // Left part has changed.
                                    {
                                        if (back != fore)
                                        {
                                            mov(src - beg);
                                            if (src != end)
                                            {
                                                auto& next = *src;
                                                if (next.wdt() < 3) left_half(fore);
                                                else // next.wdt() == 3
                                                {
                                                    tie(fore, next);
                                                    ++src;
                                                    ++dst;
                                                }
                                            }
                                            else left_half(fore);
                                        }
                                        else // Check right part.
                                        {
                                            if (src != end)
                                            {
                                                auto& next = *src;
                                                if (next.wdt() < 3) mov(src - beg), left_half(fore);
                                                else // next.wdt() == 3
                                                {
                                                    if (next != *dst)
                                                    {
                                                        mov(src - beg);
                                                        tie(fore, next);
                                                    }
                                                    ++src;
                                                    ++dst;
                                                }
                                            }
                                            else mov(src - beg), left_half(fore);
                                        }
                                    }
                                    else mov(src - beg), right_half(fore); // w == 3
                                }
                            }
                            ++coord.y;
                        }

                        std::swap(image, cache);
                        delta = commit();
                    }
                }
            };
        }
    }
}

#endif // NETXS_ANSI_HPP