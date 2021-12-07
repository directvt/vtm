// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_ANSI_HPP
#define NETXS_ANSI_HPP

#include "../ui/layout.hpp"
#include "../text/utf.hpp"
#include "../abstract/tree.hpp"

#include <mutex>
#include <array>
#include <list>
#include <functional>

#define VT_PROC [](auto& q, auto& p)

namespace netxs::ansi
{
    using namespace netxs::ui::atoms;
    using namespace netxs::utf;

    static constexpr auto ESCCSI = "\033[";
    static constexpr auto ESCOCS = "\033]";

    static const char CSI   = '['; // ESC [
    static const char OCS   = ']'; // ESC ]
    static const char KEY_A = '='; // ESC =
    static const char KEY_N = '>'; // ESC >
    static const char G0SET = '('; // ESC (
    static const char DELIM = ';'; // ESC ;

    static const char ESC_SC  = '7'; // ESC 7         Save caret Position.
    static const char ESC_RC  = '8'; // ESC 8         Restore caret Position.
    static const char ESC_HTS = 'H'; // ESC H         Tab stop at the caret.
    static const char ESC_NEL = 'E'; // ESC E         Move caret down and CR.
    static const char ESC_IND = 'D'; // ESC D         Caret Down.
    static const char ESC_IR  = 'M'; // ESC M         Caret Up.
    static const char ESC_DCS = 'P'; // ESC P ... ST  DCS start
    static const char ESC_RIS = 'c'; // ESC c         Reset terminal to initial state.

    static const char CSI_SPC_SLC = '@'; // CSI n SP   @  — Shift left n columns(s).
    static const char CSI_SPC_SRC = 'A'; // CSI n SP   A  — Shift right n columns(s).
    static const char CSI_SPC_CST = 'q'; // CSI n SP   q  — Set caret style (DECSCUSR).

    static const char CSI_HSH_SCP = 'P'; // CSI n #    P  — Push current palette colors onto stack. n default is 0.
    static const char CSI_HSH_RCP = 'Q'; // CSI n #    Q  — Pop current palette colors from stack. n default is 0.
    static const char CSI_HSH_RVA = 'q'; // CSI   #    q  — Pop video attributes from stack (XTPOPSGR).

    static const char CSI_DQT_SCP = 'q'; // CSI n "    q  — Select character protection attribute.

    static const char CSI_EXL_RST = 'p'; // CSI   !    p  — Reset terminal to initial state.

    static const char CSI_CUU = 'A';     // CSI n      A  — Caret Up.
    static const char CSI_CUD = 'B';     // CSI n      B  — Caret Down.
    static const char CSI_CUD2= 'e';     // CSI n      e  — Caret Down.
    static const char CSI_CUF = 'C';     // CSI n      C  — Caret Forward.
    static const char CSI_CUB = 'D';     // CSI n      D  — Caret Back.
    static const char CSI_CNL = 'E';     // CSI n      E  — Caret Next Line.
    static const char CSI_CPL = 'F';     // CSI n      F  — Caret Previous Line.
    static const char CSI_CHX = 'G';     // CSI n      G  — Caret Horizontal Absolute.
    static const char CSI_CHY = 'd';     // CSI n      d  — Caret Vertical Absolute.
    static const char CSI_HVP = 'f';     // CSI n ; m  f  — Horizontal and Vertical Position.
    static const char CSI_CUP = 'H';     // CSI n ; m  H  — Caret Position.
    static const char CSI_CHT = 'I';     // CSI n      I  — Caret forward  n tab stops (default = 1).
    static const char CSI_CBT = 'Z';     // CSI n      Z  — Caret backward n tab stops (default = 1).
    static const char CSI_TBC = 'g';     // CSI n      g  — Reset tabstop value.
    static const char CSI_SGR = 'm';     // CSI n [;k] m  — Select Graphic Rendition.
    static const char CSI_DSR = 'n';     // CSI n      n  — Device Status Report (DSR). n==5 -> "OK"; n==6 -> CSI r ; c R
    static const char DECSTBM = 'r';     // CSI t ; b  r  — Set scrolling region (t/b: top + bottom).
    static const char CSI_SCP = 's';     // CSI        s  — Save caret Position.
    static const char CSI_RCP = 'u';     // CSI        u  — Restore caret Position.
    static const char CSI__EL = 'K';     // CSI n      K  — Erase 0: from caret to end, 1: from begin to caret, 2: all line.
    static const char CSI__IL = 'L';     // CSI n      L  — Insert n blank lines.
    static const char CSI__ED = 'J';     // CSI n      J  — Erase 0: from caret to end of screen, 1: from begin to caret, 2: all screen.
    static const char CSI__DL = 'M';     // CSI n      M  — Delete n lines.
    static const char CSI_DCH = 'P';     // CSI n      P  — Delete n character(s).
    static const char CSI_LED = 'q';     // CSI n      q  — Load keyboard LEDs.
    static const char CSI__SD = 'T';     // CSI n      T  — Scroll down by n lines, scrolled out lines are lost.
    static const char CSI__SU = 'S';     // CSI n      S  — Scroll   up by n lines, scrolled out lines are lost.
    static const char CSI_WIN = 't';     // CSI n;m;k  t  — XTWINOPS, Terminal window props.
    static const char CSI_ECH = 'X';     // CSI n      X  — Erase n character(s) ? difference with delete ?
    static const char CSI_ICH = '@';     // CSI n      @  — Insert/wedge n character(s).
    static const char DECSET  = 'h';     // CSI ? n    h  — DECSET.
    static const char DECRST  = 'l';     // CSI ? n    l  — DECRST.
    static const char CSI_hRM = 'h';     // CSI n      h  — Reset mode (always Replace mode n=4).
    static const char CSI_lRM = 'l';     // CSI n      l  — Reset mode (always Replace mode n=4).
    static const char CSI_CCC = 'p';     // CSI n [; x1; x2; ...; xn ] p — Custom Caret Command.
    static const char W32_INP = '_';     // CSI EVENT_TYPEn [; x1; x2; ...; xn ] _ — win32-input-mode.

