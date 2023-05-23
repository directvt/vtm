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

    #pragma warning(disable:4996) // disable std::getenv warnimg

    #include <Windows.h>
    #include <userenv.h>    // ::GetUserProfileDirectoryW
    #pragma comment(lib, "Userenv.lib")
    #include <Psapi.h>      // ::GetModuleFileNameEx
    #include <winternl.h>   // ::NtOpenFile

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
    using page = ui::page;
    using para = ui::para;
    using rich = ui::rich;
    using s11n = ui::s11n;
    using pipe = ui::pipe;
    using xipc = ui::pipe::xipc;

    enum class role { client, server };

    static auto is_daemon = faux;
    static constexpr auto pipebuf = si32{ 65536 };
    static constexpr auto app_wait_timeout = debugmode ? 1000000 : 10000;
    static constexpr auto unexpected_msg = " returns unexpected result"sv;

    #if defined(_WIN32)

        using sigt = DWORD;
        using pidt = DWORD;
        using fd_t = HANDLE;
        struct conmode { DWORD omode, imode, opage, ipage; };
        static const auto invalid_fd   = fd_t{ INVALID_HANDLE_VALUE              };
        static const auto stdin_fd     = fd_t{ ::GetStdHandle(STD_INPUT_HANDLE)  };
        static const auto stdout_fd    = fd_t{ ::GetStdHandle(STD_OUTPUT_HANDLE) };
        static const auto stderr_fd    = fd_t{ ::GetStdHandle(STD_ERROR_HANDLE)  };
        static const auto codepage     = ui32{ ::GetOEMCP()                      };
        static const auto wr_pipe_path = "\\\\.\\pipe\\w_";
        static const auto rd_pipe_path = "\\\\.\\pipe\\r_";

    #else

        using sigt = int;
        using pidt = pid_t;
        using fd_t = int;
        using conmode = ::termios;
        static const auto invalid_fd = fd_t{ -1            };
        static const auto stdin_fd   = fd_t{ STDIN_FILENO  };
        static const auto stdout_fd  = fd_t{ STDOUT_FILENO };
        static const auto stderr_fd  = fd_t{ STDERR_FILENO };

        void fdcleanup() // Close all file descriptors except the standard ones.
        {
            auto maxfd = ::sysconf(_SC_OPEN_MAX);
            auto minfd = std::max({ os::stdin_fd, os::stdout_fd, os::stderr_fd });
            while (++minfd < maxfd)
            {
                ::close(minfd);
            }
        }

    #endif

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
        log("  os: ", ansi::err(msg..., " (", os::error(), ") "));
    };
    template<class T, class ...Args>
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
            os::fail(std::forward<Args>(msg)...);
            return faux;
        }
        else return true;
    }

    #if defined(_WIN32)

        class nt
        {
            using NtOpenFile_ptr = std::decay<decltype(::NtOpenFile)>::type;
            using ConsoleCtl_ptr = NTSTATUS(*)(ui32, void*, ui32);

            HMODULE         ntdll_dll{};
            HMODULE        user32_dll{};
            NtOpenFile_ptr NtOpenFile{};
            ConsoleCtl_ptr ConsoleCtl{};

            nt()
            {
                ntdll_dll  = ::LoadLibraryExA("ntdll.dll",  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                user32_dll = ::LoadLibraryExA("user32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (!ntdll_dll || !user32_dll) os::fail("LoadLibraryEx(ntdll.dll | user32.dll)");
                else
                {
                    NtOpenFile = reinterpret_cast<NtOpenFile_ptr>(::GetProcAddress( ntdll_dll, "NtOpenFile"));
                    ConsoleCtl = reinterpret_cast<ConsoleCtl_ptr>(::GetProcAddress(user32_dll, "ConsoleControl"));
                    if (!NtOpenFile) os::fail("::GetProcAddress(NtOpenFile)");
                    if (!ConsoleCtl) os::fail("::GetProcAddress(ConsoleControl)");
                }
            }

            void operator=(nt const&) = delete;
            nt(nt const&)             = delete;
            nt(nt&& other)
                : ntdll_dll{ other.ntdll_dll  },
                 user32_dll{ other.user32_dll },
                 NtOpenFile{ other.NtOpenFile },
                 ConsoleCtl{ other.ConsoleCtl }
            {
                other.ntdll_dll  = {};
                other.user32_dll = {};
                other.NtOpenFile = {};
                other.ConsoleCtl = {};
            }

            static auto& get_ntdll()
            {
                static auto ref = nt{};
                return ref;
            }

        public:
           ~nt()
            {
                if (ntdll_dll ) ::FreeLibrary(ntdll_dll );
                if (user32_dll) ::FreeLibrary(user32_dll);
            }

            constexpr explicit operator bool() const { return NtOpenFile != nullptr; }

            struct status
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
            };

            template<class ...Args>
            static auto OpenFile(Args... args)
            {
                auto& inst = get_ntdll();
                return inst ? inst.NtOpenFile(std::forward<Args>(args)...)
                            : nt::status::not_found;
            }
            template<class ...Args>
            static auto UserConsoleControl(Args... args)
            {
                auto& inst = get_ntdll();
                return inst ? inst.ConsoleCtl(std::forward<Args>(args)...)
                            : nt::status::not_found;
            }
            template<class I = noop, class O = noop>
            static auto ioctl(DWORD dwIoControlCode, fd_t hDevice, I&& send = {}, O&& recv = {}) -> NTSTATUS
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
            static auto object(view        path,
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
                    log("  os: unexpected result when access system object '", path, "', ntstatus ", status);
                    return os::invalid_fd;
                }
                else return hndl;
            }

            struct console
            {
                enum fx
                {
                    undef,
                    connect,
                    disconnect,
                    create,
                    close,
                    write,
                    read,
                    subfx,
                    flush,
                    count,
                };
                struct op
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
                };
                struct inmode
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
                };
                struct outmode
                {
                    static constexpr auto preprocess    = 0x0001; // ENABLE_PROCESSED_OUTPUT
                    static constexpr auto wrap_at_eol   = 0x0002; // ENABLE_WRAP_AT_EOL_OUTPUT
                    static constexpr auto vt            = 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
                    static constexpr auto no_auto_cr    = 0x0008; // DISABLE_NEWLINE_AUTO_RETURN
                    static constexpr auto lvb_grid      = 0x0010; // ENABLE_LVB_GRID_WORLDWIDE
                };

                static auto handle(view rootpath)
                {
                    return nt::object(rootpath,
                                      GENERIC_ALL,
                                      OBJ_CASE_INSENSITIVE | OBJ_INHERIT);
                }
                static auto handle(fd_t server, view relpath, bool inheritable = {})
                {
                    return nt::object(relpath,
                                      GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                      OBJ_CASE_INSENSITIVE | (inheritable ? OBJ_INHERIT : 0),
                                      FILE_SYNCHRONOUS_IO_NONALERT,
                                      server);
                }
                static auto handle(fd_t cloned_handle)
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
                        log("  os: unexpected result when duplicate system object handle, errcode ", os::error());
                        return os::invalid_fd;
                    }
                }
            };

            template<class T1, class T2 = si32>
            static auto kbstate(si32& modstate, T1 ms_ctrls, T2 scancode = {}, bool pressed = {})
            {
                if (scancode == 0x2a)
                {
                    if (pressed) modstate |= input::hids::LShift;
                    else         modstate &=~input::hids::LShift;
                }
                if (scancode == 0x36)
                {
                    if (pressed) modstate |= input::hids::RShift;
                    else         modstate &=~input::hids::RShift;
                }
                auto lshift = modstate & input::hids::LShift;
                auto rshift = modstate & input::hids::RShift;
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
                if (nums  ) state |= input::hids::NumLock;
                if (caps  ) state |= input::hids::CapsLock;
                if (scrl  ) state |= input::hids::ScrlLock;
                return state;
            }
            template<class T1>
            static auto ms_kbstate(T1 ctrls)
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
                if (lshift) state |= SHIFT_PRESSED;
                if (rshift) state |= SHIFT_PRESSED;
                if (lalt  ) state |= LEFT_ALT_PRESSED;
                if (ralt  ) state |= RIGHT_ALT_PRESSED;
                if (lctrl ) state |= LEFT_CTRL_PRESSED;
                if (rctrl ) state |= RIGHT_CTRL_PRESSED;
                if (nums  ) state |= NUMLOCK_ON;
                if (caps  ) state |= CAPSLOCK_ON;
                if (scrl  ) state |= SCROLLLOCK_ON;
                return state;
            }
        };

    #endif

    namespace io
    {
        void close(fd_t& h)
        {
            if (h != os::invalid_fd)
            {
                #if defined(_WIN32)
                    ::CloseHandle(h);
                #else
                    ::close(h);
                #endif
                h = os::invalid_fd;
            }
        }

        struct file
        {
            fd_t r; // file: Read descriptor.
            fd_t w; // file: Send descriptor.

            operator bool ()
            {
                return r != os::invalid_fd && w != os::invalid_fd;
            }
            void close()
            {
                if (w == r)
                {
                    io::close(r);
                    w = r;
                }
                else
                {
                    io::close(w); // Wriite end should be closed first.
                    io::close(r);
                }
            }
            void shutdown() // Reset writing end of the pipe to interrupt reading call.
            {
                if (w == r)
                {
                    // Use ::shutdown() for full duplex sockets.
                }
                else
                {
                    io::close(w);
                }
            }
            friend auto& operator << (std::ostream& s, file const& handle)
            {
                if (handle.w != handle.r) s << handle.r << ",";
                return s << handle.w;
            }
            auto& operator = (file&& f)
            {
                r = f.r;
                w = f.w;
                f.r = os::invalid_fd;
                f.w = os::invalid_fd;
                return *this;
            }

            file(file const&) = delete;
            file(file&& f)
                : r{ f.r },
                  w{ f.w }
            {
                f.r = os::invalid_fd;
                f.w = os::invalid_fd;
            }
            file(fd_t r = os::invalid_fd, fd_t w = os::invalid_fd)
                : r{ r },
                  w{ w }
            { }
           ~file()
            {
                close();
            }
        };

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
        auto send(fd_t fd, T* buffer, Size_t size)
        {
            return io::send(fd, (char const*)buffer, size);
        }
        template<class View>
        auto send(fd_t fd, View&& buffer)
        {
            return io::send(fd, buffer.data(), buffer.size());
        }
        template<class View>
        auto send(View&& buffer)
        {
            return io::send(os::stdout_fd, std::forward<View>(buffer));
        }
        template<class ...Args>
        auto recv(file& handle, Args&&... args)
        {
            return io::recv(handle.r, std::forward<Args>(args)...);
        }
        template<class ...Args>
        auto send(file& handle, Args&&... args)
        {
            return io::send(handle.w, std::forward<Args>(args)...);
        }

        namespace
        {
            #if defined(_WIN32)

                template<class A, std::size_t... I>
                constexpr auto _repack(fd_t h, A const& a, std::index_sequence<I...>)
                {
                    return std::array{ a[I]..., h };
                }
                template<std::size_t N, class P, class Index = std::make_index_sequence<N>, class ...Args>
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
                    FD_SET(handle, &socks);
                    if constexpr (sizeof...(args))
                    {
                        return std::max(handle, _fd_set(socks, std::forward<Args>(args)...));
                    }
                    else
                    {
                        return handle;
                    }
                }
                template<class P, class ...Args>
                auto _select(fd_set& socks, fd_t handle, P&& proc, Args&&... args)
                {
                    if (FD_ISSET(handle, &socks))
                    {
                        proc();
                    }
                    else
                    {
                        if constexpr (sizeof...(args)) _select(socks, std::forward<Args>(args)...);
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
                if (::select(nfds, &socks, 0, 0, timeout) > 0)
                {
                    _select(socks, std::forward<Args>(args)...);
                }

            #endif
        }

        struct fire
        {
            #if defined(_WIN32)

                fd_t h; // fire: Descriptor for IO interrupt.

                operator auto () { return h; }
                fire(bool i = 1) { ok(h = ::CreateEventW(NULL, i, FALSE, NULL), "::CreateEventW()", os::unexpected_msg); }
               ~fire()           { io::close(h); }
                void reset()     { ok(::SetEvent(h), "::SetEvent()", os::unexpected_msg); }
                void flush()     { ok(::ResetEvent(h), "::ResetEvent()", os::unexpected_msg); }

            #else

                fd_t h[2] = { os::invalid_fd, os::invalid_fd }; // fire: Descriptors for IO interrupt.

                operator auto () { return h[0]; }
                fire()           { ok(::pipe(h), "::pipe(2)", os::unexpected_msg); }
               ~fire()           { for (auto& f : h) io::close(f); }
                void reset()     { static auto c = ' '; io::send(h[1], &c, sizeof(c)); }
                void flush()     { static auto c = ' '; io::recv(h[0], &c, sizeof(c)); }

            #endif
            void bell() { reset(); }
        };
    }

    namespace env
    {
        // os::env: Get envvar value.
        auto get(view variable)
        {
            #if defined(_WIN32)
                auto var = utf::to_utf(variable);
                auto len = ::GetEnvironmentVariableW(var.c_str(), 0, 0);
                auto val = wide(len, 0);
                ::GetEnvironmentVariableW(var.c_str(), val.data(), len);
                if (len && val.back() == 0) val.pop_back();
                return utf::to_utf(val);
            #else
                auto var = text{ variable };
                auto val = std::getenv(var.c_str());
                return val ? text{ val } : text{};
            #endif
        }
        // os::env: Set envvar value.
        auto set(view variable, view value)
        {
            #if defined(_WIN32)
                auto var = utf::to_utf(variable);
                auto val = utf::to_utf(value);
                ok(::SetEnvironmentVariableW(var.c_str(), val.c_str()), "::SetEnvironmentVariableW()", os::unexpected_msg);
            #else
                auto var = text{ variable };
                auto val = text{ value    };
                ok(::setenv(var.c_str(), val.c_str(), 1), "::setenv()", os::unexpected_msg);
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
                else os::fail("can't detect user profile path");
                return fs::path{ utf::to_utf(buffer) };
            #else
                return fs::path{ os::env::get("HOME") };
            #endif
        }
        // os::env: Get user shell.
        auto shell()
        {
            #if defined(_WIN32)

                return "cmd"s;

            #else

                auto shell = os::env::get("SHELL");
                if (shell.empty()
                 || shell.ends_with("vtm"))
                {
                    shell = "bash"; //todo request it from user if empty; or make it configurable
                    log("  os: using '", shell, "' as a fallback login shell");
                }
                return shell;

            #endif
        }
        // os::env: Get user name.
        auto user()
        {
            #if defined(_WIN32)

                auto buffer = wide{};
                auto length = DWORD{};
                ::GetUserNameW(buffer.data(), &length);
                buffer.resize(length);
                if(ok(::GetUserNameW(buffer.data(), &length), "::GetUserNameW()", os::unexpected_msg))
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

        auto set(view mime, view utf8)
        {
            // Generate the following formats:
            //   clip::textonly | clip::disabled
            //        CF_UNICODETEXT: Raw UTF-16
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //   clip::ansitext
            //               cf_rich: RTF-group UTF-8
            //        CF_UNICODETEXT: ANSI-text UTF-16
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //   clip::richtext
            //               cf_rich: RTF-group UTF-8
            //        CF_UNICODETEXT: Plaintext UTF-16
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //   clip::htmltext
            //               cf_ansi: ANSI-text UTF-8 with mime mark
            //               cf_html: HTML-code UTF-8
            //        CF_UNICODETEXT: HTML-code UTF-16
            //   clip::safetext (https://learn.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats#cloud-clipboard-and-clipboard-history-formats)
            //    ExcludeClipboardContentFromMonitorProcessing: 1
            //                    CanIncludeInClipboardHistory: 0
            //                       CanUploadToCloudClipboard: 0
            //                                  CF_UNICODETEXT: Raw UTF-16
            //                                         cf_ansi: ANSI-text UTF-8 with mime mark
            //
            //  cf_ansi format: payload=mime_type/size_x/size_y;utf8_data

            using ansi::clip;

            auto success = faux;
            auto size = twod{ 80,25 };
            {
                auto i = 1;
                utf::divide<feed::rev>(mime, '/', [&](auto frag)
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
                                ok(::SetClipboardData(cf_format, gmem) && (success = true), "::SetClipboardData()", os::unexpected_msg, ", cf_format=", cf_format);
                            }
                            else log("  os: ::GlobalLock()", os::unexpected_msg);
                            ::GlobalFree(gmem);
                        }
                        else log("  os: ::GlobalAlloc()", os::unexpected_msg);
                    };
                    cf_format == cf_text ? _send(utf::to_utf(data))
                                         : _send(data);
                };

                auto lock = std::lock_guard{ os::clipboard::mutex };
                ok(::OpenClipboard(nullptr), "::OpenClipboard()", os::unexpected_msg);
                ok(::EmptyClipboard(), "::EmptyClipboard()", os::unexpected_msg);
                if (utf8.size())
                {
                    if (mime.size() < 5 || mime.starts_with(ansi::mimetext))
                    {
                        send(cf_text, utf8);
                    }
                    else
                    {
                        auto post = page{ utf8 };
                        auto info = CONSOLE_FONT_INFOEX{ sizeof(CONSOLE_FONT_INFOEX) };
                        ::GetCurrentConsoleFontEx(os::stdout_fd, faux, &info);
                        auto font = utf::to_utf(info.FaceName);
                        if (mime.starts_with(ansi::mimerich))
                        {
                            auto rich = post.to_rich(font);
                            auto utf8 = post.to_utf8();
                            send(cf_rich, rich);
                            send(cf_text, utf8);
                        }
                        else if (mime.starts_with(ansi::mimehtml))
                        {
                            auto [html, code] = post.to_html(font);
                            send(cf_html, html);
                            send(cf_text, code);
                        }
                        else if (mime.starts_with(ansi::mimeansi))
                        {
                            auto rich = post.to_rich(font);
                            send(cf_rich, rich);
                            send(cf_text, utf8);
                        }
                        else if (mime.starts_with(ansi::mimesafe))
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
                    auto crop = ansi::add(mime, ";", utf8);
                    send(cf_ansi, crop);
                }
                else
                {
                    success = true;
                }
                ok(::CloseClipboard(), "::CloseClipboard()", os::unexpected_msg);
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
                if (mime.starts_with(ansi::mimerich))
                {
                    auto post = page{ utf8 };
                    auto rich = post.to_rich();
                    send(rich);
                }
                else if (mime.starts_with(ansi::mimehtml))
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

                auto yield = ansi::esc{};
                if (mime.starts_with(ansi::mimerich))
                {
                    auto post = page{ utf8 };
                    auto rich = post.to_rich();
                    yield.clipbuf(size, rich, clip::richtext);
                }
                else if (mime.starts_with(ansi::mimehtml))
                {
                    auto post = page{ utf8 };
                    auto [html, code] = post.to_html();
                    yield.clipbuf(size, code, clip::htmltext);
                }
                else if (mime.starts_with(ansi::mimeansi)) //todo GH#216
                {
                    yield.clipbuf(size, utf8, clip::ansitext);
                }
                else
                {
                    yield.clipbuf(size, utf8, clip::textonly);
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

        struct proxy
        {
            text cache;
            time stamp;
            text brand;

            proxy()
                : stamp{ datetime::now() }
            { }

            auto operator() (auto& yield) // Redirect clipboard data.
            {
                if (cache.size() || yield.starts_with(ocs52head))
                {
                    auto now = datetime::now();
                    if (now - stamp > 3s) // Drop outdated cache.
                    {
                        cache.clear();
                        brand.clear();
                    }
                    auto start = cache.size();
                    cache += yield;
                    yield = cache;
                    stamp = now;
                    if (brand.empty())
                    {
                        yield.remove_prefix(ocs52head.size());
                        if (auto p = yield.find(';'); p != view::npos)
                        {
                            brand = yield.substr(0, p++/*;*/);
                            yield.remove_prefix(p);
                            start = ocs52head.size() + p;
                        }
                        else return faux;
                    }
                    else yield.remove_prefix(start);

                    if (auto p = yield.find(ansi::c0_bel); p != view::npos)
                    {
                        auto temp = view{ cache };
                        auto skip = ocs52head.size() + brand.size() + 1/*;*/;
                        auto data = temp.substr(skip, p + start - skip);
                        if (os::clipboard::set(brand, utf::unbase64(data)))
                        {
                            yield.remove_prefix(p + 1/* c0_bel */);
                        }
                        else yield = temp; // Forward all out;
                        cache.clear();
                        brand.clear();
                        if (yield.empty()) return faux;
                    }
                    else return faux;
                }
                return true;
            }
        };
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
                        io::close(handle);
                    }

                #endif
                return utf8;
            }
            static auto set(view data)
            {
                #if defined(_WIN32)

                    auto source = view{ data.data(), data.size() + 1/*trailing null*/ };
                    auto handle = ::CreateFileMappingA(os::invalid_fd, nullptr, PAGE_READWRITE, 0, (DWORD)source.size(), nullptr); ok(handle, "::CreateFileMappingA()", os::unexpected_msg);
                    auto buffer = ::MapViewOfFile(handle, FILE_MAP_WRITE, 0, 0, 0);                                                ok(buffer, "::MapViewOfFile()", os::unexpected_msg);
                    std::copy(std::begin(source), std::end(source), (char*)buffer);
                    ok(::UnmapViewOfFile(buffer), "::UnmapViewOfFile()", os::unexpected_msg);
                    ok(::SetHandleInformation(handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT), "::SetHandleInformation()", os::unexpected_msg);
                    return handle;

                #endif
            }
        };

        struct stdcon : pipe
        {
            using file = io::file;

            file handle; // ipc::stdcon: IO descriptor.
            text buffer; // ipc::stdcon: Receive buffer.

            stdcon()
                : pipe{ faux },
                  buffer(os::pipebuf, 0)
            { }
            stdcon(file&& fd)
                : pipe{ true },
                  handle{ std::move(fd) },
                  buffer(os::pipebuf, 0)
            { }
            stdcon(fd_t r, fd_t w)
                : pipe{ true },
                  handle{ r, w },
                  buffer(os::pipebuf, 0)
            { }

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
                return io::recv(handle, buff, size); // The read call can be interrupted by the write side when its read call is interrupted.
            }
            virtual qiew recv()  override // It's not thread safe!
            {
                return recv(buffer.data(), buffer.size());
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

                fifo(flag& busy)
                    : alive{ true },
                      going{ busy }
                { }

                auto send(view block)
                {
                    auto guard = std::unique_lock{ mutex };
                    if (alive)
                    {
                        store += block;
                        wsync.notify_one();
                    }
                    return alive;
                }
                auto read(text& yield)
                {
                    auto guard = std::unique_lock{ mutex };
                    wsync.wait(guard, [&]{ return store.size() || !alive; });
                    if (alive)
                    {
                        std::swap(store, yield);
                        going = faux;
                        going.notify_all();
                        store.clear();
                        return qiew{ yield };
                    }
                    else return qiew{};
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
                return s << "local pipe: server=" << server.get() << " client=" << client.get();
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
            using fire = io::fire;

            text scpath; // ipc:socket: Socket path (in order to unlink).
            fire signal; // ipc:socket: Interruptor.

            socket(file& fd)
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
                    #ifndef __ANDROID__
                        auto size = socklen_t{ sizeof(cred) };
                    #else
                        auto size = unsigned{ sizeof(cred) };
                    #endif

                    if (!ok(::getsockopt(handle.r, SOL_SOCKET, SO_PEERCRED, &cred, &size), "::getsockopt(SOL_SOCKET)", os::unexpected_msg))
                    {
                        return faux;
                    }

                    if (cred.uid && id != utf::concat(cred.uid))
                    {
                        fail("sock: foreign users are not allowed to the session");
                        return faux;
                    }

                    log("sock: creds from SO_PEERCRED:",
                      "\n      pid : ", cred.pid,
                      "\n      euid: ", cred.uid,
                      "\n      egid: ", cred.gid);

                #elif defined(__BSD__)

                    auto euid = uid_t{};
                    auto egid = gid_t{};

                    if (!ok(::getpeereid(handle.r, &euid, &egid), "::getpeereid()", os::unexpected_msg))
                    {
                        return faux;
                    }

                    if (euid && id != utf::concat(euid))
                    {
                        fail("sock: foreign users are not allowed to the session");
                        return faux;
                    }

                    log("sock: creds from ::getpeereid():",
                      "\n      pid : ", id,
                      "\n      euid: ", euid,
                      "\n      egid: ", egid);

                #endif

                return true;
            }
            auto meet()
            {
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
                        else if (pipe::active) os::fail("meet: not active");

                        return next_waiting_point;
                    };

                    auto r = next_link(handle.r, to_server, PIPE_ACCESS_INBOUND);
                    if (r == os::invalid_fd)
                    {
                        if (pipe::active) os::fail("meet: ::CreateNamedPipe(r)", os::unexpected_msg);
                    }
                    else
                    {
                        auto w = next_link(handle.w, to_client, PIPE_ACCESS_OUTBOUND);
                        if (w == os::invalid_fd)
                        {
                            ::CloseHandle(r);
                            if (pipe::active) os::fail("meet: ::CreateNamedPipe(w)", os::unexpected_msg);
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
                        auto s = io::file{ h, h };
                        if (s) client = ptr::shared<ipc::socket>(s);
                    };
                    auto f_proc = [&]
                    {
                        log("meet: signal fired");
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
                    log("xipc: server shuts down: ", handle);
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
                    log("xipc: client disconnects: ", handle);
                    #if defined(_WIN32)
                        ::DisconnectNamedPipe(handle.w);
                        handle.shutdown(); // To trigger the read end to close.
                    #else
                        ok(::shutdown(handle.w, SHUT_RDWR), "::shutdown()", os::unexpected_msg); // Further sends and receives are disallowed.
                        // An important conceptual reason to want to use shutdown:
                        //    To signal EOF to the peer and still be able
                        //    to receive pending data the peer sent.
                        //    "shutdown() doesn't actually close the file descriptor
                        //     — it just changes its usability.
                        // To free a socket descriptor, you need to use io::close().
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
                           if constexpr (Log) os::fail("failed to start server");
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

                    //security_descriptor pipe_acl(security_descriptor_string);
                    //log("pipe: DACL=", pipe_acl.security_string);
                    // https://docs.microsoft.com/en-us/windows/win32/secauthz/sid-strings

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

                                if (hits) log("path: ", path);
                                ::FindClose(hndl);
                            }
                            return hits;
                        };
                        if (test(to_server))
                        {
                            os::fail("server already running");
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
                            os::fail("::CreateNamedPipe(r)", os::unexpected_msg);
                        }
                        else
                        {
                            w = pipe(to_client, PIPE_ACCESS_OUTBOUND);
                            if (w == os::invalid_fd)
                            {
                                os::fail("::CreateNamedPipe(w)", os::unexpected_msg);
                                io::close(r);
                            }
                            else
                            {
                                auto inpmode = DWORD{ 0 };
                                ok(::GetConsoleMode(os::stdin_fd, &inpmode), "::GetConsoleMode(os::stdin_fd)", os::unexpected_msg);
                                inpmode |= 0
                                        | nt::console::inmode::quickedit
                                        ;
                                ok(::SetConsoleMode(os::stdin_fd, inpmode), "::SetConsoleMode(os::stdin_fd)", os::unexpected_msg);

                                auto outmode = DWORD{ 0 }
                                            | nt::console::outmode::preprocess
                                            | nt::console::outmode::vt
                                            | nt::console::outmode::no_auto_cr
                                            ;
                                ok(::SetConsoleMode(os::stdout_fd, outmode), "::SetConsoleMode(os::stdout_fd)", os::unexpected_msg);
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
                                io::close(w);
                                return faux;
                            }
                            return true;
                        };
                        if (!try_start(play, retry_proc))
                        {
                            if constexpr (Log) os::fail("connection error");
                        }
                    }

                #else

                    if (!ok(::signal(SIGPIPE, SIG_IGN)))
                    {
                        if constexpr (Log) log("failed to set SIG_IGN");
                    }

                    auto addr = sockaddr_un{};
                    auto sun_path = addr.sun_path + 1; // Abstract namespace socket (begins with zero). The abstract socket namespace is a nonportable Linux extension.

                    #if defined(__BSD__)
                        //todo unify "/.config/vtm"
                        auto home = os::env::homepath() / ".config/vtm";
                        if (!fs::exists(home))
                        {
                            if constexpr (Log) log("path: create home directory '", home.string(), "'");
                            auto ec = std::error_code{};
                            fs::create_directory(home, ec);
                            if (ec && Log) log("path: directory '", home.string(), "' creation error ", ec.value());
                        }
                        path = (home / path).string() + ".sock";
                        sun_path--; // File system unix domain socket.
                        if constexpr (Log) log("open: file system socket ", path);
                    #endif

                    if (path.size() > sizeof(sockaddr_un::sun_path) - 2)
                    {
                        if constexpr (Log) os::fail("unix socket path too long");
                    }
                    else if ((w = ::socket(AF_UNIX, SOCK_STREAM, 0)) == os::invalid_fd)
                    {
                        if constexpr (Log) os::fail("unix socket opening error");
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
                                        os::fail("server already running");
                                        io::close(r);
                                    }
                                    else
                                    {
                                        log("path: removing file system socket file ", path);
                                        ::unlink(path.c_str()); // Cleanup file system socket.
                                    }
                                }
                            #endif

                            if (r != os::invalid_fd && ::bind(r, (struct sockaddr*)&addr, sock_addr_len) == -1)
                            {
                                os::fail("unix socket binding error for ", path);
                                io::close(r);
                            }
                            else if (::listen(r, 5) == -1)
                            {
                                os::fail("unix socket listening error for ", path);
                                io::close(r);
                            }
                        }
                        else if constexpr (Role == role::client)
                        {
                            path.clear(); // No need to unlink a file system socket on client disconnect.
                            if (!try_start(play, retry_proc))
                            {
                                if constexpr (Log) os::fail("connection failed");
                                io::close(r);
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

        struct pool
        {
        private:
            struct item
            {
                bool        state;
                std::thread guest;
            };

            std::recursive_mutex            mutex;
            std::condition_variable_any     synch;
            std::map<std::thread::id, item> index;
            si32                            count;
            bool                            alive;
            std::thread                     agent;

            void worker()
            {
                auto guard = std::unique_lock{ mutex };
                log("pool: session control started");

                while (alive || index.size())
                {
                    if (alive) synch.wait(guard);
                    for (auto it = index.begin(); it != index.end();)
                    {
                        auto& [sid, session] = *it;
                        auto& [state, guest] = session;
                        if (state == faux || !alive)
                        {
                            if (guest.joinable())
                            {
                                guard.unlock();
                                guest.join();
                                guard.lock();
                                log("pool: id: ", sid, " session joined");
                            }
                            it = index.erase(it);
                        }
                        else ++it;
                    }
                }
            }
            void checkout()
            {
                auto guard = std::lock_guard{ mutex };
                auto session_id = std::this_thread::get_id();
                index[session_id].state = faux;
                synch.notify_one();
                log("pool: id: ", session_id, " session deleted");
            }

        public:
            template<class Proc>
            void run(Proc process)
            {
                auto guard = std::lock_guard{ mutex };
                if (!alive) return;
                auto next_id = count++;
                auto session = std::thread([&, process, next_id]
                {
                    process(next_id);
                    checkout();
                });
                auto session_id = session.get_id();
                index[session_id] = { true, std::move(session) };
                log("pool: id: ", session_id, " session created");
            }
            auto size()
            {
                return index.size();
            }

            pool()
                : count{ 0    },
                  alive{ true },
                  agent{ &pool::worker, this }
            { }
           ~pool()
            {
                mutex.lock();
                alive = faux;
                synch.notify_one();
                mutex.unlock();

                if (agent.joinable())
                {
                    log("pool: joining agent");
                    agent.join();
                }
                log("pool: session control ended");
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
                os::fail("can't get current module file path, fallback to '", process::arg0, "`");
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
        void exit(int code)
        {
            #if defined(_WIN32)
                ::ExitProcess(code);
            #else
                if (os::is_daemon) ::closelog();
                ::exit(code);
            #endif
        }
        template<class ...Args>
        void exit(int code, Args&&... args)
        {
            log(args...);
            process::exit(code);
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
            if constexpr (Logs) log("exec: '" + cmdline + "'");
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

                auto p_id = ::fork();
                if (p_id == 0) // Child branch.
                {
                    if constexpr (Daemon)
                    {
                        ::umask(0); // Set the file mode creation mask for child process (all access bits are set by default).
                        ::close(os::stdin_fd );
                        ::close(os::stdout_fd);
                        ::close(os::stderr_fd);
                    }
                    os::process::execvp(cmdline);
                    auto errcode = errno;
                    if constexpr (Logs) os::fail("exec: failed to spawn '", cmdline, "'");
                    os::process::exit(errcode);
                }
                else if (p_id > 0) // Parent branch.
                {
                    auto stat = int{};
                    ::waitpid(p_id, &stat, 0); // Wait for the child to avoid zombies.
                    if (WIFEXITED(stat) && (WEXITSTATUS(stat) == 0))
                    {
                        return true; // Child forked and exited successfully.
                    }
                }

            #endif
            if constexpr (Logs) os::fail("exec: failed to spawn '", cmdline, "'");
            return faux;
        }
        auto fork(bool& result, view prefix, view config)
        {
            result = faux;
            #if defined(_WIN32)

                auto handle = os::ipc::memory::set(config);
                auto cmdarg = utf::to_utf(utf::concat(os::process::binary(), " -p ", prefix, " -c :", handle, " -s"));
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
                io::close(handle);
                if (result)
                {
                    io::close(proinf.hProcess);
                    io::close(proinf.hThread);
                    log("  os: process forked");
                    return faux; // Success. The fork concept is not supported on Windows.
                }

            #else

                auto p_id = ::fork();
                if (p_id == 0) // Child process.
                {
                    ::setsid(); // Make this process the session leader of a new session.
                    p_id = ::fork(); // Second fork to avoid zombies.
                    if (p_id == 0) // GrandChild process.
                    {
                        process::id = process::getid();
                        ::umask(0); // Set the file mode creation mask for child process (all access bits are set by default).
                        ::close(os::stdin_fd );
                        ::close(os::stdout_fd);
                        ::close(os::stderr_fd);
                        return true;
                    }
                    else if (p_id > 0) os::process::exit(0);
                }
                else if (p_id > 0) // Parent branch. Reap the child, leaving the grandchild to be inherited by init.
                {
                    auto stat = int{};
                    ::waitpid(p_id, &stat, 0);
                    if (WIFEXITED(stat) && (WEXITSTATUS(stat) == 0))
                    {
                        log("  os: process forked");
                        result = true;
                        return faux; // Child forked and exited successfully.
                    }
                }

            #endif
            os::fail("  os: can't fork process");
            return faux;
        }
    }

    namespace logging
    {
        void start(text srv_name)
        {
            os::is_daemon = true;
            #if defined(_WIN32)
                //todo implement
            #else
                ::openlog(srv_name.c_str(), LOG_NOWAIT | LOG_PID, LOG_USER);
            #endif
        }
        void stdlog(view data)
        {
            static auto logs = directvt::binary::logs_t{};
            //todo view -> text
            logs.set(os::process::id.first, os::process::id.second, text{ data });
            logs.send([&](auto& block){ io::send(os::stdout_fd, block); });
        }
        void syslog(view data)
        {
            if (os::is_daemon)
            {
                #if defined(_WIN32)
                    //todo implement
                #else
                    auto copy = text{ data };
                    ::syslog(LOG_NOTICE, "%s", copy.c_str());
                #endif
            }
            else std::cout << data << std::flush;
        }
    }

    namespace vt
    {
        static constexpr auto clean  = 0;
        static constexpr auto mouse  = 1 << 0;
        static constexpr auto vga16  = 1 << 1;
        static constexpr auto vga256 = 1 << 2;
        static constexpr auto direct = 1 << 3;
        template<class T>
        static auto str(T mode)
        {
            auto result = text{};
            if (mode)
            {
                if (mode & mouse ) result += "mouse ";
                if (mode & vga16 ) result += "vga16 ";
                if (mode & vga256) result += "vga256 ";
                if (mode & direct) result += "direct ";
                if (result.size()) result.pop_back();
            }
            else result = "fresh";
            return result;
        }
        static auto console()
        {
            auto conmode = -1;
            #if defined(__linux__)

                if (-1 != ::ioctl(os::stdout_fd, KDGETMODE, &conmode))
                {
                    switch (conmode)
                    {
                        case KD_TEXT:     break;
                        case KD_GRAPHICS: break;
                        default:          break;
                    }
                }

            #endif
            return conmode != -1;
        }
        static auto vgafont(si32 mode)
        {
            #if defined(__linux__)

                if (mode & vt::mouse)
                {
                    auto chars = std::vector<unsigned char>(512 * 32 * 4);
                    auto fdata = console_font_op{ .op        = KD_FONT_OP_GET,
                                                  .flags     = 0,
                                                  .width     = 32,
                                                  .height    = 32,
                                                  .charcount = 512,
                                                  .data      = chars.data() };
                    if (!ok(::ioctl(os::stdout_fd, KDFONTOP, &fdata), "::ioctl(KDFONTOP, KD_FONT_OP_GET)", os::unexpected_msg)) return;

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
                    if (!ok(::ioctl(os::stdout_fd, KDFONTOP, &fdata), "::ioctl(KDFONTOP, KD_FONT_OP_SET)", os::unexpected_msg)) return;

                    auto max_sz = std::numeric_limits<unsigned short>::max();
                    auto spairs = std::vector<unipair>(max_sz);
                    auto dpairs = std::vector<unipair>(max_sz);
                    auto srcmap = unimapdesc{ max_sz, spairs.data() };
                    auto dstmap = unimapdesc{ max_sz, dpairs.data() };
                    auto dstptr = dstmap.entries;
                    auto srcptr = srcmap.entries;
                    if (!ok(::ioctl(os::stdout_fd, GIO_UNIMAP, &srcmap), "::ioctl(os::stdout_fd, GIO_UNIMAP)", os::unexpected_msg)) return;
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
                        if (!ok(::ioctl(os::stdout_fd, PIO_UNIMAP, &dstmap), "::ioctl(os::stdout_fd, PIO_UNIMAP)", os::unexpected_msg)) return;
                    }
                    else log("  os: vgafont loading failed - UNIMAP is full");
                }

            #endif
        }

        template<class Term>
        class vtty // Note: STA.
        {
            Term&                   terminal;
            ipc::stdcon             termlink;
            std::thread             stdinput;
            std::thread             stdwrite;
            std::thread             waitexit;
            testy<twod>             termsize;
            pidt                    proc_pid;
            fd_t                    prochndl;
            fd_t                    srv_hndl;
            fd_t                    ref_hndl;
            text                    writebuf;
            std::mutex              writemtx;
            std::condition_variable writesyn;

            #if defined(_WIN32)
                #include "consrv.hpp"
                //todo con_serv is a termlink
                consrv con_serv{ terminal, srv_hndl };
            #endif

        public:
            vtty(Term& uiterminal)
                : terminal{ uiterminal     },
                  prochndl{ os::invalid_fd },
                  srv_hndl{ os::invalid_fd },
                  ref_hndl{ os::invalid_fd },
                  proc_pid{                }
            { }
           ~vtty()
            {
                log("vtty: dtor started");
                stop();
                log("vtty: dtor complete");
            }

            operator bool () { return connected(); }

            bool connected()
            {
                #if defined(_WIN32)
                    return srv_hndl != os::invalid_fd;
                #else
                    return !!termlink;
                #endif
            }
            // vtty: Cleaning in order to be able to restart.
            void cleanup()
            {
                if (stdwrite.joinable())
                {
                    writesyn.notify_one();
                    log("vtty: id: ", stdwrite.get_id(), " writing thread joining");
                    stdwrite.join();
                }
                if (stdinput.joinable())
                {
                    log("vtty: id: ", stdinput.get_id(), " reading thread joining");
                    stdinput.join();
                }
                if (waitexit.joinable())
                {
                    log("vtty: id: ", waitexit.get_id(), " child process waiter thread joining");
                    waitexit.join();
                }
                auto guard = std::lock_guard{ writemtx };
                termlink = {};
                writebuf = {};
            }
            auto wait_child()
            {
                auto guard = std::lock_guard{ writemtx };
                auto exit_code = si32{};
                log("vtty: wait child process ", proc_pid);

                if (proc_pid != 0)
                {
                #if defined(_WIN32)

                    io::close( srv_hndl );
                    io::close( ref_hndl );
                    con_serv.stop();
                    auto code = DWORD{ 0 };
                    if (!::GetExitCodeProcess(prochndl, &code))
                    {
                        log("vtty: ::GetExitCodeProcess() return code: ", ::GetLastError());
                    }
                    else if (code == STILL_ACTIVE)
                    {
                        log("vtty: child process still running");
                        auto result = WAIT_OBJECT_0 == ::WaitForSingleObject(prochndl, app_wait_timeout /*10 seconds*/);
                        if (!result || !::GetExitCodeProcess(prochndl, &code))
                        {
                            ::TerminateProcess(prochndl, 0);
                            code = 0;
                        }
                    }
                    else log("vtty: child process exit code 0x", utf::to_hex(code), " (", code, ")");
                    exit_code = code;
                    io::close(prochndl);

                #else

                    termlink.stop();
                    auto status = int{};
                    ok(::kill(proc_pid, SIGKILL), "::kill(pid, SIGKILL)", os::unexpected_msg);
                    ok(::waitpid(proc_pid, &status, 0), "::waitpid(pid)", os::unexpected_msg); // Wait for the child to avoid zombies.
                    if (WIFEXITED(status))
                    {
                        exit_code = WEXITSTATUS(status);
                        log("vtty: child process exit code ", exit_code);
                    }
                    else
                    {
                        exit_code = 0;
                        log("vtty: warning: child process exit code not detected");
                    }

                #endif
                }
                log("vtty: child waiting complete");
                return exit_code;
            }
            void start(twod winsz)
            {
                auto cwd     = terminal.curdir;
                auto cmdline = terminal.cmdarg;
                utf::change(cmdline, "\\\"", "\"");
                log("vtty: new child process: '", utf::debase(cmdline), "' at the ", cwd.empty() ? "current working directory"s
                                                                                                 : "'" + cwd + "'");
                #if defined(_WIN32)

                    termsize(winsz);
                    auto startinf = STARTUPINFOEXW{ sizeof(STARTUPINFOEXW) };
                    auto procsinf = PROCESS_INFORMATION{};
                    auto attrbuff = std::vector<byte>{};
                    auto attrsize = SIZE_T{ 0 };

                    srv_hndl = nt::console::handle("\\Device\\ConDrv\\Server");
                    ref_hndl = nt::console::handle(srv_hndl, "\\Reference");

                    if (ERROR_SUCCESS != nt::ioctl(nt::console::op::set_server_information, srv_hndl, con_serv.events.ondata))
                    {
                        auto errcode = os::error();
                        os::fail("vtty: console server creation error");
                        terminal.onexit(errcode, "Console server creation error");
                        return;
                    }

                    con_serv.start();
                    startinf.StartupInfo.dwX = 0;
                    startinf.StartupInfo.dwY = 0;
                    startinf.StartupInfo.dwXCountChars = 0;
                    startinf.StartupInfo.dwYCountChars = 0;
                    startinf.StartupInfo.dwXSize = winsz.x;
                    startinf.StartupInfo.dwYSize = winsz.y;
                    startinf.StartupInfo.dwFillAttribute = 1;
                    startinf.StartupInfo.hStdInput  = nt::console::handle(srv_hndl, "\\Input",  true);
                    startinf.StartupInfo.hStdOutput = nt::console::handle(srv_hndl, "\\Output", true);
                    startinf.StartupInfo.hStdError  = nt::console::handle(startinf.StartupInfo.hStdOutput);
                    startinf.StartupInfo.dwFlags = STARTF_USESTDHANDLES
                                                 | STARTF_USESIZE
                                                 | STARTF_USEPOSITION
                                                 | STARTF_USECOUNTCHARS
                                                 | STARTF_USEFILLATTRIBUTE;
                    ::InitializeProcThreadAttributeList(nullptr, 2, 0, &attrsize);
                    attrbuff.resize(attrsize);
                    startinf.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrbuff.data());
                    ::InitializeProcThreadAttributeList(startinf.lpAttributeList, 2, 0, &attrsize);
                    ::UpdateProcThreadAttribute(startinf.lpAttributeList,
                                                0,
                                                PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                               &startinf.StartupInfo.hStdInput,
                                         sizeof(startinf.StartupInfo.hStdInput) * 3,
                                                nullptr,
                                                nullptr);
                    ::UpdateProcThreadAttribute(startinf.lpAttributeList,
                                                0,
                                                ProcThreadAttributeValue(sizeof("Reference"), faux, true, faux),
                                               &ref_hndl,
                                         sizeof(ref_hndl),
                                                nullptr,
                                                nullptr);
                    auto wide_cmdline = utf::to_utf(cmdline);
                    auto ret = ::CreateProcessW(nullptr,                             // lpApplicationName
                                                wide_cmdline.data(),                 // lpCommandLine
                                                nullptr,                             // lpProcessAttributes
                                                nullptr,                             // lpThreadAttributes
                                                TRUE,                                // bInheritHandles
                                                EXTENDED_STARTUPINFO_PRESENT,        // dwCreationFlags (override startupInfo type)
                                                nullptr,                             // lpEnvironment
                                                cwd.size() ? utf::to_utf(cwd).c_str()// lpCurrentDirectory
                                                           : nullptr,
                                               &startinf.StartupInfo,                // lpStartupInfo (ptr to STARTUPINFOEX)
                                               &procsinf);                           // lpProcessInformation
                    if (ret == 0)
                    {
                        auto errcode = os::error();
                        os::fail("vtty: child process creation error");
                        io::close( srv_hndl );
                        io::close( ref_hndl );
                        con_serv.stop();
                        terminal.onexit(errcode, "Process creation error \n"s
                                                 " cwd: "s + (cwd.empty() ? "not specified"s : cwd) + " \n"s
                                                 " cmd: "s + cmdline + " "s);
                        return;
                    }

                    io::close( procsinf.hThread );
                    prochndl = procsinf.hProcess;
                    proc_pid = procsinf.dwProcessId;
                    waitexit = std::thread([&]
                    {
                        io::select(prochndl, []{ log("vtty: child process terminated"); });
                        if (srv_hndl != os::invalid_fd)
                        {
                            auto exit_code = wait_child();
                            terminal.onexit(exit_code);
                        }
                        log("vtty: child process waiter ended");
                    });

                #else

                    auto fdm = ::posix_openpt(O_RDWR | O_NOCTTY); // Get master PTY.
                    auto rc1 = ::grantpt     (fdm);               // Grant master PTY file access.
                    auto rc2 = ::unlockpt    (fdm);               // Unlock master PTY.
                    auto fds = ::open(ptsname(fdm), O_RDWR);      // Open slave PTY via string ptsname(fdm).

                    termlink = { fdm, fdm };
                    termsize = {};
                    resize(winsz);

                    proc_pid = ::fork();
                    if (proc_pid == 0) // Child branch.
                    {
                        io::close(fdm);
                        auto rc0 = ::setsid(); // Make the current process a new session leader, return process group id.

                        // In order to receive WINCH signal make fds the controlling
                        // terminal of the current process.
                        // Current process must be a session leader (::setsid()) and not have
                        // a controlling terminal already.
                        // arg = 0: 1 - to stole fds from another process, it doesn't matter here.
                        auto rc1 = ::ioctl(fds, TIOCSCTTY, 0);

                        ::signal(SIGINT,  SIG_DFL); // Reset control signals to default values.
                        ::signal(SIGQUIT, SIG_DFL); //
                        ::signal(SIGTSTP, SIG_DFL); //
                        ::signal(SIGTTIN, SIG_DFL); //
                        ::signal(SIGTTOU, SIG_DFL); //
                        ::signal(SIGCHLD, SIG_DFL); //

                        if (cwd.size())
                        {
                            auto err = std::error_code{};
                            fs::current_path(cwd, err);
                            if (err) std::cerr << "vtty: failed to change current working directory to '" << cwd << "', error code: " << err.value() << "\n" << std::flush;
                            else     std::cerr << "vtty: change current working directory to '" << cwd << "'" << "\n" << std::flush;
                        }

                        ::dup2(fds, os::stdin_fd ); // Assign stdio lines atomically
                        ::dup2(fds, os::stdout_fd); // = close(new);
                        ::dup2(fds, os::stderr_fd); // fcntl(old, F_DUPFD, new)
                        os::fdcleanup();

                        ::setenv("TERM", "xterm-256color", 1); //todo too hacky
                        if (os::env::get("TERM_PROGRAM") == "Apple_Terminal")
                        {
                            ::setenv("TERM_PROGRAM", "vtm", 1);
                        }

                        os::process::execvp(cmdline);
                        auto errcode = errno;
                        std::cerr << ansi::bgc(reddk).fgc(whitelt).add("Process creation error ").add(errcode).add(" \n"s
                                                                       " cwd: "s + (cwd.empty() ? "not specified"s : cwd) + " \n"s
                                                                       " cmd: "s + cmdline + " "s).nil() << std::flush;
                        os::process::exit(errcode);
                    }

                    // Parent branch.
                    io::close(fds);

                    stdinput = std::thread([&] { read_socket_thread(); });

                #endif

                stdwrite = std::thread([&] { send_socket_thread(); });
                writesyn.notify_one(); // Flush temp buffer.

                log("vtty: new vtty created with size ", winsz);
            }
            void stop()
            {
                if (connected())
                {
                    wait_child();
                }
                cleanup();
            }
            void read_socket_thread()
            {
                #if not defined(_WIN32)

                    log("vtty: id: ", stdinput.get_id(), " reading thread started");
                    auto flow = text{};
                    while (termlink)
                    {
                        auto shot = termlink.recv();
                        if (shot && termlink)
                        {
                            flow += shot;
                            auto crop = ansi::purify(flow);
                            terminal.ondata(crop);
                            flow.erase(0, crop.size()); // Delete processed data.
                        }
                        else break;
                    }
                    if (termlink)
                    {
                        auto exit_code = wait_child();
                        terminal.onexit(exit_code);
                    }
                    log("vtty: id: ", stdinput.get_id(), " reading thread ended");

                #endif
            }
            void send_socket_thread()
            {
                log("vtty: id: ", stdwrite.get_id(), " writing thread started");
                auto guard = std::unique_lock{ writemtx };
                auto cache = text{};
                while ((void)writesyn.wait(guard, [&]{ return writebuf.size() || !connected(); }), connected())
                {
                    std::swap(cache, writebuf);
                    guard.unlock();
                    #if defined(_WIN32)

                        con_serv.events.write(cache);
                        cache.clear();

                    #else

                        if (termlink.send(cache)) cache.clear();
                        else                      break;

                    #endif
                    guard.lock();
                }
                log("vtty: id: ", stdwrite.get_id(), " writing thread ended");
            }
            void resize(twod const& newsize)
            {
                if (connected() && termsize(newsize))
                {
                    #if defined(_WIN32)

                        con_serv.resize(termsize);

                    #else

                        auto winsz = winsize{};
                        winsz.ws_col = newsize.x;
                        winsz.ws_row = newsize.y;
                        ok(::ioctl(termlink.handle.w, TIOCSWINSZ, &winsz), "::ioctl(termlink.get(), TIOCSWINSZ)", os::unexpected_msg);

                    #endif
                }
            }
            void reset()
            {
                #if defined(_WIN32)
                    con_serv.reset();
                #endif
            }
            void focus(bool state)
            {
                #if defined(_WIN32)
                    con_serv.events.focus(state);
                #endif
            }
            void keybd(input::hids& gear, bool decckm)
            {
                #if defined(_WIN32)
                    con_serv.events.keybd(gear, decckm);
                #endif
            }
            void mouse(input::hids& gear, bool moved, twod const& coord)
            {
                #if defined(_WIN32)
                    con_serv.events.mouse(gear, moved, coord);
                #endif
            }
            void write(view data)
            {
                auto guard = std::lock_guard{ writemtx };
                writebuf += data;
                if (connected()) writesyn.notify_one();
            }
            void undo(bool undoredo)
            {
                #if defined(_WIN32)
                    con_serv.events.undo(undoredo);
                #endif
            }
        };
    }

    namespace dtvt
    {
        namespace
        {
            auto& _state()
            {
                static auto state = faux;
                return state;
            }
            auto& _start()
            {
                static auto start = text{};
                return start;
            }
            auto& _ready()
            {
                static auto ready = faux;
                return ready;
            }
        }
        auto haspty(fd_t stdin_fd)
        {
            #if defined(_WIN32)
                return FILE_TYPE_CHAR == ::GetFileType(stdin_fd);
            #else
                return ::isatty(stdin_fd);
            #endif
        }
        auto& config()
        {
            static auto setup = text{};
            return setup;
        }
        void send(fd_t m_pipe_w, view config)
        {
            auto buffer = directvt::binary::marker{ config.size() };
            io::send(m_pipe_w, buffer);
        }
        auto peek(fd_t stdin_fd)
        {
            auto& ready = _ready();
            auto& state = _state();
            auto& start = _start();
            auto& setup = config();
            auto cfsize = size_t{};
            if (ready) return state;
            ready = true;
            #if defined(_WIN32)

                auto buffer = directvt::binary::marker{};
                auto length = DWORD{ 0 };
                if (haspty(stdin_fd))
                {
                    // ::WaitForMultipleObjects() does not work with pipes (DirectVT).
                    if (::PeekNamedPipe(stdin_fd, buffer.data(), (DWORD)buffer.size(), &length, NULL, NULL)
                     && length)
                    {
                        state = buffer.size() == length && buffer.get_sz(cfsize);
                        if (state)
                        {
                            io::recv(stdin_fd, buffer.data(), buffer.size());
                        }
                    }
                }
                else
                {
                    length = (DWORD)io::recv(stdin_fd, buffer.data(), buffer.size()).size();
                    state = buffer.size() == length && buffer.get_sz(cfsize);
                }

            #else

                auto proc = [&](auto get)
                {
                    get(stdin_fd, [&]
                    {
                        auto buffer = directvt::binary::marker{};
                        auto header = io::recv(stdin_fd, buffer.data(), buffer.size());
                        auto length = header.length();
                        if (length)
                        {
                            state = buffer.size() == length && buffer.get_sz(cfsize);
                            if (!state)
                            {
                                start = header; //todo use it when the reading thread starts
                            }
                        }
                    });
                };
                haspty(stdin_fd) ? proc([&](auto ...args){ return io::select<true>(args...); })
                                 : proc([&](auto ...args){ return io::select<faux>(args...); });

            #endif
            if (cfsize)
            {
                setup.resize(cfsize);
                auto buffer = setup.data();
                while (cfsize)
                {
                    if (auto crop = io::recv(stdin_fd, buffer, cfsize))
                    {
                        cfsize -= crop.size();
                        buffer += crop.size();
                    }
                    else
                    {
                        state = faux;
                        break;
                    }
                }
            }
            return state;
        }

        class vtty
        {
            fd_t                      prochndl{ os::invalid_fd };
            pidt                      proc_pid{};
            ipc::stdcon               termlink{};
            std::thread               stdinput{};
            std::thread               stdwrite{};
            std::function<void(view)> receiver{};
            std::function<void(si32)> shutdown{};
            std::function<void(si32)> preclose{};
            text                      writebuf{};
            std::mutex                writemtx{};
            std::condition_variable   writesyn{};

        public:
           ~vtty()
            {
                log("dtvt: dtor started");
                cleanup();
                log("dtvt: dtor complete");
            }

            operator bool () { return termlink; }

            auto start(text cwd, text cmdline, text config, std::function<void(view)> input_hndl,
                                                            std::function<void(si32)> preclose_hndl,
                                                            std::function<void(si32)> shutdown_hndl)
            {
                receiver = input_hndl;
                preclose = preclose_hndl;
                shutdown = shutdown_hndl;
                utf::change(cmdline, "\\\"", "'");
                log("dtvt: new child process: '", utf::debase(cmdline), "' at the ", cwd.empty() ? "current working directory"s
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
                            os::dtvt::send(m_pipe_w, config);
                            startinf.StartupInfo.dwFlags    = STARTF_USESTDHANDLES;
                            startinf.StartupInfo.hStdInput  = s_pipe_r;
                            startinf.StartupInfo.hStdOutput = s_pipe_w;
                            return true;
                        }
                        else
                        {
                            io::close(m_pipe_w);
                            io::close(m_pipe_r);
                            io::close(s_pipe_w);
                            io::close(s_pipe_r);
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
                        io::close( procsinf.hThread );
                        prochndl = procsinf.hProcess;
                        proc_pid = procsinf.dwProcessId;
                        termlink = { m_pipe_r, m_pipe_w };
                    }
                    else os::fail("dtvt: child process creation error");

                    io::close(s_pipe_w); // Close inheritable handles to avoid deadlocking at process exit.
                    io::close(s_pipe_r); // Only when all write handles to the pipe are closed, the ReadFile function returns zero.

                #else

                    fd_t to_server[2] = { os::invalid_fd, os::invalid_fd };
                    fd_t to_client[2] = { os::invalid_fd, os::invalid_fd };
                    ok(::pipe(to_server), "::pipe(to_server)", os::unexpected_msg);
                    ok(::pipe(to_client), "::pipe(to_client)", os::unexpected_msg);

                    termlink = { to_server[0], to_client[1] };
                    os::dtvt::send(to_client[1], config);

                    proc_pid = ::fork();
                    if (proc_pid == 0) // Child branch.
                    {
                        io::close(to_client[1]);
                        io::close(to_server[0]);

                        ::signal(SIGINT,  SIG_DFL); // Reset control signals to default values.
                        ::signal(SIGQUIT, SIG_DFL); //
                        ::signal(SIGTSTP, SIG_DFL); //
                        ::signal(SIGTTIN, SIG_DFL); //
                        ::signal(SIGTTOU, SIG_DFL); //
                        ::signal(SIGCHLD, SIG_DFL); //

                        ::dup2(to_client[0], os::stdin_fd ); // Assign stdio lines atomically
                        ::dup2(to_server[1], os::stdout_fd); // = close(new); fcntl(old, F_DUPFD, new).
                        os::fdcleanup();

                        if (cwd.size())
                        {
                            auto err = std::error_code{};
                            fs::current_path(cwd, err);
                            //todo use dtvt to log
                            //if (err) os::fail("dtvt: failed to change current working directory to '", cwd, "', error code: ", err.value());
                            //else     log("dtvt: change current working directory to '", cwd, "'");
                        }

                        os::process::execvp(cmdline);
                        auto errcode = errno;
                        //todo use dtvt to log
                        //os::fail("dtvt: exec error");
                        ::close(os::stdout_fd);
                        ::close(os::stdin_fd );
                        os::process::exit(errcode);
                    }

                    // Parent branch.
                    io::close(to_client[0]);
                    io::close(to_server[1]);

                #endif

                if (config.size())
                {
                    auto guard = std::lock_guard{ writemtx };
                    writebuf = config + writebuf;
                }

                stdinput = std::thread([&] { read_socket_thread(); });
                stdwrite = std::thread([&] { send_socket_thread(); });

                if (termlink) log("dtvt: vtty created: proc_pid ", proc_pid);
                writesyn.notify_one(); // Flush temp buffer.

                return proc_pid;
            }
            auto wait_child()
            {
                //auto guard = std::lock_guard{ writemtx };
                auto exit_code = si32{};
                log("dtvt: wait child process, tty=", termlink);
                if (proc_pid)
                {
                    #if defined(_WIN32)

                        auto code = DWORD{ 0 };
                        if (!::GetExitCodeProcess(prochndl, &code))
                        {
                            log("dtvt: ::GetExitCodeProcess() return code: ", ::GetLastError());
                        }
                        else if (code == STILL_ACTIVE)
                        {
                            log("dtvt: child process still running");
                            auto result = WAIT_OBJECT_0 == ::WaitForSingleObject(prochndl, app_wait_timeout /*10 seconds*/);
                            if (!result || !::GetExitCodeProcess(prochndl, &code))
                            {
                                ::TerminateProcess(prochndl, 0);
                                code = 0;
                            }
                        }
                        else log("dtvt: child process exit code ", code);
                        exit_code = code;
                        io::close(prochndl);

                    #else

                        int status;
                        //todo wait app_wait_timeout before kill
                        ok(::kill(proc_pid, SIGKILL), "::kill(pid, SIGKILL)", os::unexpected_msg);
                        ok(::waitpid(proc_pid, &status, 0), "::waitpid(pid)", os::unexpected_msg); // Wait for the child to avoid zombies.
                        if (WIFEXITED(status))
                        {
                            exit_code = WEXITSTATUS(status);
                            log("dtvt: child process exit code ", exit_code);
                        }
                        else
                        {
                            exit_code = 0;
                            log("dtvt: warning: child process exit code not detected");
                        }

                    #endif
                    proc_pid = 0;
                }
                log("dtvt: child waiting complete");
                return exit_code;
            }
            void cleanup()
            {
                if (stdwrite.joinable())
                {
                    writesyn.notify_one();
                    log("dtvt: id: ", stdwrite.get_id(), " writing thread joining");
                    stdwrite.join();
                }
                if (stdinput.joinable())
                {
                    log("dtvt: id: ", stdinput.get_id(), " reading thread joining");
                    stdinput.join();
                }
                auto guard = std::lock_guard{ writemtx };
                termlink = {};
                writebuf = {};
            }
            void shut()
            {
                if (termlink)
                {
                    termlink.shut();
                }
            }
            void read_socket_thread()
            {
                log("dtvt: id: ", stdinput.get_id(), " reading thread started");
                directvt::binary::stream::reading_loop(termlink, receiver);
                preclose(0);
                auto exit_code = wait_child();
                shutdown(exit_code);
                log("dtvt: id: ", stdinput.get_id(), " reading thread ended");
            }
            void send_socket_thread()
            {
                log("dtvt: id: ", stdwrite.get_id(), " writing thread started");
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
                //if (termlink) termlink.shut();
                log("dtvt: id: ", stdwrite.get_id(), " writing thread ended");
            }
            void output(view data)
            {
                auto guard = std::lock_guard{ writemtx };
                writebuf += data;
                if (termlink) writesyn.notify_one();
            }
        };
    }

    namespace tty
    {
        auto& globals()
        {
            struct
            {
                xipc        ipcio; // globals: STDIN/OUT.
                conmode     state; // globals: Saved console mode to restore at exit.
                testy<twod> winsz; // globals: Current console window size.
                s11n        wired; // globals: Serialization buffers.
                si32        kbmod; // globals: Keyboard modifiers state.
                io::fire    alarm; // globals: IO interrupter.
            }
            static vars;
            return vars;
        }
        void repair()
        {
            auto& state = globals().state;
            #if defined(_WIN32)
                ok(::SetConsoleMode(os::stdout_fd, state.omode), "::SetConsoleMode(revert_o)", os::unexpected_msg);
                ok(::SetConsoleMode(os::stdin_fd , state.imode), "::SetConsoleMode(revert_i)", os::unexpected_msg);
                ok(::SetConsoleOutputCP(           state.opage), "::SetConsoleOutputCP(revert_o)", os::unexpected_msg);
                ok(::SetConsoleCP(                 state.ipage), "::SetConsoleCP(revert_i)", os::unexpected_msg);
            #else
                ::tcsetattr(os::stdin_fd, TCSANOW, &state);
            #endif
        }
        auto vtmode()
        {
            auto mode = si32{ vt::clean };
            if (os::dtvt::peek(os::stdin_fd))
            {
                log("  os: DirectVT detected");
                mode |= vt::direct;
            }
            else
            {
                auto& state = globals().state;
                #if defined(_WIN32)

                    ok(::GetConsoleMode(os::stdout_fd, &state.omode), "::GetConsoleMode(os::stdout_fd)", os::unexpected_msg);
                    ok(::GetConsoleMode(os::stdin_fd , &state.imode), "::GetConsoleMode(os::stdin_fd)", os::unexpected_msg);
                    state.opage = ::GetConsoleOutputCP();
                    state.ipage = ::GetConsoleCP();
                    ok(::SetConsoleOutputCP(65001), "::SetConsoleOutputCP()", os::unexpected_msg);
                    ok(::SetConsoleCP(65001), "::SetConsoleCP()", os::unexpected_msg);
                    auto inpmode = DWORD{ 0
                                | nt::console::inmode::preprocess
                                | nt::console::inmode::extended
                                | nt::console::inmode::winsize
                                };
                    ok(::SetConsoleMode(os::stdin_fd, inpmode), "::SetConsoleMode(os::stdin_fd)", os::unexpected_msg);
                    auto outmode = DWORD{ 0
                                | nt::console::outmode::preprocess
                                | nt::console::outmode::vt
                                };
                    ok(::SetConsoleMode(os::stdout_fd, outmode), "::SetConsoleMode(os::stdout_fd)", os::unexpected_msg);

                #else

                    if (!ok(::tcgetattr(os::stdin_fd, &state), "::tcgetattr(os::stdin_fd)", os::unexpected_msg))
                    {
                        os::fail("warning: check you are using the proper tty device, try `ssh -tt ...` option");
                    }

                #endif
                std::atexit(repair);

                if (auto term = os::env::get("TERM"); term.size())
                {
                    log("  os: terminal type \"", term, "\"");

                    auto vga16colors = { // https://github.com//termstandard/colors
                        "ansi",
                        "linux",
                        "xterm-color",
                        "dvtm", //todo track: https://github.com/martanne/dvtm/issues/10
                        "fbcon",
                    };
                    auto vga256colors = {
                        "rxvt-unicode-256color",
                    };

                    if (term.ends_with("16color") || term.ends_with("16colour"))
                    {
                        mode |= vt::vga16;
                    }
                    else
                    {
                        for (auto& type : vga16colors)
                        {
                            if (term == type)
                            {
                                mode |= vt::vga16;
                                break;
                            }
                        }
                        if (!mode)
                        {
                            for (auto& type : vga256colors)
                            {
                                if (term == type)
                                {
                                    mode |= vt::vga256;
                                    break;
                                }
                            }
                        }
                    }

                    if (os::env::get("TERM_PROGRAM") == "Apple_Terminal")
                    {
                        log("  os: macOS Apple_Terminal detected");
                        if (!(mode & vt::vga16)) mode |= vt::vga256;
                    }

                    if (os::vt::console()) mode |= vt::mouse;

                    log("  os: color mode: ", mode & vt::vga16  ? "16-color"
                                            : mode & vt::vga256 ? "256-color"
                                                                : "true-color");
                    log("  os: mouse mode: ", mode & vt::mouse ? "console" : "vt-style");
                }
            }
            return mode;
        }
        void resize()
        {
            static constexpr auto winsz_fallback = twod{ 132, 60 };
            auto& g = globals();
            auto& ipcio =*g.ipcio;
            auto& winsz = g.winsz;
            auto& wired = g.wired;

            #if defined(_WIN32)

                auto cinfo = CONSOLE_SCREEN_BUFFER_INFO{};
                if (ok(::GetConsoleScreenBufferInfo(os::stdout_fd, &cinfo), "::GetConsoleScreenBufferInfo", os::unexpected_msg))
                {
                    winsz({ cinfo.srWindow.Right  - cinfo.srWindow.Left + 1,
                            cinfo.srWindow.Bottom - cinfo.srWindow.Top  + 1 });
                }

            #else

                auto size = winsize{};
                if (ok(::ioctl(os::stdout_fd, TIOCGWINSZ, &size), "::ioctl(os::stdout_fd, TIOCGWINSZ)", os::unexpected_msg))
                {
                    winsz({ size.ws_col, size.ws_row });
                }

            #endif

            if (winsz == dot_00)
            {
                log("xtty: fallback tty window size ", winsz_fallback, " (consider using 'ssh -tt ...')");
                winsz(winsz_fallback);
            }
            wired.winsz.send(ipcio, 0, winsz.last);
        }
        auto signal(sigt what)
        {
            #if defined(_WIN32)

                auto& g = globals();
                switch (what)
                {
                    case CTRL_C_EVENT:
                    {
                        /* placed to the input buffer - ENABLE_PROCESSED_INPUT is disabled */
                        /* never happen */
                        break;
                    }
                    case CTRL_BREAK_EVENT:
                    {
                        auto dwControlKeyState = g.kbmod;
                        auto wVirtualKeyCode  = ansi::ctrl_break;
                        auto wVirtualScanCode = ansi::ctrl_break;
                        auto bKeyDown = faux;
                        auto wRepeatCount = 1;
                        auto UnicodeChar = L'\x03'; // ansi::C0_ETX
                        g.wired.syskeybd.send(*g.ipcio,
                            0,
                            os::nt::kbstate(g.kbmod, dwControlKeyState),
                            dwControlKeyState,
                            wVirtualKeyCode,
                            wVirtualScanCode,
                            bKeyDown,
                            wRepeatCount,
                            UnicodeChar ? utf::to_utf(UnicodeChar) : text{},
                            UnicodeChar,
                            faux);
                        break;
                    }
                    case CTRL_CLOSE_EVENT:
                    case CTRL_LOGOFF_EVENT:
                    case CTRL_SHUTDOWN_EVENT:
                        g.ipcio->shut();
                        std::this_thread::sleep_for(5000ms); // The client will shut down before this timeout expires.
                        break;
                    default:
                        break;
                }
                return TRUE;

            #else

                auto shutdown = [](auto what)
                {
                    globals().ipcio->shut();
                    ::signal(what, SIG_DFL);
                    ::raise(what);
                };
                switch (what)
                {
                    case SIGWINCH: resize(); return;
                    case SIGHUP:   log(" tty: SIGHUP");  shutdown(what); break;
                    case SIGTERM:  log(" tty: SIGTERM"); shutdown(what); break;
                    default:       log(" tty: signal ", what); break;
                }

            #endif
        }
        auto logger(si32 mode, bool wipe = faux)
        {
            if (wipe) netxs::logger::wipe();
            auto direct = !!(mode & os::vt::direct);
            return direct ? netxs::logger([](auto data) { os::logging::stdlog(data); })
                          : netxs::logger([](auto data) { os::logging::syslog(data); });
        }
        void direct(xipc link)
        {
            auto cout = os::ipc::stdio();
            auto& extio = *cout;
            auto& ipcio = *link;
            auto input = std::thread{ [&]
            {
                while (extio && extio.send(ipcio.recv())) { }
                extio.shut();
            }};
            while (ipcio && ipcio.send(extio.recv())) { }
            ipcio.shut();
            input.join();
        }
        void reader(si32 mode)
        {
            log(" tty: id: ", std::this_thread::get_id(), " reading thread started");
            auto& g = globals();
            auto& ipcio =*g.ipcio;
            auto& wired = g.wired;
            auto& alarm = g.alarm;

            #if defined(_WIN32)

                // The input codepage to UTF-8 is severely broken in all Windows versions.
                // ReadFile and ReadConsoleA either replace non-ASCII characters with NUL
                // or return 0 bytes read.
                auto reply = std::vector<INPUT_RECORD>(1);
                auto count = DWORD{};
                auto stamp = ui32{};
                fd_t waits[] = { os::stdin_fd, alarm };
                while (WAIT_OBJECT_0 == ::WaitForMultipleObjects(2, waits, FALSE, INFINITE))
                {
                    if (!::GetNumberOfConsoleInputEvents(os::stdin_fd, &count))
                    {
                        // ERROR_PIPE_NOT_CONNECTED
                        // 233 (0xE9)
                        // No process is on the other end of the pipe.
                        os::process::exit(-1, " tty: ::GetNumberOfConsoleInputEvents()", os::unexpected_msg, " ", ::GetLastError());
                        break;
                    }
                    else if (count)
                    {
                        if (count > reply.size()) reply.resize(count);

                        if (!::ReadConsoleInputW(os::stdin_fd, reply.data(), (DWORD)reply.size(), &count))
                        {
                            //ERROR_PIPE_NOT_CONNECTED = 0xE9 - it's means that the console is gone/crashed
                            os::process::exit(-1, " tty: ::ReadConsoleInput()", os::unexpected_msg, " ", ::GetLastError());
                            break;
                        }
                        else
                        {
                            auto entry = reply.begin();
                            auto limit = entry + count;
                            while (entry != limit)
                            {
                                auto& reply = *entry++;
                                switch (reply.EventType)
                                {
                                    case KEY_EVENT:
                                        wired.syskeybd.send(ipcio,
                                            0,
                                            os::nt::kbstate(g.kbmod, reply.Event.KeyEvent.dwControlKeyState, reply.Event.KeyEvent.wVirtualScanCode, reply.Event.KeyEvent.bKeyDown),
                                            reply.Event.KeyEvent.dwControlKeyState,
                                            reply.Event.KeyEvent.wVirtualKeyCode,
                                            reply.Event.KeyEvent.wVirtualScanCode,
                                            reply.Event.KeyEvent.bKeyDown,
                                            reply.Event.KeyEvent.wRepeatCount,
                                            reply.Event.KeyEvent.uChar.UnicodeChar ? utf::to_utf(reply.Event.KeyEvent.uChar.UnicodeChar) : text{},
                                            reply.Event.KeyEvent.uChar.UnicodeChar,
                                            faux);
                                        break;
                                    case MOUSE_EVENT:
                                        wired.sysmouse.send(ipcio,
                                            0,
                                            input::hids::stat::ok,
                                            os::nt::kbstate(g.kbmod, reply.Event.MouseEvent.dwControlKeyState),
                                            reply.Event.MouseEvent.dwControlKeyState,
                                            reply.Event.MouseEvent.dwButtonState & 0b00011111,
                                            reply.Event.MouseEvent.dwEventFlags & DOUBLE_CLICK,
                                            reply.Event.MouseEvent.dwEventFlags & MOUSE_WHEELED,
                                            reply.Event.MouseEvent.dwEventFlags & MOUSE_HWHEELED,
                                            static_cast<int16_t>((0xFFFF0000 & reply.Event.MouseEvent.dwButtonState) >> 16), // dwButtonState too large when mouse scrolls
                                            twod{ reply.Event.MouseEvent.dwMousePosition.X, reply.Event.MouseEvent.dwMousePosition.Y },
                                            stamp++);
                                        break;
                                    case WINDOW_BUFFER_SIZE_EVENT:
                                        resize();
                                        break;
                                    case FOCUS_EVENT:
                                        wired.sysfocus.send(ipcio,
                                            0,
                                            reply.Event.FocusEvent.bSetFocus,
                                            faux,
                                            faux);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }

            #else

                auto legacy_mouse = !!(mode & os::vt::mouse);
                auto legacy_color = !!(mode & os::vt::vga16);
                auto micefd = os::invalid_fd;
                auto mcoord = twod{};
                auto buffer = text(os::pipebuf, '\0');
                auto ttynum = si32{ 0 };

                auto get_kb_state = []
                {
                    auto state = si32{ 0 };
                    #if defined(__linux__)
                        auto shift_state = si32{ 6 };
                        ok(::ioctl(os::stdin_fd, TIOCLINUX, &shift_state), "::ioctl(os::stdin_fd, TIOCLINUX)", os::unexpected_msg);
                        state = 0
                            | (shift_state & (1 << KG_ALTGR)) >> 1 // 0x1
                            | (shift_state & (1 << KG_ALT  )) >> 2 // 0x2
                            | (shift_state & (1 << KG_CTRLR)) >> 5 // 0x4
                            | (shift_state & (1 << KG_CTRL )) << 1 // 0x8
                            | (shift_state & (1 << KG_SHIFT)) << 4 // 0x10
                            ;
                    #endif
                    return state;
                };
                ok(::ttyname_r(os::stdout_fd, buffer.data(), buffer.size()), "::ttyname_r(os::stdout_fd)", os::unexpected_msg);
                auto tty_name = view(buffer.data());
                log(" tty: pseudoterminal ", tty_name);
                if (legacy_mouse)
                {
                    log(" tty: compatibility mode");
                    auto imps2_init_string = "\xf3\xc8\xf3\x64\xf3\x50"sv;
                    auto mouse_device = "/dev/input/mice";
                    auto mouse_fallback1 = "/dev/input/mice.vtm";
                    auto mouse_fallback2 = "/dev/input/mice_vtm"; //todo deprecated
                    auto fd = ::open(mouse_device, O_RDWR);
                    if (fd == -1) fd = ::open(mouse_fallback1, O_RDWR);
                    if (fd == -1) log(" tty: error opening ", mouse_device, " and ", mouse_fallback1, ", error ", errno, errno == 13 ? " - permission denied" : "");
                    if (fd == -1) fd = ::open(mouse_fallback2, O_RDWR);
                    if (fd == -1) log(" tty: error opening ", mouse_device, " and ", mouse_fallback2, ", error ", errno, errno == 13 ? " - permission denied" : "");
                    else if (io::send(fd, imps2_init_string))
                    {
                        char ack;
                        io::recv(fd, &ack, sizeof(ack));
                        micefd = fd;
                        auto tty_word = tty_name.find("tty", 0);
                        if (tty_word != text::npos)
                        {
                            tty_word += 3; /*skip tty letters*/
                            auto tty_number = utf::to_view(buffer.data() + tty_word, buffer.size() - tty_word);
                            if (auto cur_tty = utf::to_int(tty_number))
                            {
                                ttynum = cur_tty.value();
                            }
                        }
                        wired.mouse_show.send(ipcio, true);
                        if (ack == '\xfa') log(" tty: ImPS/2 mouse connected, fd: ", fd);
                        else               log(" tty: unknown PS/2 mouse connected, fd: ", fd, " ack: ", (int)ack);
                    }
                    else
                    {
                        log(" tty: mouse initialization error");
                        io::close(fd);
                    }
                }

                auto m = input::sysmouse{};
                auto k = input::syskeybd{};
                auto f = input::sysfocus{};
                auto close = faux;
                auto total = text{};
                auto digit = [](auto c) { return c >= '0' && c <= '9'; };

                // The following sequences are processed here:
                // ESC
                // ESC ESC
                // ESC [ I
                // ESC [ O
                // ESC [ < 0 ; x ; y M/m
                auto filter = [&](view accum)
                {
                    total += accum;
                    auto strv = view{ total };

                    //#ifndef PROD
                    //if (close)
                    //{
                    //    close = faux;
                    //    notify(e2::conio::preclose, close);
                    //    if (total.front() == '\033') // two consecutive escapes
                    //    {
                    //        log("\t - two consecutive escapes: \n\tstrv:        ", strv);
                    //        notify(e2::conio::quit, "pipe two consecutive escapes");
                    //        return;
                    //    }
                    //}
                    //#endif

                    //todo unify (it is just a proof of concept)
                    while (auto len = strv.size())
                    {
                        auto pos = 0_sz;
                        auto unk = faux;

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
                            //    notify(e2::conio::quit, "pipe2: two consecutive escapes");
                            //    break;
                            //}
                            //#else
                            if (pos == len) // the only one esc
                            {
                                // Pass Esc.
                                k.gear_id = 0;
                                k.pressed = true;
                                k.cluster = strv.substr(0, 1);
                                wired.syskeybd.send(ipcio, k);
                                total.clear();
                                break;
                            }
                            else if (strv.at(pos) == '\033') // two consecutive escapes
                            {
                                // Pass Esc.
                                k.gear_id = 0;
                                k.pressed = true;
                                k.cluster = strv.substr(0, 1);
                                wired.syskeybd.send(ipcio, k);
                                total = strv.substr(1);
                                break;
                            }
                            //#endif
                            else if (strv.at(pos) == '[')
                            {
                                if (++pos == len) { total = strv; break; }//incomlpete
                                if (strv.at(pos) == 'I')
                                {
                                    f.gear_id = 0;
                                    f.state = true;
                                    wired.sysfocus.send(ipcio, f);
                                    ++pos;
                                }
                                else if (strv.at(pos) == 'O')
                                {
                                    f.gear_id = 0;
                                    f.state = faux;
                                    wired.sysfocus.send(ipcio, f);
                                    ++pos;
                                }
                                else if (strv.at(pos) == '<') // \033[<0;x;yM/m
                                {
                                    if (++pos == len) { total = strv; break; }// incomlpete sequence

                                    auto tmp = strv.substr(pos);
                                    auto l = tmp.size();
                                    if (auto ctrl = utf::to_int(tmp))
                                    {
                                        pos += l - tmp.size();
                                        if (pos == len) { total = strv; break; }// incomlpete sequence
                                        {
                                            if (++pos == len) { total = strv; break; }// incomlpete sequence

                                            auto tmp = strv.substr(pos);
                                            auto l = tmp.size();
                                            if (auto pos_x = utf::to_int(tmp))
                                            {
                                                pos += l - tmp.size();
                                                if (pos == len) { total = strv; break; }// incomlpete sequence
                                                {
                                                    if (++pos == len) { total = strv; break; }// incomlpete sequence

                                                    auto tmp = strv.substr(pos);
                                                    auto l = tmp.size();
                                                    if (auto pos_y = utf::to_int(tmp))
                                                    {
                                                        pos += l - tmp.size();
                                                        if (pos == len) { total = strv; break; }// incomlpete sequence
                                                        {
                                                            auto ispressed = (strv.at(pos) == 'M');
                                                            ++pos;

                                                            auto clamp = [](auto a) { return std::clamp(a,
                                                                std::numeric_limits<si32>::min() / 2,
                                                                std::numeric_limits<si32>::max() / 2); };

                                                            auto x = clamp(pos_x.value() - 1);
                                                            auto y = clamp(pos_y.value() - 1);
                                                            auto ctl = ctrl.value();

                                                            m.gear_id = {};
                                                            m.enabled = {};
                                                            m.winctrl = {};
                                                            m.doubled = {};
                                                            m.wheeled = {};
                                                            m.hzwheel = {};
                                                            m.wheeldt = {};
                                                            m.ctlstat = {};
                                                            // 000 000 00
                                                            //   | ||| ||
                                                            //   | ||| ------ button number
                                                            //   | ---------- ctl state
                                                            if (ctl & 0x04) m.ctlstat |= input::hids::LShift;
                                                            if (ctl & 0x08) m.ctlstat |= input::hids::LAlt;
                                                            if (ctl & 0x10) m.ctlstat |= input::hids::LCtrl;
                                                            ctl &= ~0b00011100;

                                                            if (ctl == 35 && m.buttons) // Moving without buttons (case when second release not fired: apple's terminal.app)
                                                            {
                                                                m.buttons = {};
                                                                m.changed++;
                                                                wired.sysmouse.send(ipcio, m);
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
                                                            wired.sysmouse.send(ipcio, m);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    unk = true;
                                    pos = 0_sz;
                                }
                            }
                            else
                            {
                                unk = true;
                                pos = 0_sz;
                            }
                        }

                        if (!unk)
                        {
                            total = strv.substr(pos);
                            strv = total;
                        }

                        if (auto size = strv.size())
                        {
                            auto i = unk ? 1_sz : 0_sz;
                            while (i != size && (strv.at(i) != '\033'))
                            {
                                // Pass SIGINT inside the desktop
                                //if (strv.at(i) == 3 /*3 - SIGINT*/)
                                //{
                                //	log(" - SIGINT in stdin");
                                //	owner.SIGNAL(tier::release, e2::conio::quit, "pipe: SIGINT");
                                //	return;
                                //}
                                i++;
                            }

                            if (i)
                            {
                                k.gear_id = 0;
                                k.pressed = true;
                                k.cluster = strv.substr(0, i);
                                wired.syskeybd.send(ipcio, k);
                                total = strv.substr(i);
                                strv = total;
                            }
                        }
                    }
                };

                auto h_proc = [&]
                {
                    auto data = io::recv(os::stdin_fd, buffer.data(), buffer.size());
                    if (micefd != os::invalid_fd)
                    {
                        auto kb_state = get_kb_state();
                        if (m.ctlstat != kb_state)
                        {
                            m.ctlstat = kb_state;
                            wired.ctrls.send(ipcio,
                                m.gear_id,
                                m.ctlstat);
                        }
                     }
                    filter(data);
                };
                auto m_proc = [&]
                {
                    auto data = io::recv(micefd, buffer.data(), buffer.size());
                    auto size = data.size();
                    if (size == 4 /* ImPS/2 */
                     || size == 3 /* PS/2 compatibility mode */)
                    {
                    #if defined(__linux__)
                        auto vt_state = vt_stat{};
                        ok(::ioctl(os::stdout_fd, VT_GETSTATE, &vt_state), "::ioctl(VT_GETSTATE)", os::unexpected_msg);
                        if (vt_state.v_active == ttynum) // Proceed current active tty only.
                        {
                            auto scale = twod{ 6,12 }; //todo magic numbers
                            auto limit = g.winsz.last * scale;
                            auto bttns = data[0] & 7;
                            mcoord.x  += data[1];
                            mcoord.y  -= data[2];
                            if (bttns == 0) mcoord = std::clamp(mcoord, dot_00, limit);
                            m.wheeldt = size == 4 ? -data[3] : 0;
                            m.wheeled = m.wheeldt;
                            m.coordxy = { mcoord / scale };
                            m.buttons = bttns;
                            m.ctlstat = get_kb_state();
                            m.changed++;
                            wired.sysmouse.send(ipcio, m);
                        }
                    #endif
                    }
                };
                auto f_proc = [&]
                {
                    alarm.flush();
                };

                while (ipcio)
                {
                    if (micefd != os::invalid_fd)
                    {
                        io::select(os::stdin_fd, h_proc,
                                   micefd,       m_proc,
                                   alarm,        f_proc);
                    }
                    else
                    {
                        io::select(os::stdin_fd, h_proc,
                                   alarm,        f_proc);
                    }
                }

                io::close(micefd);

            #endif

            log(" tty: id: ", std::this_thread::get_id(), " reading thread ended");
        }
        void clipbd(si32 mode)
        {
            using namespace os::clipboard;

            if (mode & vt::direct) return;
            log(" tty: id: ", std::this_thread::get_id(), " clipboard watcher thread started");

            #if defined(_WIN32)

                auto wndname = utf::to_utf("vtmWindowClass");
                auto wndproc = [](auto hwnd, auto uMsg, auto wParam, auto lParam)
                {
                    auto& g = globals();
                    auto& ipcio =*g.ipcio;
                    auto& wired = g.wired;
                    auto gear_id = id_t{ 0 };
                    switch (uMsg)
                    {
                        case WM_CREATE:
                            ok(::AddClipboardFormatListener(hwnd), "::AddClipboardFormatListener()", os::unexpected_msg);
                        case WM_CLIPBOARDUPDATE:
                        {
                            auto lock = std::lock_guard{ os::clipboard::mutex };
                            while (!::OpenClipboard(hwnd)) // Waiting clipboard access.
                            {
                                if (os::error() != ERROR_ACCESS_DENIED)
                                {
                                    auto error = utf::concat("::OpenClipboard()", os::unexpected_msg, " code ", os::error());
                                    wired.osclipdata.send(ipcio, gear_id, error, ansi::clip::textonly);
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
                                            auto data = view((char*)lptr, size - 1/*trailing null*/);
                                            auto mime = ansi::clip::disabled;
                                            wired.osclipdata.send(ipcio, gear_id, text{ data }, mime);
                                            ::GlobalUnlock(hglb);
                                        }
                                        else
                                        {
                                            auto error = utf::concat("::GlobalLock()", os::unexpected_msg, " code ", os::error());
                                            wired.osclipdata.send(ipcio, gear_id, error, ansi::clip::textonly);
                                        }
                                    }
                                    else do
                                    {
                                        if (format == cf_text)
                                        {
                                            auto mime = hidden ? ansi::clip::safetext : ansi::clip::textonly;
                                            if (auto hglb = ::GetClipboardData(format))
                                            if (auto lptr = ::GlobalLock(hglb))
                                            {
                                                auto size = ::GlobalSize(hglb);
                                                wired.osclipdata.send(ipcio, gear_id, utf::to_utf((wchr*)lptr, size / 2 - 1/*trailing null*/), mime);
                                                ::GlobalUnlock(hglb);
                                                break;
                                            }
                                            auto error = utf::concat("::GlobalLock()", os::unexpected_msg, " code ", os::error());
                                            wired.osclipdata.send(ipcio, gear_id, error, ansi::clip::textonly);
                                        }
                                        else
                                        {
                                            //todo proceed other formats (rich/html/...)
                                        }
                                    }
                                    while (format = ::EnumClipboardFormats(format));
                                }
                                else wired.osclipdata.send(ipcio, gear_id, text{}, ansi::clip::textonly);
                            }
                            ok(::CloseClipboard(), "::CloseClipboard()", os::unexpected_msg);
                            break;
                        }
                        case WM_DESTROY:
                            ok(::RemoveClipboardFormatListener(hwnd), "::RemoveClipboardFormatListener()", os::unexpected_msg);
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
                if (ok(::RegisterClassExW(&wnddata) || os::error() == ERROR_CLASS_ALREADY_EXISTS, "::RegisterClassExW()", os::unexpected_msg))
                {
                    auto& alarm = globals().alarm;
                    auto hwnd = ::CreateWindowExW(0, wndname.c_str(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                    auto next = MSG{};
                    while (next.message != WM_QUIT)
                    {
                        if (auto yield = ::MsgWaitForMultipleObjects(1, (fd_t*)&alarm, FALSE, INFINITE, QS_ALLINPUT);
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

            log(" tty: id: ", std::this_thread::get_id(), " clipboard watcher thread ended");
        }
        void ignite(xipc pipe, si32 mode)
        {
            auto& g = globals();
            g.ipcio = pipe;
            auto& ipcio =*g.ipcio;
            auto& wired = g.wired;
            auto& sig_hndl = signal;

            #if defined(_WIN32)

                auto inpmode = DWORD{ 0
                            | nt::console::inmode::extended
                            | nt::console::inmode::winsize
                            | nt::console::inmode::mouse
                            };
                ok(::SetConsoleMode(os::stdin_fd, inpmode), "::SetConsoleMode(os::stdin_fd, ignite)", os::unexpected_msg);
                auto outmode = DWORD{ 0
                            | nt::console::outmode::no_auto_cr
                            | nt::console::outmode::preprocess
                            | nt::console::outmode::vt
                            };
                ok(::SetConsoleMode(os::stdout_fd, outmode), "::SetConsoleMode(os::stdout_fd, ignite)", os::unexpected_msg);
                ok(::SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(sig_hndl), TRUE), "::SetConsoleCtrlHandler()", os::unexpected_msg);

            #else

                auto raw_mode = g.state;
                ::cfmakeraw(&raw_mode);
                ok(::tcsetattr(os::stdin_fd, TCSANOW, &raw_mode), "::tcsetattr(os::stdin_fd, TCSANOW)", os::unexpected_msg);
                ok(::signal(SIGPIPE , SIG_IGN ), "::signal(SIGPIPE)", os::unexpected_msg);
                ok(::signal(SIGWINCH, sig_hndl), "::signal(SIGWINCH)", os::unexpected_msg);
                ok(::signal(SIGTERM , sig_hndl), "::signal(SIGTERM)", os::unexpected_msg);
                ok(::signal(SIGHUP  , sig_hndl), "::signal(SIGHUP)", os::unexpected_msg);

            #endif

            os::vt::vgafont(mode);
            resize();
            wired.sysfocus.send(ipcio, id_t{}, true, faux, faux);
        }
        auto splice(xipc pipe, si32 mode)
        {
            ignite(pipe, mode);
            auto& ipcio = *pipe;
            auto& alarm = globals().alarm;
            auto  proxy = os::clipboard::proxy{};
            auto  vga16 = mode & os::vt::vga16;
            auto  vtrun = ansi::save_title().altbuf(true).cursor(faux).bpmode(true).set_palette(vga16);
            auto  vtend = ansi::scrn_reset().altbuf(faux).cursor(true).bpmode(faux).load_title().rst_palette(vga16);
            #if not defined(_WIN32) // Use Win32 Console API for mouse tracking on Windows.
                vtrun.vmouse(true).setutf(true);
                vtend.vmouse(faux);
            #endif

            io::send(os::stdout_fd, vtrun);

            auto input = std::thread{ [&]{ reader(mode); } };
            auto clips = std::thread{ [&]{ clipbd(mode); } }; //todo move to os::clipboard::proxy (globals())
            while (auto yield = ipcio.recv())
            {
                if (proxy(yield) && !io::send(os::stdout_fd, yield)) break;
            }

            ipcio.shut();
            alarm.bell();
            clips.join();
            input.join();

            io::send(os::stdout_fd, vtend);
            std::this_thread::sleep_for(200ms); // Pause to complete consuming/receiving buffered input (e.g. mouse tracking) that has just been canceled.
        }
    };
}