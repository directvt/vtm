// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#if (defined(__unix__) || defined(__APPLE__)) && !defined(__linux__) && !defined(__BSD__)
    #define __BSD__
#endif

#include <type_traits>
#include <iostream>
#include <filesystem>

#if defined(_WIN32)

    #if not defined(NOMINMAX)
        #define NOMINMAX
    #endif

    #pragma warning(disable:4996) // Suppress std::getenv warning.

    #include <Windows.h>
    #include <userenv.h>                 // ::GetUserProfileDirectoryW
    #pragma comment(lib, "Userenv.lib")
    #pragma comment(lib, "Advapi32.lib") // ::GetUserNameW()
    #include <Psapi.h>                   // ::GetModuleFileNameEx
    #include <winternl.h>                // ::NtOpenFile

#else

    #include <errno.h>      // ::errno
    #include <spawn.h>      // ::exec
    #include <unistd.h>     // ::gethostname(), ::getpid(), ::read()
    #include <sys/param.h>  //
    #include <sys/types.h>  // ::getaddrinfo
    #include <sys/socket.h> // ::shutdown() ::socket(2)
    #include <netdb.h>      //

    #include <stdio.h>
    #include <unistd.h>     // ::read()
    #include <sys/un.h>
    #include <stdlib.h>

    #include <csignal>      // ::signal()
    #include <termios.h>    // console raw mode
    #include <sys/ioctl.h>  // ::ioctl
    #include <sys/wait.h>   // ::waitpid
    #include <syslog.h>     // syslog, daemonize

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>      // ::splice()

    #if defined(__linux__)
        #include <sys/vt.h> // ::console_ioctl()
        #if defined(__ANDROID__)
            #include <linux/kd.h>   // ::console_ioctl()
        #else
            #include <sys/kd.h>     // ::console_ioctl()
        #endif
        #include <linux/keyboard.h> // ::keyb_ioctl()
    #endif

    #if defined(__APPLE__)
        #include <mach-o/dyld.h>    // ::_NSGetExecutablePath()
    #elif defined(__BSD__)
        #include <sys/types.h>  // ::sysctl()
        #include <sys/sysctl.h>
    #endif

    extern char **environ;

#endif

namespace netxs::os
{
    namespace fs = std::filesystem;
    namespace key = input::key;
    using page = ui::page;
    using para = ui::para;
    using rich = ui::rich;
    using s11n = ui::s11n;
    using pipe = ui::pipe;
    using xipc = ui::pipe::xipc;
    using deco = ansi::deco;
    using escx = ansi::escx;

    enum class role { client, server };

    static constexpr auto pipebuf = si32{ 65536 };
    static constexpr auto ttysize = twod{ 2500, 50 };
    static constexpr auto app_wait_timeout = 5000;
    static constexpr auto unexpected = " returns unexpected result"sv;
    static auto autosync = true; // Auto sync viewport with cursor position (win7/8 console).
    static auto finalized = flag{ faux }; // Ready flag for clean exit.
    void release()
    {
        os::finalized.exchange(true);
        os::finalized.notify_all();
    }

    #if defined(_WIN32)

        using sigt = DWORD;
        using pidt = DWORD;
        using fd_t = HANDLE;
        struct tios { DWORD omode, imode, opage, ipage; wide title; CONSOLE_CURSOR_INFO caret{}; };
        static const auto invalid_fd   = fd_t{ INVALID_HANDLE_VALUE              };
        static       auto stdin_fd     = fd_t{ ::GetStdHandle(STD_INPUT_HANDLE)  };
        static       auto stdout_fd    = fd_t{ ::GetStdHandle(STD_OUTPUT_HANDLE) };
        static       auto stderr_fd    = fd_t{ ::GetStdHandle(STD_ERROR_HANDLE)  };
        static const auto codepage     = ui32{ ::GetOEMCP()                      };
        static const auto wr_pipe_path = "\\\\.\\pipe\\w_";
        static const auto rd_pipe_path = "\\\\.\\pipe\\r_";

    #else

        using sigt = int;
        using pidt = pid_t;
        using fd_t = int;
        using tios = ::termios;
        static const auto invalid_fd = fd_t{ -1            };
        static       auto stdin_fd   = fd_t{ STDIN_FILENO  };
        static       auto stdout_fd  = fd_t{ STDOUT_FILENO };
        static       auto stderr_fd  = fd_t{ STDERR_FILENO };
        static const auto linux_console = []
        {
            auto conmode = -1;
            #if defined(__linux__)
            ::ioctl(os::stdout_fd, KDGETMODE, &conmode);
            #endif
            return conmode != -1;
        }();

    #endif

    auto is_daemon()
    {
        return os::stdin_fd == os::invalid_fd;
    }
    auto error()
    {
        #if defined(_WIN32)
            return ::GetLastError();
        #else
            return errno;
        #endif
    }
    template<class ...Args>
    auto fail(Args&&... msg)
    {
        log(prompt::os, ansi::err(msg..., " (", os::error(), ") "));
    };
    template<bool Alert = true, class T, class ...Args>
    auto ok(T error_condition, Args&&... msg)
    {
        if (
            #if defined(_WIN32)
                error_condition == 0
            #else
                error_condition == (T)-1
            #endif
        )
        {
            if constexpr (Alert) os::fail(std::forward<Args>(msg)...);
            else                 log(std::forward<Args>(msg)...);
            return faux;
        }
        else return true;
    }
    void close(fd_t const& h)
    {
        if (h != os::invalid_fd)
        {
            #if defined(_WIN32)
                ::CloseHandle(h);
            #else
                ::close(h);
            #endif
        }
    }
    void close(fd_t& h)
    {
        if (h != os::invalid_fd)
        {
            os::close((fd_t const)h);
            h = os::invalid_fd;
        }
    }

    #if defined(_WIN32)