    static const char C0_NUL = '\x00'; // Null                - Originally used to allow gaps to be left on paper tape for edits. Later used for padding after a code that might take a terminal some time to process (e.g. a carriage return or line feed on a printing terminal). Now often used as a string terminator, especially in the programming language C.
    static const char C0_SOH = '\x01'; // Start of Heading    - First character of a message header. In Hadoop, it is often used as a field separator.
    static const char C0_STX = '\x02'; // Start of Text       - First character of message text, and may be used to terminate the message heading.
    static const char C0_ETX = '\x03'; // End of Text         - Often used as a "break" character (Ctrl-C) to interrupt or terminate a program or process.
    static const char C0_EOT = '\x04'; // End of Transmssn    - Often used on Unix to indicate end-of-file on a terminal.
    static const char C0_ENQ = '\x05'; // Enquiry             - Signal intended to trigger a response at the receiving end, to see if it is still present.
    static const char C0_ACK = '\x06'; // Acknowledge         - Response to an ENQ, or an indication of successful receipt of a message.
    static const char C0_BEL = '\x07'; // Bell, Alert     \a  - Originally used to sound a bell on the terminal. Later used for a beep on systems that didn't have a physical bell. May also quickly turn on and off inverse video (a visual bell).
    static const char C0_BS  = '\x08'; // Backspace       \b  - Move the caret one position leftwards. On input, this may delete the character to the left of the caret. On output, where in early computer technology a character once printed could not be erased, the backspace was sometimes used to generate accented characters in ASCII. For example, à could be produced using the three character sequence a BS ` (or, using the characters’ hex values, 0x61 0x08 0x60). This usage is now deprecated and generally not supported. To provide disambiguation between the two potential uses of backspace, the cancel character control code was made part of the standard C1 control set.
    static const char C0_HT  = '\x09'; // Character       \t  - Tabulation, Horizontal Tabulation	\t	Position to the next character tab stop.
    static const char C0_LF  = '\x0A'; // Line Feed       \n  - On typewriters, printers, and some terminal emulators, moves the caret down one row without affecting its column position. On Unix, used to mark end-of-line. In DOS, Windows, and various network standards, LF is used following CR as part of the end-of-line mark.
    static const char C0_VT  = '\x0B'; // Line Tab,VTab   \v  - Position the form at the next line tab stop.
    static const char C0_FF  = '\x0C'; // Form Feed       \f  - On printers, load the next page. Treated as whitespace in many programming languages, and may be used to separate logical divisions in code. In some terminal emulators, it clears the screen. It still appears in some common plain text files as a page break character, such as the RFCs published by IETF.
    static const char C0_CR  = '\x0D'; // Carriage Return \r  - Originally used to move the caret to column zero while staying on the same line. On classic Mac OS (pre-Mac OS X), as well as in earlier systems such as the Apple II and Commodore 64, used to mark end-of-line. In DOS, Windows, and various network standards, it is used preceding LF as part of the end-of-line mark. The Enter or Return key on a keyboard will send this character, but it may be converted to a different end-of-line sequence by a terminal program.
    static const char C0_SO  = '\x0E'; // Shift Out           - Switch to an alternative character set.
    static const char C0_SI  = '\x0F'; // Shift In            - Return to regular character set after Shift Out.
    static const char C0_DLE = '\x10'; // Data Link Escape    - Cause the following octets to be interpreted as raw data, not as control codes or graphic characters. Returning to normal usage would be implementation dependent.
    static const char C0_DC1 = '\x11'; // Device Control One (XON)    - These four control codes are reserved for device control, with the interpretation dependent upon the device to which they were connected.
    static const char C0_DC2 = '\x12'; // Device Control Two          > DC1 and DC2 were intended primarily to indicate activating a device while DC3 and DC4 were intended primarily to indicate pausing or turning off a device.
    static const char C0_DC3 = '\x13'; // Device Control Three (XOFF) > DC1 and DC3 (known also as XON and XOFF respectively in this usage) originated as the "start and stop remote paper-tape-reader" functions in ASCII Telex networks.
    static const char C0_DC4 = '\x14'; // Device Control Four         > This teleprinter usage became the de facto standard for software flow control.[6]
    static const char C0_NAK = '\x15'; // Negative Acknowldg  - Sent by a station as a negative response to the station with which the connection has been set up. In binary synchronous communication protocol, the NAK is used to indicate that an error was detected in the previously received block and that the receiver is ready to accept retransmission of that block. In multipoint systems, the NAK is used as the not-ready reply to a poll.
    static const char C0_SYN = '\x16'; // Synchronous Idle    - Used in synchronous transmission systems to provide a signal from which synchronous correction may be achieved between data terminal equipment, particularly when no other character is being transmitted.
    static const char C0_ETB = '\x17'; // End of Transmission Block  - Indicates the end of a transmission block of data when data are divided into such blocks for transmission purposes.
    static const char C0_CAN = '\x18'; // Cancel              - Indicates that the data preceding it are in error or are to be disregarded.
    static const char C0_EM  = '\x19'; // End of medium       - Intended as means of indicating on paper or magnetic tapes that the end of the usable portion of the tape had been reached.
    static const char C0_SUB = '\x1A'; // Substitute          - Originally intended for use as a transmission control character to indicate that garbled or invalid characters had been received. It has often been put to use for other purposes when the in-band signaling of errors it provides is unneeded, especially where robust methods of error detection and correction are used, or where errors are expected to be rare enough to make using the character for other purposes advisable. In DOS, Windows and other CP/M derivatives, it is used to indicate the end of file, both when typing on the terminal, and sometimes in text files stored on disk.
    static const char C0_ESC = '\x1B'; // Escape          \e  - The Esc key on the keyboard will cause this character to be sent on most systems. It can be used in software user interfaces to exit from a screen, menu, or mode, or in device-control protocols (e.g., printers and terminals) to signal that what follows is a special command sequence rather than normal text. In systems based on ISO/IEC 2022, even if another set of C0 control codes are used, this octet is required to always represent the escape character.
    static const char C0_FS  = '\x1C'; // File Separator      - Can be used as delimiters to mark fields of data structures. If used for hierarchical levels, US is the lowest level (dividing plain-text data items), while RS, GS, and FS are of increasing level to divide groups made up of items of the level beneath it.
    static const char C0_GS  = '\x1D'; // Group Separator.
    static const char C0_RS  = '\x1E'; // Record Separator.
    static const char C0_US  = '\x1F'; // Unit Separator.

    static const iota W32_START_EVENT = 10000; // for quick recognition.
    static const iota W32_KEYBD_EVENT = 10001;
    static const iota W32_MOUSE_EVENT = 10002;
    static const iota W32_WINSZ_EVENT = 10003;
    static const iota W32_FOCUS_EVENT = 10004;
    static const iota W32_FINAL_EVENT = 10005; // for quick recognition.

    static const auto OSC_LABEL_TITLE  = "0"   ; // Set icon label and title.
    static const auto OSC_LABEL        = "1"   ; // Set icon label.
    static const auto OSC_TITLE        = "2"   ; // Set title.
    static const auto OSC_XPROP        = "3"   ; // Set xprop.
    static const auto OSC_LINUX_COLOR  = "P"   ; // Set 16 colors palette. (Linux console)
    static const auto OSC_LINUX_RESET  = "R"   ; // Reset 16/256 colors palette. (Linux console)
    static const auto OSC_SET_PALETTE  = "4"   ; // Set 256 colors palette.
    static const auto OSC_SET_FGCOLOR  = "10"  ; // Set fg color.
    static const auto OSC_SET_BGCOLOR  = "11"  ; // Set bg color.
    static const auto OSC_RESET_COLOR  = "104" ; // Reset color N to default palette. Without params all palette reset.
    static const auto OSC_RESET_FGCLR  = "110" ; // Reset fg color to default.
    static const auto OSC_RESET_BGCLR  = "111" ; // Reset bg color to default.
    static const auto OSC_CLIPBRD      = "52"  ; // Set clipboard.
    static const auto OSC_TITLE_REPORT = "l"   ; // Get terminal window title.
    static const auto OSC_LABEL_REPORT = "L"   ; // Get terminal window icon label.

    static const iota SGR_RST       = 0;
    static const iota SGR_SAV       = 10;
    static const iota SGR_BOLD      = 1;
    static const iota SGR_FAINT     = 22;
    static const iota SGR_ITALIC    = 3;
    static const iota SGR_NONITALIC = 23;
    static const iota SGR_INV       = 7;
    static const iota SGR_NOINV     = 27;
    static const iota SGR_UND       = 4;
    static const iota SGR_DOUBLEUND = 21;
    static const iota SGR_NOUND     = 24;
    static const iota SGR_STRIKE    = 9;
    static const iota SGR_NOSTRIKE  = 29;
    static const iota SGR_OVERLN    = 53;
    static const iota SGR_NOOVERLN  = 55;
    static const iota SGR_FG_BLK    = 30;
    static const iota SGR_FG_RED    = 31;
    static const iota SGR_FG_GRN    = 32;
    static const iota SGR_FG_YLW    = 33;
    static const iota SGR_FG_BLU    = 34;
    static const iota SGR_FG_MGT    = 35;
    static const iota SGR_FG_CYN    = 36;
    static const iota SGR_FG_WHT    = 37;
    static const iota SGR_FG_RGB    = 38;
    static const iota SGR_FG        = 39;
    static const iota SGR_BG_BLK    = 40;
    static const iota SGR_BG_RED    = 41;
    static const iota SGR_BG_GRN    = 42;
    static const iota SGR_BG_YLW    = 43;
    static const iota SGR_BG_BLU    = 44;
    static const iota SGR_BG_MGT    = 45;
    static const iota SGR_BG_CYN    = 46;
    static const iota SGR_BG_WHT    = 47;
    static const iota SGR_BG_RGB    = 48;
    static const iota SGR_BG        = 49;
    static const iota SGR_FG_BLK_LT = 90;
    static const iota SGR_FG_RED_LT = 91;
    static const iota SGR_FG_GRN_LT = 92;
    static const iota SGR_FG_YLW_LT = 93;
    static const iota SGR_FG_BLU_LT = 94;
    static const iota SGR_FG_MGT_LT = 95;
    static const iota SGR_FG_CYN_LT = 96;
    static const iota SGR_FG_WHT_LT = 97;
    static const iota SGR_BG_BLK_LT = 100;
    static const iota SGR_BG_RED_LT = 101;
    static const iota SGR_BG_GRN_LT = 102;
    static const iota SGR_BG_YLW_LT = 103;
    static const iota SGR_BG_BLU_LT = 104;
    static const iota SGR_BG_MGT_LT = 105;
    static const iota SGR_BG_CYN_LT = 106;
    static const iota SGR_BG_WHT_LT = 107;

    static const iota CCC_NOP    = 0  ; // CSI             p  - no operation.
    static const iota CCC_RST    = 1  ; // CSI 1           p  - reset to zero all params (zz).
    static const iota CCC_CPP    = 2  ; // CSI 2 : x [: y] p  - caret percent position.
    static const iota CCC_CPX    = 3  ; // CSI 3 : x       p  - caret H percent position.
    static const iota CCC_CPY    = 4  ; // CSI 4 : y       p  - caret V percent position.
    static const iota CCC_TBS    = 5  ; // CSI 5 : n       p  - tab step length.
    static const iota CCC_MGN    = 6  ; // CSI 6 : l:r:t:b p  - margin left, right, top, bottom.
    static const iota CCC_MGL    = 7  ; // CSI 7 : n       p  - margin left   ╮
    static const iota CCC_MGR    = 8  ; // CSI 8 : n       p  - margin right  │ positive - native binding.
    static const iota CCC_MGT    = 9  ; // CSI 9 : n       p  - margin top    │ negative - oppisite binding.
    static const iota CCC_MGB    = 10 ; // CSI 10: n       p  - margin bottom ╯

    static const iota CCC_JET    = 11 ; // CSI 11: n       p  - text alignment (bias).
    static const iota CCC_WRP    = 12 ; // CSI 12: n       p  - text wrapping none/on/off.
    static const iota CCC_RTL    = 13 ; // CSI 13: n       p  - text right-to-left none/on/off.
    static const iota CCC_RLF    = 14 ; // CSI 14: n       p  - reverse line feed none/on/off.

    static const iota CCC_JET_or = 15 ; // CSI 15: n       p  - set text alignment (bias) if it is not set.
    static const iota CCC_WRP_or = 16 ; // CSI 16: n       p  - set text wrapping none/on/off if it is not set.
    static const iota CCC_RTL_or = 17 ; // CSI 17: n       p  - set text right-to-left none/on/off if it is not set.
    static const iota CCC_RLF_or = 18 ; // CSI 18: n       p  - set reverse line feed none/on/off if it is not set.

    static const iota CCC_IDX    = 19 ; // CSI 19: id      p  - Split the text run and associate the fragment with an id.
    static const iota CCC_CUP    = 20 ; // CSI 20: x [: y] p  - caret absolute position 0-based.
    static const iota CCC_CHX    = 21 ; // CSI 21: x       p  - caret H absolute position 0-based.
    static const iota CCC_CHY    = 22 ; // CSI 22: y       p  - caret V absolute position 0-based.
    static const iota CCC_REF    = 23 ; // CSI 23: id      p  - create the reference to the existing paragraph.
    static const iota CCC_SBS    = 24 ; // CSI 24: n: m    p  - define scrollback size: n: max size, m: grow_by step.
    static const iota CCC_EXT    = 25 ; // CSI 25: b       p  - extended functionality support.
    static const iota CCC_SMS    = 26 ; // CSI 26: b       p  - Should the mouse poiner to be drawn.
    static const iota CCC_KBD    = 27 ; // CSI 27: n       p  - Set keyboard modifiers.

    // ansi: Escaped sequences accumulator.
    class esc
        : public text
    {
        char  heap[32];
        char* tail = heap + sizeof(heap);

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
            auto size = text::size();
            resize(size + gain);
            memcpy(size + text::data(), cptr, gain);
        }

        template<class T>
        inline void fuse(T&& data)
        {
            using D = std::remove_cv_t<std::remove_reference_t<T>>;
            if constexpr (std::is_same_v<D, char>)
            {
                push_back(data);
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
                itos(static_cast<iota>(data));
            }
            else if constexpr (std::is_same_v<D, twod>)
            {
                operator+=("{ "); itos(data.x); operator+=(", ");
                                  itos(data.y); operator+=(" }");
            }
            else if constexpr (std::is_same_v<D, rect>)
            {
                operator+=("{"); fuse(data.coor); operator+=(",");
                                 fuse(data.size); operator+=("}");
            }
            else operator+=(std::forward<T>(data));
        }

    public:
        esc() = default;

        template<class T>
        esc(T&& data) { fuse(std::forward<T>(data)); }

        template<class T>
        inline auto& add(T&& data)
        {
            fuse(std::forward<T>(data));
            return *this;
        }
        template<class T, class ...Args>
        inline auto& add(T&& data, Args&&... data_list)
        {
            fuse(std::forward<T>(data));
            return add(std::forward<Args>(data_list)...);
        }