        namespace nt
        {
            auto& get_ntdll()
            {
                struct refs
                {
                    using NtOpenFile_ptr          = std::decay<decltype(::NtOpenFile)>::type;
                    using CsrClientCallServer_ptr = NTSTATUS(_stdcall *)(void*, void*, ui32, ui32);
                    //using ConsoleControl_ptr      = NTSTATUS(_stdcall *)(ui32, void*, ui32);
                    //HMODULE                 user32_dll{};
                    //ConsoleControl_ptr      ConsoleControl{};

                    HMODULE                 ntdll_dll{};
                    NtOpenFile_ptr          NtOpenFile{};
                    CsrClientCallServer_ptr CsrClientCallServer{};

                    refs()
                    {
                        //user32_dll = ::LoadLibraryExA("user32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                        //if (!ntdll_dll || !user32_dll) os::fail("LoadLibraryEx(ntdll.dll | user32.dll)");
                        ntdll_dll = ::LoadLibraryExA("ntdll.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                        if (!ntdll_dll) os::fail("LoadLibraryEx(ntdll.dll)");
                        else
                        {
                            NtOpenFile          = reinterpret_cast<NtOpenFile_ptr>(         ::GetProcAddress(ntdll_dll,  "NtOpenFile"));
                            CsrClientCallServer = reinterpret_cast<CsrClientCallServer_ptr>(::GetProcAddress(ntdll_dll,  "CsrClientCallServer"));
                            if (!NtOpenFile)          os::fail("::GetProcAddress(NtOpenFile)");
                            if (!CsrClientCallServer) os::fail("::GetProcAddress(CsrClientCallServer)");
                            //ConsoleControl = reinterpret_cast<ConsoleControl_ptr>(::GetProcAddress(user32_dll, "ConsoleControl"));
                            //if (!ConsoleControl) os::fail("::GetProcAddress(ConsoleControl)");
                        }
                    }

                    void operator=(refs const&) = delete;
                    refs(refs const&)           = delete;
                    refs(refs&& other)
                        :           ntdll_dll{ other.ntdll_dll           },
                                   NtOpenFile{ other.NtOpenFile          },
                          CsrClientCallServer{ other.CsrClientCallServer }
                               //    user32_dll{ other.user32_dll          },
                               //ConsoleControl{ other.ConsoleControl      }
                    {
                        other.ntdll_dll           = {};
                        other.NtOpenFile          = {};
                        other.CsrClientCallServer = {};
                        //other.user32_dll          = {};
                        //other.ConsoleControl      = {};
                    }
                   ~refs()
                    {
                        if (ntdll_dll) ::FreeLibrary(ntdll_dll);
                        //if (user32_dll) ::FreeLibrary(user32_dll);
                    }

                    constexpr explicit operator bool () const { return NtOpenFile != nullptr; }
                }
                static inst;
                return inst;
            }

            namespace status
            {
                static constexpr auto success               = (NTSTATUS)0x00000000L;
                static constexpr auto unsuccessful          = (NTSTATUS)0xC0000001L;
                static constexpr auto illegal_function      = (NTSTATUS)0xC00000AFL;
                static constexpr auto not_found             = (NTSTATUS)0xC0000225L;
                static constexpr auto not_supported         = (NTSTATUS)0xC00000BBL;
                static constexpr auto buffer_too_small      = (NTSTATUS)0xC0000023L;
                static constexpr auto invalid_buffer_size   = (NTSTATUS)0xC0000206L;
                static constexpr auto invalid_handle        = (NTSTATUS)0xC0000008L;
                static constexpr auto control_c_exit        = (NTSTATUS)0xC000013AL;
            }

            template<class ...Args>
            auto OpenFile(Args... args)
            {
                auto& inst = get_ntdll();
                return inst ? inst.NtOpenFile(std::forward<Args>(args)...)
                            : nt::status::not_found;
            }
            template<class ...Args>
            auto CsrClientCallServer(Args... args)
            {
                auto& inst = get_ntdll();
                return inst ? inst.CsrClientCallServer(std::forward<Args>(args)...)
                            : nt::status::not_found;
            }
            //todo: nt native api monobitness:
            //  We have to make a direct call to ntdll.dll!CsrClientCallServer
            //  due to a user32.dll!ConsoleControl does not work properly under WoW64.
            //template<class ...Args>
            //auto ConsoleControl(Args... args)
            //{
            //    auto& inst = get_ntdll();
            //    return inst ? inst.ConsoleControl(std::forward<Args>(args)...)
            //                : nt::status::not_found;
            //}
            //template<class Arch>
            //auto ConsoleTask(Arch proc_pid, ui32 what)
            //{
            //    struct nttask
            //    {
            //        size_t procid;
            //        size_t window;
            //        ui32   action;
            //        ui32   option;
            //    };
            //    auto task = nttask{ .procid = proc_pid, .action = what };
            //    auto stat = nt::ConsoleControl((ui32)sizeof("Ending"), &task, (ui32)sizeof(task));
            //    return stat;
            //}
            template<class Arch>
            auto ConsoleTask(Arch proc_pid, ui32 what)
            {
                struct nttask
                {
                    struct header
                    {
                        ui32 pad___1;
                        ui32 pad___2;
                        Arch pad___3;
                        Arch pad___4;
                        Arch pad___5;
                        Arch pad___6;
                        Arch pad___7;
                        Arch pad___8;
                    }
                    head;
                    struct payload
                    {
                        ui32 pad___1;
                        ui32 pad___2;
                        Arch pad___3;
                        Arch pad___4;
                        Arch procpid;
                        ui32 eventid;
                        ui32 optflag;
                    }
                    task;
                };
                auto task = nttask{ .task = { .procpid = (Arch)proc_pid,
                                              .eventid = what,
                                              .optflag = 1u << what, } };
                auto stat = nt::CsrClientCallServer(&task,
                                                    nullptr,
                                                    0x00030401, // private api index
                                                    (ui32)sizeof(nttask::payload)); //todo MSVC 17.7.0 requires type cast (ui32)
                return stat;
            }
            template<class I = noop, class O = noop>
            auto ioctl(DWORD dwIoControlCode, fd_t hDevice, I&& send = {}, O&& recv = {}) -> NTSTATUS
            {
                auto BytesReturned   = DWORD{};
                auto lpInBuffer      = std::is_same_v<std::decay_t<I>, noop> ? nullptr : static_cast<void*>(&send);
                auto nInBufferSize   = std::is_same_v<std::decay_t<I>, noop> ? 0       : static_cast<DWORD>(sizeof(send));
                auto lpOutBuffer     = std::is_same_v<std::decay_t<O>, noop> ? nullptr : static_cast<void*>(&recv);
                auto nOutBufferSize  = std::is_same_v<std::decay_t<O>, noop> ? 0       : static_cast<DWORD>(sizeof(recv));
                auto lpBytesReturned = &BytesReturned;
                auto ok = ::DeviceIoControl(hDevice,
                                            dwIoControlCode,
                                            lpInBuffer,
                                            nInBufferSize,
                                            lpOutBuffer,
                                            nOutBufferSize,
                                            lpBytesReturned,
                                            nullptr);
                return ok ? ERROR_SUCCESS
                          : os::error();
            }
            auto object(view        path,
                        ACCESS_MASK mask,
                        ULONG       flag,
                        ULONG       opts = {},
                        HANDLE      boss = {})
            {
                auto hndl = os::invalid_fd;
                auto wtxt = utf::to_utf(path);
                auto size = wtxt.size() * sizeof(wtxt[0]);
                auto attr = OBJECT_ATTRIBUTES{};
                auto stat = IO_STATUS_BLOCK{};
                auto name = UNICODE_STRING{
                    .Length        = (decltype(UNICODE_STRING::Length)       )(size),
                    .MaximumLength = (decltype(UNICODE_STRING::MaximumLength))(size + sizeof(wtxt[0])),
                    .Buffer        = wtxt.data(),
                };
                InitializeObjectAttributes(&attr, &name, flag, boss, nullptr);
                auto status = nt::OpenFile(&hndl, mask, &attr, &stat, FILE_SHARE_READ
                                                                    | FILE_SHARE_WRITE
                                                                    | FILE_SHARE_DELETE, opts);
                if (status != nt::status::success)
                {
                    log("%%Unexpected result when access system object '%path%', ntstatus %status%", prompt::os, path, status);
                    return os::invalid_fd;
                }
                else return hndl;
            }

            namespace console
            {
                static auto buffer = dot_11; // Scrollback/viewport size.

                enum fx
                {
                    undef, connect, disconnect, create, close,
                    write, read, subfx, flush, count,
                };
                namespace event
                {
                    static constexpr auto custom     = 0b1000'0000'0000'0000;
                    static constexpr auto ctrl_c     = CTRL_C_EVENT;
                    static constexpr auto ctrl_break = CTRL_BREAK_EVENT;
                    static constexpr auto close      = CTRL_CLOSE_EVENT;
                    static constexpr auto logoff     = CTRL_LOGOFF_EVENT;
                    static constexpr auto shutdown   = CTRL_SHUTDOWN_EVENT;
                    static constexpr auto style      = CTRL_SHUTDOWN_EVENT + 1;
                }
                namespace op
                {
                    static constexpr auto read_io                 = CTL_CODE(FILE_DEVICE_CONSOLE, 1,  METHOD_OUT_DIRECT,  FILE_ANY_ACCESS);
                    static constexpr auto complete_io             = CTL_CODE(FILE_DEVICE_CONSOLE, 2,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto read_input              = CTL_CODE(FILE_DEVICE_CONSOLE, 3,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto write_output            = CTL_CODE(FILE_DEVICE_CONSOLE, 4,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto issue_user_io           = CTL_CODE(FILE_DEVICE_CONSOLE, 5,  METHOD_OUT_DIRECT,  FILE_ANY_ACCESS);
                    static constexpr auto disconnect_pipe         = CTL_CODE(FILE_DEVICE_CONSOLE, 6,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto set_server_information  = CTL_CODE(FILE_DEVICE_CONSOLE, 7,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto get_server_pid          = CTL_CODE(FILE_DEVICE_CONSOLE, 8,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto get_display_size        = CTL_CODE(FILE_DEVICE_CONSOLE, 9,  METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto update_display          = CTL_CODE(FILE_DEVICE_CONSOLE, 10, METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto set_cursor              = CTL_CODE(FILE_DEVICE_CONSOLE, 11, METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto allow_via_uiaccess      = CTL_CODE(FILE_DEVICE_CONSOLE, 12, METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto launch_server           = CTL_CODE(FILE_DEVICE_CONSOLE, 13, METHOD_NEITHER,     FILE_ANY_ACCESS);
                    static constexpr auto get_font_size           = CTL_CODE(FILE_DEVICE_CONSOLE, 14, METHOD_NEITHER,     FILE_ANY_ACCESS);
                }
                namespace inmode
                {
                    static constexpr auto preprocess    = 0x0001; // ENABLE_PROCESSED_INPUT
                    static constexpr auto cooked        = 0x0002; // ENABLE_LINE_INPUT
                    static constexpr auto echo          = 0x0004; // ENABLE_ECHO_INPUT
                    static constexpr auto winsize       = 0x0008; // ENABLE_WINDOW_INPUT
                    static constexpr auto mouse         = 0x0010; // ENABLE_MOUSE_INPUT
                    static constexpr auto insert        = 0x0020; // ENABLE_INSERT_MODE
                    static constexpr auto quickedit     = 0x0040; // ENABLE_QUICK_EDIT_MODE
                    static constexpr auto extended      = 0x0080; // ENABLE_EXTENDED_FLAGS
                    static constexpr auto auto_position = 0x0100; // ENABLE_AUTO_POSITION
                    static constexpr auto vt            = 0x0200; // ENABLE_VIRTUAL_TERMINAL_INPUT
                }
                namespace outmode
                {
                    static constexpr auto preprocess    = 0x0001; // ENABLE_PROCESSED_OUTPUT
                    static constexpr auto wrap_at_eol   = 0x0002; // ENABLE_WRAP_AT_EOL_OUTPUT
                    static constexpr auto vt            = 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
                    static constexpr auto no_auto_cr    = 0x0008; // DISABLE_NEWLINE_AUTO_RETURN
                    static constexpr auto lvb_grid      = 0x0010; // ENABLE_LVB_GRID_WORLDWIDE
                }

                auto handle(view rootpath)
                {
                    return nt::object(rootpath,
                                      GENERIC_ALL,
                                      OBJ_CASE_INSENSITIVE | OBJ_INHERIT);
                }
                auto handle(fd_t server, view relpath, bool inheritable = {})
                {
                    return nt::object(relpath,
                                      GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                      OBJ_CASE_INSENSITIVE | (inheritable ? OBJ_INHERIT : 0),
                                      FILE_SYNCHRONOUS_IO_NONALERT,
                                      server);
                }
                auto handle(fd_t cloned_handle)
                {
                    auto handle_clone = os::invalid_fd;
                    auto ok = ::DuplicateHandle(::GetCurrentProcess(),
                                                cloned_handle,
                                                ::GetCurrentProcess(),
                                               &handle_clone,
                                                0,
                                                TRUE,
                                                DUPLICATE_SAME_ACCESS);
                    if (ok) return handle_clone;
                    else
                    {
                        log(prompt::os, "Unexpected result when duplicate system object handle, errcode ", os::error());
                        return os::invalid_fd;
                    }
                }
                template<svga Mode>
                auto attr(cell const& c)
                {
                    auto& fgc = c.fgc();
                    auto& bgc = c.bgc();
                    auto f = si32{};
                    auto b = si32{};
                    if constexpr (Mode == svga::nt16)
                    {
                        f = fgc.to_vga16(true);
                        b = bgc.to_vga16(faux);
                        if (f == b && fgc != bgc) cell::clrs::fix_collision_vga16(f);
                    }
                    else
                    {
                        f = fgc.to_vtm16(true);
                        b = bgc.to_vtm16(faux);
                        if (f == b && fgc != bgc) cell::clrs::fix_collision_vtm16(f);
                    }
                    if (c.inv()) std::swap(f, b);
                    if (c.und()) std::swap(f, b);  // Interferes with the menu scrollbar mimics.
                    auto attr = static_cast<ui16>((b << 4) | f);
                    // LEADING/TRAILINGs only for OEMs.
                    //if (c.und()) attr |= COMMON_LVB_UNDERSCORE;  // LVB attributes historically available only for DBCS code pages.
                    //if (c.ovr()) attr |= COMMON_LVB_GRID_HORIZONTAL;
                    return attr;
                }
                void fill(auto& data, auto area, auto coor)
                {
                    auto size = (si32)data.size();
                    auto dest = SMALL_RECT{ .Top = (SHORT)coor.y };
                    auto head = data.data();
                    auto tail = data.end();
                    if (size <= area.x - coor.x || coor.x) // First line.
                    {
                        auto crop = COORD{ (SHORT)std::min(size, area.x - coor.x), 1 };
                        dest.Left   = (SHORT)coor.x;
                        dest.Right  = dest.Left + crop.X - 1;
                        dest.Bottom = dest.Top;
                        ::WriteConsoleOutputW(os::stdout_fd, head, crop, {}, &dest);
                        head += crop.X;
                        size -= crop.X;
                        dest.Top++;
                    }
                    if (size)
                    {
                        dest.Left = 0;
                        if (size >= area.x) // Mid block.
                        {
                            auto height = (SHORT)(size / area.x);
                            auto crop = COORD{ (SHORT)area.x, height };
                            dest.Right  = crop.X - 1;
                            dest.Bottom = dest.Top + height - 1;
                            ::WriteConsoleOutputW(os::stdout_fd, head, crop, {}, &dest);
                            auto s = height * area.x;
                            head += s;
                            size -= s;
                        }
                        if (size) // Tail.
                        {
                            dest.Right = (SHORT)size - 1;
                            dest.Bottom++;
                            dest.Top = dest.Bottom;
                            auto crop = COORD{ (SHORT)size, 1 };
                            ::WriteConsoleOutputW(os::stdout_fd, head, crop, {}, &dest);
                        }
                    }
                }
                template<svga Mode>
                void print(auto area, auto coor, auto head, auto tail) // STA
                {
                    static auto buffer = std::vector<CHAR_INFO>{};
                    static auto toWIDE = wide{};

                    auto dist = tail - head;
                    buffer.resize(dist);
                    auto dest = buffer.begin();
                    while (head != tail)
                    {
                        auto& src = *head++;
                        auto& dst = *dest++;
                        dst.Attributes = nt::console::attr<Mode>(src);
                        toWIDE.clear();
                        utf::to_utf(src.txt(), toWIDE);
                        auto& chr = dst.Char.UnicodeChar;
                        if (auto len = toWIDE.size())
                        {
                            if (src.wdt() < 3) chr = toWIDE[0];
                            else               chr = len == 1 ? 32 : toWIDE[1]; // The second cell for wide glyph should be zero in Win7/8 console. In the Win10 console, it should be the same as the first cell.
                            if (chr == 0) chr = 32; // Null character is unsupported in SBCS codepages (eg 437) on win7/8.
                        }
                        else chr = 32;
                    }
                    fill(buffer, area, coor);
                    //todo Do we really need a wrap for wide chars? What about horizontal scrolling?
                    //auto dest = SMALL_RECT{ .Right = (SHORT)area.x, .Bottom = (SHORT)area.y };
                    //auto crop = COORD{ .Y = 1 };
                    //auto rest = (si32)dist;
                    //auto data = buffer.data();
                    //while (rest)
                    //{
                    //    crop.X = (SHORT)std::min(rest, area.x - coor.x);
                    //    dest.Left = coor.x;
                    //    dest.Right = coor.x + crop.X - 1;
                    //    dest.Top = coor.y;
                    //    dest.Bottom = coor.y;
                    //    ::WriteConsoleOutputW(os::stdout_fd, data, crop, {}, &dest);
                    //    data += crop.X;
                    //    rest -= crop.X;
                    //    coor.x = 0;
                    //    coor.y++;
                    //}
                }

                struct vtparser
                    : public ansi::parser
                {
                    using redo = std::list<std::pair<deco, ansi::mark>>;

                    redo stack; // vtparser: Style state stack.
                    grid cache; // vtparser: Temp buffer for console cells.
                    twod coord; // vtparser: Current cursor position inside console::buffer.
                    twod saved; // vtparser: Saved cursor position.
                    bool shown; // vtparser: Cursor visibility state.

                    template<class T>
                    static void parser_config(T& vt)
                    {
                        using namespace netxs::ansi;

                        #define V [](auto& q, auto& p)
                        vt.intro[ctrl::nul]              = V{ p->post(utf::frag{ emptyspace, utf::prop{ 0, 1 } }); };
                        vt.intro[ctrl::cr ]              = V{ p->task({ fn::ax, 0 }); };
                        vt.intro[ctrl::eol]              = V{ p->task({ fn::nl, 1 }); };
                        vt.intro[ctrl::tab]              = V{ p->task({ fn::tb, q.pop_all(ctrl::tab) }); };
                        vt.csier.table[csi__ed]          = V{ p->task({ fn::ed, q(0) }); }; // CSI Ps J
                        vt.csier.table[csi__el]          = V{ p->task({ fn::el, q(0) }); }; // CSI Ps K
                        vt.csier.table_hash[csi_hsh_psh] = V{ p->pushsgr(); }; // CSI # {  Push current SGR attributes and style onto stack.
                        vt.csier.table_hash[csi_hsh_pop] = V{ p->popsgr();  }; // CSI # }  Pop  current SGR attributes and style from stack.
                        vt.csier.table_quest[dec_set]    = V{ p->decset(q); };
                        vt.csier.table_quest[dec_rst]    = V{ p->decrst(q); };
                        #undef V
                    }

                    void cout(view utf8)
                    {
                        ansi::parse(utf8, this);
                        if (os::autosync) ::SetConsoleCursorPosition(os::stdout_fd, { .X = (SHORT)coord.x, .Y = (SHORT)coord.y }); // Viewport follows to cursor.
                    }
                    auto status()
                    {
                        auto state = CONSOLE_SCREEN_BUFFER_INFO{};
                        ::GetConsoleScreenBufferInfo(os::stdout_fd, &state);
                        return state;
                    }
                    void cursor(bool show)
                    {
                        auto state = status();
                        if (show)
                        {
                            if (coord.x < state.srWindow.Left // Sync viewport.
                             || coord.x > state.srWindow.Right
                             || coord.y < state.srWindow.Top
                             || coord.y > state.srWindow.Bottom)
                            {
                                auto delta = [](auto& head, auto& tail, auto coor)
                                {
                                    auto size = tail - head + 1;
                                    auto step = coor < size ?-head
                                              : coor > tail ? coor - tail
                                              : coor < head ? coor - head
                                                            : 0;
                                    head += step;
                                    tail += step;
                                };
                                delta(state.srWindow.Left, state.srWindow.Right,  coord.x); // Win10 conhost crashes if vieport is outside the buffer (e.g. in case with deferred cursor position).
                                delta(state.srWindow.Top,  state.srWindow.Bottom, coord.y);
                                ::SetConsoleWindowInfo(os::stdout_fd, TRUE, &state.srWindow);
                            }
                            auto newcoor = coord;
                            if (newcoor.x == state.dwSize.X) newcoor.x--; // win7/8 conhost isn't aware about the deferred cursor position.
                            ::SetConsoleCursorPosition(os::stdout_fd, { .X = (SHORT)newcoor.x, .Y = (SHORT)newcoor.y }); // Viewport follows to cursor.
                        }
                        if (shown == show) return;
                        shown = show;
                        auto s = CONSOLE_CURSOR_INFO{};
                        ::GetConsoleCursorInfo(os::stdout_fd, &s);
                        s.bVisible = shown;
                        ::SetConsoleCursorInfo(os::stdout_fd, &s);
                    }

                    vtparser()
                    {
                        auto state = status(); // Update current brush state.
                        auto c = cell{}.fgc(rgba::vga16[(state.wAttributes & 0x0F)])
                                       .bgc(rgba::vga16[(state.wAttributes & 0xF0) >> 4])
                                       .inv(state.wAttributes & COMMON_LVB_REVERSE_VIDEO);
                        parser::brush.reset(c);
                        parser::style.reset();
                        auto s = CONSOLE_CURSOR_INFO{};
                        ::GetConsoleCursorInfo(os::stdout_fd, &s);
                        shown = s.bVisible;
                        coord = { state.dwCursorPosition.X, state.dwCursorPosition.Y };
                    }
                    void pushsgr() // vtparser: CSI # {  Push SGR attributes.
                    {
                        parser::flush();
                        stack.push_back({ parser::style, parser::brush });
                        if (stack.size() == 10) stack.pop_front();
                    }
                    void popsgr() // vtparser: CSI # }  Pop SGR attributes.
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
                    void decset(fifo& queue)
                    {
                        parser::flush();
                        while (auto n = queue(0))
                        {
                            if (n == 25) cursor(true); // Show cursor and sync viewport.
                        }
                    }
                    void decrst(fifo& queue)
                    {
                        parser::flush();
                        while (auto n = queue(0))
                        {
                            if (n == 25) cursor(faux); // Hide cursor.
                        }
                    }
                    auto scroll()
                    {
                        if (coord.y >= console::buffer.y)
                        {
                            auto delta = coord.y - console::buffer.y - 1;
                            auto color = CHAR_INFO{ .Char = L' ', .Attributes = console::attr<svga::nt16>(parser::brush) };
                            auto block = SMALL_RECT{ .Right = (SHORT)console::buffer.x, .Bottom = (SHORT)console::buffer.y };
                            auto start = COORD{ .Y = (SHORT)delta };
                            ::ScrollConsoleScreenBufferW(os::stdout_fd, &block, nullptr, start, &color);
                            coord.y -= delta;
                            return delta;
                        }
                        else return 0;
                    }
                    void task(ansi::rule const& cmd)
                    {
                        parser::flush();
                        if (cmd.cmd == ansi::fn::tb)
                        {
                            coord.x += parser::style.tablen * cmd.arg - coord.x % parser::style.tablen;
                        }
                        else if (cmd.cmd == ansi::fn::nl)
                        {
                            coord.x = 0;
                            coord.y += cmd.arg;
                            scroll();
                        }
                        else
                        {
                            if (cmd.cmd == ansi::fn::ed)
                            {
                                if (cmd.arg < 3)
                                {
                                    auto panel = console::buffer;
                                    auto color = console::attr<svga::nt16>(parser::brush);
                                    auto total = panel.x * panel.y;
                                    auto count = panel.x * coord.y + coord.x;
                                    auto start = coord;
                                         if (cmd.arg == 0) { count = total - count;     } // Ps = 0  ⇒  Erase Below (default).
                                    else if (cmd.arg == 1) { start = {};                } // Ps = 1  ⇒  Erase Above (Exclude current).
                                    else if (cmd.arg == 2) { start = {}; count = total; } // Ps = 2  ⇒  Erase All.
                                    auto empty = std::vector<CHAR_INFO>(count, { ' ', color });
                                    fill(empty, panel, start);
                                }
                                else if (cmd.arg == 3) // Ps = 3  ⇒  Erase Scrollback
                                {
                                    auto state = status();
                                    auto color = console::attr<svga::nt16>(parser::brush);
                                    auto start = COORD{ .X = 0, .Y = state.srWindow.Top - state.srWindow.Bottom - 1 };
                                    auto brush = CHAR_INFO{ .Char = L' ', .Attributes = color };
                                    ::ScrollConsoleScreenBufferW(os::stdout_fd, &state.srWindow, nullptr, start, &brush);
                                }
                            }
                            else if (cmd.cmd == ansi::fn::el)
                            {
                                if (cmd.arg == 0) // Ps = 0  ⇒  Erase to Right (default).
                                {

                                }
                                else if (cmd.arg == 1) // Ps = 1  ⇒  Erase to Left.
                                {

                                }
                                else if (cmd.arg == 2) // Ps = 2  ⇒  Erase All.
                                {

                                }
                            }
                            else if (cmd.cmd == ansi::fn::dx) { coord.x += cmd.arg;     } // horizontal delta.
                            else if (cmd.cmd == ansi::fn::dy) { coord.y += cmd.arg;     } // vertical delta.
                            else if (cmd.cmd == ansi::fn::ax) { coord.x  = cmd.arg;     } // x absolute (0-based).
                            else if (cmd.cmd == ansi::fn::ay) { coord.y  = cmd.arg;     } // y absolute (0-based).
                            else if (cmd.cmd == ansi::fn::ox) { coord.x  = cmd.arg - 1; } // old format x absolute (1-based).
                            else if (cmd.cmd == ansi::fn::oy) { coord.y  = cmd.arg - 1; } // old format y absolute (1-based).
                            else if (cmd.cmd == ansi::fn::px) {} // x percent.
                            else if (cmd.cmd == ansi::fn::py) {} // y percent.
                            else if (cmd.cmd == ansi::fn::sc) { saved = coord; } // save cursor position.
                            else if (cmd.cmd == ansi::fn::rc) { coord = saved; } // Restore cursor position.
                            else if (cmd.cmd == ansi::fn::zz) { coord = {}; } // all params reset to zero.
                        }
                        coord = std::clamp(coord, dot_00, console::buffer - dot_11);
                    }
                    void data(si32 count, grid const& proto)
                    {
                        auto saved = coord;
                        auto panel = console::buffer;
                        coord.x += count;
                        coord.y += (coord.x + (panel.x - 1)) / panel.x - 1;
                        coord.x  = (coord.x - 1) % panel.x + 1;
                        saved.y -= scroll();
                        auto seek = coord.x + coord.y * panel.x;
                        if (count > seek)
                        {
                            count = seek;
                            saved = {};
                        }
                        cache.resize(count);
                        auto head = cache.begin();
                        auto tail = cache.end();
                        auto data = proto.end();
                        rich::reverse_fill_proc<faux>(data, tail, head, cell::shaders::full);
                        nt::console::print<svga::nt16>(panel, saved, head, tail);
                    }
                };
            }

            template<class T1, class T2 = si32>
            auto kbstate(si32& modstate, T1 ms_ctrls, T2 scancode = {}, bool pressed = {})
            {
                auto prevstate = modstate;
                if (scancode == 0x2a)
                {
                    if (pressed) modstate |= input::hids::LShift;
                    else         modstate &=~input::hids::LShift;
                }
                else if (scancode == 0x36)
                {
                    if (pressed) modstate |= input::hids::RShift;
                    else         modstate &=~input::hids::RShift;
                }
                else if (scancode == 0x5b)
                {
                    if (pressed) modstate |= input::hids::LWin;
                    else         modstate &=~input::hids::LWin;
                }
                else if (scancode == 0x5c)
                {
                    if (pressed) modstate |= input::hids::RWin;
                    else         modstate &=~input::hids::RWin;
                }
                if (!(modstate & input::hids::anyShift) && ms_ctrls & SHIFT_PRESSED) // Restore Shift after refocusing.
                {
                    modstate |= input::hids::LShift;
                }
                auto lshift = modstate & input::hids::LShift;
                auto rshift = modstate & input::hids::RShift;
                auto lwin   = modstate & input::hids::LWin;
                auto rwin   = modstate & input::hids::RWin;
                bool lalt   = ms_ctrls & LEFT_ALT_PRESSED;
                bool ralt   = ms_ctrls & RIGHT_ALT_PRESSED;
                bool lctrl  = ms_ctrls & LEFT_CTRL_PRESSED;
                bool rctrl  = ms_ctrls & RIGHT_CTRL_PRESSED;
                bool nums   = ms_ctrls & NUMLOCK_ON;
                bool caps   = ms_ctrls & CAPSLOCK_ON;
                bool scrl   = ms_ctrls & SCROLLLOCK_ON;
                auto state  = ui32{};
                if (lshift) state |= input::hids::LShift;
                if (rshift) state |= input::hids::RShift;
                if (lalt  ) state |= input::hids::LAlt;
                if (ralt  ) state |= input::hids::RAlt;
                if (lctrl ) state |= input::hids::LCtrl;
                if (rctrl ) state |= input::hids::RCtrl;
                if (lwin  ) state |= input::hids::LWin;
                if (rwin  ) state |= input::hids::RWin;
                if (nums  ) state |= input::hids::NumLock;
                if (caps  ) state |= input::hids::CapsLock;
                if (scrl  ) state |= input::hids::ScrlLock;
                auto changed = prevstate != state;
                modstate = state;
                return changed;
            }
            auto ms_kbstate()
            {
                auto vkeys = std::array<BYTE, 256>{};
                ::GetKeyboardState(vkeys.data());
                return vkeys[VK_SHIFT   ] & 0b1000'0000 ? SHIFT_PRESSED      : 0
                     | vkeys[VK_LMENU   ] & 0b1000'0000 ? LEFT_ALT_PRESSED   : 0
                     | vkeys[VK_RMENU   ] & 0b1000'0000 ? RIGHT_ALT_PRESSED  : 0
                     | vkeys[VK_LCONTROL] & 0b1000'0000 ? LEFT_CTRL_PRESSED  : 0
                     | vkeys[VK_RCONTROL] & 0b1000'0000 ? RIGHT_CTRL_PRESSED : 0
                     | vkeys[VK_NUMLOCK ] & 0b0000'0001 ? NUMLOCK_ON         : 0
                     | vkeys[VK_CAPITAL ] & 0b0000'0001 ? CAPSLOCK_ON        : 0
                     | vkeys[VK_SCROLL  ] & 0b0000'0001 ? SCROLLLOCK_ON      : 0;
            }
            template<class T1>
            auto ms_kbstate(T1 ctrls)
            {
                bool lshift = ctrls & input::hids::LShift;
                bool rshift = ctrls & input::hids::RShift;
                bool lalt   = ctrls & input::hids::LAlt;
                bool ralt   = ctrls & input::hids::RAlt;
                bool lctrl  = ctrls & input::hids::LCtrl;
                bool rctrl  = ctrls & input::hids::RCtrl;
                bool nums   = ctrls & input::hids::NumLock;
                bool caps   = ctrls & input::hids::CapsLock;
                bool scrl   = ctrls & input::hids::ScrlLock;
                auto state  = ui32{};
                if (lshift
                 || rshift) state |= SHIFT_PRESSED;
                if (lalt  ) state |= LEFT_ALT_PRESSED;
                if (ralt  ) state |= RIGHT_ALT_PRESSED;
                if (lctrl ) state |= LEFT_CTRL_PRESSED;
                if (rctrl ) state |= RIGHT_CTRL_PRESSED;
                if (nums  ) state |= NUMLOCK_ON;
                if (caps  ) state |= CAPSLOCK_ON;
                if (scrl  ) state |= SCROLLLOCK_ON;
                return state;
            }
            auto is_wow64()
            {
                if constexpr (sizeof(void*) == 4)
                {
                    auto isWow64Process = BOOL{};
                    ::IsWow64Process(::GetCurrentProcess(), &isWow64Process);
                    return !!isWow64Process;
                }
                else return faux;
            }
        }

    #else

        void fdscleanup() // Close all file descriptors except the standard ones.
        {
            auto maxfd = ::sysconf(_SC_OPEN_MAX);
            auto minfd = std::max({ STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO });
            while (++minfd < maxfd)
            {
                ::close(minfd);
            }
        }
        auto vgafont() // Add block drawing characters to the Linux console font.
        {
            #if defined(__linux__)

                if (os::linux_console)
                {
                    auto chars = std::vector<unsigned char>(512 * 32 * 4);
                    auto fdata = console_font_op{ .op        = KD_FONT_OP_GET,
                                                  .flags     = 0,
                                                  .width     = 32,
                                                  .height    = 32,
                                                  .charcount = 512,
                                                  .data      = chars.data() };
                    if (!ok(::ioctl(os::stdout_fd, KDFONTOP, &fdata), "::ioctl(KDFONTOP, KD_FONT_OP_GET)", os::unexpected)) return;

                    auto slice_bytes = (fdata.width + 7) / 8;
                    auto block_bytes = (slice_bytes * fdata.height + 31) / 32 * 32;
                    auto tophalf_idx = 10;
                    auto lowhalf_idx = 254;
                    auto tophalf_ptr = fdata.data + block_bytes * tophalf_idx;
                    auto lowhalf_ptr = fdata.data + block_bytes * lowhalf_idx;
                    for (auto row = 0; row < fdata.height; row++)
                    {
                        auto is_top = row < fdata.height / 2;
                       *tophalf_ptr = is_top ? 0xFF : 0x00;
                       *lowhalf_ptr = is_top ? 0x00 : 0xFF;
                        tophalf_ptr+= slice_bytes;
                        lowhalf_ptr+= slice_bytes;
                    }
                    fdata.op = KD_FONT_OP_SET;
                    if (!ok(::ioctl(os::stdout_fd, KDFONTOP, &fdata), "::ioctl(KDFONTOP, KD_FONT_OP_SET)", os::unexpected)) return;

                    auto max_sz = std::numeric_limits<unsigned short>::max();
                    auto spairs = std::vector<unipair>(max_sz);
                    auto dpairs = std::vector<unipair>(max_sz);
                    auto srcmap = unimapdesc{ max_sz, spairs.data() };
                    auto dstmap = unimapdesc{ max_sz, dpairs.data() };
                    auto dstptr = dstmap.entries;
                    auto srcptr = srcmap.entries;
                    if (!ok(::ioctl(os::stdout_fd, GIO_UNIMAP, &srcmap), "::ioctl(os::stdout_fd, GIO_UNIMAP)", os::unexpected)) return;
                    auto srcend = srcmap.entries + srcmap.entry_ct;
                    while (srcptr != srcend) // Drop 10, 211, 254 and 0x2580▀ + 0x2584▄.
                    {
                        auto& smap = *srcptr++;
                        if (smap.fontpos != 10
                         && smap.fontpos != 211
                         && smap.fontpos != 254
                         && smap.unicode != 0x2580
                         && smap.unicode != 0x2584) *dstptr++ = smap;
                    }
                    dstmap.entry_ct = dstptr - dstmap.entries;
                    unipair new_recs[] = { { 0x2580,  10 },
                                           { 0x2219, 211 },
                                           { 0x2022, 211 },
                                           { 0x25CF, 211 },
                                           { 0x25A0, 254 },
                                           { 0x25AE, 254 },
                                           { 0x2584, 254 } };
                    if (dstmap.entry_ct < max_sz - std::size(new_recs)) // Add new records.
                    {
                        for (auto& p : new_recs) *dstptr++ = p;
                        dstmap.entry_ct += std::size(new_recs);
                        if (!ok(::ioctl(os::stdout_fd, PIO_UNIMAP, &dstmap), "::ioctl(os::stdout_fd, PIO_UNIMAP)", os::unexpected)) return;
                    }
                    else log(prompt::os, "VGA font loading failed - 'UNIMAP' is full");
                }

            #endif
        }

    #endif

    template<class T>
    struct syscall
    {
        using E = std::invoke_result_t<decltype(os::error)>;

        T value;
        E error;

        syscall(T&& value)
            : value{ value       },
              error{ os::error() }
        { }

        explicit operator bool () const // Return true if success.
        {
            #if defined(_WIN32)
                return value != 0;
            #else
                return value != (T)-1;
            #endif
        }
    };

    struct sock
    {
        fd_t r; // sock: Read descriptor.
        fd_t w; // sock: Send descriptor.

        operator bool ()
        {
            return r != os::invalid_fd && w != os::invalid_fd;
        }
        void close()
        {
            if (w == r)
            {
                os::close(r);
                w = r;
            }
            else
            {
                os::close(w); // Wriite end should be closed first.
                os::close(r);
            }
        }
        void shutdown() // Reset writing end of the pipe to interrupt reading call.
        {
            #if not defined(_WIN32) // Use ::shutdown() for full duplex sockets. Socket the same fd could be assigned as stdin, stdout and stderr, e.g. it is how inetd does.

                auto statbuf = (struct stat){};
                ::fstat(w, &statbuf);
                if (S_ISSOCK(statbuf.st_mode))
                {
                    ::shutdown(w, SHUT_WR);
                }

            #endif
            if (w != r)
            {
                os::close(w);
            }
        }
        friend auto& operator << (std::ostream& s, sock const& handle)
        {
            if (handle.w != handle.r) s << utf::to_hex_0x(handle.r) << '-';
            return                    s << utf::to_hex_0x(handle.w);
        }
        auto& operator = (sock&& f)
        {
            r = f.r;
            w = f.w;
            f.r = os::invalid_fd;
            f.w = os::invalid_fd;
            return *this;
        }

        sock(sock const&) = delete;
        sock(sock&& f)
            : r{ f.r },
              w{ f.w }
        {
            f.r = os::invalid_fd;
            f.w = os::invalid_fd;
        }
        sock(fd_t r = os::invalid_fd, fd_t w = os::invalid_fd)
            : r{ r },
              w{ w }
        { }
       ~sock()
        {
            close();
        }
    };

    struct fire
    {
        flag fired{};

        #if defined(_WIN32)

            fd_t h; // fire: Descriptor for IO interrupt.

            operator auto () { return h; }
            fire(bool i = 1) { ok(h = ::CreateEventW(NULL, i, FALSE, NULL), "::CreateEventW()", os::unexpected); }
           ~fire()           { os::close(h); }
            void reset()     { fired.exchange(true); ::SetEvent(h);   }
            void flush()     { fired.exchange(faux); ::ResetEvent(h); }

        #else

            fd_t h[2] = { os::invalid_fd, os::invalid_fd }; // fire: Descriptors for IO interrupt.

            operator auto () { return h[0]; }
            fire()           { ok(::pipe(h), "::pipe(2)", os::unexpected); }
           ~fire()           { for (auto& f : h) os::close(f); }
            void reset()     { fired.exchange(true); auto c = ' '; auto rc = ::write(h[1], &c, sizeof(c)); }
            void flush()     { fired.exchange(faux); auto c = ' '; auto rc = ::read(h[0], &c, sizeof(c)); }

        #endif
        void bell() { reset(); }
    };

    namespace signals // Process-wide signals disposition.
    {
        #if defined(_WIN32)

            static auto state = []
            {
                static auto handler = [](sigt what) // Put the console control events at the head of the input event queue.
                {
                    auto count = DWORD{};
                    ::GetNumberOfConsoleInputEvents(stdin_fd, &count);
                    auto events = std::vector<INPUT_RECORD>(count + 1);
                    if (count) ::ReadConsoleInputW(stdin_fd, events.data() + 1, count, &count);
                    events[0] = { .EventType = MENU_EVENT, .Event = { .MenuEvent = what | nt::console::event::custom, }}; // Too hacky (MENU_EVENT).
                    ::WriteConsoleInputW(stdin_fd, events.data(), count + 1, &count);
                    if (what > nt::console::event::ctrl_break) // Waiting for the process to complete.
                    {
                        os::finalized.wait(faux);
                    }
                    return TRUE;
                };
                ::SetConsoleCtrlHandler(handler, TRUE);
                auto deleter = [](auto*){ ::SetConsoleCtrlHandler(handler, FALSE); };
                return std::unique_ptr<decltype(handler), decltype(deleter)>(&handler);
            }();

        #else

            static auto sigset = ::sigset_t{};
            static auto backup = ::sigset_t{};
            static auto state = []
            {
                auto action = [](auto){ };
                auto forced_EINTR = (struct sigaction){};
                forced_EINTR.sa_flags = 0 & ~SA_RESTART; // BSD systems require it for EINTR.
                forced_EINTR.sa_handler = action;
                ::sigaction(SIGUSR2, &forced_EINTR, nullptr); // Readfile interruptor.
                ::signal(SIGWINCH, action); // BSD systems require a dummy action for this signal.
                ::signal(SIGPIPE, SIG_IGN); // Ignore writing to a broken pipe.
                sigemptyset(&sigset);
                sigemptyset(&backup);
                sigaddset(&sigset, SIGWINCH);
                sigaddset(&sigset, SIGINT);
                sigaddset(&sigset, SIGHUP);
                sigaddset(&sigset, SIGTERM); // System shutdown.
                sigaddset(&sigset, SIGUSR1); // Signal s11n thread interruptor.
                ::pthread_sigmask(SIG_BLOCK, &sigset, &backup);
                auto deleter = [](auto*)
                {
                    ::pthread_sigmask(SIG_SETMASK, &backup, nullptr);
                    ::signal(SIGPIPE,  SIG_DFL);
                    ::signal(SIGUSR2,  SIG_DFL);
                    ::signal(SIGWINCH, SIG_DFL);
                };
                return std::unique_ptr<decltype(backup), decltype(deleter)>(&backup);
            }();

            struct fd // Signal s11n.
            {
                flag        active = { true };
                fd_t        handle[2] = { os::invalid_fd, os::invalid_fd };
                std::thread thread;

                fd()
                {
                    ok(::pipe(handle), "::pipe(h)", os::unexpected); // ::pipe2() is not available on macOS.
                    ok(::fcntl(handle[1], F_SETFL, ::fcntl(handle[1], F_GETFL) | O_NONBLOCK), "::fcntl(h, O_NONBLOCK)", os::unexpected);
                    thread = std::thread{[&]
                    {
                        auto signal = sigt{};
                        while (true)
                        {
                            auto rc = ::sigwait(&signals::sigset, &signal);
                            if (signal == SIGUSR1 && !active) break;
                            if (signal > 0) ok(::write(handle[1], &signal, sizeof(signal)), "::write(h[1])", os::unexpected);
                        }
                    }};
                }
               ~fd()
                {
                    active.exchange(faux);
                    ok(::pthread_kill(thread.native_handle(), SIGUSR1), "::pthread_kill(SIGUSR1)", os::unexpected);
                    if (thread.joinable()) thread.join();
                    os::close(handle[1]);
                    os::close(handle[0]);
                }
                operator fd_t () const { return handle[0]; }
            };

        #endif
    }

    namespace io
    {
        template<class Size_t>
        auto recv(fd_t fd, char* buffer, Size_t size)
        {
            #if defined(_WIN32)
                auto count = DWORD{};
                ::ReadFile(fd, buffer, (DWORD)size, &count, nullptr);
            #else
                auto count = ::read(fd, buffer, size);
            #endif
            return count > 0 ? qiew{ buffer, count }
                             : qiew{}; // An empty result is always an error condition.
        }
        void abort(std::thread& thread) // Abort a blocked reading thread.
        {
            std::this_thread::yield(); // Try to ensure that the reading thread has already called a syscall (30-180ms lag).
            auto h = thread.native_handle();
            #if defined(_WIN32)
                ::CancelSynchronousIo(h);
            #else
                ::pthread_kill(h, SIGUSR2);
            #endif
        }
        template<class Size_t>
        auto send(fd_t fd, char const* buffer, Size_t size)
        {
            while (size > 0)
            {
                #if defined(_WIN32)
                    auto count = DWORD{};
                    ::WriteFile(fd, buffer, (DWORD)size, &count, nullptr);
                #else
                    auto count = ::write(fd, buffer, size);
                #endif
                if (count == size) return true;
                if (count > 0)
                {
                    buffer += count;
                    size -= count;
                }
                else break;
            }
            return faux;
        }
        template<class T, class Size_t>
        auto recv(fd_t fd, T* buffer, Size_t size)
        {
            return io::recv(fd, (char*)buffer, size);
        }
        template<class T, class Size_t>
        auto send(fd_t fd, T* buffer, Size_t size)
        {
            return io::send(fd, (char const*)buffer, size);
        }
        template<class Span>
        auto recv(fd_t fd, Span& buffer)
        {
            return io::recv(fd, buffer.data(), buffer.size());
        }
        template<class View>
        auto send(fd_t fd, View&& buffer)
        {
            return io::send(fd, buffer.data(), buffer.size());
        }
        auto send(qiew buffer)
        {
            return io::send(os::stdout_fd, buffer);
        }
        template<class ...Args>
        auto recv(sock& handle, Args&&... args)
        {
            return io::recv(handle.r, std::forward<Args>(args)...);
        }
        template<class ...Args>
        auto send(sock& handle, Args&&... args)
        {
            return io::send(handle.w, std::forward<Args>(args)...);
        }

        namespace
        {
            #if defined(_WIN32)

                template<class A, sz_t... I>
                constexpr auto _repack(fd_t h, A const& a, std::index_sequence<I...>)
                {
                    return std::array{ a[I]..., h };
                }
                template<sz_t N, class P, class Index = std::make_index_sequence<N>, class ...Args>
                constexpr auto _combine(std::array<fd_t, N> const& a, fd_t h, P&& proc, Args&&... args)
                {
                    if constexpr (sizeof...(args)) return _combine(_repack(h, a, Index{}), std::forward<Args>(args)...);
                    else                           return _repack(h, a, Index{});
                }
                template<class P, class ...Args>
                constexpr auto _fd_set(fd_t handle, P&& proc, Args&&... args)
                {
                    if constexpr (sizeof...(args)) return _combine(std::array{ handle }, std::forward<Args>(args)...);
                    else                           return std::array{ handle };
                }
                template<class R, class P, class ...Args>
                constexpr auto _handle(R i, fd_t handle, P&& proc, Args&&... args)
                {
                    if (i == 0)
                    {
                        proc();
                        return true;
                    }
                    else
                    {
                        if constexpr (sizeof...(args)) return _handle(--i, std::forward<Args>(args)...);
                        else                           return faux;
                    }
                }

            #else

                template<class P, class ...Args>
                auto _fd_set(fd_set& socks, fd_t handle, P&& proc, Args&&... args)
                {
                    if (handle != os::invalid_fd) FD_SET(handle, &socks);
                    if constexpr (sizeof...(args))
                    {
                        return std::max(handle, _fd_set(socks, std::forward<Args>(args)...));
                    }
                    else
                    {
                        return handle;
                    }
                }
                template<class T, class P, class ...Args>
                auto _select(T count, fd_set& socks, fd_t handle, P&& proc, Args&&... args)
                {
                    if (count > 0)
                    {
                        if (handle != os::invalid_fd && FD_ISSET(handle, &socks))
                        {
                            proc();
                            count--;
                        }
                        // Multiple descriptors can be ready in a single ::select() iteration.
                        if constexpr (sizeof...(args)) _select(count, socks, std::forward<Args>(args)...);
                    }
                }

            #endif
        }
        template<bool NonBlocked = faux, class ...Args>
        void select(Args&&... args)
        {
            #if defined(_WIN32)

                static constexpr auto timeout = NonBlocked ? 0 /*milliseconds*/ : INFINITE;
                auto socks = _fd_set(std::forward<Args>(args)...);
                // Note: ::WaitForMultipleObjects() does not work with pipes (DirectVT).
                auto yield = ::WaitForMultipleObjects((DWORD)socks.size(), socks.data(), FALSE, timeout);
                yield -= WAIT_OBJECT_0;
                _handle(yield, std::forward<Args>(args)...);

            #else

                auto timeval = ::timeval{ .tv_sec = 0, .tv_usec = 0 };
                auto timeout = NonBlocked ? &timeval/*returns immediately*/ : nullptr;
                auto socks = fd_set{};
                FD_ZERO(&socks);
                auto nfds = 1 + _fd_set(socks, std::forward<Args>(args)...);
                auto count = ::select(nfds, &socks, 0, 0, timeout);
                _select(count, socks, std::forward<Args>(args)...);

            #endif
        }
        void drop()
        {
            #if defined(_WIN32)
                ::FlushConsoleInputBuffer(os::stdin_fd);
            #else
                auto flush = text(os::pipebuf, 0);
                while (true)
                {
                    auto empty = true;
                    io::select<true>(os::stdin_fd, [&]{
                        empty = flush.size() != io::recv(os::stdin_fd, flush).length(); });
                    if (empty) break;
                }
            #endif
        }
    }

    namespace env
    {
        // os::env: Get envvar value.
        auto get(qiew variable)
        {
            #if defined(_WIN32)
                auto var = utf::to_utf(variable);
                auto len = ::GetEnvironmentVariableW(var.c_str(), 0, 0);
                auto val = wide(len, 0);
                ::GetEnvironmentVariableW(var.c_str(), val.data(), len);
                if (len && val.back() == 0) val.pop_back();
                return utf::to_utf(val);
            #else
                auto var = variable.str();
                auto val = std::getenv(var.c_str());
                return val ? text{ val } : text{};
            #endif
        }
        // os::env: Set envvar value.
        auto set(qiew variable, qiew value)
        {
            #if defined(_WIN32)
                auto var = utf::to_utf(variable);
                auto val = utf::to_utf(value);
                ok(::SetEnvironmentVariableW(var.c_str(), val.c_str()), "::SetEnvironmentVariableW()", os::unexpected);
            #else
                auto var = variable.str();
                auto val = value.str();
                ok(::setenv(var.c_str(), val.c_str(), 1), "::setenv()", os::unexpected);
            #endif
        }
        // os::env: Get list of envvars using wildcard.
        auto list(text&& var)
        {
            auto crop = std::vector<text>{};
            auto list = environ;
            while (*list)
            {
                auto v = view{ *list++ };
                if (v.starts_with(var))
                {
                    crop.emplace_back(v);
                }
            }
            std::sort(crop.begin(), crop.end());
            return crop;
        }
        // os::env: Get user home path.
        auto homepath()
        {
            #if defined(_WIN32)
                auto handle = ::GetCurrentProcessToken();
                auto length = DWORD{};
                auto buffer = wide{};
                ::GetUserProfileDirectoryW(handle, nullptr, &length);
                if (length)
                {
                    buffer.resize(length);
                    ::GetUserProfileDirectoryW(handle, buffer.data(), &length);
                    if (buffer.back() == '\0') buffer.pop_back(); // Pop terminating null.
                }
                else os::fail("Can't detect user profile path");
                return fs::path{ utf::to_utf(buffer) };
            #else
                return fs::path{ os::env::get("HOME") };
            #endif
        }
        // os::env: Get user shell.
        auto shell(qiew param = {})
        {
            #if defined(_WIN32)
            auto shell = "cmd"s;
            if (param.size())
            {
                shell += " /c ";
                shell += param;
            }
            #else
            auto shell = os::env::get("SHELL");
            if (shell.empty() || shell.ends_with("vtm"))
            {
                shell = "bash"; //todo request it from user if empty; or make it configurable
                log("%%Using '%shell%' as a fallback login shell", prompt::os, shell);
            }
            if (param.size())
            {
                shell += " -c ";
                shell += param;
            }
            #endif
            return shell;
        }
        // os::env: Get user name.
        auto user()
        {
            #if defined(_WIN32)

                auto buffer = wide{};
                auto length = DWORD{};
                ::GetUserNameW(buffer.data(), &length);
                buffer.resize(length);
                if(ok(::GetUserNameW(buffer.data(), &length), "::GetUserNameW()", os::unexpected))
                {
                    if (length && buffer.back() == 0) buffer.pop_back();
                    return utf::to_utf(buffer);
                }
                else return text{};

            #else

                uid_t id;
                id = ::geteuid();
                return utf::concat(id);

            #endif
        }
    }

    namespace clipboard
    {
        static constexpr auto ocs52head = "\033]52;"sv;
        #if defined(_WIN32)
            static auto sequence = std::numeric_limits<DWORD>::max();
            static auto mutex   = std::mutex();
            static auto cf_text = CF_UNICODETEXT;
            static auto cf_utf8 = CF_TEXT;
            static auto cf_rich = ::RegisterClipboardFormatA("Rich Text Format");
            static auto cf_html = ::RegisterClipboardFormatA("HTML Format");
            static auto cf_ansi = ::RegisterClipboardFormatA("ANSI/VT Format");
            static auto cf_sec1 = ::RegisterClipboardFormatA("ExcludeClipboardContentFromMonitorProcessing");
            static auto cf_sec2 = ::RegisterClipboardFormatA("CanIncludeInClipboardHistory");
            static auto cf_sec3 = ::RegisterClipboardFormatA("CanUploadToCloudClipboard");
        #endif

        auto set(view type, view utf8)
        {
            // Generate the following formats:
            //   mime::textonly | mime::disabled
            //        CF_UNICODETEXT: Raw UTF-16
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //   mime::ansitext
            //               cf_rich: RTF-group UTF-8
            //        CF_UNICODETEXT: ANSI-text UTF-16
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //   mime::richtext
            //               cf_rich: RTF-group UTF-8
            //        CF_UNICODETEXT: Plaintext UTF-16
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //   mime::htmltext
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //               cf_html: HTML-code UTF-8
            //        CF_UNICODETEXT: HTML-code UTF-16
            //   mime::safetext (https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats#cloud-clipboard-and-clipboard-history-formats)
            //    ExcludeClipboardContentFromMonitorProcessing: 1
            //                    CanIncludeInClipboardHistory: 0
            //                       CanUploadToCloudClipboard: 0
            //                                  CF_UNICODETEXT: Raw UTF-16
            //                                         cf_ansi: ANSI-text UTF-8 with mime mark
            //
            //  cf_ansi format: payload=mime_type/size_x/size_y;utf8_data

            auto success = faux;
            auto size = twod{ 80,25 };
            {
                auto i = 1;
                utf::divide<feed::rev>(type, '/', [&](auto frag)
                {
                    if (auto v = utf::to_int(frag)) size[i] = v.value();
                    return i--;
                });
            }

            #if defined(_WIN32)

                auto send = [&](auto cf_format, view data)
                {
                    auto _send = [&](auto const& data)
                    {
                        auto size = (data.size() + 1/*null terminator*/) * sizeof(*(data.data()));
                        if (auto gmem = ::GlobalAlloc(GMEM_MOVEABLE, size))
                        {
                            if (auto dest = ::GlobalLock(gmem))
                            {
                                std::memcpy(dest, data.data(), size);
                                ::GlobalUnlock(gmem);
                                ok(::SetClipboardData(cf_format, gmem) && (success = true), "::SetClipboardData()", os::unexpected, ", cf_format=", cf_format);
                            }
                            else log(prompt::os, "::GlobalLock()", os::unexpected);
                            ::GlobalFree(gmem);
                        }
                        else log(prompt::os, "::GlobalAlloc()", os::unexpected);
                    };
                    cf_format == cf_text ? _send(utf::to_utf(data))
                                         : _send(data);
                };

                auto lock = std::lock_guard{ os::clipboard::mutex };
                ok(::OpenClipboard(nullptr), "::OpenClipboard()", os::unexpected);
                ok(::EmptyClipboard(), "::EmptyClipboard()", os::unexpected);
                if (utf8.size())
                {
                    if (type.size() < 5 || type.starts_with(mime::tag::text))
                    {
                        send(cf_text, utf8);
                    }
                    else
                    {
                        auto post = page{ utf8 };
                        auto info = CONSOLE_FONT_INFOEX{ sizeof(CONSOLE_FONT_INFOEX) };
                        ::GetCurrentConsoleFontEx(os::stdout_fd, faux, &info);
                        auto font = utf::to_utf(info.FaceName);
                        if (type.starts_with(mime::tag::rich))
                        {
                            auto rich = post.to_rich(font);
                            auto utf8 = post.to_utf8();
                            send(cf_rich, rich);
                            send(cf_text, utf8);
                        }
                        else if (type.starts_with(mime::tag::html))
                        {
                            auto [html, code] = post.to_html(font);
                            send(cf_html, html);
                            send(cf_text, code);
                        }
                        else if (type.starts_with(mime::tag::ansi))
                        {
                            auto rich = post.to_rich(font);
                            send(cf_rich, rich);
                            send(cf_text, utf8);
                        }
                        else if (type.starts_with(mime::tag::safe))
                        {
                            send(cf_sec1, "1");
                            send(cf_sec2, "0");
                            send(cf_sec3, "0");
                            send(cf_text, utf8);
                        }
                        else
                        {
                            send(cf_utf8, utf8);
                        }
                    }
                    auto crop = ansi::add(type, ";", utf8);
                    send(cf_ansi, crop);
                }
                else
                {
                    success = true;
                }
                ok(::CloseClipboard(), "::CloseClipboard()", os::unexpected);
                os::clipboard::sequence = ::GetClipboardSequenceNumber(); // The sequence number is incremented while closing the clipboard.

            #elif defined(__APPLE__)

                auto send = [&](auto& data)
                {
                    if (auto fd = ::popen("/usr/bin/pbcopy", "w"))
                    {
                        ::fwrite(data.data(), data.size(), 1, fd);
                        ::pclose(fd);
                        success = true;
                    }
                };
                if (type.starts_with(mime::tag::rich))
                {
                    auto post = page{ utf8 };
                    auto rich = post.to_rich();
                    send(rich);
                }
                else if (type.starts_with(mime::tag::html))
                {
                    auto post = page{ utf8 };
                    auto [html, code] = post.to_html();
                    send(code);
                }
                else
                {
                    send(utf8);
                }

            #else

                auto yield = escx{};
                if (type.starts_with(mime::tag::rich))
                {
                    auto post = page{ utf8 };
                    auto rich = post.to_rich();
                    yield.clipbuf(size, rich, mime::richtext);
                }
                else if (type.starts_with(mime::tag::html))
                {
                    auto post = page{ utf8 };
                    auto [html, code] = post.to_html();
                    yield.clipbuf(size, code, mime::htmltext);
                }
                else if (type.starts_with(mime::tag::ansi)) //todo GH#216
                {
                    yield.clipbuf(size, utf8, mime::ansitext);
                }
                else
                {
                    yield.clipbuf(size, utf8, mime::textonly);
                }
                io::send(os::stdout_fd, yield);
                success = true;

                #if defined(__ANDROID__)

                    //todo implement

                #else

                    //todo implement X11 clipboard server

                #endif

            #endif
            return success;
        }
    }

    namespace ipc
    {
        struct memory
        {
            static auto get(view reference)
            {
                auto utf8 = text{};
                #if defined(_WIN32)

                    if (auto ref = utf::to_int<intptr_t, 0x10>(reference))
                    {
                        auto handle = (HANDLE)ref.value();
                        if (auto data = ::MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0))
                        {
                            utf8 = (char*)data;
                            ::UnmapViewOfFile(data);
                        }
                        os::close(handle);
                    }

                #endif
                return utf8;
            }
            static auto set(view data)
            {
                #if defined(_WIN32)

                    auto source = view{ data.data(), data.size() + 1/*trailing null*/ };
                    auto handle = ::CreateFileMappingA(os::invalid_fd, nullptr, PAGE_READWRITE, 0, (DWORD)source.size(), nullptr); ok(handle, "::CreateFileMappingA()", os::unexpected);
                    auto buffer = ::MapViewOfFile(handle, FILE_MAP_WRITE, 0, 0, 0);                                                ok(buffer, "::MapViewOfFile()", os::unexpected);
                    std::copy(std::begin(source), std::end(source), (char*)buffer);
                    ok(::UnmapViewOfFile(buffer), "::UnmapViewOfFile()", os::unexpected);
                    ok(::SetHandleInformation(handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT), "::SetHandleInformation()", os::unexpected);
                    return handle;

                #endif
            }
        };

        struct stdcon : pipe
        {
            sock handle; // ipc::stdcon: IO descriptor.
            text buffer; // ipc::stdcon: Receive buffer.
            flag inread; // ipc::stdcon: Reading is incomplete.

            stdcon()
                : pipe{ faux },
                  buffer(os::pipebuf, 0)
            { }
            stdcon(sock&& fd)
                : pipe{ true },
                  handle{ std::move(fd) },
                  buffer(os::pipebuf, 0)
            { }
            stdcon(fd_t r, fd_t w)
                : pipe{ true },
                  handle{ r, w },
                  buffer(os::pipebuf, 0)
            { }

            void start(fd_t fd)
            {
                pipe::start();
                handle = { fd, fd };
            }
            void cleanup()
            {
                pipe::cleanup();
                handle = {};
            }
            void operator = (stdcon&& p)
            {
                handle = std::move(p.handle);
                buffer = std::move(p.buffer);
                pipe::active.exchange(p.pipe::active);
                p.pipe::active = faux;
            }

            virtual bool send(view buff) override
            {
                pipe::isbusy = faux; // io::send blocks until the send is complete.
                return io::send(handle.w, buff);
            }
            virtual qiew recv(char* buff, size_t size) override
            {
                auto result = qiew{};
                inread.exchange(true);
                if (pipe::active) // The read call can be interrupted by io::abort().
                {
                    result = io::recv(handle, buff, size); // The read call can be interrupted by the write side when their read call is interrupted.
                }
                inread.exchange(faux);
                return result;
            }
            virtual qiew recv()  override // It's not thread safe!
            {
                return recv(buffer.data(), buffer.size());
            }
            void abort(auto& thread)
            {
                if (pipe::shut() && inread)
                {
                    io::abort(thread);
                }
            }
            virtual bool shut() override
            {
                auto state = pipe::shut();
                handle.shutdown(); // Close the writing handle to interrupt a reading call on the server side and trigger to close the server writing handle to interrupt owr reading call.
                return state;
            }
            virtual bool stop() override
            {
                return shut();
            }
            virtual flux& show(flux& s) const override
            {
                return s << handle;
            }
        };

        struct xcross
            : public stdcon
        {
            struct fifo
            {
                using lock = std::mutex;
                using sync = std::condition_variable;

                bool  alive; // fifo: .
                lock  mutex; // fifo: .
                sync  wsync; // fifo: .
                sync  rsync; // fifo: .
                text  store; // fifo: .
                flag& going; // fifo: Sending not completed.
                flag  fired; // fifo: Read interruptor.

                fifo(flag& busy)
                    : alive{ true },
                      going{ busy },
                      fired{ faux }
                { }

                auto send(view block)
                {
                    if (block.size())
                    {
                        auto guard = std::unique_lock{ mutex };
                        if (alive)
                        {
                            store += block;
                            wsync.notify_one();
                        }
                        return alive;
                    }
                    return faux;
                }
                auto read(text& yield)
                {
                    auto guard = std::unique_lock{ mutex };
                    wsync.wait(guard, [&]{ return store.size() || !alive || fired; });
                    if (fired)
                    {
                        fired = faux;
                    }
                    else if (alive)
                    {
                        std::swap(store, yield);
                        going = faux;
                        going.notify_all();
                        store.clear();
                        return qiew{ yield };
                    }
                    return qiew{};
                }
                void wake()
                {
                    fired = true;
                    wsync.notify_one();
                }
                void stop()
                {
                    auto guard = std::lock_guard{ mutex };
                    alive = faux;
                    going = faux;
                    going.notify_all();
                    wsync.notify_one();
                }
            };

            sptr<fifo>   client;
            sptr<fifo>   server;
            sptr<xcross> remote;

            xcross()
            { }
            xcross(sptr<xcross> remote)
                : client{ ptr::shared<fifo>(isbusy)         },
                  server{ ptr::shared<fifo>(remote->isbusy) },
                  remote{ remote }
            {
                remote->client = server;
                remote->server = client;
                remote->active = true;
                active = true;
            }

            void wake() override
            {
                server->wake();
            }
            qiew recv() override
            {
                return server->read(buffer);
            }
            bool send(view data) override
            {
                return client->send(data);
            }
            flux& show(flux& s) const override
            {
                return s << "local pipe: server=" << std::showbase << std::hex << server.get() << " client=" << std::showbase << std::hex << client.get();
            }
            bool shut() override
            {
                auto state = stdcon::shut();
                server->stop();
                client->stop();
                return state;
            }
        };

        struct socket
            : public stdcon
        {
            text scpath; // ipc:socket: Socket path (in order to unlink).
            fire signal; // ipc:socket: Interruptor.

            socket(sock& fd)
                : stdcon{ std::move(fd) }
            { }
            socket(fd_t r, fd_t w, text path)
                : stdcon{ r, w },
                  scpath{ path }
            { }
           ~socket()
            {
                #if defined(__BSD__)

                    if (scpath.length())
                    {
                        ::unlink(scpath.c_str()); // Cleanup file system unix domain socket.
                    }

                #endif
            }

            auto auth(view id) const // Check peer cred.
            {
                #if defined(_WIN32)

                    //Note: Named Pipes - default ACL used for a named pipe grant full control to the LocalSystem, admins, and the creator owner
                    //https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-security-and-access-rights

                #elif defined(__linux__)

                    auto cred = ucred{};
                    #ifdef __ANDROID__
                        auto size = socklen_t{ sizeof(cred) };
                    #else
                        auto size = unsigned{ sizeof(cred) };
                    #endif

                    if (!ok(::getsockopt(handle.r, SOL_SOCKET, SO_PEERCRED, &cred, &size), "::getsockopt(SOL_SOCKET)", os::unexpected))
                    {
                        return faux;
                    }

                    if (cred.uid && id != utf::concat(cred.uid))
                    {
                        fail(prompt::sock, "Foreign users are not allowed to the session");
                        return faux;
                    }

                    log(prompt::sock, "Creds from SO_PEERCRED:",
                      "\n      pid : ", cred.pid,
                      "\n      euid: ", cred.uid,
                      "\n      egid: ", cred.gid);

                #elif defined(__BSD__)

                    auto euid = uid_t{};
                    auto egid = gid_t{};

                    if (!ok(::getpeereid(handle.r, &euid, &egid), "::getpeereid()", os::unexpected))
                    {
                        return faux;
                    }

                    if (euid && id != utf::concat(euid))
                    {
                        fail(prompt::sock, "Foreign users are not allowed to the session");
                        return faux;
                    }

                    log(prompt::sock, "Creds from ::getpeereid():",
                      "\n      pid : ", id,
                      "\n      euid: ", euid,
                      "\n      egid: ", egid);

                #endif

                return true;
            }
            auto meet()
            {
                if constexpr (debugmode) log(prompt::xipc, "Active server side link ", handle);
                auto client = sptr<ipc::socket>{};
                #if defined(_WIN32)

                    auto to_server = os::rd_pipe_path + scpath;
                    auto to_client = os::wr_pipe_path + scpath;
                    auto next_link = [&](auto h, auto const& path, auto type)
                    {
                        auto next_waiting_point = os::invalid_fd;
                        auto connected = ::ConnectNamedPipe(h, NULL)
                            ? true
                            : (::GetLastError() == ERROR_PIPE_CONNECTED);

                        if (pipe::active && connected) // Recreate the waiting point for the next client.
                        {
                            next_waiting_point =
                                ::CreateNamedPipeW(utf::to_utf(path).c_str(),// pipe path
                                                   type,                     // read/write access
                                                   PIPE_TYPE_BYTE |          // message type pipe
                                                   PIPE_READMODE_BYTE |      // message-read mode
                                                   PIPE_WAIT,                // blocking mode
                                                   PIPE_UNLIMITED_INSTANCES, // max. instances
                                                   os::pipebuf,              // output buffer size
                                                   os::pipebuf,              // input buffer size
                                                   0,                        // client time-out
                                                   NULL);                    // DACL (pipe_acl)
                            // DACL: auto pipe_acl = security_descriptor(security_descriptor_string);
                            //       The ACLs in the default security descriptor for a named pipe grant full control to the
                            //       LocalSystem account, administrators, and the creator owner. They also grant read access to
                            //       members of the Everyone group and the anonymous account.
                            //       Without write access, the desktop will be inaccessible to non-owners.
                        }
                        else if (pipe::active) os::fail(prompt::meet, "Not active");

                        return next_waiting_point;
                    };

                    auto r = next_link(handle.r, to_server, PIPE_ACCESS_INBOUND);
                    if (r == os::invalid_fd)
                    {
                        if (pipe::active) os::fail(prompt::meet, "::CreateNamedPipe(r)", os::unexpected);
                    }
                    else
                    {
                        auto w = next_link(handle.w, to_client, PIPE_ACCESS_OUTBOUND);
                        if (w == os::invalid_fd)
                        {
                            ::CloseHandle(r);
                            if (pipe::active) os::fail(prompt::meet, "::CreateNamedPipe(w)", os::unexpected);
                        }
                        else
                        {
                            client = ptr::shared<ipc::socket>(handle);
                            handle = { r, w };
                        }
                    }

                #else

                    auto h_proc = [&]
                    {
                        auto h = ::accept(handle.r, 0, 0);
                        auto s = sock{ h, h };
                        if (s) client = ptr::shared<ipc::socket>(s);
                    };
                    auto f_proc = [&]
                    {
                        signal.flush();
                    };
                    io::select(handle.r, h_proc,
                               signal  , f_proc);

                #endif
                return client;
            }
            bool stop() override
            {
                auto state = pipe::stop();
                if (state)
                {
                    if constexpr (debugmode) log(prompt::xipc, "Closing server side link ", handle);
                    #if defined(_WIN32)
                        auto to_client = os::wr_pipe_path + scpath;
                        auto to_server = os::rd_pipe_path + scpath;
                        if (handle.w != os::invalid_fd) ::DeleteFileW(utf::to_utf(to_client).c_str()); // Interrupt ::ConnectNamedPipe(). Disconnection order does matter.
                        if (handle.r != os::invalid_fd) ::DeleteFileW(utf::to_utf(to_server).c_str()); // This may fail, but this is ok - it means the client is already disconnected.
                    #else
                        signal.reset();
                    #endif
                }
                return state;
            }
            bool shut() override
            {
                auto state = pipe::shut();
                if (state)
                {
                    if constexpr (debugmode) log(prompt::xipc, "Client disconnects: ", handle);
                    #if defined(_WIN32)
                        ::DisconnectNamedPipe(handle.w);
                        handle.shutdown(); // To trigger the read end to close.
                    #else
                        ok(::shutdown(handle.w, SHUT_RDWR), "::shutdown()", os::unexpected); // Further sends and receives are disallowed.
                        // An important conceptual reason to want to use shutdown:
                        //    To signal EOF to the peer and still be able
                        //    to receive pending data the peer sent.
                        //    "shutdown() doesn't actually close the file descriptor
                        //     — it just changes its usability.
                        // To free a socket descriptor, you need to use os::close().
                        // Note: .r == .w, it is a full duplex socket handle on POSIX.
                    #endif
                }
                return state;
            }
            template<role Role, bool Log = true, class P = noop>
            static auto open(text path, span retry_timeout = {}, P retry_proc = P())
            {
                auto r = os::invalid_fd;
                auto w = os::invalid_fd;
                auto socket = sptr<ipc::socket>{};
                auto try_start = [&](auto play, auto retry_proc)
                {
                    auto done = play();
                    if (!done)
                    {
                        if (!retry_proc())
                        {
                           if constexpr (Log) os::fail("Failed to start server");
                        }
                        else
                        {
                            auto stop = datetime::now() + retry_timeout;
                            do
                            {
                                std::this_thread::sleep_for(100ms);
                                done = play();
                            }
                            while (!done && stop > datetime::now());
                        }
                    }
                    return done;
                };

                #if defined(_WIN32)

                    auto to_server = os::rd_pipe_path + path;
                    auto to_client = os::wr_pipe_path + path;

                    if constexpr (Role == role::server)
                    {
                        auto test = [](text const& path, text prefix = "\\\\.\\pipe\\")
                        {
                            auto hits = faux;
                            auto next = WIN32_FIND_DATAW{};
                            auto name = utf::to_utf(path.substr(prefix.size()));
                            auto what = utf::to_utf(prefix + '*');
                            auto hndl = ::FindFirstFileW(what.c_str(), &next);
                            if (hndl != os::invalid_fd)
                            {
                                do hits = next.cFileName == name;
                                while (!hits && ::FindNextFileW(hndl, &next));

                                if (hits) log(prompt::path, path);
                                ::FindClose(hndl);
                            }
                            return hits;
                        };
                        if (test(to_server))
                        {
                            os::fail("Server already running");
                            return socket;
                        }

                        auto pipe = [](auto const& path, auto type)
                        {
                            return ::CreateNamedPipeW(utf::to_utf(path).c_str(),// pipe path
                                                      type,                     // read/write access
                                                      PIPE_TYPE_BYTE |          // message type pipe
                                                      PIPE_READMODE_BYTE |      // message-read mode
                                                      PIPE_WAIT,                // blocking mode
                                                      PIPE_UNLIMITED_INSTANCES, // max instances
                                                      os::pipebuf,              // output buffer size
                                                      os::pipebuf,              // input buffer size
                                                      0,                        // client time-out
                                                      NULL);                    // DACL
                        };

                        r = pipe(to_server, PIPE_ACCESS_INBOUND);
                        if (r == os::invalid_fd)
                        {
                            os::fail("::CreateNamedPipe(r)", os::unexpected);
                        }
                        else
                        {
                            w = pipe(to_client, PIPE_ACCESS_OUTBOUND);
                            if (w == os::invalid_fd)
                            {
                                os::fail("::CreateNamedPipe(w)", os::unexpected);
                                os::close(r);
                            }
                        }
                    }
                    else if constexpr (Role == role::client)
                    {
                        auto pipe = [](auto const& path, auto type)
                        {
                            return ::CreateFileW(utf::to_utf(path).c_str(),
                                                 type,
                                                 0,             // no sharing
                                                 NULL,          // default security attributes
                                                 OPEN_EXISTING, // opens existing pipe
                                                 0,             // default attributes
                                                 NULL);         // no template file
                        };
                        auto play = [&]
                        {
                            w = pipe(to_server, GENERIC_WRITE);
                            if (w == os::invalid_fd)
                            {
                                return faux;
                            }

                            r = pipe(to_client, GENERIC_READ);
                            if (r == os::invalid_fd)
                            {
                                os::close(w);
                                return faux;
                            }
                            return true;
                        };
                        if (!try_start(play, retry_proc))
                        {
                            if constexpr (Log) os::fail("Connection error");
                        }
                    }

                #else

                    auto addr = sockaddr_un{};
                    auto sun_path = addr.sun_path + 1; // Abstract namespace socket (begins with zero). The abstract socket namespace is a nonportable Linux extension.

                    #if defined(__BSD__)
                        //todo unify "/.config/vtm"
                        auto home = os::env::homepath() / ".config/vtm";
                        if (!fs::exists(home))
                        {
                            if constexpr (Log) log("%%Create home directory '%path%'", prompt::path, home.string());
                            auto ec = std::error_code{};
                            fs::create_directory(home, ec);
                            if (ec && Log) log("%%Directory '%path%' creation error %error%", prompt::path, home.string(), ec.value());
                        }
                        path = (home / path).string() + ".sock";
                        sun_path--; // File system unix domain socket.
                        if constexpr (Log) log(prompt::open, "File system socket ", path);
                    #endif

                    if (path.size() > sizeof(sockaddr_un::sun_path) - 2)
                    {
                        if constexpr (Log) os::fail("Unix socket path too long");
                    }
                    else if ((w = ::socket(AF_UNIX, SOCK_STREAM, 0)) == os::invalid_fd)
                    {
                        if constexpr (Log) os::fail("Unix socket opening error");
                    }
                    else
                    {
                        r = w;
                        addr.sun_family = AF_UNIX;
                        auto sock_addr_len = (socklen_t)(sizeof(addr) - (sizeof(sockaddr_un::sun_path) - path.size() - 1));
                        std::copy(path.begin(), path.end(), sun_path);

                        auto play = [&]
                        {
                            return -1 != ::connect(r, (struct sockaddr*)&addr, sock_addr_len);
                        };

                        if constexpr (Role == role::server)
                        {
                            #if defined(__BSD__)
                                if (fs::exists(path))
                                {
                                    if (play())
                                    {
                                        os::fail("Server already running");
                                        os::close(r);
                                    }
                                    else
                                    {
                                        log(prompt::path, "Removing filesystem socket file ", path);
                                        ::unlink(path.c_str()); // Cleanup file system socket.
                                    }
                                }
                            #endif

                            if (r != os::invalid_fd && ::bind(r, (struct sockaddr*)&addr, sock_addr_len) == -1)
                            {
                                os::fail("Unix socket binding error for ", path);
                                os::close(r);
                            }
                            else if (::listen(r, 5) == -1)
                            {
                                os::fail("Unix socket listening error for ", path);
                                os::close(r);
                            }
                        }
                        else if constexpr (Role == role::client)
                        {
                            path.clear(); // No need to unlink a file system socket on client disconnect.
                            if (!try_start(play, retry_proc))
                            {
                                if constexpr (Log) os::fail("Connection failed");
                                os::close(r);
                            }
                        }
                    }

                #endif
                if (r != os::invalid_fd && w != os::invalid_fd)
                {
                    socket = ptr::shared<ipc::socket>(r, w, path);
                }
                return socket;
            }
        };

        auto stdio()
        {
            return ptr::shared<ipc::stdcon>(os::stdin_fd, os::stdout_fd);
        }
        auto xlink()
        {
            auto a = ptr::shared<ipc::xcross>();
            auto b = ptr::shared<ipc::xcross>(a); // Queue entanglement for xlink.
            return std::pair{ a, b };
        }
    }

    namespace process
    {
        auto getid()
        {
            #if defined(_WIN32)
                auto id = static_cast<ui32>(::GetCurrentProcessId());
            #else
                auto id = static_cast<ui32>(::getpid());
            #endif
            return std::pair{ id, datetime::now() };
        }
        static auto id = process::getid();
        static auto arg0 = text{};

        struct args
        {
        private:
            using list = std::list<text>;
            using it_t = list::iterator;

            list data{};
            it_t iter{};

            // args: Recursive argument matching.
            template<class I>
            auto test(I&& item) { return faux; }
            // args: Recursive argument matching.
            template<class I, class T, class ...Args>
            auto test(I&& item, T&& sample, Args&&... args)
            {
                return item == sample || test(item, std::forward<Args>(args)...);
            }

        public:
            args(int argc, char** argv)
            {
                #if defined(_WIN32)
                    auto line = utf::to_utf(::GetCommandLineW());
                    data.splice(data.end(), split(line));
                #else
                    auto head = argv;
                    auto tail = argv + argc;
                    while (head != tail)
                    {
                        data.splice(data.end(), split(*head++));
                    }
                #endif
                if (data.size())
                {
                    process::arg0 = data.front();
                    data.pop_front();
                }
                reset();
            }
            // args: Split command line options into tokens.
            template<class T = list>
            static T split(view line)
            {
                auto args = T{};
                line = utf::trim(line);
                while (line.size())
                {
                    auto item = utf::get_token(line);
                    if (item.size()) args.emplace_back(item);
                }
                return args;
            }
            // args: Reset arg iterator to begin.
            void reset()
            {
                iter = data.begin();
            }
            // args: Return true if not the end.
            operator bool () const { return iter != data.end(); }
            // args: Test the current argument and step forward if met.
            template<class ...Args>
            auto match(Args&&... args)
            {
                auto result = iter != data.end() && test(*iter, std::forward<Args>(args)...);
                if (result) ++iter;
                return result;
            }
            // args: Return current argument and step forward.
            template<class ...Args>
            auto next()
            {
                return iter != data.end() ? view{ *iter++ }
                                        : view{};
            }
            // args: Return the rest of the command line arguments.
            auto rest()
            {
                auto crop = text{};
                if (iter != data.end())
                {
                    crop += *iter++;
                    while (iter != data.end())
                    {
                        crop.push_back(' ');
                        crop += *iter++;
                    }
                }
                return crop;
            }
        };

        template<bool NameOnly = faux>
        auto binary()
        {
            auto result = text{};
            #if defined(_WIN32)

                auto handle = ::GetCurrentProcess();
                auto buffer = wide(MAX_PATH, 0);
                while (buffer.size() <= 32768)
                {
                    auto length = ::GetModuleFileNameExW(handle,         // hProcess
                                                         NULL,           // hModule
                                                         buffer.data(),  // lpFilename
                                      static_cast<DWORD>(buffer.size()));// nSize
                    if (length == 0) break;
                    if (buffer.size() > length + 1)
                    {
                        result = utf::to_utf(buffer.data(), length);
                        break;
                    }
                    buffer.resize(buffer.size() << 1);
                }

            #elif defined(__linux__) || defined(__NetBSD__)

                auto path = ::realpath("/proc/self/exe", nullptr);
                if (path)
                {
                    result = text(path);
                    ::free(path);
                }

            #elif defined(__APPLE__)

                auto size = uint32_t{};
                if (-1 == ::_NSGetExecutablePath(nullptr, &size))
                {
                    auto buff = std::vector<char>(size);
                    if (0 == ::_NSGetExecutablePath(buff.data(), &size))
                    {
                        auto path = ::realpath(buff.data(), nullptr);
                        if (path)
                        {
                            result = text(path);
                            ::free(path);
                        }
                    }
                }

            #elif defined(__FreeBSD__)

                auto size = 0_sz;
                int  name[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
                if (::sysctl(name, std::size(name), nullptr, &size, nullptr, 0) == 0)
                {
                    auto buff = std::vector<char>(size);
                    if (::sysctl(name, std::size(name), buff.data(), &size, nullptr, 0) == 0)
                    {
                        result = text(buff.data(), size);
                    }
                }

            #endif
            #if not defined(_WIN32)

                if (result.empty())
                {
                          auto path = ::realpath("/proc/self/exe",        nullptr);
                    if (!path) path = ::realpath("/proc/curproc/file",    nullptr);
                    if (!path) path = ::realpath("/proc/self/path/a.out", nullptr);
                    if (path)
                    {
                        result = text(path);
                        ::free(path);
                    }
                }

            #endif
            if (result.empty())
            {
                os::fail("Can't get current module file path, fallback to '", process::arg0, "`");
                result = process::arg0;
            }
            if constexpr (NameOnly)
            {
                auto code = std::error_code{};
                auto file = fs::directory_entry(result, code);
                if (!code)
                {
                    result = file.path().filename().string();
                }
            }
            auto c = result.front();
            if (c != '\"' && c != '\'' && result.find(' ') != text::npos)
            {
                result = '\"' + result + '\"';
            }
            return result;
        }
        template<bool Fast = faux>
        void exit(int code)
        {
            if constexpr (Fast) ::_exit(code); // Skip atexit hooks and stdio buffer flushes.
            else
            {
                #if defined(_WIN32)
                ::ExitProcess(code);
                #else
                ::exit(code);
                #endif
            }
        }
        template<class ...Args>
        void exit(int code, Args&&... args)
        {
            log(args...);
            process::exit(code);
        }
        auto sysfork()
        {
            #if defined(_WIN32)
            #else

                auto lock = netxs::logger::globals();
                auto crop = ::fork();
                if (!crop)
                {
                    os::process::id = os::process::getid();
                }
                return crop;

            #endif
        }
        auto execvp(text cmdline)
        {
            #if defined(_WIN32)
            #else

                if (auto args = os::process::args::split(cmdline); args.size())
                {
                    auto& binary = args.front();
                    if (binary.size() > 2) // Remove quotes,
                    {
                        auto c = binary.front();
                        if (binary.back() == c && (c == '\"' || c == '\''))
                        {
                            binary = binary.substr(1, binary.size() - 2);
                        }
                    }
                    auto argv = std::vector<char*>{};
                    for (auto& c : args)
                    {
                        argv.push_back(c.data());
                    }
                    argv.push_back(nullptr);
                    ::execvp(argv.front(), argv.data());
                }

            #endif
        }
        //todo deprecated
        template<bool Logs = true, bool Daemon = faux>
        auto exec(text cmdline)
        {
            if constexpr (Logs) log(prompt::exec, "'", cmdline, "'");
            #if defined(_WIN32)
                
                auto shadow = view{ cmdline };
                auto binary = utf::to_utf(utf::get_token(shadow));
                auto params = utf::to_utf(shadow);
                auto ShExecInfo = ::SHELLEXECUTEINFOW{};
                ShExecInfo.cbSize = sizeof(::SHELLEXECUTEINFOW);
                ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
                ShExecInfo.hwnd = NULL;
                ShExecInfo.lpVerb = NULL;
                ShExecInfo.lpFile = binary.c_str();
                ShExecInfo.lpParameters = params.c_str();
                ShExecInfo.lpDirectory = NULL;
                ShExecInfo.nShow = 0;
                ShExecInfo.hInstApp = NULL;
                if (::ShellExecuteExW(&ShExecInfo)) return true;

            #else

                auto p_id = os::process::sysfork();
                if (p_id == 0) // Child branch.
                {
                    if constexpr (Daemon)
                    {
                        ::umask(0); // Set the file mode creation mask for child process (all access bits are set by default).
                        os::close(os::stdin_fd );
                        os::close(os::stdout_fd);
                        os::close(os::stderr_fd);
                    }
                    os::process::execvp(cmdline);
                    auto errcode = errno;
                    if constexpr (Logs) os::fail(prompt::exec, "Failed to spawn '", cmdline, "'");
                    os::process::exit<true>(errcode);
                }
                else if (p_id > 0) // Parent branch.
                {
                    auto stat = int{};
                    ::waitpid(p_id, &stat, 0); // Wait for the child to avoid zombies.
                    if (WIFEXITED(stat) && WEXITSTATUS(stat) == 0)
                    {
                        return true; // Child forked and exited successfully.
                    }
                }

            #endif
            if constexpr (Logs) os::fail(prompt::exec, "Failed to spawn '", cmdline, "'");
            return faux;
        }
        auto fork(bool& result, view prefix, view config)
        {
            result = faux;
            #if defined(_WIN32)

                auto handle = os::ipc::memory::set(config);
                auto cmdarg = utf::to_utf(utf::concat(os::process::binary(), " --onlylog", " -p ", prefix, " -c :", handle, " -s"));
                auto proinf = PROCESS_INFORMATION{};
                auto srtinf = STARTUPINFOEXW{ sizeof(STARTUPINFOEXW) };
                auto buffer = std::vector<byte>{};
                auto buflen = SIZE_T{ 0 };
                ::InitializeProcThreadAttributeList(nullptr, 1, 0, &buflen);
                result = buflen;
                buffer.resize(buflen);
                srtinf.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
                result = result && ::InitializeProcThreadAttributeList(srtinf.lpAttributeList, 1, 0, &buflen);
                result = result && ::UpdateProcThreadAttribute(srtinf.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, &handle, sizeof(handle), nullptr, nullptr);
                result = result && ::CreateProcessW(nullptr,                      // lpApplicationName
                                                    cmdarg.data(),                // lpCommandLine
                                                    nullptr,                      // lpProcessAttributes
                                                    nullptr,                      // lpThreadAttributes
                                                    TRUE,                         // bInheritHandles
                                                    DETACHED_PROCESS |            // dwCreationFlags
                                                    EXTENDED_STARTUPINFO_PRESENT, // override startupInfo type
                                                    nullptr,                      // lpEnvironment
                                                    nullptr,                      // lpCurrentDirectory
                                                    &srtinf.StartupInfo,          // lpStartupInfo
                                                    &proinf);                     // lpProcessInformation
                os::close(handle);
                if (result) // Success. The fork concept is not supported on Windows.
                {
                    os::close(proinf.hProcess);
                    os::close(proinf.hThread);
                }

            #else

                auto p_id = os::process::sysfork();
                if (p_id == 0) // Child process.
                {
                    p_id = os::process::sysfork(); // Second fork to detach process and avoid zombies.
                    if (p_id == 0) // GrandChild process.
                    {
                        ::setsid(); // Open new session and new process group in it.
                        ::umask(0); // Set the file mode creation mask for child process (all access bits are set by default).
                        os::close(os::stdin_fd ); // No stdio needed in daemon mode.
                        os::close(os::stdout_fd); //
                        os::close(os::stderr_fd); //
                        return true;
                    }
                    else if (p_id > 0) os::process::exit<true>(0); // Success.
                    else               os::process::exit<true>(1); // Fail.
                }
                else if (p_id > 0) // Parent branch. Reap the child process and leaving the grandchild process detached.
                {
                    auto stat = int{};
                    ::waitpid(p_id, &stat, 0);
                    result = WIFEXITED(stat) && WEXITSTATUS(stat) == 0;
                }

            #endif

            if (result) log(prompt::os, "Process forked");
            else   os::fail(prompt::os, "Can't fork process");

            return faux; // Parent branch.
        }
        void spawn(text cwd, text cmdline)
        {
            #if defined(_WIN32)
            #else

                if (cwd.size())
                {
                    auto err = std::error_code{};
                    fs::current_path(cwd, err);
                    if (err) log("%%Failed to change current working directory to '%cwd%', error code: %code%\n", prompt::os, cwd, err.value());
                }
                os::process::execvp(cmdline);
                auto err_code = os::error();
                log(ansi::bgc(reddk).fgc(whitelt).add("Process creation error ", err_code, " \n"
                                                      " cwd: ", cwd.empty() ? "not specified"s : cwd, " \n"
                                                      " cmd: ", cmdline, " ").nil());
                os::process::exit<true>(err_code);

            #endif
        }
    }

    namespace dtvt
    {
        static constexpr auto mouse   = 1 << 0;
        static constexpr auto onlylog = 1 << 5;
        //todo make 3-bit field for color mode
        static constexpr auto vtrgb   = 0;
        static constexpr auto nt16    = 1 << 1;
        static constexpr auto vt16    = 1 << 2;
        static constexpr auto vt256   = 1 << 3;
        static constexpr auto direct  = 1 << 4;
        static auto mode = vtrgb;
        static auto scroll = faux; // Viewport/scrollback selector for windows console.
        auto consize()
        {
            static constexpr auto winsz_fallback = twod{ 132, 60 };

            auto winsz = dot_00;
            #if defined(_WIN32)
                auto cinfo = CONSOLE_SCREEN_BUFFER_INFO{};
                if (ok(::GetConsoleScreenBufferInfo(os::stdout_fd, &cinfo), "::GetConsoleScreenBufferInfo", os::unexpected))
                {
                    nt::console::buffer = { cinfo.dwSize.X, cinfo.dwSize.Y };
                    winsz = dtvt::scroll ? nt::console::buffer
                                         : twod{ cinfo.srWindow.Right  - cinfo.srWindow.Left + 1,
                                                 cinfo.srWindow.Bottom - cinfo.srWindow.Top  + 1 };
                }
            #else
                auto size = ::winsize{};
                if (ok(::ioctl(os::stdout_fd, TIOCGWINSZ, &size), "::ioctl(os::stdout_fd, TIOCGWINSZ)", os::unexpected))
                {
                    winsz = { size.ws_col, size.ws_row };
                }
            #endif

            if (winsz == dot_00)
            {
                log("%%Fallback tty window size %defsize% (consider using 'ssh -tt ...')", prompt::tty, winsz_fallback);
                winsz = winsz_fallback;
            }
            return winsz;
        }
        static auto config = text{}; // dtvt: DirectVT configuration XML data.
        static auto backup = tios{}; // dtvt: Saved console state to restore at exit.
        static auto header = text{}; // dtvt: Extra read block from stdin.
        static auto win_sz = twod{}; // dtvt: Initial window size.
        static auto client = xipc{}; // dtvt: Internal IO link.
        static auto active = []      // dtvt: DirectVT mode is active.
        {
            auto active = faux;
            auto cfsize = sz_t{};
            #if defined(_WIN32)

                auto buffer = directvt::binary::marker{};
                auto length = DWORD{ 0 };
                auto haspty = FILE_TYPE_CHAR == ::GetFileType(os::stdin_fd);
                if (haspty)
                {
                    // ::WaitForMultipleObjects() does not work with pipes (DirectVT).
                    if (::PeekNamedPipe(os::stdin_fd, buffer.data(), (DWORD)buffer.size(), &length, NULL, NULL)
                     && length)
                    {
                        active = buffer.size() == length && buffer.get(cfsize, dtvt::win_sz);
                        if (active)
                        {
                            io::recv(os::stdin_fd, buffer);
                        }
                    }
                }
                else
                {
                    length = (DWORD)io::recv(os::stdin_fd, buffer).size();
                    active = buffer.size() == length && buffer.get(cfsize, dtvt::win_sz);
                }

            #else

                auto proc = [&](auto get)
                {
                    get(os::stdin_fd, [&]
                    {
                        auto buffer = directvt::binary::marker{};
                        auto header = io::recv(os::stdin_fd, buffer);
                        auto length = header.length();
                        if (length)
                        {
                            active = buffer.size() == length && buffer.get(cfsize, dtvt::win_sz);
                            if (!active)
                            {
                                dtvt::header = header; //todo use it when the reading thread starts
                            }
                        }
                    });
                };
                auto haspty = ::isatty(os::stdin_fd);
                haspty ? proc([&](auto ...args){ return io::select<true>(args...); })
                       : proc([&](auto ...args){ return io::select<faux>(args...); });

            #endif
            if (cfsize)
            {
                dtvt::config.resize(cfsize);
                auto buffer = dtvt::config.data();
                while (cfsize)
                {
                    if (auto crop = io::recv(os::stdin_fd, buffer, cfsize))
                    {
                        auto s = (sz_t)crop.size();
                        cfsize -= s;
                        buffer += s;
                    }
                    else
                    {
                        active = faux;
                        break;
                    }
                }
            }
            if (active) // If we in dtvt mode.
            {
                os::close(os::stderr_fd); // Close because not in use.
                #if not defined(_WIN32)
                fdscleanup(); // There are duplicate stdin/stdout handles among the leaked parent process handles, and this prevents them from being closed. Affected ssh, nc, ncat, socat.
                #endif
            }
            else
            {
                #if defined(_WIN32)

                    ok(::GetConsoleMode(os::stdout_fd, &dtvt::backup.omode), "::GetConsoleMode(os::stdout_fd)", os::unexpected);
                    ok(::GetConsoleMode(os::stdin_fd , &dtvt::backup.imode), "::GetConsoleMode(os::stdin_fd)", os::unexpected);
                    dtvt::backup.opage = ::GetConsoleOutputCP();
                    dtvt::backup.ipage = ::GetConsoleCP();
                    ok(::SetConsoleOutputCP(65001), "::SetConsoleOutputCP()", os::unexpected);
                    ok(::SetConsoleCP(65001), "::SetConsoleCP()", os::unexpected);
                    auto inpmode = DWORD{ nt::console::inmode::extended
                                        | nt::console::inmode::winsize
                                        | nt::console::inmode::quickedit };
                    ok(::SetConsoleMode(os::stdin_fd, inpmode), "::SetConsoleMode(os::stdin_fd)", os::unexpected);
                    auto outmode = DWORD{ nt::console::outmode::no_auto_cr
                                        | nt::console::outmode::wrap_at_eol
                                        | nt::console::outmode::preprocess
                                        | nt::console::outmode::vt };
                    if (!::SetConsoleMode(os::stdout_fd, outmode))
                    {
                        dtvt::mode |= dtvt::nt16; // Legacy console detected - nt::console::outmode::vt + no_auto_cr not supported.
                        outmode &= ~(nt::console::outmode::no_auto_cr | nt::console::outmode::vt);
                        ok(::SetConsoleMode(os::stdout_fd, outmode), "::SetConsoleMode(os::stdout_fd)", os::unexpected);
                        log(prompt::os, "16-color windows console");
                    }
                    auto size = DWORD{ os::pipebuf };
                    auto wstr = wide(size, 0);
                    ok(::GetConsoleTitleW(wstr.data(), size), "::GetConsoleTitleW()", os::unexpected);
                    dtvt::backup.title = wstr.data();
                    ok(::GetConsoleCursorInfo(os::stdout_fd, &dtvt::backup.caret), "::GetConsoleCursorInfo()", os::unexpected);

                #else

                    if (ok(::tcgetattr(os::stdin_fd, &dtvt::backup), "::tcgetattr(os::stdin_fd)", os::unexpected))
                    {
                        auto raw_mode = dtvt::backup;
                        ::cfmakeraw(&raw_mode);
                        ok(::tcsetattr(os::stdin_fd, TCSANOW, &raw_mode), "::tcsetattr(os::stdin_fd, TCSANOW)", os::unexpected);
                        os::vgafont();
                        io::send(os::stdout_fd, ansi::save_title());
                    }
                    else os::fail("Check you are using the proper tty device");

                #endif
                dtvt::win_sz = dtvt::consize();
                auto repair = []
                {
                    #if defined(_WIN32)
                        ok(::SetConsoleMode(os::stdout_fd,        dtvt::backup.omode), "::SetConsoleMode(omode)", os::unexpected);
                        ok(::SetConsoleMode(os::stdin_fd,         dtvt::backup.imode), "::SetConsoleMode(imode)", os::unexpected);
                        ok(::SetConsoleOutputCP(                  dtvt::backup.opage), "::SetConsoleOutputCP(opage)", os::unexpected);
                        ok(::SetConsoleCP(                        dtvt::backup.ipage), "::SetConsoleCP(ipage)", os::unexpected);
                        ok(::SetConsoleTitleW(                    dtvt::backup.title.c_str()), "::GetConsoleTitleW()", os::unexpected);
                        ok(::SetConsoleCursorInfo(os::stdout_fd, &dtvt::backup.caret), "::SetConsoleCursorInfo()", os::unexpected);
                    #else
                        ::tcsetattr(os::stdin_fd, TCSANOW, &dtvt::backup);
                        io::send(os::stdout_fd, ansi::load_title());
                    #endif
                };
                std::atexit(repair);
            }
            return active;
        }();
        static auto vtmode = []       // tty: VT mode bit set.
        {
            if (os::dtvt::active)
            {
                log(prompt::os, "DirectVT mode");
                mode |= dtvt::direct;
            }
            else
            {
                #if defined(__linux__)
                if (os::linux_console) mode |= dtvt::mouse;
                #endif
                if (auto term = os::env::get("TERM"); term.size())
                {
                    log(prompt::os, "Terminal type \"", term, "\"");

                    auto vt16colors = { // https://github.com//termstandard/colors
                        "ansi",
                        "linux",
                        "xterm-color",
                        "dvtm", //todo track: https://github.com/martanne/dvtm/issues/10
                        "fbcon",
                    };
                    auto vt256colors = {
                        "rxvt-unicode-256color",
                    };

                    if (term.ends_with("16color") || term.ends_with("16colour"))
                    {
                        mode |= dtvt::vt16;
                    }
                    else
                    {
                        for (auto& type : vt16colors)
                        {
                            if (term == type)
                            {
                                mode |= dtvt::vt16;
                                break;
                            }
                        }
                        if (!mode)
                        {
                            for (auto& type : vt256colors)
                            {
                                if (term == type)
                                {
                                    mode |= dtvt::vt256;
                                    break;
                                }
                            }
                        }
                    }

                    if (os::env::get("TERM_PROGRAM") == "Apple_Terminal")
                    {
                        log("%%macOS Apple Terminal detected", prompt::os);
                        if (!(mode & dtvt::vt16)) mode |= dtvt::vt256;
                    }
                    log(prompt::os, "Color mode: ", mode & dtvt::vt16  ? "16-color"
                                                  : mode & dtvt::vt256 ? "256-color"
                                                                       : "true-color");
                    log(prompt::os, "Mouse mode: ", mode & dtvt::mouse ? "console" : "vt-style");
                }
            }
            return mode;
        }();
        template<class T>
        auto str(T mode)
        {
            auto result = text{};
            if (mode)
            {
                if (mode & mouse  ) result += "mouse ";
                if (mode & nt16   ) result += "nt16 ";
                if (mode & vt16   ) result += "vt16 ";
                if (mode & vt256  ) result += "vt256 ";
                if (mode & direct ) result += "direct ";
                if (mode & onlylog) result += "onlylog ";
                if (result.size()) result.pop_back();
            }
            else result = "vtrgb";
            return result;
        }

        struct vtty
        {
            using s11n = directvt::binary::s11n;

            fd_t                    prochndl{ os::invalid_fd };
            flag                    attached{};
            ipc::stdcon             termlink{};
            std::thread             stdinput{};
            text                    writebuf{};
            std::mutex              writemtx{};
            std::condition_variable writesyn{};

            operator bool () { return attached; }

            void cleanup()
            {
                if constexpr (debugmode) log(prompt::dtvt, "Destructor started");
                if (attached.exchange(faux)) // Detach child process and forget.
                {
                    writesyn.notify_one(); // Interrupt writing thread.
                    termlink.abort(stdinput); // Interrupt reading thread.
                }
                if (stdinput.joinable())
                {
                    if constexpr (debugmode) log(prompt::dtvt, "Reading thread joining", ' ', utf::to_hex_0x(stdinput.get_id()));
                    stdinput.join();
                }
                if constexpr (debugmode) log(prompt::dtvt, "Destructor complete");
            }
            auto attach_process(text cwd, text cmdline, twod winsz, size_t config_size)
            {
                auto marker = directvt::binary::marker{ config_size, winsz };
                utf::change(cmdline, "\\\"", "'");
                log("%%New process '%cmdline%' at the %path%", prompt::dtvt, utf::debase(cmdline), cwd.empty() ? "current working directory"s : "'" + cwd + "'");
                auto onerror = [&]()
                {
                    log(prompt::dtvt, ansi::err("Process creation error", ' ', utf::to_hex_0x(os::error())),
                        "\r\n\tcwd: '", cwd, "'",
                        "\r\n\tcmd: '", cmdline, "'");
                };
                #if defined(_WIN32)

                    auto s_pipe_r = os::invalid_fd;
                    auto s_pipe_w = os::invalid_fd;
                    auto m_pipe_r = os::invalid_fd;
                    auto m_pipe_w = os::invalid_fd;
                    auto startinf = STARTUPINFOEXW{ sizeof(STARTUPINFOEXW) };
                    auto procsinf = PROCESS_INFORMATION{};
                    auto attrbuff = std::vector<byte>{};
                    auto attrsize = SIZE_T{ 0 };
                    auto stdhndls = std::array<HANDLE, 2>{};

                    auto tunnel = [&]
                    {
                        auto sa = SECURITY_ATTRIBUTES{};
                        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                        sa.lpSecurityDescriptor = NULL;
                        sa.bInheritHandle = TRUE;
                        if (::CreatePipe(&s_pipe_r, &m_pipe_w, &sa, 0)
                        && ::CreatePipe(&m_pipe_r, &s_pipe_w, &sa, 0))
                        {
                            io::send(m_pipe_w, marker);
                            startinf.StartupInfo.dwFlags    = STARTF_USESTDHANDLES;
                            startinf.StartupInfo.hStdInput  = s_pipe_r;
                            startinf.StartupInfo.hStdOutput = s_pipe_w;
                            return true;
                        }
                        else
                        {
                            os::close(m_pipe_w);
                            os::close(m_pipe_r);
                            os::close(s_pipe_w);
                            os::close(s_pipe_r);
                            return faux;
                        }
                    };
                    auto fillup = [&]
                    {
                        stdhndls[0] = s_pipe_r;
                        stdhndls[1] = s_pipe_w;
                        ::InitializeProcThreadAttributeList(nullptr, 1, 0, &attrsize);
                        attrbuff.resize(attrsize);
                        startinf.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrbuff.data());
                        if (::InitializeProcThreadAttributeList(startinf.lpAttributeList, 1, 0, &attrsize)
                         && ::UpdateProcThreadAttribute(startinf.lpAttributeList,
                                                        0,
                                                        PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                                        &stdhndls,
                                                        sizeof(stdhndls),
                                                        nullptr,
                                                        nullptr))
                        {
                            return true;
                        }
                        else return faux;
                    };
                    auto create = [&]
                    {
                        auto wide_cmdline = utf::to_utf(cmdline);
                        return ::CreateProcessW(nullptr,                             // lpApplicationName
                                                wide_cmdline.data(),                 // lpCommandLine
                                                nullptr,                             // lpProcessAttributes
                                                nullptr,                             // lpThreadAttributes
                                                TRUE,                                // bInheritHandles
                                                DETACHED_PROCESS |                   // create without attached console, dwCreationFlags
                                                EXTENDED_STARTUPINFO_PRESENT,        // override startupInfo type
                                                nullptr,                             // lpEnvironment
                                                cwd.size() ? utf::to_utf(cwd).c_str()// lpCurrentDirectory
                                                           : nullptr,
                                                &startinf.StartupInfo,               // lpStartupInfo (ptr to STARTUPINFO)
                                                &procsinf);                          // lpProcessInformation
                    };
                    auto result = tunnel() && fillup() && create();
                    if (result)
                    {
                        os::close( procsinf.hThread );
                        prochndl = procsinf.hProcess;
                    }
                    else
                    {
                        onerror();
                    }

                #else

                    fd_t to_server[2] = { os::invalid_fd, os::invalid_fd };
                    fd_t to_client[2] = { os::invalid_fd, os::invalid_fd };
                    ok(::pipe(to_server), "::pipe(to_server)", os::unexpected);
                    ok(::pipe(to_client), "::pipe(to_client)", os::unexpected);
                    auto s_pipe_r = to_client[0];
                    auto s_pipe_w = to_server[1];
                    auto m_pipe_r = to_server[0];
                    auto m_pipe_w = to_client[1];
                    io::send(m_pipe_w, marker);

                    auto p_id = os::process::sysfork(); // dtvt-app can be either a real dtvt-app or a proxy
                                                        // like SSH/netcat/inetd that forwards traffic from a real dtvt-app.
                    if (p_id == 0) // Child branch.
                    {
                        auto p_id = os::process::sysfork(); // Second fork to detach process and avoid zombies.
                        if (p_id == 0) // Grandchild process.
                        {
                            os::dtvt::active = true;
                            ::dup2(s_pipe_r, STDIN_FILENO);  os::stdin_fd  = STDIN_FILENO;
                            ::dup2(s_pipe_w, STDOUT_FILENO); os::stdout_fd = STDOUT_FILENO;
                            if (cwd.size())
                            {
                                auto err = std::error_code{};
                                fs::current_path(cwd, err);
                                if (err) log("%%%err%Failed to change current working directory to '%cwd%', error code: %code%%nil%", prompt::dtvt, ansi::err(), cwd, utf::to_hex_0x(err.value()), ansi::nil());
                                else     log("%%Change current working directory to '%cwd%'", prompt::dtvt, cwd);
                            }
                            os::fdscleanup();
                            os::signals::state.reset();
                            os::process::execvp(cmdline);
                            onerror();
                            os::process::exit<true>(0);
                        }
                        else if (p_id > 0) os::process::exit<true>(0); // Fast exit the child process and leave the grandchild process detached.
                        else
                        {
                            onerror();
                            os::process::exit<true>(1); // Something went wrong. Fast exit anyway.
                        }
                    }
                    else if (p_id > 0) // Parent branch. Reap the child process to avoid zombies.
                    {
                        auto stat = int{};
                        ::waitpid(p_id, &stat, 0); // Close zombie.
                        if (WIFEXITED(stat) && WEXITSTATUS(stat) != 0) onerror(); // Catch fast exit(1).
                    }
                    else onerror();

                #endif
                os::close(s_pipe_w); // Close inheritable handles to avoid deadlocking at process exit.
                os::close(s_pipe_r); // Only when all write handles to the pipe are closed, the ReadFile function returns zero.
                return ipc::stdcon{ m_pipe_r, m_pipe_w };
            }
            void writer()
            {
                if constexpr (debugmode) log(prompt::dtvt, "Writing thread started", ' ', utf::to_hex_0x(std::this_thread::get_id()));
                auto cache = text{};
                auto guard = std::unique_lock{ writemtx };
                while ((void)writesyn.wait(guard, [&]{ return writebuf.size() || !attached; }), attached)
                {
                    std::swap(cache, writebuf);
                    guard.unlock();
                    if (termlink.send(cache)) cache.clear();
                    else
                    {
                        if constexpr (debugmode) log(prompt::dtvt, "Unexpected disconnection");
                        if (attached.exchange(faux)) termlink.abort(stdinput); // Interrupt reading thread.
                        break;
                    }
                    guard.lock();
                }
                if constexpr (debugmode) log(prompt::dtvt, "Writing thread ended", ' ', utf::to_hex_0x(std::this_thread::get_id()));
            }
            void reader(auto receiver)
            {
                if constexpr (debugmode) log(prompt::dtvt, "Reading thread started", ' ', utf::to_hex_0x(std::this_thread::get_id()));
                directvt::binary::stream::reading_loop(termlink, receiver);
                if constexpr (debugmode) log(prompt::dtvt, "Reading thread ended", ' ', utf::to_hex_0x(std::this_thread::get_id()));
            }
            void start(text cwd, text cmdline, text config, twod winsz, auto receiver, auto shutdown)
            {
                stdinput = std::thread{[&, cwd, cmdline, config, winsz, receiver, shutdown]
                {
                    auto config_size = config.size();
                    termlink = attach_process(cwd, cmdline, winsz, config_size);
                    if (config_size)
                    {
                        auto guard = std::lock_guard{ writemtx };
                        writebuf = config + writebuf;
                    }
                    attached.exchange(!!termlink);
                    if (attached)
                    {
                        if constexpr (debugmode) log("%%DirectVT console created for process '%cmdline%'", prompt::dtvt, utf::debase(cmdline));
                        writesyn.notify_one(); // Flush temp buffer.
                        auto stdwrite = std::thread{[&] { writer(); }};
                        reader(receiver);
                        if (attached.exchange(faux)) writesyn.notify_one(); // Interrupt writing thread.
                        if constexpr (debugmode) log(prompt::dtvt, "Writing thread joining", ' ', utf::to_hex_0x(stdinput.get_id()));
                        stdwrite.join();
                        log("%%Process '%cmdline%' disconnected", prompt::dtvt, utf::debase(cmdline));
                        shutdown();
                    }
                }};
            }
            void output(view data)
            {
                auto guard = std::lock_guard{ writemtx };
                writebuf += data;
                if (attached) writesyn.notify_one();
            }
        };
    }

    namespace vt
    {
        #include "consrv.hpp"

        struct vtty
        {
            std::thread             stdwrite{};
            testy<twod>             termsize{};
            flag                    attached{};
            flag                    signaled{};
            escx                    writebuf{};
            std::mutex              writemtx{};
            std::condition_variable writesyn{};
            sptr<consrv>            termlink{};

            operator bool () { return attached; }

            void cleanup(bool io_log)
            {
                if (stdwrite.joinable())
                {
                    writesyn.notify_one();
                    if (io_log) log(prompt::vtty, "Writing thread joining", ' ', utf::to_hex_0x(stdwrite.get_id()));
                    stdwrite.join();
                }
                auto guard = std::lock_guard{ writemtx };
                writebuf = {};
                if (termlink) termlink->cleanup(io_log);
            }
            template<class Term>
            void attach_process(Term& terminal, text cwd, text cmdline, twod win_size)
            {
                utf::change(cmdline, "\\\"", "\"");
                if (terminal.io_log) log("%%New TTY of size %win_size%", prompt::vtty, win_size);
                                     log("%%New process '%cmdline%' at the %path%", prompt::vtty, utf::debase(cmdline), cwd.empty() ? "current working directory"s : "'" + cwd + "'");
                if (!termlink)
                {
                    termlink = consrv::create(terminal);
                }
                termsize(win_size);
                auto trailer = [&, cmdline]
                {
                    if (attached.exchange(faux))
                    {
                        auto exitcode = termlink->wait();
                        log("%%Process '%cmdline%' exited with code %code%", prompt::vtty, utf::debase(cmdline), utf::to_hex_0x(exitcode));
                        writesyn.notify_one(); // Interrupt writing thread.
                        if (!signaled.exchange(true))
                        {
                            terminal.onexit(exitcode); // Only if the process terminates on its own (not forced by sighup).
                        }
                    }
                };
                auto errcode = termlink->attach(terminal, win_size, cwd, cmdline, trailer);
                if (errcode)
                {
                    terminal.onexit(errcode, "Process creation error \r\n"s
                                             " cwd: "s + (cwd.empty() ? "not specified"s : cwd) + " \r\n"s
                                             " cmd: "s + cmdline + " "s);
                }
                attached.exchange(!errcode);
                writesyn.notify_one(); // Flush temp buffer.
            }
            template<class Term>
            void writer(Term& terminal)
            {
                auto guard = std::unique_lock{ writemtx };
                auto cache = text{};
                while ((void)writesyn.wait(guard, [&]{ return writebuf.size() || !attached; }), attached)
                {
                    std::swap(cache, writebuf);
                    guard.unlock();
                    if (terminal.io_log) log(prompt::cin, "\n\t", utf::change(ansi::hi(utf::debase(cache)), "\n", ansi::pushsgr().nil().add("\n\t").popsgr()));
                    if (termlink->send(cache)) cache.clear();
                    else
                    {
                        if (terminal.io_log) log(prompt::vtty, "Unexpected disconnection");
                        termlink->sighup(); //todo interrupt reading thread
                        break;
                    }
                    guard.lock();
                }
            }
            template<class Term>
            void start(Term& terminal, text cwd, text cmdline, twod win_size)
            {
                signaled.exchange(faux);
                stdwrite = std::thread{[&, cwd, cmdline, win_size]
                {
                    if (terminal.io_log) log(prompt::vtty, "Writing thread started", ' ', utf::to_hex_0x(stdwrite.get_id()));
                    attach_process(terminal, cwd, cmdline, win_size);
                    writer(terminal);
                    if (terminal.io_log) log(prompt::vtty, "Writing thread ended", ' ', utf::to_hex_0x(stdwrite.get_id()));
                }};
            }
            auto sighup()
            {
                if (attached && !signaled.exchange(true))
                {
                    termlink->sighup();
                    return true;
                }
                return faux;
            }
            void resize(twod const& newsize)
            {
                if (attached)
                {
                    if (termsize(newsize))
                    {
                        termlink->winsz(termsize);
                    }
                }
            }
            void reset()
            {
                if (termlink) termlink->reset();
            }
            void focus(bool state, input::focus::prot encod)
            {
                using prot = input::focus::prot;

                if (attached)
                {
                    if (encod == prot::w32) termlink->focus(state);
                    else
                    {
                        auto guard = std::lock_guard{ writemtx };
                        writebuf.fcs(state);
                        writesyn.notify_one();
                    }
                }
            }
            void keybd(input::hids& gear, bool decckm, bool bpmode, input::keybd::prot encod)
            {
                using prot = input::keybd::prot;

                if (attached)
                {
                    if (encod == prot::w32) termlink->keybd(gear, decckm, bpmode);
                    else
                    {
                        //todo generate vt from keycode
                        auto utf8 = gear.interpret();
                        if (!bpmode)
                        {
                            utf::change(utf8, "\033[200~", "");
                            utf::change(utf8, "\033[201~", "");
                        }
                        if (decckm)
                        {
                            utf::change(utf8, "\033[A",  "\033OA");
                            utf::change(utf8, "\033[B",  "\033OB");
                            utf::change(utf8, "\033[C",  "\033OC");
                            utf::change(utf8, "\033[D",  "\033OD");
                            utf::change(utf8, "\033[1A", "\033OA");
                            utf::change(utf8, "\033[1B", "\033OB");
                            utf::change(utf8, "\033[1C", "\033OC");
                            utf::change(utf8, "\033[1D", "\033OD");
                        }
                        auto guard = std::lock_guard{ writemtx };
                        writebuf += utf8;
                        writesyn.notify_one();
                    }
                }
            }
            void mouse(input::hids& gear, bool moved, twod const& coord, input::mouse::prot encod, input::mouse::mode state)
            {
                using mode = input::mouse::mode;
                using prot = input::mouse::prot;

                if (attached)
                {
                    if (encod == prot::w32) termlink->mouse(gear, moved, coord, encod, state);
                    else
                    {
                        if (state & mode::move
                        || (state & mode::drag && (gear.m.buttons && moved))
                        || (state & mode::bttn && (gear.m.buttons != gear.s.buttons || gear.m.wheeled)))
                        {
                            auto guard = std::lock_guard{ writemtx };
                                 if (encod == prot::sgr) writebuf.mouse_sgr(gear, coord);
                            else if (encod == prot::x11) writebuf.mouse_x11(gear, coord, state & mode::utf8);
                            writesyn.notify_one();
                        }
                    }
                }
            }
            void style(deco style, input::keybd::prot encod)
            {
                using prot = input::keybd::prot;

                if (attached)
                {
                    if (encod == prot::w32) termlink->style(style.format());
                    else
                    {
                        auto guard = std::lock_guard{ writemtx };
                        writebuf.style(style.format());
                        writesyn.notify_one();
                    }
                }
            }
            template<bool LFtoCR = true>
            void write(view data)
            {
                auto guard = std::lock_guard{ writemtx };
                if constexpr (LFtoCR) // Clipboard paste. The Return key should send a CR character.
                {
                    writebuf.reserve(writebuf.size() + data.size());
                    auto head = data.begin();
                    auto tail = data.end();
                    while (head != tail)
                    {
                        auto c = *head++;
                             if (c == '\n') c = '\r'; // LF -> CR.
                        else if (c == '\r' && head != tail && *head == '\n') head++; // CRLF -> CR.
                        writebuf.push_back(c);
                    }
                }
                else
                {
                    writebuf += data;
                }
                if (attached) writesyn.notify_one();
            }
            void undo(bool undoredo)
            {
                if (attached) termlink->undo(undoredo);
            }
        };
    }

    namespace runspace
    {
        template<class Term>
        struct base_tty
        {
            Term& terminal;

            base_tty(Term& terminal)
                : terminal{ terminal }
            { }

            virtual void write(view data) = 0;
            virtual void start(text cwd, text cmdline, twod winsz, std::function<void(view)> input_hndl,
                                                                   std::function<void(si32, view)> shutdown_hndl) = 0;
            virtual void shut() = 0;
            virtual bool connected() = 0;
        };
        template<class Term>
        struct raw : public base_tty<Term>
        {
            using base_tty = runspace::base_tty<Term>;

            ipc::stdcon               termlink;
            std::thread               stdinput;
            std::thread               stdwrite;
            std::thread               waitexit;
            pidt                      proc_pid;
            fd_t                      prochndl;
            text                      writebuf;
            std::mutex                writemtx;
            std::condition_variable   writesyn;
            std::function<void(view)> receiver;
            std::function<void(si32, view)> shutdown;

            raw(Term& terminal)
                : base_tty{ terminal       },
                  prochndl{ os::invalid_fd },
                  proc_pid{                }
            { }
           ~raw()
            {
                if constexpr (debugmode) log(prompt::task, "Destructor started");
                stop();
                if constexpr (debugmode) log(prompt::task, "Destructor complete");
            }

            operator bool () { return termlink; }

            virtual bool connected() override
            {
                return !!termlink;
            }
            void disconnect()
            {
                auto guard = std::lock_guard{ writemtx };
                termlink.stop();
            }
            // task: Cleaning in order to be able to restart.
            void cleanup()
            {
                if (stdwrite.joinable())
                {
                    writesyn.notify_one();
                    if constexpr (debugmode) log(prompt::task, "Writing thread joining", ' ', utf::to_hex_0x(stdwrite.get_id()));
                    stdwrite.join();
                }
                if (stdinput.joinable())
                {
                    if constexpr (debugmode) log(prompt::task, "Reading thread joining", ' ', utf::to_hex_0x(stdinput.get_id()));
                    stdinput.join();
                }
                if (waitexit.joinable())
                {
                    if constexpr (debugmode) log(prompt::task, "Process waiter joining", ' ', utf::to_hex_0x(waitexit.get_id()));
                    waitexit.join();
                }
                auto guard = std::lock_guard{ writemtx };
                termlink = {};
                writebuf = {};
            }
            virtual void shut() override
            {
                if (termlink)
                {
                    termlink.shut();
                }
            }
            auto wait_child()
            {
                disconnect();
                auto exit_code = 0;// os::process::wait(prompt::task, proc_pid, prochndl);
                return exit_code;
            }
            virtual void start(text cwd, text cmdline, twod winsz, std::function<void(view)> input_hndl,
                                                                   std::function<void(si32, view)> shutdown_hndl) override
            {
                receiver = input_hndl;
                shutdown = shutdown_hndl;
                utf::change(cmdline, "\\\"", "'");
                log("%%New process '%cmdline%' at the %cwd%", prompt::task, utf::debase(cmdline), cwd.empty() ? "current working directory"s
                                                                                                              : "'" + cwd + "'");
                #if defined(_WIN32)

                    auto s_pipe_r = os::invalid_fd;
                    auto s_pipe_w = os::invalid_fd;
                    auto m_pipe_r = os::invalid_fd;
                    auto m_pipe_w = os::invalid_fd;
                    auto startinf = STARTUPINFOEXW{ sizeof(STARTUPINFOEXW) };
                    auto procsinf = PROCESS_INFORMATION{};
                    auto attrbuff = std::vector<byte>{};
                    auto attrsize = SIZE_T{ 0 };
                    auto stdhndls = std::array<HANDLE, 2>{};

                    auto tunnel = [&]
                    {
                        auto sa = SECURITY_ATTRIBUTES{};
                        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                        sa.lpSecurityDescriptor = NULL;
                        sa.bInheritHandle = TRUE;
                        if (::CreatePipe(&s_pipe_r, &m_pipe_w, &sa, 0)
                         && ::CreatePipe(&m_pipe_r, &s_pipe_w, &sa, 0))
                        {
                            startinf.StartupInfo.dwFlags    = STARTF_USESTDHANDLES;
                            startinf.StartupInfo.hStdInput  = s_pipe_r;
                            startinf.StartupInfo.hStdOutput = s_pipe_w;
                            startinf.StartupInfo.hStdError  = s_pipe_w;
                            return true;
                        }
                        else
                        {
                            os::close(m_pipe_w);
                            os::close(m_pipe_r);
                            os::close(s_pipe_w);
                            os::close(s_pipe_r);
                            return faux;
                        }
                    };
                    auto fillup = [&]
                    {
                        stdhndls[0] = s_pipe_r;
                        stdhndls[1] = s_pipe_w;
                        ::InitializeProcThreadAttributeList(nullptr, 1, 0, &attrsize);
                        attrbuff.resize(attrsize);
                        startinf.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrbuff.data());

                        if (::InitializeProcThreadAttributeList(startinf.lpAttributeList, 1, 0, &attrsize)
                         && ::UpdateProcThreadAttribute(startinf.lpAttributeList,
                                                        0,
                                                        PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                                        &stdhndls,
                                                        sizeof(stdhndls),
                                                        nullptr,
                                                        nullptr))
                        {
                            return true;
                        }
                        else return faux;
                    };
                    auto create = [&]
                    {
                        auto wide_cmdline = utf::to_utf(cmdline);
                        return ::CreateProcessW(nullptr,                             // lpApplicationName
                                                wide_cmdline.data(),                 // lpCommandLine
                                                nullptr,                             // lpProcessAttributes
                                                nullptr,                             // lpThreadAttributes
                                                TRUE,                                // bInheritHandles
                                                DETACHED_PROCESS |                   // create without attached console, dwCreationFlags
                                                EXTENDED_STARTUPINFO_PRESENT,        // override startupInfo type
                                                nullptr,                             // lpEnvironment
                                                cwd.size() ? utf::to_utf(cwd).c_str()// lpCurrentDirectory
                                                           : nullptr,
                                                &startinf.StartupInfo,               // lpStartupInfo (ptr to STARTUPINFO)
                                                &procsinf);                          // lpProcessInformation
                    };

                    if (tunnel()
                     && fillup()
                     && create())
                    {
                        os::close( procsinf.hThread );
                        prochndl = procsinf.hProcess;
                        proc_pid = procsinf.dwProcessId;
                        termlink = { m_pipe_r, m_pipe_w };
                    }
                    else os::fail(prompt::task, "Process creation error");

                    os::close(s_pipe_w); // Close inheritable handles to avoid deadlocking at process exit.
                    os::close(s_pipe_r); // Only when all write handles to the pipe are closed, the ReadFile function returns zero.

                #else

                    fd_t to_server[2] = { os::invalid_fd, os::invalid_fd };
                    fd_t to_client[2] = { os::invalid_fd, os::invalid_fd };
                    ok(::pipe(to_server), "::pipe(to_server)", os::unexpected);
                    ok(::pipe(to_client), "::pipe(to_client)", os::unexpected);
                    termlink = { to_server[0], to_client[1] };
                    proc_pid = os::process::sysfork();
                    if (proc_pid == 0) // Child branch.
                    {
                        os::dtvt::active = faux; // Update tty::logger.
                        os::dtvt::client = {};   //
                        ::dup2(to_client[0], STDIN_FILENO);  os::stdin_fd  = STDIN_FILENO;
                        ::dup2(to_server[1], STDOUT_FILENO); os::stdout_fd = STDOUT_FILENO;
                        ::dup2(to_server[1], STDERR_FILENO); os::stderr_fd = STDERR_FILENO;
                        os::fdscleanup();
                        os::signals::state.reset();
                        os::process::spawn(cwd, cmdline);
                    }
                    // Parent branch.
                    os::close(to_client[0]);
                    os::close(to_server[1]);

                #endif

                stdinput = std::thread([&] { read_socket_thread(); });
                stdwrite = std::thread([&] { send_socket_thread(); });

                if (termlink) log(prompt::task, "Standard I/O has been redirected for process ", proc_pid);
            }
            void stop()
            {
                if (termlink)
                {
                    wait_child();
                }
                cleanup();
            }
            void read_socket_thread()
            {
                if constexpr (debugmode) log(prompt::task, "Reading thread started", ' ', utf::to_hex_0x(stdinput.get_id()));
                auto flow = text{};
                while (termlink)
                {
                    auto shot = termlink.recv();
                    if (shot && termlink)
                    {
                        flow += shot;
                        auto crop = view{ flow };
                        utf::purify(crop);
                        receiver(crop);
                        flow.erase(0, crop.size()); // Delete processed data.
                    }
                    else break;
                }
                if (termlink) // Skip if stop was called via dtor.
                {
                    auto exit_code = wait_child();
                    shutdown(exit_code, "");
                }
                if constexpr (debugmode) log(prompt::task, "Reading thread ended", ' ', utf::to_hex_0x(stdinput.get_id()));
            }
            void send_socket_thread()
            {
                if constexpr (debugmode) log(prompt::task, "Writing thread started", ' ', utf::to_hex_0x(stdwrite.get_id()));
                auto guard = std::unique_lock{ writemtx };
                auto cache = text{};
                while ((void)writesyn.wait(guard, [&]{ return writebuf.size() || !termlink; }), termlink)
                {
                    std::swap(cache, writebuf);
                    guard.unlock();
                    if (termlink.send(cache)) cache.clear();
                    else                      break;
                    guard.lock();
                }
                if constexpr (debugmode) log(prompt::task, "Writing thread ended", ' ', utf::to_hex_0x(stdwrite.get_id()));
            }
            virtual void write(view data) override
            {
                auto guard = std::lock_guard{ writemtx };
                writebuf += data;
                if (termlink) writesyn.notify_one();
            }
        };

        template<class Term>
        struct tty : public base_tty<Term>, vt::vtty
        {
            using vtty = vt::vtty;
            using base_tty = runspace::base_tty<Term>;

            virtual void write(view data) override
            {
                vtty::write(data);
            }
            virtual bool connected() override
            {
                return vtty::operator bool();
            }
            virtual void shut() override
            {
                vtty::sighup();
            }
            virtual void start(text cwd, text cmdline, twod winsz, std::function<void(view)> input_hndl,
                                                                   std::function<void(si32, view)> shutdown_hndl) override
            {
                vtty::start(base_tty::terminal, cwd, cmdline, winsz);
            }
            tty(Term& terminal)
                : base_tty{ terminal }
            { }
        };
    }

    namespace tty
    {
        static auto cout = std::function([](qiew utf8)
        {
            if (dtvt::vtmode & dtvt::nt16)
            {
                #if defined(_WIN32)
                static auto parser = nt::console::vtparser{};
                parser.cout(utf8);
                #endif
            }
            else io::send(utf8);
        });
        void title(view utf8)
        {
            #if defined(_WIN32)
                ::SetConsoleTitleW(utf::to_utf(utf8).c_str());
            #else
                io::send(ansi::header(utf8));
            #endif
            if constexpr (debugmode) log(prompt::tty, "Console title changed to ", ansi::hi(utf::debase<faux, faux>(utf8)));
        }
        static auto clipboard = text{};
        struct proxy : s11n
        {
            void direct(s11n::xs::bitmap_vt16      lock, view& data) { io::send(data); }
            void direct(s11n::xs::bitmap_vt256     lock, view& data) { io::send(data); }
            void direct(s11n::xs::bitmap_vtrgb     lock, view& data) { io::send(data); }
            void direct(s11n::xs::bitmap_dtvt      lock, view& data) // Decode for nt16 mode.
            {
                auto update = [](auto size, auto head, auto iter, auto tail)
                {
                    #if defined(_WIN32)
                    auto offset = (si32)(iter - head);
                    auto coor = twod{ offset % size.x, offset / size.x };
                    nt::console::print<svga::vt16>(size, coor, iter, tail);
                    #endif
                };
                auto& bitmap = lock.thing;
                bitmap.get(data, update);
            }
            void handle(s11n::xs::header_request   lock)
            {
                auto& header_request = lock.thing;
                auto header = s11n::header.freeze();
                header.thing.window_id = {};
                header.thing.sendby<faux, faux>(dtvt::client);
            }
            void handle(s11n::xs::footer_request   lock)
            {
                auto& footer_request = lock.thing;
                auto footer = s11n::footer.freeze();
                footer.thing.sendby<faux, faux>(dtvt::client);
            }
            void handle(s11n::xs::footer           lock)
            {
                auto& footer = lock.thing;
            }
            void handle(s11n::xs::header           lock)
            {
                auto& utf8 = lock.thing.utf8;
                if (utf8.length())
                {
                    auto filtered = para{ utf8 }.lyric->utf8();
                    tty::title(filtered);
                }
            }
            void handle(s11n::xs::clipdata         lock)
            {
                auto& clipdata = lock.thing;
                if (!clipdata.size && clipdata.utf8.size()) clipdata.size = dtvt::win_sz / 2; // Set the default size if no size is specified.
                auto metadata = mime::meta(clipdata.size, clipdata.form);
                os::clipboard::set(metadata, clipdata.utf8);
                s11n::sysboard.send(dtvt::client, id_t{}, clipdata.size, clipdata.utf8, clipdata.form);
                clipdata.set();
            }
            void handle(s11n::xs::clipdata_request lock)
            {
                auto& clipdata_request = lock.thing;
                auto clipdata = s11n::clipdata.freeze();
                if (clipdata.thing.hash != clipdata_request.hash)
                {
                    clipdata.thing.sendby<faux, faux>(dtvt::client);
                }
                else // Send without payload if hash the same.
                {
                    auto temp = std::move(clipdata.thing.utf8);
                    clipdata.thing.set();
                    clipdata.thing.sendby<faux, faux>(dtvt::client);
                    clipdata.thing.utf8 = std::move(temp);
                    clipdata.thing.set();
                }
            }

            proxy()
                : s11n{ *this }
            {
                //s11n::header.set(id_t{}, tty::title());
            }
        };
        static auto stream = proxy{}; // tty: Serialization proxy.
        auto logger()
        {
            static auto dtvt_output = [](auto& data){ io::send(os::stdout_fd, data); };
            return netxs::logger::attach([](qiew utf8)
            {
                if (utf8.empty()) return;
                if (dtvt::active || dtvt::client)
                {
                    auto lock = tty::stream.logs.freeze();
                    lock.thing.set(os::process::id.first, os::process::id.second, utf8);
                    dtvt::active ? lock.thing.sendfx(dtvt_output)   // Send logs to the dtvt-app hoster.
                                 : lock.thing.sendby(dtvt::client); // Send logs to the dtvt-app.
                }
                else if (os::is_daemon())
                {
                    #if defined(_WIN32)
                        //todo implement
                    #else
                        //todo it's too chatty
                        //auto copy = utf8.str();
                        //::syslog(LOG_NOTICE, "%s", copy.c_str());
                    #endif
                }
                else
                {
                    tty::cout(utf8);
                }
            });
        }
        void direct()
        {
            auto  stdio = os::ipc::stdio();
            auto& extio = *stdio;
            auto& intio = *dtvt::client;
            auto  input = std::thread{ [&]
            {
                while (extio && extio.send(intio.recv())) { }
                extio.shut();
            }};
            //todo forward signals to intio
            while (intio && intio.send(extio.recv())) { }

            //todo wait extio reconnection
            //extio.shut();
            //while (true)
            //{
            //    std::this_thread::sleep_for(1s);
            //}

            intio.shut();
            input.join();
        }
        void reader(auto& alarm, auto keybd, auto mouse, auto winsz, auto focus, auto paste, auto close, auto style)
        {
            if constexpr (debugmode) log(prompt::tty, "Reading thread started", ' ', utf::to_hex_0x(std::this_thread::get_id()));
            auto alive = true;
            auto m = input::sysmouse{};
            auto k = input::syskeybd{};
            auto f = input::sysfocus{};
            auto c = input::sysclose{};
            auto p = input::syspaste{};
            auto w = input::syswinsz{};
            m.enabled = input::hids::stat::ok;
            m.coordxy = { si16min, si16min };
            c.fast = true;
            f.state = true;
            w.winsize = os::dtvt::win_sz;
            focus(f);

            #if defined(_WIN32)

                auto reply = std::vector<INPUT_RECORD>(1);
                auto count = DWORD{};
                auto point = utfx{};
                auto toutf = text{};
                auto kbmod = si32{};
                auto cinfo = CONSOLE_SCREEN_BUFFER_INFO{};
                auto check = [](auto& changed, auto& oldval, auto newval)
                {
                    if (oldval != newval)
                    {
                        changed++;
                        oldval = newval;
                    }
                };
                fd_t waits[] = { os::stdin_fd, alarm };
                while (alive && WAIT_OBJECT_0 == ::WaitForMultipleObjects(2, waits, FALSE, INFINITE))
                {
                    if (!::GetNumberOfConsoleInputEvents(os::stdin_fd, &count))
                    {
                        break;
                    }
                    else if (count)
                    {
                        if (count > reply.size()) reply.resize(count);
                        if (!::ReadConsoleInputW(os::stdin_fd, reply.data(), (DWORD)reply.size(), &count))
                        {
                            break;
                        }
                        else
                        {
                            auto entry = reply.begin();
                            auto limit = entry + count;
                            while (alive && entry != limit)
                            {
                                auto& r = *entry++;
                                switch (r.EventType)
                                {
                                    case KEY_EVENT:
                                        if (os::nt::kbstate(kbmod, r.Event.KeyEvent.dwControlKeyState, r.Event.KeyEvent.wVirtualScanCode, r.Event.KeyEvent.bKeyDown))
                                        {
                                            k.ctlstat = kbmod;
                                            m.ctlstat = kbmod;
                                            m.doubled = faux;
                                            m.doubled = faux;
                                            m.wheeled = faux;
                                            m.hzwheel = faux;
                                            m.wheeldt = 0;
                                            m.changed++;
                                            mouse(m); // Fire mouse event to update kb modifiers.
                                        }
                                        if (utf::tocode(r.Event.KeyEvent.uChar.UnicodeChar, point))
                                        {
                                            if (point) utf::to_utf_from_code(point, toutf);
                                            k.extflag = r.Event.KeyEvent.dwControlKeyState & ENHANCED_KEY;
                                            k.virtcod = r.Event.KeyEvent.wVirtualKeyCode;
                                            k.scancod = r.Event.KeyEvent.wVirtualScanCode;
                                            k.pressed = r.Event.KeyEvent.bKeyDown;
                                            k.keycode = input::key::xlat(r.Event.KeyEvent.wVirtualKeyCode, r.Event.KeyEvent.wVirtualScanCode, r.Event.KeyEvent.dwControlKeyState);
                                            k.cluster = toutf;
                                            do
                                            {
                                                keybd(k);
                                            }
                                            while (r.Event.KeyEvent.wRepeatCount-- > 1);
                                        }
                                        else if (std::distance(entry, limit) > 2) // Surrogate pairs special case.
                                        {
                                            auto& dn_1 = r;
                                            auto& up_1 = *entry;
                                            auto& dn_2 = *(entry + 1);
                                            auto& up_2 = *(entry + 2);
                                            if (dn_1.Event.KeyEvent.uChar.UnicodeChar == up_1.Event.KeyEvent.uChar.UnicodeChar && dn_1.Event.KeyEvent.bKeyDown != 0 && up_1.Event.KeyEvent.bKeyDown == 0
                                             && dn_2.Event.KeyEvent.uChar.UnicodeChar == up_2.Event.KeyEvent.uChar.UnicodeChar && dn_2.Event.KeyEvent.bKeyDown != 0 && up_2.Event.KeyEvent.bKeyDown == 0
                                             && utf::tocode(up_2.Event.KeyEvent.uChar.UnicodeChar, point))
                                            {
                                                entry += 3;
                                                utf::to_utf_from_code(point, toutf);
                                                k.extflag = r.Event.KeyEvent.dwControlKeyState & ENHANCED_KEY;
                                                k.virtcod = r.Event.KeyEvent.wVirtualKeyCode;
                                                k.scancod = r.Event.KeyEvent.wVirtualScanCode;
                                                k.cluster = toutf;
                                                k.keycode = input::key::xlat(r.Event.KeyEvent.wVirtualKeyCode, r.Event.KeyEvent.wVirtualScanCode, r.Event.KeyEvent.dwControlKeyState);
                                                do
                                                {
                                                    k.pressed = true;
                                                    keybd(k);
                                                    k.pressed = faux;
                                                    keybd(k);
                                                }
                                                while (r.Event.KeyEvent.wRepeatCount-- > 1);
                                            }
                                        }
                                        point = {};
                                        toutf.clear();
                                        break;
                                    case MENU_EVENT: // Forward console control events.
                                        if (r.Event.MenuEvent.dwCommandId & nt::console::event::custom)
                                        switch (r.Event.MenuEvent.dwCommandId ^ nt::console::event::custom)
                                        {
                                            case nt::console::event::ctrl_c:
                                                k.extflag = faux;
                                                k.virtcod = 'C';
                                                k.scancod = ::MapVirtualKeyW('C', MAPVK_VK_TO_VSC);
                                                k.pressed = true;
                                                k.keycode = input::key::KeyC;
                                                k.cluster = "\x03";
                                                keybd(k);
                                                break;
                                            case nt::console::event::ctrl_break:
                                                k.extflag = faux;
                                                k.virtcod = ansi::c0_etx;
                                                k.scancod = ansi::ctrl_break;
                                                k.pressed = true;
                                                k.keycode = input::key::Break;
                                                k.cluster = "\x03";
                                                keybd(k);
                                                break;
                                            case nt::console::event::close:
                                            case nt::console::event::logoff:
                                            case nt::console::event::shutdown:
                                                alive = faux;;
                                                break;
                                            case nt::console::event::style:
                                                if (entry != limit && entry->EventType == MENU_EVENT)
                                                {
                                                    auto r = *entry++;
                                                    style(deco{ r.Event.MenuEvent.dwCommandId });
                                                }
                                                break;
                                            //todo
                                            //case PASTE_BEGIN:
                                            //    break;
                                            //case PASTE_END:
                                            //    break;
                                        }
                                        break;
                                    case MOUSE_EVENT:
                                    {
                                        os::nt::kbstate(kbmod, r.Event.MouseEvent.dwControlKeyState);
                                        auto changed = 0;
                                        check(changed, m.ctlstat, kbmod);
                                        check(changed, m.buttons, r.Event.MouseEvent.dwButtonState & 0b00011111);
                                        check(changed, m.doubled, !!(r.Event.MouseEvent.dwEventFlags & DOUBLE_CLICK));
                                        check(changed, m.wheeled, !!(r.Event.MouseEvent.dwEventFlags & MOUSE_WHEELED));
                                        check(changed, m.hzwheel, !!(r.Event.MouseEvent.dwEventFlags & MOUSE_HWHEELED));
                                        check(changed, m.wheeldt, static_cast<int16_t>((0xFFFF0000 & r.Event.MouseEvent.dwButtonState) >> 16)); // dwButtonState too large when mouse scrolls
                                        if (!(dtvt::vtmode & dtvt::nt16 && m.wheeldt)) // Skip the mouse coord update when wheeling on win7/8 (broken coords).
                                        {
                                            check(changed, m.coordxy, twod{ r.Event.MouseEvent.dwMousePosition.X, r.Event.MouseEvent.dwMousePosition.Y });
                                        }
                                        if (changed || m.wheeled || m.hzwheel) // Don't fire the same state (conhost fires the same events every second).
                                        {
                                            m.changed++;
                                            mouse(m); // Fire mouse event to update kb modifiers.
                                        }
                                        break;
                                    }
                                    case WINDOW_BUFFER_SIZE_EVENT:
                                    {
                                        auto changed = 0;
                                        check(changed, w.winsize, dtvt::consize());
                                        if (changed) winsz(w);
                                        break;
                                    }
                                    case FOCUS_EVENT:
                                        f.state = r.Event.FocusEvent.bSetFocus;
                                        focus(f);
                                        if (!f.state) kbmod = {}; // To keep the modifiers from sticking.
                                        break;
                                }
                            }
                        }
                    }
                }

            #else

                auto micefd = os::invalid_fd;
                auto buffer = text(os::pipebuf, '\0');
                auto ttynum = si32{ 0 };
                auto sig_fd = os::signals::fd{};

                auto get_kb_state = []
                {
                    auto state = si32{ 0 };
                    #if defined(__linux__)
                        auto shift_state = si32{ 6 /*TIOCL_GETSHIFTSTATE*/ };
                        ok(::ioctl(os::stdin_fd, TIOCLINUX, &shift_state), "::ioctl(os::stdin_fd, TIOCLINUX)", os::unexpected);
                        auto lalt   = shift_state & (1 << KG_ALT   );
                        auto ralt   = shift_state & (1 << KG_ALTGR );
                        auto ctrl   = shift_state & (1 << KG_CTRL  );
                        auto rctrl  = shift_state & (1 << KG_CTRLR );
                        auto lctrl  = shift_state & (1 << KG_CTRLL ) || (!rctrl && ctrl);
                        auto shift  = shift_state & (1 << KG_SHIFT );
                        auto rshift = shift_state & (1 << KG_SHIFTR);
                        auto lshift = shift_state & (1 << KG_SHIFTL) || (!rshift && shift);
                        if (lalt  ) state |= input::hids::LAlt;
                        if (ralt  ) state |= input::hids::RAlt;
                        if (lctrl ) state |= input::hids::LCtrl;
                        if (rctrl ) state |= input::hids::RCtrl;
                        if (lshift) state |= input::hids::LShift;
                        if (rshift) state |= input::hids::RShift;
                        auto led_state = si32{};
                        ok(::ioctl(os::stdin_fd, KDGKBLED, &led_state), "::ioctl(os::stdin_fd, KDGKBLED)", os::unexpected);
                        // CapsLock can always be 0 due to poorly coded drivers.
                        if (led_state & LED_NUM) state |= input::hids::NumLock;
                        if (led_state & LED_CAP) state |= input::hids::CapsLock;
                        if (led_state & LED_SCR) state |= input::hids::ScrlLock;
                    #endif
                    return state;
                };
                ok(::ttyname_r(os::stdout_fd, buffer.data(), buffer.size()), "::ttyname_r(os::stdout_fd)", os::unexpected);
                auto tty_name = view(buffer.data());
                if (!os::linux_console)
                {
                    log(prompt::tty, "Pseudoterminal ", tty_name);
                }
                else // Trying to get direct access to a PS/2 mouse.
                {
                    log(prompt::tty, "Linux console ", tty_name);
                    auto imps2_init_string = "\xf3\xc8\xf3\x64\xf3\x50"sv;
                    auto mouse_device = "/dev/input/mice";
                    auto mouse_fallback1 = "/dev/input/mice.vtm";
                    auto mouse_fallback2 = "/dev/input/mice_vtm"; //todo deprecated
                    auto fd = ::open(mouse_device, O_RDWR);
                    if (fd == -1) fd = ::open(mouse_fallback1, O_RDWR);
                    if (fd == -1) log("%%Error opening %mouse_device% and % mouse_fallback%, error %code%%desc%", prompt::tty, mouse_device, mouse_fallback1, errno, errno == 13 ? " - permission denied" : "");
                    if (fd == -1) fd = ::open(mouse_fallback2, O_RDWR);
                    if (fd == -1) log("%%Error opening %mouse_device% and % mouse_fallback%, error %code%%desc%", prompt::tty, mouse_device, mouse_fallback2, errno, errno == 13 ? " - permission denied" : "");
                    else if (io::send(fd, imps2_init_string))
                    {
                        auto ack = char{};
                        io::recv(fd, &ack, sizeof(ack));
                        micefd = fd;
                        auto tty_word = tty_name.find("tty", 0);
                        if (tty_word != text::npos)
                        {
                            auto tty_number = tty_name.substr(tty_word + 3/*skip tty letters*/);
                            if (auto cur_tty = utf::to_int(tty_number))
                            {
                                ttynum = cur_tty.value();
                            }
                        }
                        if (ack == '\xfa') log(prompt::tty, "ImPS/2 mouse connected");
                        else               log(prompt::tty, "Unknown PS/2 mouse connected, ack: ", utf::to_hex_0x((int)ack));
                    }
                    else
                    {
                        log(prompt::tty, "No PS/2 mouse detected");
                        os::close(fd);
                    }
                }

                // The following sequences are processed here:
                // ESC
                // ESC ESC
                // ESC [ I
                // ESC [ O
                // ESC [ < 0 ; x ; y M/m
                // ESC [ 33 : format p
                auto filter = [&, total = text{}](view accum) mutable
                {
                    if (os::linux_console && accum.starts_with("\033["sv)) // Replace Linux console specific keys.
                    {
                             if (accum == "\033[[A"sv ) total += "\033OP"sv;     // F1
                        else if (accum == "\033[[B"sv ) total += "\033OQ"sv;     // F2
                        else if (accum == "\033[[C"sv ) total += "\033OR"sv;     // F3
                        else if (accum == "\033[[D"sv ) total += "\033OS"sv;     // F4
                        else if (accum == "\033[[E"sv ) total += "\033[15~"sv;   // F5
                        else if (accum == "\033[25~"sv) total += "\033[1;2P"sv;  // Shift+F1
                        else if (accum == "\033[26~"sv) total += "\033[1;2Q"sv;  // Shift+F2
                        else if (accum == "\033[28~"sv) total += "\033[1;2R"sv;  // Shift+F3
                        else if (accum == "\033[29~"sv) total += "\033[1;2S"sv;  // Shift+F4
                        else if (accum == "\033[31~"sv) total += "\033[15;2~"sv; // Shift+F5
                        else if (accum == "\033[32~"sv) total += "\033[17;2~"sv; // Shift+F6
                        else if (accum == "\033[33~"sv) total += "\033[18;2~"sv; // Shift+F7
                        else if (accum == "\033[34~"sv) total += "\033[19;2~"sv; // Shift+F8
                        else total += accum;
                    }
                    else total += accum;
                    auto strv = view{ total };

                    //#ifndef PROD
                    //if (close)
                    //{
                    //    close = faux;
                    //    notify(e2::conio::preclose, close);
                    //    if (total.front() == '\033') // two consecutive escapes
                    //    {
                    //        log("\t - two consecutive escapes: \n\tstrv:        ", strv);
                    //        notify(e2::conio::quit, 0);
                    //        return;
                    //    }
                    //}
                    //#endif

                    //todo unify (it is just a proof of concept)
                    while (auto len = strv.size())
                    {
                        auto pos = 0_sz;
                        auto unk = true;

                        if (strv.at(0) == '\033')
                        {
                            ++pos;

                            //#ifndef PROD
                            //if (pos == len) // the only one esc
                            //{
                            //    close = true;
                            //    total = strv;
                            //    log("\t - preclose: ", canal);
                            //    notify(e2::conio::preclose, close);
                            //    break;
                            //}
                            //else if (strv.at(pos) == '\033') // two consecutive escapes
                            //{
                            //    total.clear();
                            //    log("\t - two consecutive escapes: ", canal);
                            //    notify(e2::conio::quit,0);
                            //    break;
                            //}
                            //#else
                            if (pos == len) // the only one esc
                            {
                                // Pass Esc.
                                k.pressed = true;
                                k.cluster = strv.substr(0, 1);
                                keybd(k);
                                total.clear();
                                break;
                            }
                            else if (strv.at(pos) == '\033') // two consecutive escapes
                            {
                                // Pass Esc.
                                k.pressed = true;
                                k.cluster = strv.substr(0, 1);
                                keybd(k);
                                total = strv.substr(1);
                                break;
                            }
                            //#endif
                            else if (strv.at(pos) == '[')
                            {
                                if (++pos == len) { total = strv; break; } // incomlpete
                                if (strv.at(pos) == 'I')
                                {
                                    f.state = true;
                                    focus(f);
                                    ++pos;
                                    unk = faux;
                                }
                                else if (strv.at(pos) == 'O')
                                {
                                    f.state = faux;
                                    focus(f);
                                    ++pos;
                                    unk = faux;
                                }
                                else if (strv.at(pos) == '<') // \033[<0;x;yM/m
                                {
                                    if (++pos == len) { total = strv; break; } // incomlpete sequence

                                    auto tmp = strv.substr(pos);
                                    auto l = tmp.size();
                                    if (auto ctrl = utf::to_int(tmp))
                                    {
                                        pos += l - tmp.size();
                                        if (pos == len) { total = strv; break; } // incomlpete sequence
                                        if (++pos == len) { total = strv; break; } // incomlpete sequence

                                        auto tmp = strv.substr(pos);
                                        auto l = tmp.size();
                                        if (auto pos_x = utf::to_int(tmp))
                                        {
                                            pos += l - tmp.size();
                                            if (pos == len) { total = strv; break; }// incomlpete sequence
                                            if (++pos == len) { total = strv; break; }// incomlpete sequence

                                            auto tmp = strv.substr(pos);
                                            auto l = tmp.size();
                                            if (auto pos_y = utf::to_int(tmp))
                                            {
                                                pos += l - tmp.size();
                                                if (pos == len) { total = strv; break; } // incomlpete sequence
                                                if (strv.at(pos) == 'M' || strv.at(pos) == 'm')
                                                {
                                                    auto ispressed = (strv.at(pos) == 'M');
                                                    ++pos;

                                                    auto clamp = [](auto a) { return std::clamp(a, si32min / 2, si32max / 2); };
                                                    auto x = clamp(pos_x.value() - 1);
                                                    auto y = clamp(pos_y.value() - 1);
                                                    auto ctl = ctrl.value();

                                                    m.enabled = {};
                                                    m.doubled = {};
                                                    m.wheeled = {};
                                                    m.hzwheel = {};
                                                    m.wheeldt = {};
                                                    m.ctlstat = {};
                                                    // 000 000 00
                                                    //   | ||| ||
                                                    //   | ||| └----- button number
                                                    //   | └--------- ctl state
                                                    if (ctl & 0x04) m.ctlstat |= input::hids::LShift;
                                                    if (ctl & 0x08) m.ctlstat |= input::hids::LAlt;
                                                    if (ctl & 0x10) m.ctlstat |= input::hids::LCtrl;
                                                    ctl &= ~0b00011100;
                                                    k.ctlstat = m.ctlstat;

                                                    if (ctl == 35 && m.buttons) // Moving without buttons (case when second release not fired: apple's terminal.app)
                                                    {
                                                        m.buttons = {};
                                                        m.changed++;
                                                        mouse(m);
                                                    }
                                                    m.coordxy = { x, y };
                                                    switch (ctl)
                                                    {
                                                        case 0: netxs::set_bit<input::hids::left  >(m.buttons, ispressed); break;
                                                        case 1: netxs::set_bit<input::hids::middle>(m.buttons, ispressed); break;
                                                        case 2: netxs::set_bit<input::hids::right >(m.buttons, ispressed); break;
                                                        case 64:
                                                            m.wheeled = true;
                                                            m.wheeldt = 1;
                                                            break;
                                                        case 65:
                                                            m.wheeled = true;
                                                            m.wheeldt = -1;
                                                            break;
                                                    }
                                                    m.changed++;
                                                    mouse(m);
                                                    unk = faux;
                                                }
                                            }
                                        }
                                    }
                                }
                                else if (strv.substr(pos).starts_with("33:")) // \033[33: format p
                                {
                                    pos += 3;
                                    if (pos == len) { total = strv; break; } // incomlpete sequence

                                    auto tmp = strv.substr(pos);
                                    auto l = tmp.size();
                                    if (auto format = utf::to_int<ui32>(tmp))
                                    {
                                        pos += l - tmp.size();
                                        if (pos == len) { total = strv; break; } // incomlpete sequence
                                        if (strv.at(pos) == ansi::csi_ccc)
                                        {
                                            ++pos;
                                            style(deco{ format.value() });
                                            unk = faux;
                                        }
                                    }
                                }
                            }
                        }

                        if (unk)
                        {
                            pos = 0_sz;
                        }
                        else
                        {
                            total = strv.substr(pos);
                            strv = total;
                        }

                        if (auto size = strv.size())
                        {
                            auto i = unk ? 1_sz : 0_sz;
                            while (i != size && (strv.at(i) != '\033'))
                            {
                                i++;
                            }

                            if (i)
                            {
                                k.pressed = true;
                                k.cluster = strv.substr(0, i);
                                keybd(k);
                                total = strv.substr(i);
                                strv = total;
                            }
                        }
                    }
                };
                auto h_proc = [&]
                {
                    if (auto data = io::recv(os::stdin_fd, buffer))
                    {
                        if (micefd != os::invalid_fd)
                        {
                            auto kbmod = get_kb_state();
                            if (k.ctlstat != kbmod)
                            {
                                k.ctlstat = kbmod;
                                m.ctlstat = kbmod;
                                m.doubled = faux;
                                m.doubled = faux;
                                m.wheeled = faux;
                                m.hzwheel = faux;
                                m.wheeldt = 0;
                                m.changed++;
                                mouse(m); // Fire mouse event to update kb modifiers.
                            }
                        }
                        filter(data);
                    }
                    else alive = faux;
                };
                auto m_proc = [&, mcoord = twod{}]() mutable
                {
                    auto data = io::recv(micefd, buffer);
                    auto size = data.size();
                    if (size == 4 /* ImPS/2 */
                     || size == 3 /* PS/2 compatibility mode */)
                    {
                    #if defined(__linux__)
                        auto vt_state = ::vt_stat{};
                        ok(::ioctl(os::stdout_fd, VT_GETSTATE, &vt_state), "::ioctl(VT_GETSTATE)", os::unexpected);
                        if (vt_state.v_active == ttynum) // Proceed current active tty only.
                        {
                            auto scale = twod{ 6,12 }; //todo magic numbers
                            auto limit = w.winsize * scale;
                            auto bttns = data[0] & 7;
                            mcoord.x  += data[1];
                            mcoord.y  -= data[2];
                            mcoord = std::clamp(mcoord, dot_00, limit - dot_11);
                            k.ctlstat = get_kb_state();
                            m.wheeldt = size == 4 ? -data[3] : 0;
                            m.wheeled = m.wheeldt;
                            m.coordxy = { mcoord / scale };
                            m.buttons = bttns;
                            m.ctlstat = k.ctlstat;
                            m.changed++;
                            mouse(m);
                        }
                    #endif
                    }
                };
                auto s_proc = [&]
                {
                    auto signal = sigt{};
                    if (io::recv(sig_fd, &signal, sizeof(signal)))
                    {
                        switch (signal)
                        {
                            case SIGWINCH: w.winsize = dtvt::consize(); winsz(w); break;
                            case SIGINT:  // App close.
                            case SIGHUP:  // App close.
                            case SIGTERM: // System shutdown.
                                if constexpr (debugmode) log("%%Process %pid% received signal %signo%", prompt::tty, os::process::id.first, signal);
                                alive = faux;
                            default: break;
                        }
                    }
                };
                auto f_proc = [&]
                {
                    alive = faux;
                };

                while (alive)
                {
                    io::select(os::stdin_fd, h_proc,
                               sig_fd,       s_proc,
                               micefd,       m_proc,
                               alarm,        f_proc);
                }

                os::close(micefd);

            #endif

            if constexpr (debugmode) log(prompt::tty, "Reading thread ended", ' ', utf::to_hex_0x(std::this_thread::get_id()));
            close(c);
        }
        void clipbd(auto& alarm)
        {
            using namespace os::clipboard;

            if (os::dtvt::active) return;
            if constexpr (debugmode) log(prompt::tty, "Clipboard watcher started", ' ', utf::to_hex_0x(std::this_thread::get_id()));

            #if defined(_WIN32)

                auto wndname = utf::to_utf("vtmWindowClass");
                auto wndproc = [](auto hwnd, auto uMsg, auto wParam, auto lParam)
                {
                    auto sync = [](auto&& utf8, auto form)
                    {
                        auto clipdata = tty::stream.clipdata.freeze();
                        input::board::set(clipdata.thing, id_t{}, dtvt::win_sz / 2, std::forward<decltype(utf8)>(utf8), form);
                        clipdata.thing.set();
                        auto crop = utf::trunc(clipdata.thing.utf8, dtvt::win_sz.y / 2); // Trim preview before sending.
                        tty::stream.sysboard.send(dtvt::client, id_t{}, clipdata.thing.size, crop.str(), clipdata.thing.form);
                    };
                    switch (uMsg)
                    {
                        case WM_CREATE:
                            ok(::AddClipboardFormatListener(hwnd), "::AddClipboardFormatListener()", os::unexpected);
                            // Continue processing the switch to initialize the clipboard state after startup.
                        case WM_CLIPBOARDUPDATE:
                        {
                            auto lock = std::lock_guard{ os::clipboard::mutex };
                            while (!::OpenClipboard(hwnd)) // Waiting clipboard access.
                            {
                                if (os::error() != ERROR_ACCESS_DENIED)
                                {
                                    auto error = utf::concat("::OpenClipboard()", os::unexpected, " code ", os::error());
                                    sync(error, mime::textonly);
                                    return (LRESULT) NULL;
                                }
                                std::this_thread::yield();
                            }
                            if (auto seqno = ::GetClipboardSequenceNumber();
                                     seqno != os::clipboard::sequence)
                            {
                                os::clipboard::sequence = seqno;
                                if (auto format = ::EnumClipboardFormats(0))
                                {
                                    auto hidden = ::GetClipboardData(cf_sec1);
                                    if (auto hglb = ::GetClipboardData(cf_ansi)) // Our clipboard format.
                                    {
                                        if (auto lptr = ::GlobalLock(hglb))
                                        {
                                            auto size = ::GlobalSize(hglb);
                                            auto data = text((char*)lptr, size - 1/*trailing null*/);
                                            sync(data, mime::disabled);
                                            ::GlobalUnlock(hglb);
                                        }
                                        else
                                        {
                                            auto error = utf::concat("::GlobalLock()", os::unexpected, " code ", os::error());
                                            sync(error, mime::textonly);
                                        }
                                    }
                                    else do
                                    {
                                        if (format == cf_text)
                                        {
                                            auto type = hidden ? mime::safetext : mime::textonly;
                                            if (auto hglb = ::GetClipboardData(format))
                                            if (auto lptr = ::GlobalLock(hglb))
                                            {
                                                auto size = ::GlobalSize(hglb);
                                                sync(utf::to_utf((wchr*)lptr, size / 2 - 1/*trailing null*/), type);
                                                ::GlobalUnlock(hglb);
                                                break;
                                            }
                                            auto error = utf::concat("::GlobalLock()", os::unexpected, " code ", os::error());
                                            sync(error, mime::textonly);
                                        }
                                        else
                                        {
                                            //todo proceed other formats (rich/html/...)
                                        }
                                    }
                                    while (format = ::EnumClipboardFormats(format));
                                }
                                else sync(text{}, mime::textonly);
                            }
                            ok(::CloseClipboard(), "::CloseClipboard()", os::unexpected);
                            break;
                        }
                        case WM_DESTROY:
                            ok(::RemoveClipboardFormatListener(hwnd), "::RemoveClipboardFormatListener()", os::unexpected);
                            ::PostQuitMessage(0);
                            break;
                        default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
                    }
                    return (LRESULT) NULL;
                };
                auto wnddata = WNDCLASSEXW
                {
                    .cbSize        = sizeof(WNDCLASSEXW),
                    .lpfnWndProc   = wndproc,
                    .lpszClassName = wndname.c_str(),
                };
                if (ok(::RegisterClassExW(&wnddata) || os::error() == ERROR_CLASS_ALREADY_EXISTS, "::RegisterClassExW()", os::unexpected))
                {
                    auto stop = fd_t{ alarm };
                    auto hwnd = ::CreateWindowExW(0, wndname.c_str(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                    auto next = MSG{};
                    while (next.message != WM_QUIT)
                    {
                        if (auto yield = ::MsgWaitForMultipleObjects(1, &stop, FALSE, INFINITE, QS_ALLINPUT);
                                 yield == WAIT_OBJECT_0)
                        {
                            ::DestroyWindow(hwnd);
                            break;
                        }
                        while (::PeekMessageW(&next, NULL, 0, 0, PM_REMOVE) && next.message != WM_QUIT)
                        {
                            ::DispatchMessageW(&next);
                        }
                    }
                }

            #elif defined(__APPLE__)

                //todo macOS clipboard watcher

            #else

                //todo X11 and Wayland clipboard watcher

            #endif

            if constexpr (debugmode) log(prompt::tty, "Clipboard watcher ended", ' ', utf::to_hex_0x(std::this_thread::get_id()));
        }
        auto legacy()
        {
            auto& intio = *dtvt::client;
            #if defined(_WIN32)
                auto inpmode = DWORD{};
                ok(::GetConsoleMode(os::stdin_fd, &inpmode), "::GetConsoleMode()", os::unexpected);
                inpmode |= nt::console::inmode::mouse;
                inpmode &=~nt::console::inmode::quickedit;
                ok(::SetConsoleMode(os::stdin_fd, inpmode), "::SetConsoleMode()", os::unexpected);

                // Switch to altbuf.
                auto saved_fd = os::stdout_fd;
                if (!ok(os::stdout_fd = ::CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr), "::CreateConsoleScreenBuffer()", os::unexpected))
                {
                    os::stdout_fd = saved_fd;
                    saved_fd = os::invalid_fd;
                }
                else ok(::SetConsoleActiveScreenBuffer(os::stdout_fd), "::SetConsoleActiveScreenBuffer(", utf::to_hex_0x(os::stdout_fd), ")", os::unexpected);
                io::send(os::stdout_fd, ansi::altbuf(true).cursor(faux).bpmode(true)); // Windows 10 console compatibility (turning scrollback off, cursor not hidden by WinAPI).
                auto palette = CONSOLE_SCREEN_BUFFER_INFOEX{ .cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX), .wAttributes = {} };
                ok(::GetConsoleScreenBufferInfoEx(os::stdout_fd, &palette), "::GetConsoleScreenBufferInfoEx()", os::unexpected);

                auto caret = dtvt::backup.caret; // Doesn't work on modern windows console. Additiom vt command required, see below.
                caret.bVisible = FALSE; // Will be restored by the dtvt::backup.caret on exit.
                ok(::SetConsoleCursorInfo(os::stdout_fd, &caret), "::SetConsoleCursorInfo()", os::unexpected);

                if (dtvt::vtmode & os::dtvt::nt16)
                {
                    auto c16 = palette;
                    c16.srWindow = { .Right = (si16)dtvt::win_sz.x, .Bottom = (si16)dtvt::win_sz.y }; // Suppress unexpected scrollbars.
                    rgba::set_vtm16_palette([&](auto index, auto color){ c16.ColorTable[index] = color & 0x00FFFFFF; }); // conhost crashed if alpha non zero.
                    ok(::SetConsoleScreenBufferInfoEx(os::stdout_fd, &c16), "::SetConsoleScreenBufferInfoEx()", os::unexpected);
                }
            #else 
                auto vtrun = ansi::altbuf(true).bpmode(true).cursor(faux).vmouse(true).set_palette(dtvt::vtmode & os::dtvt::vt16);
                auto vtend = ansi::scrn_reset().altbuf(faux).bpmode(faux).cursor(true).vmouse(faux).rst_palette(dtvt::vtmode & os::dtvt::vt16);
                io::send(os::stdout_fd, vtrun);
            #endif

            tty::stream.mousebar.send(intio, !!(dtvt::vtmode & os::dtvt::mouse));

            auto alarm = fire{};
            auto alive = flag{ true };
            auto keybd = [&](auto& data) { if (alive)                tty::stream.syskeybd.send(intio, data); };
            auto mouse = [&](auto& data) { if (alive)                tty::stream.sysmouse.send(intio, data); };
            auto winsz = [&](auto& data) { if (alive)                tty::stream.syswinsz.send(intio, data); };
            auto focus = [&](auto& data) { if (alive)                tty::stream.sysfocus.send(intio, data); };
            auto paste = [&](auto& data) { if (alive)                tty::stream.syspaste.send(intio, data); };
            auto close = [&](auto& data) { if (alive.exchange(faux)) tty::stream.sysclose.send(intio, data); };
            auto input = std::thread{ [&]{ tty::reader(alarm, keybd, mouse, winsz, focus, paste, close, noop{}); }};
            auto clips = std::thread{ [&]{ tty::clipbd(alarm); } };
            directvt::binary::stream::reading_loop(intio, [&](view data){ tty::stream.s11n::sync(data); });
            tty::stream.s11n::stop(); // Wake up waiting objects, if any.
            alarm.bell(); // Forced to call close().
            clips.join();
            input.join(); // Wait close() to complete.
            intio.shut(); // Close link to server.
            //test: std::this_thread::sleep_for(2000ms); // Uncomment to test for delayed input events.

            #if defined(_WIN32)
                io::send(os::stdout_fd, ansi::altbuf(faux).cursor(true).bpmode(faux));
                if (dtvt::vtmode & os::dtvt::nt16) // Restore pelette.
                {
                    auto count = DWORD{};
                    ok(::FillConsoleOutputAttribute(os::stdout_fd, 0, dtvt::win_sz.x * dtvt::win_sz.y, {}, &count), "::FillConsoleOutputAttribute()", os::unexpected); // To avoid palette flickering.
                    ok(::SetConsoleScreenBufferInfoEx(os::stdout_fd, &palette), "::SetConsoleScreenBufferInfoEx()", os::unexpected);
                }
                if (saved_fd != os::invalid_fd)
                {
                    os::close(os::stdout_fd);
                    os::stdout_fd = saved_fd;
                    ok(::SetConsoleActiveScreenBuffer(os::stdout_fd), "::SetConsoleActiveScreenBuffer()", os::unexpected);
                }
            #else 
                io::send(os::stdout_fd, vtend);
            #endif

            std::this_thread::sleep_for(200ms); // Wait for delayed input events (e.g. mouse reports lagging over remote ssh).
            io::drop(); // Discard delayed events to avoid garbage in the shell's readline.
        }
        auto splice(xipc client)
        {
            os::dtvt::client = client;
            os::dtvt::active ? tty::direct()
                             : tty::legacy();
        }

        struct readline
        {
            std::thread thread;
            fire alarm;
            flag alive;

            readline(auto send, auto shut)
                : alive{ true }
            {
                if (os::dtvt::vtmode & os::dtvt::onlylog) return;
                thread = std::thread{ [&, send, shut]
                {
                    dtvt::scroll = true;
                    auto osout = tty::cout;
                    auto width = si32{};
                    auto block = escx{};
                    auto yield = escx{};
                    auto mutex = std::mutex{};
                    auto panel = dtvt::consize();
                    auto wraps = true;
                    auto clear = [&](auto&& ...args) // Erase the readline block and output the args.
                    {
                        if (width)
                        {
                            if (wraps && width >= panel.x) yield.cuu(width / panel.x);
                            yield.add("\r").del_below();
                            width = 0;
                        }
                        if constexpr (sizeof...(args))
                        {
                            yield.add(std::forward<decltype(args)>(args)...);
                            osout(yield);
                            yield.clear();
                        }
                    };
                    auto print = [&](bool renew)
                    {
                        if (renew && width)
                        {
                            if (wraps && width >= panel.x) yield.cuu(width / panel.x);
                            yield.add("\r");
                        }
                        utf::change(block, "\n", "\r\n"); // Disabled post-processing.
                        yield.pushsgr().nil().fgc(yellowlt);
                        width = utf::debase<faux, faux>(block, yield);
                        yield.nil().popsgr();
                        if (wraps && width && width % panel.x == 0) yield.add("\r\n");
                        if (renew) yield.del_below();
                        yield.cursor(true);
                        osout(yield);
                        yield.clear();
                    };
                    auto enter = [&](auto&& ...args)
                    {
                        if (block.length()) yield.add("\r\n");
                        if constexpr (sizeof...(args)) yield.add(std::forward<decltype(args)>(args)...);
                        if (yield.length())
                        {
                            yield.cursor(true);
                            osout(yield);
                            yield.clear();
                        }
                        block.clear();
                        width = 0;
                    };
                    auto write = std::function([&](qiew utf8)
                    {
                        if (utf8)
                        {
                            auto guard = std::lock_guard{ mutex };
                            if (utf8.back() == '\n') // Print the readline block after a new line only.
                            {
                                clear();
                                yield.add(utf8);
                                print(faux);
                            }
                            else clear(utf8);
                        }
                    });
                    auto keybd = [&](auto& data)
                    {
                        auto guard = std::unique_lock{ mutex };
                        if (!alive || !data.pressed || data.cluster.empty()) return;
                        switch (data.cluster.front()) 
                        {
                            case 0x03: enter(ansi::err("Ctrl+C\r\n")); alarm.bell(); break;
                            case 0x04: enter(ansi::err("Ctrl+D\r\n")); alarm.bell(); break;
                            case 0x1A: enter(ansi::err("Ctrl+Z\r\n")); alarm.bell(); break;
                            case 0x08: // Backspace
                            case 0x7F: //
                                if (block.size())
                                {
                                    block.pop_back();
                                    print(true);
                                }
                                break;
                            case '\r': // Enter
                            {
                                auto line = block + '\n';
                                block.clear();
                                clear();
                                print(faux);
                                guard.unlock(); // Allow to use log() inside send().
                                send(line);
                                break;
                            }
                            default:
                                block += data.cluster;
                                print(true);
                                break;
                        }
                    };
                    auto mouse = [&](auto& data) { if (!alive) return; }; // Not used.
                    auto winsz = [&](auto& data)
                    {
                        if (!alive) return;
                        auto guard = std::lock_guard{ mutex };
                        panel = data.winsize;
                    };
                    auto focus = [&](auto& data) { if (!alive) return;/*if (data.state) log<faux>('-');*/ };
                    auto paste = [&](auto& data) { if (!alive) return; };
                    auto close = [&](auto& data) 
                    {
                        if (alive.exchange(faux))
                        {
                            {
                                auto guard = std::lock_guard{ mutex };
                                enter(ansi::styled(faux)); // Disable style reporting.
                            }
                            {
                                auto lock = logger::globals(); // Sync with logger.
                                os::autosync = true;
                                std::swap(tty::cout, write); // Restore original logger.
                            }
                            shut();
                        }
                    };
                    auto style = [&](deco format) 
                    {
                        if (!alive) return;
                        wraps = format.wrp() != wrap::off;
                    };
                    {
                        auto lock = logger::globals(); // Sync with logger.
                        enter(ansi::styled(true)); // Enable style reporting (wrapping).
                        os::autosync = faux; // Synchronize viewport only when the vt-sequence "show caret" is received.
                        std::swap(tty::cout, write); // Activate log proxy.
                    }
                    tty::reader(alarm, keybd, mouse, winsz, focus, paste, close, style);
                }};
            }
           ~readline()
            {
                stop();
            }
            void stop()
            {
                if (alive) alarm.bell();
                if (thread.joinable()) thread.join();
            }
        };
    }
}