        esc& vmouse      (bool b)   { return add(b ? "\033[?1002;1003;1004;1006;10060h"
                                                   : "\033[?1002;1003;1004;1006;10060l"); } // esc: Focus and Mouse position reporting/tracking.
        esc& locate(iota x, iota y) { return add("\033[", y,     ';', x,     'H'       ); } // esc: 1-Based caret position.
        esc& locate(twod const& p)  { return add("\033[", p.y+1, ';', p.x+1, 'H'       ); } // esc: 0-Based caret position.
        esc& report(twod const& p)  { return add("\033[", p.y+1, ";", p.x+1, "R"       ); } // esc: Report 1-Based caret position (CPR).
        esc& locate_wipe ()         { return add("\033[r"                              ); } // esc: Enable scrolling for entire display (clear screen).
        esc& locate_call ()         { return add("\033[6n"                             ); } // esc: Report caret position.
        esc& scroll_wipe ()         { return add("\033[2J"                             ); } // esc: Erase scrollback.
        esc& tag         (view t)   { return add("\033]2;", t, '\07'                   ); } // esc: Window title.
        esc& setbuf      (view t)   { return add("\033]52;;", utf::base64(t), C0_BEL   ); } // esc: Set clipboard.
        esc& setutf      (bool b)   { return add(b ? "\033%G"      : "\033%@"          ); } // esc: Select UTF-8 character set (true) or default (faux).
        esc& altbuf      (bool b)   { return add(b ? "\033[?1049h" : "\033[?1049l"     ); } // esc: Alternative buffer.
        esc& cursor      (bool b)   { return add(b ? "\033[?25h"   : "\033[?25l"       ); } // esc: Caret visibility.
        esc& appkey      (bool b)   { return add(b ? "\033[?1h"    : "\033[?1l"        ); } // ansi: Application(=on)/ANSI(=off) Caret Keys (DECCKM).
        esc& bpmode      (bool b)   { return add(b ? "\033[?2004h" : "\033[?2004l"     ); } // esc: Set bracketed paste mode.
        esc& autowr      (bool b)   { return add(b ? "\033[?7h"    : "\033[?7l"        ); } // esc: Set autowrap mode.
        esc& scrn_reset  ()         { return add("\033[H\033[m\033[2J"                 ); } // esc: Reset palette, erase scrollback and reset caret location.
        esc& save_title  ()         { return add("\033[22;0t"                          ); } // esc: Save terminal window title.
        esc& load_title  ()         { return add("\033[23;0t"                          ); } // esc: Restore terminal window title.
        esc& save_palette()         { return add("\033[#P"                             ); } // esc: Push palette onto stack XTPUSHCOLORS.
        esc& load_palette()         { return add("\033[#Q"                             ); } // esc: Pop  palette from stack XTPOPCOLORS.
        esc& osc_palette (iota i, rgba const& c) // esc: Set color palette. ESC ] 4 ; <i> ; rgb : <r> / <g> / <b> BEL.
        {
            return add("\033]4;", i, ";rgb:", utf::to_hex(c.chan.r), '/',
                                              utf::to_hex(c.chan.g), '/',
                                              utf::to_hex(c.chan.b), '\07');
        }
        esc& osc_palette_reset() // esc: Reset color palette.
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
        esc& old_palette_reset() { return add("\033]R"); } // esc: Reset color palette (Linux console).
        esc& old_palette(iota i, rgba const& c) // esc: Set color palette (Linux console).
        {
            return add("\033]P", utf::to_hex(i, 1), utf::to_hex(c.chan.r, 2),
                                                    utf::to_hex(c.chan.g, 2),
                                                    utf::to_hex(c.chan.b, 2), '\033');
        }

        esc& w32input(bool b) { return add(b ? "\033[?9001h" : "\033[?9001l"); } // ansi: Application Caret Keys (DECCKM).
        esc& w32begin()       { return add("\033["                          ); }
        esc& w32close()
        {
            if (back() == ';') pop_back();
            return add(W32_INP);
        }
        // ansi: win32-input-mode sequence (keyboard).
        esc& w32keybd(iota id, iota kc, iota sc, iota kd, iota ks, iota rc, iota uc)
        {
            add(ansi::W32_KEYBD_EVENT, ':');
            if (id) fuse(id);
            return add(':', kc, ':',
                            sc, ':',
                            kd, ':',
                            ks, ':',
                            rc, ':',
                            uc, ';');
        }
        // ansi: win32-input-mode sequence (mouse).
        esc& w32mouse(iota id, iota bttns, iota ctrls, iota flags, iota wheel, iota xcoor, iota ycoor)
        {
            add(ansi::W32_MOUSE_EVENT, ':');
            if (id) fuse(id);
            return add(':', bttns, ':',
                            ctrls, ':',
                            flags, ':',
                            wheel, ':',
                            xcoor, ':',
                            ycoor, ';');
        }
        // ansi: win32-input-mode sequence (focus).
        esc& w32focus(iota id, iota focus)
        {
            add(ansi::W32_FOCUS_EVENT, ':');
            if (id) fuse(id);
            return add(':', focus, ';');
        }
        // ansi: win32-input-mode sequence (window resize).
        esc& w32winsz(twod size)
        {
            return add(ansi::W32_WINSZ_EVENT, ':', size.x, ':',
                                                   size.y, ';');
        }

        esc& cup(twod const& p) { return add("\033[20:", p.y, ':',
                                                         p.x, CSI_CCC  ); } // esc: 0-Based caret position.
        esc& cuu(iota n)        { return add("\033[", n, 'A'           ); } // esc: Caret up.
        esc& cud(iota n)        { return add("\033[", n, 'B'           ); } // esc: Caret down.
        esc& cuf(iota n)        { return add("\033[", n, 'C'           ); } // esc: Caret forward.
        esc& cub(iota n)        { return add("\033[", n, 'D'           ); } // esc: Caret backward.
        esc& cnl(iota n)        { return add("\033[", n, 'E'           ); } // esc: caret next line.
        esc& cpl(iota n)        { return add("\033[", n, 'F'           ); } // esc: Caret previous line.
        esc& ocx(iota n)        { return add("\033[", n, 'G'           ); } // esc: Caret 1-based horizontal absolute.
        esc& ocy(iota n)        { return add("\033[", n, 'd'           ); } // esc: Caret 1-based vertical absolute.
        esc& chx(iota n)        { return add("\033[21:", n, CSI_CCC    ); } // esc: Caret 0-based horizontal absolute.
        esc& chy(iota n)        { return add("\033[22:", n, CSI_CCC    ); } // esc: Caret 0-based vertical absolute.
        esc& scp()              { return add("\033[s"                  ); } // esc: Save caret position in memory.
        esc& rcp()              { return add("\033[u"                  ); } // esc: Restore caret position from memory.
        esc& bld(bool b = true) { return add(b ? "\033[1m" : "\033[22m"); } // esc: SGR 𝗕𝗼𝗹𝗱 attribute.
        esc& und(bool b = true) { return add(b ? "\033[4m" : "\033[24m"); } // esc: SGR 𝗨𝗻𝗱𝗲𝗿𝗹𝗶𝗻𝗲 attribute.
        esc& inv(bool b = true) { return add(b ? "\033[7m" : "\033[27m"); } // esc: SGR 𝗡𝗲𝗴𝗮𝘁𝗶𝘃𝗲 attribute.
        esc& itc(bool b = true) { return add(b ? "\033[3m" : "\033[23m"); } // esc: SGR 𝑰𝒕𝒂𝒍𝒊𝒄 attribute.
        esc& stk(bool b = true) { return add(b ? "\033[9m" : "\033[29m"); } // esc: SGR Strikethrough attribute.
        esc& dnl(bool b = true) { return add(b ? "\033[21m": "\033[24m"); } // esc: SGR Double underline attribute.
        esc& ovr(bool b = true) { return add(b ? "\033[53m": "\033[55m"); } // esc: SGR Overline attribute.
        esc& fgc()              { return add("\033[39m"                ); } // esc: Set default foreground color.
        esc& bgc()              { return add("\033[49m"                ); } // esc: Set default background color.

        // Colon-separated variant.
        esc& fgc4(rgba const& c) { return add("\033[38:2:", c.chan.r, ':',  // esc: SGR Foreground color. RGB: red, green, blue and alpha.
                                                            c.chan.g, ':',
                                                            c.chan.b, ':',
                                                            c.chan.a, 'm'); }
        esc& bgc4(rgba const& c) { return add("\033[48:2:", c.chan.r, ':',  // esc: SGR Background color. RGB: red, green, blue and alpha.
                                                            c.chan.g, ':',
                                                            c.chan.b, ':',
                                                            c.chan.a, 'm'); }
        // esc: SGR Foreground color (256-color mode).
        esc& fgc256(rgba const& c)
        {
            return add("\033[38;5;", c.to256cube(), 'm');
        }
        // esc: SGR Background color (256-color mode).
        esc& bgc256(rgba const& c)
        {
            return add("\033[48;5;", c.to256cube(), 'm');
        }
        // esc: SGR Foreground color (16-color mode).
        esc& fgc16(rgba const& c)
        {
            iota clr = 30;
            switch (c.token)
            {
                case 0xFF000000: clr += 0;
                    return add("\033[22;", clr, 'm');
                case 0xFFffffff: clr += 5;
                    return add("\033[22;", clr, 'm');
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
                case rgba{ rgba::color256[tint::redlt    ] }.token:
                    clr += 6;
                    return add("\033[22;", clr, 'm');
                case rgba{ rgba::color256[tint::blacklt  ] }.token:
                    clr += 4;
                    return add("\033[22;", clr, 'm');
                case 0xFFff0000:
                case rgba{ rgba::color256[tint::bluelt   ] }.token:
                    clr += 7;
                    return add("\033[22;", clr, 'm');
                default: // grayscale
                    auto l = c.luma();
                    if      (l < 42)  clr += 1;
                    else if (l < 90)  clr += 2;
                    else if (l < 170) clr += 3;
                    else if (l < 240) clr += 4;
                    else              clr += 5;
                    return add("\033[22;", clr, 'm');
            }
            return add("\033[", clr, 'm');
        }
        // esc: SGR Background color (16-color mode).
        esc& bgc16(rgba const& c)
        {
            iota clr = 40;
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
                        if      (l < 42)  clr += 1;
                        else if (l < 90)  clr += 2;
                        else if (l < 170) clr += 3;
                        else if (l < 240) clr += 4;
                        else              clr += 5;
                    }
            }
            return add("\033[", clr, 'm');
        }
        // esc: SGR Foreground color. RGB: red, green, blue.
        template<svga VGAMODE = svga::truecolor>
        esc& fgc(rgba const& c)
        {
            switch (VGAMODE)
            {
                case svga::truecolor:
                    return add("\033[38;2;", c.chan.r, ';',
                                             c.chan.g, ';',
                                             c.chan.b, 'm');
                case svga::vga16:  return fgc16 (c);
                case svga::vga256: return fgc256(c);
                default:           return *this;
            }
        }
        // esc: SGR Background color. RGB: red, green, blue.
        template<svga VGAMODE = svga::truecolor>
        esc& bgc(rgba const& c)
        {
            switch (VGAMODE)
            {
                case svga::truecolor:
                    return add("\033[48;2;", c.chan.r, ';',
                                             c.chan.g, ';',
                                             c.chan.b, 'm');
                case svga::vga16:  return bgc16 (c);
                case svga::vga256: return bgc256(c);
                default:           return *this;
            }
        }
        esc& sav ()              { return add("\033[10m"              ); } // esc: Save SGR attributes.
        esc& nil ()              { return add("\033[m"                ); } // esc: Reset SGR attributes to zero.
        esc& nop ()              { return add("\033["   ,      CSI_CCC); } // esc: No operation. Split the text run.
        esc& rst ()              { return add("\033[1"  ,      CSI_CCC); } // esc: Reset formatting parameters.
        esc& cpp (twod const& p) { return add("\033[2:" , p.x, ':',        // esc: Caret percent position.
                                                          p.y, CSI_CCC); }
        esc& cpx (iota n)        { return add("\033[3:" , n  , CSI_CCC); } // esc: Caret horizontal percent position.
        esc& cpy (iota n)        { return add("\033[4:" , n  , CSI_CCC); } // esc: Caret vertical percent position.
        esc& tbs (iota n)        { return add("\033[5:" , n  , CSI_CCC); } // esc: Tabulation step length.
        esc& mgn (side const& n) { return add("\033[6:" , n.l, ':',        // esc: Margin (left, right, top, bottom).
                                                          n.r, ':',
                                                          n.t, ':',
                                                          n.b, CSI_CCC); }
        esc& mgl (iota n)        { return add("\033[7:" , n  , CSI_CCC); } // esc: Left margin. Positive - native binding. Negative - opposite binding.
        esc& mgr (iota n)        { return add("\033[8:" , n  , CSI_CCC); } // esc: Right margin. Positive - native binding. Negative - opposite binding.
        esc& mgt (iota n)        { return add("\033[9:" , n  , CSI_CCC); } // esc: Top margin. Positive - native binding. Negative - opposite binding.
        esc& mgb (iota n)        { return add("\033[10:", n  , CSI_CCC); } // esc: Bottom margin. Positive - native binding. Negative - opposite binding.
        esc& jet (bias n)        { return add("\033[11:", n  , CSI_CCC); } // esc: Text alignment.
        esc& wrp (wrap n)        { return add("\033[12:", n  , CSI_CCC); } // esc: Text wrapping.
        esc& rtl (rtol n)        { return add("\033[13:", n  , CSI_CCC); } // esc: Text right-to-left.
        esc& rlf (feed n)        { return add("\033[14:", n  , CSI_CCC); } // esc: Reverse line feed.
        esc& jet_or (bias n)     { return add("\033[15:", n  , CSI_CCC); } // esc: Text alignment.
        esc& wrp_or (wrap n)     { return add("\033[16:", n  , CSI_CCC); } // esc: Text wrapping.
        esc& rtl_or (rtol n)     { return add("\033[17:", n  , CSI_CCC); } // esc: Text right-to-left.
        esc& rlf_or (feed n)     { return add("\033[18:", n  , CSI_CCC); } // esc: Reverse line feed.
        esc& idx (iota i)        { return add("\033[19:", i  , CSI_CCC); } // esc: Split the text run and associate the fragment with an id.
        esc& ref (iota i)        { return add("\033[23:", i  , CSI_CCC); } // esc: Create the reference to the existing paragraph.
        esc& ext (iota b)        { return add("\033[25:", b  , CSI_CCC); } // esc: Extended functionality support, 0 - faux, 1 - true.
        esc& show_mouse (iota b) { return add("\033[26:", b  , CSI_CCC); } // esc: Should the mouse poiner to be drawn.
        esc& meta_state (iota m) { return add("\033[27:", m  , CSI_CCC); } // esc: Set keyboard meta modifiers (Ctrl, Shift, Alt, etc).
        //todo unify
        //esc& win (twod const& p){ return add("\033[20:", p.x, ':',              // esc: Terminal window resize report.
        //                                                 p.y, CSI_CCC); }
        esc& win (twod const& p) { return add("\033]", p.x, ';',           // esc: Terminal window resize report.
                                                       p.y, 'w'       ); }
        esc& fcs (bool b)        { return add("\033[", b ? 'I' : 'O'  ); } // ansi: Terminal window focus.
        esc& eol ()              { return add("\n"                    ); } // esc: EOL.
        esc& edl ()              { return add("\033[K"                ); } // esc: EDL.

        esc& mouse_sgr(iota ctrl, twod const& coor, bool ispressed) // esc: Mouse tracking report (SGR).
        {
            return add("\033[<", ctrl, ';',
                           coor.x + 1, ';',
                           coor.y + 1, ispressed ? 'M' : 'm');
        }
        esc& mouse_x11(iota ctrl, twod const& coor) // esc: Mouse tracking report (X11).
        {
            return add("\033[M", static_cast<char>(std::clamp(ctrl,       0, 255-32) + 32),
                                 static_cast<char>(std::clamp(coor.x + 1, 1, 255-32) + 32),
                                 static_cast<char>(std::clamp(coor.y + 1, 1, 255-32) + 32));
        }
        esc& osc(text const& cmd, text const& param) // esc: OSC report.
        {
            return add(ESCOCS, cmd, ';', param, C0_BEL);
        }
    };

    static esc vmouse (bool b)       { return esc{}.vmouse(b);     } // ansi: Mouse position reporting/tracking.
    static esc locate(twod const& n) { return esc{}.locate(n);     } // ansi: 1-Based caret position.
    static esc locate_wipe ()        { return esc{}.locate_wipe(); } // ansi: Enable scrolling for entire display (clear screen).
    static esc locate_call ()        { return esc{}.locate_call(); } // ansi: Report caret position.
    static esc setutf (bool b)       { return esc{}.setutf(b);     } // ansi: Select UTF-8 character set.
    static esc tag (view t)          { return esc{}.tag(t);        } // ansi: Window title.
    static esc altbuf (bool b)       { return esc{}.altbuf(b);     } // ansi: Alternative buffer.
    static esc cursor (bool b)       { return esc{}.cursor(b);     } // ansi: Caret visibility.
    static esc appkey (bool b)       { return esc{}.appkey(b);     } // ansi: Application Caret Keys (DECCKM).
    static esc setbuf (view t)       { return esc{}.setbuf(t);     } // ansi: Set clipboard.

    static esc w32input (bool b)     { return esc{}.w32input(b); } // ansi: Turn on w32-input-mode (Microsoft specific, not released yet).
    template<class ...Args> static esc w32keybd (Args&&... p){ return esc{}.w32keybd(std::forward<Args>(p)...); } // ansi: win32-input-mode sequence (keyboard).
    template<class ...Args> static esc w32mouse (Args&&... p){ return esc{}.w32mouse(std::forward<Args>(p)...); } // ansi: win32-input-mode sequence (mouse).
    template<class ...Args> static esc w32focus (Args&&... p){ return esc{}.w32focus(std::forward<Args>(p)...); } // ansi: win32-input-mode sequence (focus).
    template<class ...Args> static esc w32winsz (Args&&... p){ return esc{}.w32winsz(std::forward<Args>(p)...); } // ansi: win32-input-mode sequence (window resize).

    static esc cup (twod const& n)   { return esc{}.cup (n); } // ansi: 0-Based caret position.
    static esc cuu (iota n)          { return esc{}.cuu (n); } // ansi: Caret up.
    static esc cud (iota n)          { return esc{}.cud (n); } // ansi: Caret down.
    static esc cuf (iota n)          { return esc{}.cuf (n); } // ansi: Caret forward.
    static esc cub (iota n)          { return esc{}.cub (n); } // ansi: Caret backward.
    static esc cnl (iota n)          { return esc{}.cnl (n); } // ansi: Caret next line.
    static esc cpl (iota n)          { return esc{}.cpl (n); } // ansi: Caret previous line.

    static esc ocx (iota n)          { return esc{}.ocx (n); } // ansi: Caret 1-based horizontal absolute.
    static esc ocy (iota n)          { return esc{}.ocy (n); } // ansi: Caret 1-based vertical absolute.

    static esc chx (iota n)          { return esc{}.chx (n); } // ansi: Caret 0-based horizontal absolute.
    static esc chy (iota n)          { return esc{}.chy (n); } // ansi: Caret 0-based vertical absolute.

    static esc bld (bool b = true)   { return esc{}.bld (b); } // ansi: SGR 𝗕𝗼𝗹𝗱 attribute.
    static esc und (bool b = true)   { return esc{}.und (b); } // ansi: SGR 𝗨𝗻𝗱𝗲𝗿𝗹𝗶𝗻𝗲 attribute.
    static esc inv (bool b = true)   { return esc{}.inv (b); } // ansi: SGR 𝗡𝗲𝗴𝗮𝘁𝗶𝘃𝗲 attribute.
    static esc itc (bool b = true)   { return esc{}.itc (b); } // ansi: SGR 𝑰𝒕𝒂𝒍𝒊𝒄 attribute.
    static esc stk (bool b = true)   { return esc{}.stk (b); } // ansi: SGR Strikethrough attribute.
    static esc dnl (bool b = true)   { return esc{}.dnl (b); } // ansi: SGR Double underline attribute.
    static esc ovr (bool b = true)   { return esc{}.ovr (b); } // ansi: SGR Overline attribute.

    template<class ...Args>
    static esc add (Args&&... data)  { return esc{}.add (std::forward<Args>(data)...); } // ansi: Add text.
    static esc fgc ()                { return esc{}.fgc ( ); } // ansi: Set default foreground color.
    static esc bgc ()                { return esc{}.bgc ( ); } // ansi: Set default background color.
    static esc fgc (rgba const& n)   { return esc{}.fgc (n); } // ansi: SGR Foreground color.
    static esc bgc (rgba const& n)   { return esc{}.bgc (n); } // ansi: SGR Background color.
    static esc fgc4(rgba const& n)   { return esc{}.fgc4(n); } // ansi: SGR Foreground color with alpha.
    static esc bgc4(rgba const& n)   { return esc{}.bgc4(n); } // ansi: SGR Background color with alpha.
    static esc sav ()                { return esc{}.sav ( ); } // ansi: Save SGR attributes.
    static esc nil ()                { return esc{}.nil ( ); } // ansi: Reset (restore) SGR attributes.
    static esc scp ()                { return esc{}.scp ( ); } // ansi: Save caret position in memory.
    static esc rcp ()                { return esc{}.rcp ( ); } // ansi: Restore caret position from memory.
    static esc cpp (twod const& n)   { return esc{}.cpp (n); } // ansi: Caret percent position.
    static esc cpx (iota n)          { return esc{}.cpx (n); } // ansi: Caret horizontal percent position.
    static esc cpy (iota n)          { return esc{}.cpy (n); } // ansi: Caret vertical percent position.
    static esc tbs (iota n)          { return esc{}.tbs (n); } // ansi: Tabulation step length.
    static esc mgn (side const& n)   { return esc{}.mgn (n); } // ansi: Margin (left, right, top, bottom).
    static esc mgl (iota n)          { return esc{}.mgl (n); } // ansi: Left margin.
    static esc mgr (iota n)          { return esc{}.mgr (n); } // ansi: Right margin.
    static esc mgt (iota n)          { return esc{}.mgt (n); } // ansi: Top margin.
    static esc mgb (iota n)          { return esc{}.mgb (n); } // ansi: Bottom margin.
    static esc ext (bool b)          { return esc{}.ext (b); } // ansi: Extended functionality.
    static esc show_mouse(bool b)    { return esc{}.show_mouse(b); } // esc: Should the mouse poiner to be drawn.

    static esc jet (bias n)          { return esc{}.jet (n); } // ansi: Text alignment.
    static esc wrp (wrap n)          { return esc{}.wrp (n); } // ansi: Text wrapping.
    static esc rtl (rtol n)          { return esc{}.rtl (n); } // ansi: Text right-to-left.
    static esc rlf (feed n)          { return esc{}.rlf (n); } // ansi: Reverse line feed.
    static esc jet_or (bias n)       { return esc{}.jet_or (n); } // ansi: Set text alignment if it is not set.
    static esc wrp_or (wrap n)       { return esc{}.wrp_or (n); } // ansi: Set text wrapping if it is not set.
    static esc rtl_or (rtol n)       { return esc{}.rtl_or (n); } // ansi: Set text right-to-left if it is not set.
    static esc rlf_or (feed n)       { return esc{}.rlf_or (n); } // ansi: Set reverse line feed if it is not set.

    static esc rst ()                { return esc{}.rst ( ); } // ansi: Reset formatting parameters.
    static esc nop ()                { return esc{}.nop ( ); } // ansi: No operation. Split the text run.
    //ansi: Split the text run and associate the fragment with an id.
    //      All following text is under the IDX until the next command is issued.
    //      Redefine if the id already exists.
    static esc win (twod const& p)   { return esc{}.win (p); } // ansi: Terminal window resize.
    static esc fcs (bool b)          { return esc{}.fcs (b); } // ansi: Terminal window focus.
    static esc idx (iota i)          { return esc{}.idx (i); }
    static esc ref (iota i)          { return esc{}.ref (i); } // ansi: Create the reference to the existing paragraph. Create new id if it is not existing.
    static esc eol ()                { return esc{}.eol ( ); } // ansi: EOL.
    static esc edl ()                { return esc{}.edl ( ); } // ansi: EDL.

    // ansi: Caret forwarding instructions.
    // The order is important (see the richtext::flow::exec constexpr).

    // todo tie with richtext::flow::exec
    enum fn : iota
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
        iota cmd;
        iota arg;
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
    #pragma pack(push,1)
    union deco
    {
        enum type : iota
        {
            leftside, // default
            rghtside,
            centered,
            autowrap,
            count,
        };

        static constexpr auto defwrp = wrap::on; // deco: Default autowrap behavior.
        static constexpr iota maxtab = 255; // deco: Tab length limit.
        struct
        {
            wrap wrapln : 2; // deco: Autowrap.
            bias adjust : 2; // deco: Horizontal alignment.
            rtol r_to_l : 2; // deco: RTL.
            feed rlfeed : 2; // deco: Reverse line feed.
            byte tablen : 8; // deco: Tab length.
            dent margin;     // deco: Page margins.
        } v;
        ui64 token;

        friend    auto operator!= (deco const& a, deco const& b)
                                                  { return a.token != b.token; }
        constexpr deco()                          : token { 0 }              { }
        constexpr deco            (deco const& d) : token { d.token }        { }
        constexpr void operator = (deco const& d) { token = d.token;           }
        constexpr deco(iota)
            : v{ .wrapln = deco::defwrp,
                 .adjust = bias::left,
                 .r_to_l = rtol::ltr,
                 .rlfeed = feed::fwd,
                 .tablen = 8,
                 .margin = {} }
        { }

        auto  wrp   () const  { return v.wrapln;                                        } // deco: Return Auto wrapping.
        auto  jet   () const  { return v.adjust;                                        } // deco: Return Paragraph adjustment.
        auto  rtl   () const  { return v.r_to_l;                                        } // deco: Return RTL.
        auto  rlf   () const  { return v.rlfeed;                                        } // deco: Return Reverse line feed.
        auto  tbs   () const  { return v.tablen;                                        } // deco: Return Reverse line feed.
        auto& mgn   () const  { return v.margin;                                        } // deco: Return margins.
        auto& wrp   (bool  b) { v.wrapln = b ? wrap::on  : wrap::off;     return *this; } // deco: Set auto wrapping.
        auto& rtl   (bool  b) { v.r_to_l = b ? rtol::rtl : rtol::ltr;     return *this; } // deco: Set RTL.
        auto& rlf   (bool  b) { v.rlfeed = b ? feed::rev : feed::fwd;     return *this; } // deco: Set revverse line feed.
        auto& wrp   (wrap  n) { v.wrapln = n;                             return *this; } // deco: Auto wrapping.
        auto& jet   (bias  n) { v.adjust = n;                             return *this; } // deco: Paragraph adjustment.
        auto& rtl   (rtol  n) { v.r_to_l = n;                             return *this; } // deco: RTL.
        auto& rlf   (feed  n) { v.rlfeed = n;                             return *this; } // deco: Reverse line feed.
        auto& wrp_or(wrap  n) { if (v.wrapln == wrap::none) v.wrapln = n; return *this; } // deco: Auto wrapping.
        auto& jet_or(bias  n) { if (v.adjust == bias::none) v.adjust = n; return *this; } // deco: Paragraph adjustment.
        auto& rtl_or(rtol  n) { if (v.r_to_l == rtol::none) v.r_to_l = n; return *this; } // deco: RTL.
        auto& rlf_or(feed  n) { if (v.rlfeed == feed::none) v.rlfeed = n; return *this; } // deco: Reverse line feed.
        auto& tbs   (iota  n) { v.tablen = std::min(n, maxtab);           return *this; } // deco: fx_ccc_tbs.
        auto& mgl   (iota  n) { v.margin.west.step = n;                   return *this; } // deco: fx_ccc_mgl.
        auto& mgr   (iota  n) { v.margin.east.step = n;                   return *this; } // deco: fx_ccc_mgr.
        auto& mgt   (iota  n) { v.margin.head.step = n;                   return *this; } // deco: fx_ccc_mgt.
        auto& mgb   (iota  n) { v.margin.foot.step = n;                   return *this; } // deco: fx_ccc_mgb.
        auto& mgn   (fifo& q) { v.margin.set(q);                          return *this; } // deco: fx_ccc_mgn.
        auto& rst   ()        { token = 0;                                return *this; } // deco: Reset.
        constexpr auto& glb() { operator=(deco(0));                       return *this; }  // deco: Reset to default.
        auto get_kind() const
        {
            return v.wrapln == wrap::on    ? type::autowrap :
                   v.adjust == bias::left  ? type::leftside :
                   v.adjust == bias::right ? type::rghtside :
                                             type::centered ;
        }
    };
    #pragma pack(pop)

    static constexpr auto def_style = deco(0);

    struct runtime
    {
        bool iswrapln;
        bool isr_to_l;
        bool isrlfeed;
        bool straight; // runtime: Text substring retrieving direction.
        bool centered;
        bool arighted;
        iota tabwidth;
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
    using func = netxs::generics::tree <Q, C*, std::function<void(Q&, C*&)>>;

    template<class T>
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
            * - void data(iota count, grid const& proto);  // Proceed new cells.
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
            * - void und(bool b);                    // Set underline attribute.
            * - void dnl(bool b);                    // Set double underline attribute.
            * - void ovr(bool b);                    // Set overline attribute.
            * - void wrp(bool b);                    // Set auto wrap.
            * - void jet(iota b);                    // Set adjustment.
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
                csi_ccc.enable_multi_arg();
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
                    csi_ccc[CCC_EXT] = nullptr;
                    csi_ccc[CCC_SMS] = nullptr;
                    csi_ccc[CCC_KBD] = nullptr;

                auto& csi_sgr = table[CSI_SGR].resize(0x100);
                csi_sgr.enable_multi_arg();
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
                    csi_sgr[SGR_UND      ] = VT_PROC{ p->brush.und(true); }; // fx_sgr_und;
                    csi_sgr[SGR_DOUBLEUND] = VT_PROC{ p->brush.dnl(true); }; // fx_sgr_dnl;
                    csi_sgr[SGR_NOUND    ] = VT_PROC{ p->brush.und(faux); }; // fx_sgr_und;
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

        void proceed(iota cmd, T*& client)  { table.execute(cmd, client); }
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
                esc[CSI   ] = xcsi;
                esc[OCS   ] = xosc;
                esc[KEY_A ] = keym;
                esc[KEY_N ] = keym;
                esc[G0SET ] = g0__;
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
            using fifo = netxs::generics::bank <iota, maxarg>;

            if (ascii.length())
            {
                auto b = '\0';
                auto ints = []  (unsigned char cmd) { return cmd >= 0x20 && cmd <= 0x2f; }; // "intermediate bytes" in the range 0x20–0x2F
                auto pars = []  (unsigned char cmd) { return cmd >= 0x3C && cmd <= 0x3f; }; // "parameter bytes" in the range 0x30–0x3F
                auto cmds = []  (unsigned char cmd) { return cmd >= 0x40 && cmd <= 0x7E; };
                auto fill = [&] (auto& queue)
                {
                    auto a = ';';
                    auto push = [&](auto num)
                    {	// Parse subparameters divided by colon ':' (max arg value<int32_t> is 1,073,741,823)
                        if (a == ':') queue.template push<true>(num);
                        else          queue.template push<faux>(num);
                    };

                    while (ascii.length())
                    {
                        if (auto param = utf::to_int(ascii))
                        {
                            push(param.value());
                            if  (ascii.empty()) break;
                        }
                        else push(0); // default zero parameter expressed by the standalone delimiter/semicolon.

                        a = ascii.front(); // delimiter or cmd after number
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
                    fifo queue{ CCC_NOP }; // Reserve for the command type.
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
            // n: iota
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
                    text cmd = OSC_LINUX_COLOR;
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
                    text cmd = OSC_LINUX_RESET;
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
                    text cmd(base, delm);
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
        //todo use C++20 requires expressions
        template <class A>
        struct has
        {
            template <class B> static int16_t _(decltype(&B::template parser_config<ansi::vt_parser<B>>));
            template <class B> static uint8_t _(...);
            static constexpr bool parser_config = sizeof(_<A>(nullptr)) - 1;
        };

    public:
        deco style{}; // parser: Parser style.
        deco state{}; // parser: Parser style last state.
        grid proto{}; // parser: Proto lyric.
        iota count{}; // parser: Proto lyric length.
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
                if constexpr (has<T>::parser_config) T::parser_config(*this);
            }
        };

        void post(utf::frag const& cluster)
        {
            static ansi::marker marker;

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
        inline void flush()
        {
            if (state != style)
            {
                meta(state);
                state = style;
            }
            if (count)
            {
                data(count, proto);
                proto.clear();
                count = 0;
            }
        }
        virtual void meta(deco const& old_style)         { };
        virtual void data(iota count, grid const& proto) { };
    };

    // ansi: Caret manipulation command list.
    struct writ
        : public std::list<ansi::rule>
    {
        using list = std::list<ansi::rule>;

        inline void  push(rule const& cmd)    { list::push_back(cmd); } // Append single command to the locus.
        inline void   pop()                   { list::pop_back();     } // Append single command to the locus.
        inline bool  bare()    const          { return list::empty(); } // Is it empty the list of commands?
        inline writ& kill()    { list::clear();         return *this; } // Clear command list.

        writ& rst ()           { push({ fn::zz, 0   }); return *this; } // Reset formatting parameters. Do not clear the command list.
        writ& cpp (twod p)     { push({ fn::px, p.x });                 // Caret percent position.
                                 push({ fn::py, p.y }); return *this; }
        writ& cpx (iota x)     { push({ fn::px, x   }); return *this; } // Caret horizontal percent position.
        writ& cpy (iota y)     { push({ fn::py, y   }); return *this; } // Caret vertical percent position.
        writ& cup (twod p)     { push({ fn::ay, p.y });                 // 0-Based caret position.
                                 push({ fn::ax, p.x }); return *this; }
        writ& cuu (iota n = 1) { push({ fn::dy,-n   }); return *this; } // Caret up.
        writ& cud (iota n = 1) { push({ fn::dy, n   }); return *this; } // Caret down.
        writ& cuf (iota n = 1) { push({ fn::dx, n   }); return *this; } // Caret forward.
        writ& cub (iota n = 1) { push({ fn::dx,-n   }); return *this; } // Caret backward.
        writ& cnl (iota n = 1) { push({ fn::nl, n   }); return *this; } // Caret next line.
        writ& cpl (iota n = 1) { push({ fn::nl,-n   }); return *this; } // Caret previous line.
        writ& chx (iota x)     { push({ fn::ax, x   }); return *this; } // Caret o-based horizontal absolute.
        writ& chy (iota y)     { push({ fn::ay, y   }); return *this; } // Caret o-based vertical absolute.
        writ& scp ()           { push({ fn::sc, 0   }); return *this; } // Save caret position in memory.
        writ& rcp ()           { push({ fn::rc, 0   }); return *this; } // Restore caret position from memory.
    };

    // ansi: Checking ANSI/UTF-8 integrity and return a valid view.
    template<class TEXT_OR_VIEW>
    auto purify(TEXT_OR_VIEW&& utf8)
    {
        /*
        - Occurrences of characters 00-1F or 7F-FF in an escape sequence
          or control sequence is an error condition whose recovery is not specified.

        - For control sequences, the maximum length of parameter string
          is defined by implementation.

        - For control sequences, occurrences of a parameter character
          after an intermediate character is an error condition.

        */

        view crop{ std::forward<TEXT_OR_VIEW>(utf8) };

        // check ansi integrity
        if (auto size = crop.size())
        {
            //todo unify the BEL searching

            // find ESC \x1b
            while (size && (crop[--size] != 0x1b))
            { }

            auto start = size;
            if (crop[start] == 0x1b)
            {
                start++;

                // test single byte after ESC is good: ESC x
                if (start < crop.size())
                {
                    auto c = crop[start];
                    // test CSI: ESC [ pn;...;pn cmd
                    if (c == '[')
                    {
                        // find CSI command: cmd >= 0x40 && cmd <= 0x7E;
                        while (++start < crop.size())
                        {
                            auto cmd = crop[start];
                            if (cmd >= 0x40 && cmd <= 0x7E) break;
                        }

                        if (start == crop.size())
                        {
                            crop = crop.substr(0, size);
                            return crop;
                        }
                    }
                    // test OSC: ESC ] ... BEL
                    else if (c == ']')
                    {
                        // test OSC: ESC ] P Nrrggbb
                        auto step = start + 1;
                        if (step < crop.size())
                        {
                            auto c = crop[step];
                            if(c == 'P') // Set linux console palette.
                            {
                                if (crop.size() < step + 8)
                                {
                                    crop = crop.substr(0, size);
                                }
                                else
                                {
                                    utf::purify(crop);
                                }
                                return crop;
                            }
                            else if(c == 'R') // Reset linux console palette.
                            {
                                utf::purify(crop);
                                return crop;
                            }
                        }

                        // find BEL
                        while (++start < crop.size())
                        {
                            auto cmd = crop[start];
                            if (cmd == 0x07) break;
                        }

                        if (start == crop.size())
                        {
                            crop = crop.substr(0, size);
                            return crop;
                        }
                    }
                    // test G0SET: ESC ( c
                    else if (c == '(')
                    {
                        if (++start == crop.size())
                        {
                            crop = crop.substr(0, size);
                            return crop;
                        }
                    }
                    // test ST: ESC \...
                    else if (c == '\\')
                    {
                        if (++start == crop.size())
                        {
                            return crop;
                        }
                    }
                    // test Esc+byte: ESC 7 8 D E H M ...
                    else if (c == '7' ||
                             c == '8' ||
                             c == 'D' ||
                             c == 'E' ||
                             c == 'H' ||
                             c == 'M' ||
                             c == 'c')
                    {
                        if (++start == crop.size())
                        {
                            return crop;
                        }
                    }
                    // test PM: ESC ^
                    else if (c == '^')
                    {
                        // find BEL
                        while (++start < crop.size())
                        {
                            auto cmd = crop[start];
                            if (cmd == 0x07) break;
                        }

                        if (start == crop.size())
                        {
                            crop = crop.substr(0, size);
                            return crop;
                        }
                    }
                    // test APC: ESC _ ... ST
                    else if (c == '_')
                    {
                        // find BEL
                        while (++start < crop.size())
                        {
                            auto cmd = crop[start];
                            if (cmd == 0x07) break;
                        }

                        if (start == crop.size())
                        {
                            crop = crop.substr(0, size);
                            return crop;
                        }
                    }
                    // test DCS: ESC P ... ST
                    else if (c == 'P')
                    {
                        // find BEL
                        while (++start < crop.size())
                        {
                            auto cmd = crop[start];
                            if (cmd == 0x07) break;
                        }

                        if (start == crop.size())
                        {
                            crop = crop.substr(0, size);
                            return crop;
                        }
                    }
                }
                else
                {
                    // preserve ESC at the end
                    crop = crop.substr(0, size);
                    return crop;
                }
            }
        }

        utf::purify(crop);

        return crop;
    }
}

#endif // NETXS_ANSI_HPP