// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_SYSTEM_HPP
#define NETXS_SYSTEM_HPP

#ifndef VTM_USE_CLASSICAL_WIN32_INPUT
#define VTM_USE_CLASSICAL_WIN32_INPUT // Turns on classical console win32 input mode.
#endif

#if (defined(__unix__) || defined(__APPLE__)) && !defined(__linux__)
    #define __BSD__
#endif

#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunused-function"
#endif

#include "file_system.hpp"
#include "../text/logger.hpp"
#include "../datetime/quartz.hpp"
#include "../abstract/ptr.hpp"
#include "../console/ansi.hpp"
#include "../ui/layout.hpp"

#include <type_traits>
#include <iostream>         // std::cout

#if defined(_WIN32)

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #pragma warning(disable:4996) // disable std::getenv warnimg
    #pragma comment(lib, "Advapi32.lib")  // GetUserName()

    #include <Windows.h>
    #include <Psapi.h>      // GetModuleFileNameEx
    #include <Wtsapi32.h>   // WTSEnumerateSessions, get_logged_usres

    #include <Shlwapi.h>
    #include <algorithm>
    #include <Wtsapi32.h>
    #include <shobjidl.h>   // IShellLink
    #include <shlguid.h>    // IID_IShellLink
    #include <Shlobj.h>     // create_shortcut: SHGetFolderLocation / (SHGetFolderPathW - vist and later) CLSID_ShellLink
    #include <Psapi.h>      // GetModuleFileNameEx

    #include <DsGetDC.h>    // DsGetDcName
    #include <LMCons.h>     // DsGetDcName
    #include <Lmapibuf.h>   // DsGetDcName

    #include <Sddl.h>       //security_descriptor

#else

    #include <errno.h>      // ::errno
    #include <spawn.h>      // ::exec
    #include <unistd.h>     // ::gethostname(), ::getpid(), ::read()
    #include <sys/param.h>  //
    #include <sys/types.h>  // ::getaddrinfo
    #include <sys/socket.h> // ::shutdown() ::socket(2)
    #include <netdb.h>      //

    #include <stdio.h>
    #include <unistd.h>     // ::read(), PIPE_BUF
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
        #include <sys/kd.h> // ::console_ioctl()
        #include <linux/keyboard.h> // ::keyb_ioctl()
    #endif
#endif

namespace netxs::os
{
    using list = std::vector<text>;
    using ui32 = uint32_t;
    using iota = int32_t;
    using xipc = std::shared_ptr<class ipc>;
    using namespace std::chrono_literals;
    using namespace netxs::console;
    using namespace netxs::ui::atoms;

    enum role { client, server };

    static constexpr iota STDIN_BUF = 1024;
    static bool is_daemon = faux;

    #if defined(_WIN32)

        using fd_t = HANDLE;
        using conmode = DWORD[2];
        static const fd_t INVALID_FD = INVALID_HANDLE_VALUE;
        static const fd_t STDIN_FD  = GetStdHandle(STD_INPUT_HANDLE);
        static const fd_t STDOUT_FD = GetStdHandle(STD_OUTPUT_HANDLE);
        static const iota PIPE_BUF = 65536;

        //static constexpr char* security_descriptor_string =
        //	//"D:P(A;NP;GA;;;SY)(A;NP;GA;;;BA)(A;NP;GA;;;WD)";
        //	"O:AND:P(A;NP;GA;;;SY)(A;NP;GA;;;BA)(A;NP;GA;;;CO)";
        //	//"D:P(A;NP;GA;;;SY)(A;NP;GA;;;BA)(A;NP;GA;;;AU)";// Authenticated users
        //	//"D:P(A;NP;GA;;;SY)(A;NP;GA;;;BA)(A;NP;GA;;;CO)"; // CREATOR OWNER
        //	//"D:P(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;CO)";
        //	//  "D:"  DACL
        //	//  "P"   SDDL_PROTECTED        The SE_DACL_PROTECTED flag is set.
        //	//  "A"   SDDL_ACCESS_ALLOWED
        //	// ace_flags:
        //	//  "OI"  SDDL_OBJECT_INHERIT
        //	//  "CI"  SDDL_CONTAINER_INHERIT
        //	//  "NP"  SDDL_NO_PROPAGATE
        //	// rights:
        //	//  "GA"  SDDL_GENERIC_ALL
        //	// account_sid: see https://docs.microsoft.com/en-us/windows/win32/secauthz/sid-strings
        //	//  "SY"  SDDL_LOCAL_SYSTEM
        //	//  "BA"  SDDL_BUILTIN_ADMINISTRATORS
        //	//  "CO"  SDDL_CREATOR_OWNER
        //	//  "WD"  SDDL_EVERYONE

    #else

        using fd_t = int;
        using conmode = ::termios;
        static constexpr fd_t INVALID_FD = -1;
        static constexpr fd_t STDIN_FD  = STDIN_FILENO;
        static constexpr fd_t STDOUT_FD = STDOUT_FILENO;

    #endif

    struct args
    {
        int    argc;
        char** argv;
        int    iter;

        args(int argc, char** argv)
            : argc{ argc }, argv{ argv }, iter{ 1 }
        { }

        operator bool() const { return iter < argc; }
        auto next()
        {
            if (iter < argc)
            {
                if (*(argv[iter]) == '-' || *(argv[iter]) == '/')
                {
                    return *(argv[iter++] + 1);
                }
                ++iter;
            }
            return '\0';
        }
    };
    struct nothing
    {
        template<class T>
        operator T () { return T{}; }
    };

    inline auto error()
    {
        #if defined(_WIN32)

            return GetLastError();

        #else

            return errno;

        #endif
    }
    template<class ...Args>
    auto fail(Args&&... msg)
    {
        log("  os: ", msg..., " (", error(), ") ");
        return nothing{};
    };
    template<class T>
    bool ok(T error_condition, text msg = {})
    {
        if(
            #if defined(_WIN32)
                error_condition == 0
            #else
                error_condition == (T)-1
            #endif
        )
        {
            fail(msg);
            return faux;
        }
        else return true;
    }
    static void exit(int code)
    {
        #if defined(_WIN32)

            ExitProcess(code);

        #else

            if (is_daemon) ::closelog();
            ::exit(code);

        #endif
    }
    template<class ...Args>
    void exit(int code, Args&&... args)
    {
        log(args...);
        exit(code);
    }
    static auto get_env(text&& var)
    {
        auto val = std::getenv(var.c_str());
        return val ? text{ val }
                   : text{};
    }

    struct legacy
    {
        static constexpr auto clean  = 0;
        static constexpr auto mouse  = 1 << 0;
        static constexpr auto vga16  = 1 << 1;
        static constexpr auto vga256 = 1 << 2;
    };
    static auto local_mode()
    {
        auto conmode = -1;
        #if defined (__linux__)
            
            if (ok(::ioctl(STDOUT_FD, KDGETMODE, &conmode), "KDGETMODE failed"))
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
    static auto legacy_mode()
    {
        auto vga16colors = {
            "ansi",
            "linux",
            "xterm",
            "xterm-color",
            "dvtm",
            "tmux",
            "fbcon",
        };
        iota mode = legacy::clean;
        auto term = os::get_env("TERM");
        if (term.size())
        {
            log("  os: terminal type \"", term, "\"");
            if (term.ends_with("16color") || term.ends_with("16colour"))
            {
                mode |= legacy::vga16;
            }
            else
            {
                for (auto& type : vga16colors)
                {
                    if (term == type)
                    {
                        mode |= legacy::vga16;
                        break;
                    }
                }
            }
            if (os::get_env("TERM_PROGRAM") == "Apple_Terminal")
            {
                log("  os: macOS Apple_Terminal detected");
                if (!(mode & legacy::vga16)) mode |= legacy::vga256;
            }

            if (local_mode()) mode |= legacy::mouse;
            log("  os: color mode: ", mode & legacy::vga16  ? "16-color"
                                    : mode & legacy::vga256 ? "256-color"
                                                            : "true-color");
            log("  os: mouse mode: ", mode & legacy::mouse ? "console" : "VT-style");
        }
        return mode;
    }
    static auto vgafont_update(iota mode)
    {
        #if defined (__linux__)

            if (mode & legacy::mouse)
            {
                auto chars = std::vector<unsigned char>(512 * 32 * 4);
                auto fdata = console_font_op{ .op        = KD_FONT_OP_GET,
                                              .flags     = 0,
                                              .width     = 32,
                                              .height    = 32,
                                              .charcount = 512,
                                              .data      = chars.data() };
                if (!ok(::ioctl(STDOUT_FD, KDFONTOP, &fdata), "KDFONTOP + KD_FONT_OP_GET failed")) return;

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
                if (!ok(::ioctl(STDOUT_FD, KDFONTOP, &fdata), "KDFONTOP + KD_FONT_OP_SET failed")) return;

                auto max_sz = std::numeric_limits<unsigned short>::max();
                auto spairs = std::vector<unipair>(max_sz);
                auto dpairs = std::vector<unipair>(max_sz);
                auto srcmap = unimapdesc{ max_sz, spairs.data() };
                auto dstmap = unimapdesc{ max_sz, dpairs.data() };
                auto dstptr = dstmap.entries;
                auto srcptr = srcmap.entries;
                if (!ok(::ioctl(STDOUT_FD, GIO_UNIMAP, &srcmap), "GIO_UNIMAP failed")) return;
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
                    if (!ok(::ioctl(STDOUT_FD, PIO_UNIMAP, &dstmap), "PIO_UNIMAP failed")) return;
                }
                else log("  os: vgafont_update failed - UNIMAP is full");
            }

        #endif
    }
    static auto vtgafont_revert()
    {

    }
    static auto current_module_file()
    {
        text result;

        #if defined(_WIN32)

            HANDLE h = GetCurrentProcess();
            std::vector<char> buffer(MAX_PATH);

            while (buffer.size() <= 32768)
            {
                DWORD length = GetModuleFileNameEx(h, NULL,
                    buffer.data(), static_cast<DWORD>(buffer.size()));

                if (!length) break;

                if (buffer.size() > length + 1)
                {
                    //result = utf::to_utf(std::wstring(buffer.data(), length));
                    result = text(buffer.data(), length);
                    break;
                }

                buffer.resize(buffer.size() << 1);
            }

        #else

            char* resolved = realpath("/proc/self/exe", NULL);
            if (resolved)
            {
                result = std::string(resolved);
                free(resolved);
            }

        #endif

        return result;
    }
    static auto split_cmdline(view cmdline)
    {
        std::vector<text> args;
        auto mark = '\0';
        text temp;
        temp.reserve(cmdline.length());

        auto push = [&]() {
            args.push_back(temp);
            temp.clear();
        };

        for (auto c : cmdline)
        {
            if (mark)
            {
                if (c != mark)
                {
                    temp.push_back(c);
                }
                else
                {
                    mark = '\0';
                    push();
                }
            }
            else
            {
                if (c == ' ')
                {
                    if (temp.length()) push();
                }
                else
                {
                    if (c == '\'' || c == '"') mark = c;
                    else                       temp.push_back(c);
                }
            }
        }
        if (temp.length()) push();

        return args;
    }
    static auto exec(text binary, text params = {}, int window_state = 0)
    {
        #if defined(_WIN32)

            //auto binary_w = utf::to_utf(binary);
            //auto params_w = utf::to_utf(params);

            SHELLEXECUTEINFO ShExecInfo = {};
            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
            ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
            ShExecInfo.hwnd = NULL;
            ShExecInfo.lpVerb = NULL;
            ShExecInfo.lpFile = binary.c_str();
            ShExecInfo.lpParameters = params.c_str();
            ShExecInfo.lpDirectory = NULL;
            ShExecInfo.nShow = window_state;
            ShExecInfo.hInstApp = NULL;
            ShellExecuteEx(&ShExecInfo);
            return true;

        #else

            auto p_id = ::fork();
            if (p_id == 0) // Child branch
            {
                log("exec: executing '", binary, " ", params, "'");
                char* argv[] = { binary.data(), params.data(), nullptr };

                ::execvp(argv[0], argv);
                os::exit(1, "exec: error ", errno);
            }

            if (p_id > 0) // Parent branch
            {
                int stat;
                ::waitpid(p_id, &stat, 0); // wait for the child to avoid zombies

                if (WIFEXITED(stat) && (WEXITSTATUS(stat) == 0))
                {
                    return true; // child forked and exited successfully
                }
            }

            log("exec: failed to spawn '", binary, " ' with args '", params, "'");
            return faux;

        #endif
    }
    static void start_log(view srv_name)
    {
        #if defined(_WIN32)

            //todo inplement

        #else

            ::openlog(srv_name.data(), LOG_NOWAIT | LOG_PID, LOG_USER);
            is_daemon = true;

        #endif
    }
    static void syslog(text const& data)
    {
        #if defined(_WIN32)

            std::cout << data << std::flush;

        #else

            if (os::is_daemon) ::syslog(LOG_NOTICE, "%s", data.c_str());
            else               std::cout << data << std::flush;

        #endif
    }
    static auto daemonize(view srv_name)
    {
        #if defined(_WIN32)

            return true;

        #else

            auto pid = ::fork();
            if (pid < 0)
            {
                os::exit(1, "daemon: fork error");
            }

            if (pid == 0)
            { // CHILD
                ::setsid(); // Make this process the session leader of a new session.

                pid = ::fork();
                if (pid < 0)
                {
                    os::exit(1, "daemon: fork error");
                }

                if (pid == 0)
                { // GRANDCHILD
                    umask(0);
                    start_log(srv_name);

                    ::close(STDIN_FILENO);  // A daemon cannot use the terminal,
                    ::close(STDOUT_FILENO); // so close standard file descriptors
                    ::close(STDERR_FILENO); // for security reasons.
                    return true;
                }

                os::exit(0); // SUCCESS (This child is reaped below with waitpid()).
            }

            // Reap the child, leaving the grandchild to be inherited by init.
            int stat;
            ::waitpid(pid, &stat, 0);
            if (WIFEXITED(stat) && (WEXITSTATUS(stat) == 0))
            {
                os::exit(0); // Child forked and exited successfully.
            }
            return faux;

        #endif
    }
    static auto host_name()
    {
        text hostname;

        #if defined(_WIN32)

            DWORD dwSize = 0;
            GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, NULL, &dwSize);

            if (dwSize)
            {
                std::vector<char> buffer(dwSize);
                if (GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, buffer.data(), &dwSize))
                {
                    //hostname = utf::to_utf(std::wstring(wc_buffer.data(), dwSize));
                    hostname = text(buffer.data(), dwSize);
                }
            }

        #else

            // APPLE: AI_CANONNAME undeclared
            //std::vector<char> buffer(MAXHOSTNAMELEN);
            //if (0 == gethostname(buffer.data(), buffer.size()))
            //{
            //	struct addrinfo hints, * info;
            //
            //	::memset(&hints, 0, sizeof hints);
            //	hints.ai_family = AF_UNSPEC;
            //	hints.ai_socktype = SOCK_STREAM;
            //	hints.ai_flags = AI_CANONNAME | AI_CANONIDN;
            //
            //	if (0 == getaddrinfo(buffer.data(), "http", &hints, &info))
            //	{
            //		hostname = std::string(info->ai_canonname);
            //		//for (auto p = info; p != NULL; p = p->ai_next)
            //		//{
            //		//	hostname = std::string(p->ai_canonname);
            //		//}
            //		freeaddrinfo(info);
            //	}
            //}

        #endif
        return hostname;
    }
    static auto is_mutex_exists(text&& mutex_name)
    {
        bool result = false;

        #if defined(_WIN32)

            HANDLE mutex = CreateMutex(0, 0, mutex_name.c_str());
            result = GetLastError() == ERROR_ALREADY_EXISTS;
            CloseHandle(mutex);
            return result;

        #else

            //todo linux implementation
            return true;

        #endif
    }
    static auto process_id()
    {
        uint32_t result;

        #if defined(_WIN32)

            result = GetCurrentProcessId();

        #else

            result = getpid();

        #endif
        return result;
    }
    static auto logged_in_users(view domain_delimiter = "\\", view record_delimiter = "\0")
    {
        text active_users_array;

        #if defined(_WIN32)

            PWTS_SESSION_INFO SessionInfo_pointer;
            DWORD count;
            if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &SessionInfo_pointer, &count))
            {
                for (DWORD i = 0; i < count; i++)
                {
                    WTS_SESSION_INFO si = SessionInfo_pointer[i];

                    LPTSTR	buffer_pointer = NULL;
                    DWORD	buffer_length;

                    WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, si.SessionId, WTSUserName, &buffer_pointer, &buffer_length);
                    auto user = text(utf::to_view(buffer_pointer, buffer_length));
                    WTSFreeMemory(buffer_pointer);

                    if (user.length())
                    {
                        WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, si.SessionId, WTSDomainName, &buffer_pointer, &buffer_length);
                        auto domain = text(utf::to_view(buffer_pointer, buffer_length / sizeof(wchar_t)));
                        WTSFreeMemory(buffer_pointer);

                        active_users_array += domain;
                        active_users_array += domain_delimiter;
                        active_users_array += user;
                        active_users_array += domain_delimiter;
                        active_users_array += "local";
                        active_users_array += record_delimiter;
                    }
                }
                WTSFreeMemory(SessionInfo_pointer);
                if (active_users_array.size())
                {
                    active_users_array = utf::remove(active_users_array, record_delimiter);
                }
            }

        #else

            static constexpr auto NAME_WIDTH = 8;

            // APPLE: utmp is deprecated

            // if (FILE* ufp = ::fopen(_PATH_UTMP, "r"))
            // {
            // 	struct utmp usr;
            //
            // 	while (::fread((char*)&usr, sizeof(usr), 1, ufp) == 1)
            // 	{
            // 		if (*usr.ut_user && *usr.ut_host && *usr.ut_line && *usr.ut_line != '~')
            // 		{
            // 			active_users_array += usr.ut_host;
            // 			active_users_array += domain_delimiter;
            // 			active_users_array += usr.ut_user;
            // 			active_users_array += domain_delimiter;
            // 			active_users_array += usr.ut_line;
            // 			active_users_array += record_delimiter;
            // 		}
            // 	}
            // 	::fclose(ufp);
            // }

        #endif
        return active_users_array;
    }
    static auto user()
    {
        #if defined(_WIN32)

            static constexpr auto INFO_BUFFER_SIZE = 32767UL;
            //TCHAR  infoBuf[INFO_BUFFER_SIZE];
            char   infoBuf[INFO_BUFFER_SIZE];
            DWORD  bufCharCount = INFO_BUFFER_SIZE;

            if (!GetUserName(infoBuf, &bufCharCount))
                log("error GetUserName");

            return text(infoBuf, bufCharCount);

        #else

            uid_t id;
            id = ::geteuid();
            return id;

        #endif
    }

    #if defined(_WIN32)

    /* cl.exe issue
    class security_descriptor
    {
        SECURITY_ATTRIBUTES descriptor;

    public:
        text security_string;

        operator SECURITY_ATTRIBUTES* () throw()
        {
            return &descriptor;
        }

        security_descriptor(text SSD)
            : security_string{ SSD }
        {
            ZeroMemory(&descriptor, sizeof(descriptor));
            descriptor.nLength = sizeof(descriptor);

            // four main components of a security descriptor https://docs.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format
            //       "O:" - owner
            //       "G:" - primary group
            //       "D:" - DACL discretionary access control list https://docs.microsoft.com/en-us/windows/desktop/SecGloss/d-gly
            //       "S:" - SACL system access control list https://docs.microsoft.com/en-us/windows/desktop/SecGloss/s-gly
            //
            // the object's owner:
            //   O:owner_sid
            //
            // the object's primary group:
            //   G:group_sid
            //
            // Security descriptor control flags that apply to the DACL:
            //   D:dacl_flags(string_ace1)(string_ace2)... (string_acen)
            //
            // Security descriptor control flags that apply to the SACL
            //   S:sacl_flags(string_ace1)(string_ace2)... (string_acen)
            //
            //   dacl_flags/sacl_flags:
            //   "P"                 SDDL_PROTECTED        The SE_DACL_PROTECTED flag is set.
            //   "AR"                SDDL_AUTO_INHERIT_REQ The SE_DACL_AUTO_INHERIT_REQ flag is set.
            //   "AI"                SDDL_AUTO_INHERITED   The SE_DACL_AUTO_INHERITED flag is set.
            //   "NO_ACCESS_CONTROL" SSDL_NULL_ACL         The ACL is null.
            //
            //   string_ace - The fields of the ACE are in the following
            //                order and are separated by semicolons (;)
            //   for syntax see https://docs.microsoft.com/en-us/windows/win32/secauthz/ace-strings
            //   ace_type;ace_flags;rights;object_guid;inherit_object_guid;account_sid;(resource_attribute)
            // ace_type:
            //  "A" SDDL_ACCESS_ALLOWED
            // ace_flags:
            //  "OI"	SDDL_OBJECT_INHERIT
            //  "CI"	SDDL_CONTAINER_INHERIT
            //  "NP"	SDDL_NO_PROPAGATE
            // rights:
            //  "GA"	SDDL_GENERIC_ALL
            // account_sid: see https://docs.microsoft.com/en-us/windows/win32/secauthz/sid-strings
            //  "SY"	SDDL_LOCAL_SYSTEM
            //  "BA"	SDDL_BUILTIN_ADMINISTRATORS
            //  "CO"	SDDL_CREATOR_OWNER
            if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
                //"D:P(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;CO)",
                SSD.c_str(),
                SDDL_REVISION_1,
                &descriptor.lpSecurityDescriptor,
                NULL))
            {
                log("ConvertStringSecurityDescriptorToSecurityDescriptor error:",
                    GetLastError());
            }
        }

        ~security_descriptor()
        {
            LocalFree(descriptor.lpSecurityDescriptor);
        }
    };

    static security_descriptor global_access{ "D:P(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;CO)" };
    */

    static auto take_partition(text&& file)
    {
        text result;
        std::vector<char> volume(std::max<size_t>(MAX_PATH, file.size() + 1));
        if (GetVolumePathName(file.c_str(), volume.data(), (DWORD)volume.size()) != 0)
        {
            std::vector<char> partition(50 + 1);
            if (GetVolumeNameForVolumeMountPoint( volume.data(),
                                                  partition.data(),
                                                  (DWORD)partition.size()) != 0)
                result = text(partition.data());
            else
            {
                //error_handler();
            }
        }
        else
        {
            //error_handler();
        }
        return result;
    }
    static auto take_temp(text&& file)
    {
        text tmp_dir;

        auto file_guid = take_partition(std::move(file));
        uint8_t i = 0;

        while (i < 100 && netxs::os::test_path(tmp_dir = file_guid + "\\$temp_" + utf::adjust(std::to_string(i++), 3, "0", true)))
        {
        }

        if (i == 100) tmp_dir.clear();

        return tmp_dir;
    }
    static auto trusted_domain_name()
    {
        PDS_DOMAIN_TRUSTS info;
        text  domain_name;
        ULONG DomainCount;

        bool result = DsEnumerateDomainTrusts(nullptr, DS_DOMAIN_PRIMARY, &info, &DomainCount);
        if (result == ERROR_SUCCESS)
        {
            domain_name = text(info->DnsDomainName);
            NetApiBufferFree(info);
        }
        return domain_name;
    }
    static auto trusted_domain_guid()
    {
        PDS_DOMAIN_TRUSTS info;
        text  domain_guid;
        ULONG domain_count;

        bool result = DsEnumerateDomainTrusts(nullptr, DS_DOMAIN_PRIMARY, &info, &domain_count);
        if (result == ERROR_SUCCESS && domain_count > 0)
        {
            auto& guid = info->DomainGuid;
            domain_guid = utf::to_hex(guid.Data1)
                + '-' + utf::to_hex(guid.Data2)
                + '-' + utf::to_hex(guid.Data3)
                + '-' + utf::to_hex(std::string(guid.Data4, guid.Data4 + 2))
                + '-' + utf::to_hex(std::string(guid.Data4 + 2, guid.Data4 + sizeof(guid.Data4)));

            NetApiBufferFree(info);
        }
        return domain_guid;
    }
    static auto create_shortcut(text&& path_to_object, text&& path_to_link)
    {
        HRESULT result;
        IShellLink* psl;
        CoInitialize(0);
        result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);

        if (result == S_OK)
        {
            IPersistFile* ppf;
            //auto path_to_object_w = utf::to_utf(path_to_object);
            auto path_to_link_w = utf::to_utf(path_to_link);

            psl->SetPath(path_to_object.c_str());
            result = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
            if (SUCCEEDED(result))
            {
                result = ppf->Save(path_to_link_w.c_str(), TRUE);
                ppf->Release();
                CoUninitialize();
                return true;
            }
            else
            {
                //todo
                //shell_error_handler(result);
            }
            psl->Release();
        }
        else
        {
            //todo
            //shell_error_handler(result);
        }
        CoUninitialize();

        return false;
    }
    static auto expand(text&& directory)
    {
        text result;
        //auto directory_w = utf::to_utf(directory);
        if (auto len = ExpandEnvironmentStrings(directory.c_str(), NULL, 0))
        {
            std::vector<char> buffer(len);
            if (ExpandEnvironmentStrings(directory.c_str(), buffer.data(), (DWORD)buffer.size()))
            {
                result = text(buffer.data());
            }
        }
        return result;
    }
    static auto set_registry_key(text&& key_path, text&& parameter_name, text&& value)
    {
        LONG status;
        HKEY hKey;
        //status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_path, 0, KEY_ALL_ACCESS, &hKey);
        status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            key_path.c_str(),
            0,
            0,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            0);

        if (status == ERROR_SUCCESS && hKey != NULL)
        {
            //auto value_w = utf::to_utf(value);
            status = RegSetValueEx(hKey,
                parameter_name.empty() ? nullptr : parameter_name.c_str(),
                0,
                REG_SZ,
                (BYTE*)value.c_str(),
                ((DWORD)value.size() + 1) * sizeof(wchar_t)
            );

            RegCloseKey(hKey);
        }
        else
        {
            //todo
            //error_handler();
        }

        return (status == ERROR_SUCCESS);
    }
    static auto get_registry_string_value(text&& key_path, text&& parameter_name)
    {
        text result;

        HKEY hKey;
        DWORD value_type;
        DWORD data_length = 0;

        LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            key_path.c_str(),
            0,
            KEY_ALL_ACCESS,
            &hKey);

        if (status == ERROR_SUCCESS && hKey != NULL)
        {
            //auto parameter_name_w = utf::to_utf(parameter_name);
            auto param = parameter_name.empty() ? nullptr : parameter_name.c_str();

            status = RegQueryValueEx(hKey,
                param,
                NULL,
                &value_type,
                NULL,
                &data_length);

            if (status == ERROR_SUCCESS && value_type == REG_SZ)
            {
                std::vector<BYTE> data(data_length);
                status = RegQueryValueEx(hKey,
                    param,
                    NULL,
                    &value_type,
                    data.data(),
                    &data_length);

                if (status == ERROR_SUCCESS)
                {
                    //result = utf::to_utf(std::wstring(reinterpret_cast<wchar_t*>(data.data()), data.size()));
                    result = text(utf::to_view(reinterpret_cast<char*>(data.data()), data.size()));
                }
            }
        }
        if (status != ERROR_SUCCESS)
        {
            //todo
            //error_handler();
        }

        return result;
    }
    static auto get_registry_subkeys(text&& key_path)
    {
        list result;

        HKEY hKey;
        LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            key_path.c_str(),
            0,
            KEY_ALL_ACCESS,
            &hKey);

        if (status == ERROR_SUCCESS && hKey != NULL)
        {
            DWORD lpcbMaxSubKeyLen;
            if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, &lpcbMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL))
            {
                auto size = lpcbMaxSubKeyLen;
                DWORD index = 0;
                std::vector<char> szName(size);

                while (ERROR_SUCCESS == RegEnumKeyEx(hKey,
                    index++,
                    szName.data(),
                    &lpcbMaxSubKeyLen,
                    NULL,
                    NULL,
                    NULL,
                    NULL))
                {
                    //result.push_back(utf::to_utf(std::wstring(szName.data(), std::min(lpcbMaxSubKeyLen, size))));
                    result.push_back(text(
                        utf::to_view(szName.data(), std::min<size_t>(lpcbMaxSubKeyLen, size))));
                    lpcbMaxSubKeyLen = size;
                }
            }

            RegCloseKey(hKey);
        }

        return result;
    }
    static auto cmdline()
    {
        list result;
        int  argc = 0;
        //auto params = std::shared_ptr<void>(CommandLineToArgvW(GetCommandLine(), &argc), LocalFree);
        auto params = std::shared_ptr<void>(
            CommandLineToArgvW(GetCommandLineW(), &argc), LocalFree);

        auto argv = (LPWSTR*)params.get();
        for (int i = 0; i < argc; i++)
        {
            //result.push_back(utf::to_low(text(argv[i])));
            result.push_back(utf::to_utf(argv[i]));
        }

        return result;
    }
    static auto delete_registry_tree(text&& path)
    {
        bool result;
        HKEY hKey;
        //auto path_w = utf::to_utf(path);

        LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            path.c_str(),
            0,
            KEY_ALL_ACCESS,
            &hKey);

        if (status == ERROR_SUCCESS && hKey != NULL)
        {
            status = SHDeleteKey(hKey, path.c_str());
            RegCloseKey(hKey);
        }

        result = status == ERROR_SUCCESS;

        if (!result)
        {
            //todo
            //error_handler();
        }

        return result;
    }
    static void update_process_privileges(void)
    {
        HANDLE hToken;
        TOKEN_PRIVILEGES tp;
        LUID luid;

        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
        }
    }
    static auto kill_process(unsigned long proc_id)
    {
        bool result = false;

        update_process_privileges();
        HANDLE process_handle = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, proc_id);
        if (process_handle && TerminateProcess(process_handle, 0))
        {
            result = WaitForSingleObject(process_handle, 10000) == WAIT_OBJECT_0;
        }
        else
        {
            //todo
            //error_handler();
        }

        if (process_handle) CloseHandle(process_handle);

        return result;
    }
    static auto global_startup_dir()
    {
        text result;

        //todo vista & later
        // use SHGetFolderPath
        //PWSTR path;
        //if (S_OK != SHGetKnownFolderPath(FOLDERID_CommonStartup, 0, NULL, &path))
        //{
        //	//todo
        //	//error_handler();
        //}
        //else
        //{
        //	result = utils::to_utf(std::wstring(path)) + '\\';
        //	CoTaskMemFree(path);
        //}

        PIDLIST_ABSOLUTE pidl;
        if (S_OK == SHGetFolderLocation(NULL, CSIDL_COMMON_STARTUP, NULL, 0, &pidl))
        {
            char path[MAX_PATH];
            if (TRUE == SHGetPathFromIDList(pidl, path))
            {
                result = text(path) + '\\';
            }
            ILFree(pidl);
        }
        else
        {
            //todo
            //error_handler();
        }

        return result;
    }

    #endif  // Windows specific

    template<class SIZE_T>
    auto recv(fd_t fd, char* buff, SIZE_T size)
    {
        #if defined(_WIN32)

            DWORD count;
            auto fSuccess = ReadFile(fd,          // pipe handle
                                     buff,        // buffer to receive reply
                                     (DWORD)size, // size of buffer
                                     &count,      // number of bytes read
                                     nullptr);    // not overlapped
            if (!fSuccess) count = 0;

        #else

            auto count = ::read(fd, buff, size);

        #endif

        return count > 0 ? qiew{ buff, count }
                         : qiew{};
    }
    template<bool IS_TTY = true, class SIZE_T>
    auto send(fd_t fd, char const* buff, SIZE_T size)
    {
        while (size)
        {
            #if defined(_WIN32)

                DWORD count;
                auto fSuccess = WriteFile(fd,          // pipe handle
                                          buff,        // message
                                          (DWORD)size, // message length
                                          &count,      // bytes written
                                          nullptr);    // not overlapped

            #else

                auto count = IS_TTY ? ::write(fd, buff, size)
                                    : ::send (fd, buff, size, MSG_NOSIGNAL); // not work with open_pty
                                                                             // recursive connection causes sigpipe on destroy when using write(2) despite using ::signal(SIGPIPE, SIG_IGN)
                //  send(2) does not work with file descriptors, only sockets.
                // write(2) works with fds as well as sockets.

            #endif

            if (count != size)
            {
                if (count > 0)
                {
                    log("send: partial writing: socket=", fd,
                        " total=", size, ", written=", count);
                    buff += count;
                    size -= count;
                }
                else
                {
                    fail("send: error write to socket=", fd, " count=", count, " size=", size, " IS_TTY=", IS_TTY ?"true":"faux");
                    return faux;
                }
            }
            else return true;
        }
        return faux;
    }
    static auto set_palette(iota legacy)
    {
        ansi::esc yield;
        bool legacy_mouse = legacy & os::legacy::mouse;
        bool legacy_color = legacy & os::legacy::vga16;
        if (legacy_color)
        {
            auto set_pal = [&](auto p)
            {
                (yield.*p)(0,  rgba::color16[tint16::blackdk  ]);
                (yield.*p)(1,  rgba::color16[tint16::blacklt  ]);
                (yield.*p)(2,  rgba::color16[tint16::graydk   ]);
                (yield.*p)(3,  rgba::color16[tint16::graylt   ]);
                (yield.*p)(4,  rgba::color16[tint16::whitedk  ]);
                (yield.*p)(5,  rgba::color16[tint16::whitelt  ]);
                (yield.*p)(6,  rgba::color16[tint16::redlt    ]);
                (yield.*p)(7,  rgba::color16[tint16::bluelt   ]);
                (yield.*p)(8,  rgba::color16[tint16::greenlt  ]);
                (yield.*p)(9,  rgba::color16[tint16::yellowlt ]);
                (yield.*p)(10, rgba::color16[tint16::magentalt]);
                (yield.*p)(11, rgba::color16[tint16::reddk    ]);
                (yield.*p)(12, rgba::color16[tint16::bluedk   ]);
                (yield.*p)(13, rgba::color16[tint16::greendk  ]);
                (yield.*p)(14, rgba::color16[tint16::yellowdk ]);
                (yield.*p)(15, rgba::color16[tint16::cyanlt   ]);
            };
            yield.save_palette();
            if (legacy_mouse) set_pal(&ansi::esc::old_palette);
            else              set_pal(&ansi::esc::osc_palette);
            os::send(STDOUT_FD, yield.data(), yield.size());
            yield.clear();
        }
    }
    static auto rst_palette(iota legacy)
    {
        ansi::esc yield;
        bool legacy_mouse = legacy & os::legacy::mouse;
        bool legacy_color = legacy & os::legacy::vga16;
        if (legacy_color)
        {
            if (legacy_mouse) yield.old_palette_reset();
            else              yield.osc_palette_reset();
            yield.load_palette();
            os::send(STDOUT_FD, yield.data(), yield.size());
            yield.clear();
            log(" tty: palette restored");
        }
    }

    #if defined(_WIN32)

        struct file
        {
            fd_t r; // file: Read descriptor.
            fd_t w; // file: Write descriptor.
            fd_t& get_r() { return r; };
            fd_t& get_w() { return w; };
            operator bool () { return r != INVALID_FD && w != INVALID_FD; }
            void close()
            {
                if (r != INVALID_FD) CloseHandle(r);
                if (w != INVALID_FD) CloseHandle(w);
            }
            friend auto& operator<< (std::ostream& s, file const& handle)
            {
                return s << handle.r << "," << handle.w;
            }
            file(fd_t fd_r = INVALID_FD, fd_t fd_w = INVALID_FD)
                : r{ fd_r }, w{ fd_w }
            { }
        };
        
        class flash_t
        {
            fd_t h; // flash_t: Descriptor for IO interrupt.

        public:
            operator fd_t () { return h; }
            flash_t()        { ok(h = CreateEvent(NULL, TRUE, FALSE, NULL), "CreateEvent error"); }
           ~flash_t()        { if (h != INVALID_FD) CloseHandle(h); }
            void reset()     { ok(SetEvent(h), "SetEvent error"); }
        };

    #else

        struct file
        {
            fd_t h; // file: RW descriptor.
            fd_t& get_r() { return h; };
            fd_t& get_w() { return h; };
            operator fd_t () { return h; }
            operator bool () { return h != INVALID_FD; }
            void close()
            {
                if (h != INVALID_FD) ::close(h);
            }
            friend auto& operator<< (std::ostream& s, file const& handle)
            {
                return s << handle.h;
            }
            file(fd_t fd = INVALID_FD) : h{ fd } { }
        };

        class flash_t
        {
            fd_t h[2] = { INVALID_FD, INVALID_FD }; // flash_t: Descriptors for IO interrupt.
            char x = 1;

        public:
            operator fd_t () { return h[0]; }
            flash_t()        { ok(::pipe(h), "pipe(2) error"); }
           ~flash_t()        { for(auto f : h) if (f != INVALID_FD) ::close(f); }
            void reset()     { send(h[1], &x, sizeof(x)); }
            void flush()     { recv(h[0], &x, sizeof(x)); }
        };

    #endif

    class ipc
    {
        using vect = std::vector<char>;
        bool active; // ipc: Used by the os::tty.
        file handle; // ipc: Socket file descriptor.
        vect buffer; // ipc: Receive buffer.
        bool sealed; // ipc: Provide autoclosing.
        text scpath; // ipc: Socket path (in order to unlink).

        void init(iota buff_size = PIPE_BUF) { buffer.resize(buff_size); }

    public:
        ipc(file const& descriptor = {}, bool sealed = faux)
            : handle{ descriptor },
              sealed{ sealed },
              active{ true }
        {
            if (handle) init();
        }
        ~ipc()
        {
            #if defined(__BSD__)

                if (scpath.length())
                {
                    ::unlink(scpath.c_str()); // Cleanup file system unix domain socket.
                }

            #endif

            if (sealed) handle.close();
        }

        operator bool () { return active; }

        void set(file const& h, bool s)
        {
            handle = h;
            sealed = s;
            if (handle) init();
        }
        auto get()
        {
            return handle;
        }
        void reset()
        {
            active = faux;
        }
        template<class T>
        auto cred(T id) const // Check peer cred.
        {
            #if defined(_WIN32)

                //todo implement for win32

            #elif defined(__linux__)

                ucred cred = {};
                unsigned size = sizeof(cred);

                if (!ok(::getsockopt(handle.h, SOL_SOCKET, SO_PEERCRED, &cred, &size), "getsockopt error"))
                    return faux;

                if (cred.uid && id != cred.uid)
                {
                    log("sock: other users are not allowed to the session, abort");
                    return faux;
                }

                log("sock: creds from SO_PEERCRED",
                    ":  pid=", cred.pid,
                    ", euid=", cred.uid,
                    ", egid=", cred.gid);

            #elif defined(__BSD__)

                uid_t euid;
                gid_t egid;

                if (!ok(::getpeereid(handle.h, &euid, &egid), "getpeereid error"))
                    return faux;

                if (euid && id != euid)
                {
                    log("sock: other users are not allowed to the session, abort");
                    return faux;
                }

                log("sock: creds from getpeereid",
                    ":  pid=", id,
                    ", euid=", euid,
                    ", egid=", egid);

            #endif

            return true;
        }
        auto meet() -> std::shared_ptr<ipc>
        {
            #if defined(_WIN32)

                //security_descriptor pipe_acl(security_descriptor_string);

                auto sock_ptr = std::make_shared<ipc>(handle, true);

                auto to_server = "\\\\.\\pipe\\r_" + scpath;
                auto to_client = "\\\\.\\pipe\\w_" + scpath;

                auto r_fConnected = ConnectNamedPipe(handle.r, NULL)
                    ? true
                    : (GetLastError() == ERROR_PIPE_CONNECTED);

                // recreate the waiting point for the next client
                handle.r = CreateNamedPipe(
                    to_server.c_str(),        // pipe name
                    PIPE_ACCESS_INBOUND,      // read/write access
                    PIPE_TYPE_BYTE |          // message type pipe
                    PIPE_READMODE_BYTE |      // message-read mode
                    PIPE_WAIT,                // blocking mode
                    PIPE_UNLIMITED_INSTANCES, // max. instances
                    PIPE_BUF,                 // output buffer size
                    PIPE_BUF,                 // input buffer size
                    0,                        // client time-out
                    NULL);  // DACL
                    // DACL: The ACLs in the default security descriptor
                    //       for a named pipe grant full control to the
                    //       LocalSystem account, administrators, and the
                    //       creator owner.They also grant read access to
                    //       members of the Everyone groupand the anonymous
                    //       account.
                    //       Without write access, the desktop will be inaccessible to non-owners.
                    //pipe_acl);                // DACL

                if (handle.r == INVALID_FD)
                {
                    handle.r = sock_ptr->handle.r;
                    return fail("CreateNamedPipe error (read)");
                }

                auto w_fConnected = ConnectNamedPipe(handle.w, NULL)
                    ? true
                    : (GetLastError() == ERROR_PIPE_CONNECTED);

                handle.w = CreateNamedPipe(
                    to_client.c_str(),        // pipe name
                    PIPE_ACCESS_OUTBOUND,     // read/write access
                    PIPE_TYPE_BYTE |          // message type pipe
                    PIPE_READMODE_BYTE |      // message-read mode
                    PIPE_WAIT,                // blocking mode
                    PIPE_UNLIMITED_INSTANCES, // max. instances
                    PIPE_BUF,                 // output buffer size
                    PIPE_BUF,                 // input buffer size
                    0,                        // client time-out
                    NULL);

                if (handle.w == INVALID_FD)
                {
                    CloseHandle(handle.r);
                    handle.r = sock_ptr->handle.r;
                    handle.w = sock_ptr->handle.w;
                    return fail("CreateNamedPipe error (write)");
                }

                return sock_ptr;

            #else

                auto s = file{ ::accept(handle.h, 0, 0) };
                return s ? std::make_shared<ipc>(s, true)
                         : nullptr;
            #endif
        }
        template<class SIZE_T>
        auto recv(char* buff, SIZE_T size)
        {
            return os::recv(handle.get_r(), buff, size);
        }
        auto recv() // It's not thread safe!
        {
            return recv(buffer.data(), buffer.size());
        }
        template<bool IS_TTY = faux>
        auto send(char const* buff, size_t size)
        {
            return os::send<IS_TTY>(handle.get_w(), buff, size);
        }
        template<bool IS_TTY = faux, class T>
        auto send(T const& buff)
        {
            auto data = buff.data();
            auto size = buff.size();
            return send<IS_TTY>(data, size);
        }
        auto send(char c)
        {
            return send(&c, 1);
        }
        // ipc: Read until the delimeter appears.
        auto line(char delim)
        {
            char c;
            text crop;
            while (recv(&c, sizeof(c)) && c != delim)
            {
                crop.push_back(c);
            }
            return crop;
        }
        auto shut() -> bool
        {
            active = faux;
            #if defined(_WIN32)

                if (sealed)
                { // Disconnection order does matter.
                    if (handle.get_w() != INVALID_FD) ok(DisconnectNamedPipe(handle.get_w()));
                    if (handle.get_r() != INVALID_FD) ok(DisconnectNamedPipe(handle.get_r()));
                }
                return true;

            #else

                //an important conceptual reason to want
                //to use shutdown:
                //             to signal EOF to the peer
                //             and still be able to receive
                //             pending data the peer sent.
                //"shutdown() doesn't actually close the file descriptor
                //            — it just changes its usability.
                //To free a socket descriptor, you need to use close()."

                log(" ipc: shutdown descriptor ", handle.h);
                if (!ok(::shutdown(handle.h, SHUT_RDWR), "descriptor shutdown error"))  // Further sends and receives are disallowed.
                {
                    switch (errno)
                    {
                    case EBADF:
                        return fail("EBADF: The socket argument is not a valid file descriptor.");
                    case EINVAL:
                        return fail("EINVAL: The how argument is invalid.");
                    case ENOTCONN:
                        return fail("ENOTCONN: The socket is not connected.");
                    case ENOTSOCK:
                        return fail("ENOTSOCK: The socket argument does not refer to a socket.");
                    case ENOBUFS:
                        return fail("ENOBUFS: Insufficient resources were available in the system to perform the operation.");
                    default:
                        return fail("unknown reason");
                    }
                }
                else return true;
                return true;

            #endif
        }
        template<role ROLE, class P = noop>
        static auto open(text path, datetime::period retry_timeout = {}, P retry_proc = P())
            -> std::shared_ptr<ipc>
        {
            auto sock_ptr = std::make_shared<ipc>(file{}, true);
            auto try_start = [&](auto play) -> bool
            {
                auto done = play();
                if (!done)
                {
                    if constexpr (ROLE == role::client)
                    if (!retry_proc())
                        return fail("failed to start server");

                    auto stop = datetime::tempus::now() + retry_timeout;
                    do
                    {
                        std::this_thread::sleep_for(100ms);
                        done = play();
                    }
                    while (!done && stop > datetime::tempus::now());
                }
                return done;
            };

            #if defined(_WIN32)

                //security_descriptor pipe_acl(security_descriptor_string);
                //log("pipe: DACL=", pipe_acl.security_string);

                auto& r_sock = sock_ptr->handle.get_r();
                auto& w_sock = sock_ptr->handle.get_w();

                sock_ptr->scpath = path;
                auto to_server = "\\\\.\\pipe\\r_" + path;
                auto to_client = "\\\\.\\pipe\\w_" + path;

                if constexpr (ROLE == role::server)
                {
                    r_sock = CreateNamedPipe(
                        to_server.c_str(),        // pipe path
                        PIPE_ACCESS_INBOUND,      // read/write access
                        PIPE_TYPE_BYTE |          // message type pipe
                        PIPE_READMODE_BYTE |      // message-read mode
                        PIPE_WAIT,                // blocking mode
                        PIPE_UNLIMITED_INSTANCES, // max instances
                        PIPE_BUF,                 // output buffer size
                        PIPE_BUF,                 // input buffer size
                        0,                        // client time-out
                        NULL);                    // DACL
                        //pipe_acl);                // DACL
                    if (r_sock == INVALID_FD)
                        return fail("CreateNamedPipe error (read)");

                    w_sock = CreateNamedPipe(
                        to_client.c_str(),        // pipe path
                        PIPE_ACCESS_OUTBOUND,     // read/write access
                        PIPE_TYPE_BYTE |          // message type pipe
                        PIPE_READMODE_BYTE |      // message-read mode
                        PIPE_WAIT,                // blocking mode
                        PIPE_UNLIMITED_INSTANCES, // max instances
                        PIPE_BUF,                 // output buffer size
                        PIPE_BUF,                 // input buffer size
                        0,                        // client time-out
                        NULL);                    // DACL
                        //pipe_acl);                // DACL
                    if (w_sock == INVALID_FD)
                    {
                        CloseHandle(r_sock);
                        return fail("CreateNamedPipe error (write)");
                    }
                }
                else if constexpr (ROLE == role::client)
                {
                    auto play = [&]() -> bool
                    {
                        w_sock = CreateFile(
                            to_server.c_str(), // pipe path
                            GENERIC_WRITE,
                            0,                 // no sharing
                            NULL,              // default security attributes
                            OPEN_EXISTING,     // opens existing pipe
                            0,                 // default attributes
                            NULL);             // no template file

                        if (w_sock == INVALID_FD)
                            return faux;

                        r_sock = CreateFile(
                            to_client.c_str(), // pipe path
                            GENERIC_READ,
                            0,                 // no sharing
                            NULL,              // default security attributes
                            OPEN_EXISTING,     // opens existing pipe
                            0,                 // default attributes
                            NULL);             // no template file

                        if (r_sock == INVALID_FD)
                        {
                            CloseHandle(w_sock);
                            return faux;
                        }
                        else return true;
                    };
                    if (!try_start(play))
                        return fail("connection error");
                }

            #else

                ok(::signal(SIGPIPE, SIG_IGN), "failed to set SIG_IGN");

                auto& sock = sock_ptr->handle.get_w();
                sockaddr_un addr = {};
                auto sun_path = addr.sun_path + 1; // Abstract namespace socket (begins with zero). The abstract socket namespace is a nonportable Linux extension.

                #if defined(__BSD__)
                    //todo unify see vtmd.cpp:1564, file system socket
                    path = "/tmp/" + path + ".sock";
                    sun_path--; // File system unix domain socket.
                    log("open: file system socket ", path);
                #endif

                if (path.size() > sizeof(sockaddr_un::sun_path) - 2)
                    return fail("socket path too long");

                if ((sock = ::socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_FD)
                    return fail("open unix domain socket error");

                addr.sun_family = AF_UNIX;
                auto sock_addr_len = (socklen_t)(sizeof(addr) - (sizeof(sockaddr_un::sun_path) - path.size() - 1));
                std::copy(path.begin(), path.end(), sun_path);

                if constexpr (ROLE == role::server)
                {
                    #if defined(__BSD__)
                        log("role server: cleanup file system socket ", path);
                        ::unlink(path.c_str()); // Cleanup file system socket.
                    #endif

                    sock_ptr->scpath = path; // For unlink on exit (file system socket).

                    if (::bind(sock, (struct sockaddr*)&addr, sock_addr_len) == -1)
                        return fail("error unix socket bind for ", path);

                    if (::listen(sock, 5) == -1)
                        return fail("error listen socket for ", path);
                }
                else if constexpr (ROLE == role::client)
                {
                    auto play = [&]()
                    {
                        return -1 != ::connect(sock, (struct sockaddr*)&addr, sock_addr_len);
                    };
                    if (!try_start(play))
                        return fail("connection error");
                }

            #endif

            sock_ptr->init();
            return sock_ptr;
        }

        friend auto& operator<< (std::ostream& s, netxs::os::ipc const& sock)
        {
            return s << "{ xipc: " << sock.handle << " }";
        }
        friend auto& operator<< (std::ostream& s, netxs::os::xipc const& sock)
        {
            return s << *sock;
        }
    };

    class tty
    {
        flash_t flash;

        template<class V>
        struct _globals
        {
            static xipc ipcio;
            static conmode state;
            static testy<twod> winsz;
            static void resize_handler()
            {
                #if defined(_WIN32)

                    CONSOLE_SCREEN_BUFFER_INFO cinfo;
                    if(ok(GetConsoleScreenBufferInfo(STDOUT_FD, &cinfo)))
                    {
                        winsz({ cinfo.srWindow.Right - cinfo.srWindow.Left + 1,
                                cinfo.srWindow.Bottom - cinfo.srWindow.Top + 1 });
                    }

                #else

                    winsize size;
                    if(ok(::ioctl(STDOUT_FD, TIOCGWINSZ, &size)))
                    {
                        winsz({ size.ws_col, size.ws_row });
                    }

                #endif

                if (winsz.test)
                {
                    ipcio->send<faux>(ansi::win(winsz.last));
                }
            }

            #if defined(_WIN32)

                static void default_mode()
                {
                    ok(SetConsoleMode(STDOUT_FD, state[0]), "SetConsoleMode error (revert_o)");
                    ok(SetConsoleMode(STDIN_FD , state[1]), "SetConsoleMode error (revert_i)");
                }
                static BOOL signal_handler(DWORD signal)
                {
                    switch (signal)
                    {
                        case CTRL_C_EVENT:
                            ipcio->send(ansi::C0_ETX);
                            break;
                        case CTRL_BREAK_EVENT:
                            ipcio->send(ansi::C0_ETX);
                            break;
                        case CTRL_CLOSE_EVENT:
                            /**/
                            break;
                        case CTRL_LOGOFF_EVENT:
                            /**/
                            break;
                        case CTRL_SHUTDOWN_EVENT:
                            /**/
                            break;
                        default:
                            break;
                    }
                    return TRUE;
                }

            #else

                static void default_mode()
                {
                    ::tcsetattr(STDIN_FD, TCSANOW, &state);
                }
                static void shutdown_handler(int signal)
                {
                    ipcio->shut();
                    log(" tty: sock->xipc::shut called");
                    ::signal(signal, SIG_DFL);
                    ::raise(signal);
                }
                static void signal_handler(int signal)
                {
                    switch (signal)
                    {
                        case SIGWINCH:
                            resize_handler();
                            return;
                        case SIGHUP:
                            log(" tty: SIGHUP");
                            shutdown_handler(signal);
                            break;
                        case SIGTERM:
                            log(" tty: SIGTERM");
                            shutdown_handler(signal);
                            break;
                        default:
                            break;
                    }
                    log(" tty: signal_handler, signal=", signal);
                }

            #endif
        };

        void reader(iota mode)
        {
            log(" tty: reader thread started");
            auto& ipcio =*_globals<void>::ipcio;

            #if defined(_WIN32)

            // The input codepage to UTF-8 is severely broken in all Windows versions.
            // ReadFile and ReadConsoleA either replace non-ASCII characters with NUL
            // or return 0 bytes read.
            std::vector<INPUT_RECORD> reply(1);
            HANDLE                    waits[2] = { STDIN_FD, flash };
            DWORD                     count;
            ansi::esc                 yield;

            #ifdef VTM_USE_CLASSICAL_WIN32_INPUT

            while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, waits, FALSE, INFINITE))
            {
                if (!GetNumberOfConsoleInputEvents(STDIN_FD, &count))
                {
                    // ERROR_PIPE_NOT_CONNECTED
                    // 233 (0xE9)
                    // No process is on the other end of the pipe.
                    //defeat("GetNumberOfConsoleInputEvents error");
                    os::exit(-1, " tty: GetNumberOfConsoleInputEvents error ", GetLastError());
                    break;
                }
                else if (count)
                {
                    if (count > reply.size()) reply.resize(count);

                    if (!ReadConsoleInputW(STDIN_FD, reply.data(), (DWORD)reply.size(), &count))
                    {
                        //ERROR_PIPE_NOT_CONNECTED = 0xE9 - it's means that the console is gone/crashed
                        //defeat("ReadConsoleInput error");
                        os::exit(-1, " tty: ReadConsoleInput error ", GetLastError());
                        break;
                    }
                    else
                    {
                        auto entry = reply.begin();
                        auto limit = entry + count;
                        yield.w32begin();
                        while (entry != limit)
                        {
                            auto& reply = *entry++;
                            switch (reply.EventType)
                            {
                                case KEY_EVENT:
                                    yield.w32keybd(0,
                                        reply.Event.KeyEvent.wVirtualKeyCode,
                                        reply.Event.KeyEvent.wVirtualScanCode,
                                        reply.Event.KeyEvent.bKeyDown,
                                        reply.Event.KeyEvent.dwControlKeyState,
                                        reply.Event.KeyEvent.wRepeatCount,
                                        utf::tocode(reply.Event.KeyEvent.uChar.UnicodeChar));
                                    break;
                                case MOUSE_EVENT:
                                    yield.w32mouse(0,
                                        reply.Event.MouseEvent.dwButtonState & 0xFFFF,
                                        reply.Event.MouseEvent.dwControlKeyState,
                                        reply.Event.MouseEvent.dwEventFlags,
                                        static_cast<int16_t>((0xFFFF0000 & reply.Event.MouseEvent.dwButtonState) >> 16), // dwButtonState too large when mouse scrolls
                                        reply.Event.MouseEvent.dwMousePosition.X,
                                        reply.Event.MouseEvent.dwMousePosition.Y);
                                    break;
                                case WINDOW_BUFFER_SIZE_EVENT:
                                    _globals<void>::resize_handler();
                                    break;
                                case FOCUS_EVENT:
                                    yield.w32focus(0,
                                        reply.Event.FocusEvent.bSetFocus);
                                    break;
                                default:
                                    break;
                            }
                        }
                        yield.w32close();
                        ipcio.send(yield);
                        yield.clear();
                    }
                }
            }

            #else

            std::vector<wchar_t> buff(STDIN_BUF);
            while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, waits, FALSE, INFINITE))
            {
                if (!GetNumberOfConsoleInputEvents(STDIN_FD, &count))
                {
                    defeat("GetNumberOfConsoleInputEvents error");
                }
                else if (count)
                {
                    if (count > reply.size()) reply.resize(count);

                    if (!PeekConsoleInput(STDIN_FD, reply.data(), (DWORD)reply.size(), &count))
                    {
                        //ERROR_PIPE_NOT_CONNECTED = 0xE9 - it's means that the console is gone/crashed
                        defeat("PeekConsoleInput error");
                    }
                    else
                    {
                        auto entry = reply.begin();
                        auto limit = entry + count;
                        auto vtcon = faux;
                        yield.clear();
                        while (entry != limit)
                        {
                            auto& reply = *entry++;
                            switch (reply.EventType)
                            {
                                case KEY_EVENT:
                                    // ReadConsole ignores key up event
                                    vtcon |= static_cast<bool>(reply.Event.KeyEvent.bKeyDown);
                                    break;
                                case MOUSE_EVENT:
                                    break;
                                case WINDOW_BUFFER_SIZE_EVENT: // Valid only for alt buffer.
                                    yield += console::ansi::win({
                                        reply.Event.WindowBufferSizeEvent.dwSize.X,
                                        reply.Event.WindowBufferSizeEvent.dwSize.Y });
                                    break;
                                case FOCUS_EVENT:
                                    yield += console::ansi::fcs(
                                        reply.Event.FocusEvent.bSetFocus);
                                    break;
                                default:
                                    break;
                            }
                        }

                        if (vtcon)
                        {
                            CONSOLE_READCONSOLE_CONTROL state = { sizeof(state) };

                            ReadConsoleW( // Auto flushed after reading.
                                input,
                                buff.data(),
                                (DWORD)buff.size(),
                                &count,
                                &state);

                            //todo forward key ctrl state too
                            yield += utf::to_utf(buff.data(), count);
                        }
                        else FlushConsoleInputBuffer(STDIN_FD);

                        ipcio.send(yield);
                    }
                }
            }

            #endif // USE_WIN32_INPUT

            #else

                bool legacy_mouse = mode & os::legacy::mouse;
                bool legacy_color = mode & os::legacy::vga16;
                file micefd;
                twod mcoor;
                auto buffer = text(STDIN_BUF, '\0');
                iota ttynum = 0;
                ansi::esc yield;

                struct
                {
                    testy<twod> coord;
                    testy<iota> shift = 0;
                    testy<iota> bttns = 0;
                    iota        flags = 0;
                } state;
                auto get_kb_state = []()
                {
                    iota state = 0;
                    #if defined(__linux__)
                        iota shift_state = 6;
                        ok(::ioctl(STDIN_FD, TIOCLINUX, &shift_state));
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
                ok(::ttyname_r(STDOUT_FD, buffer.data(), buffer.size()), "ttyname_r error");
                auto tty_name = view(buffer.data());
                log(" tty: pseudoterminal ", tty_name);
                if (legacy_mouse)
                {
                    log(" tty: compatibility mode");
                    auto imps2_init_string = "\xf3\xc8\xf3\x64\xf3\x50";
                    auto mouse_device = "/dev/input/mice";
                    auto fd = ::open(mouse_device, O_RDWR);
                    if(fd == -1) log(" tty: error opening ", mouse_device, ", error ", errno, errno == 13 ? " - permission denied" : "");
                    else if (os::send(fd, imps2_init_string, sizeof(imps2_init_string)))
                    {
                        char ack;
                        recv(fd, &ack, sizeof(ack));
                        if (ack == '\xfa')
                        {
                            micefd = file{ fd };
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
                            yield.show_mouse(true);
                            ipcio.send<faux>(view(yield));
                            yield.clear();
                            log(" tty: mouse successfully connected, fd=", fd);
                        }
                    }
                    if (!micefd)
                    {
                        log(" tty: mouse initialization error");
                        ::close(fd);
                    }
                }

                while (ipcio)
                {
                    fd_set socks;
                    FD_ZERO(&socks);
                    FD_SET(STDIN_FD,    &socks);
                    FD_SET((fd_t)flash, &socks);
                    auto nfds = std::max({ STDIN_FD, (fd_t)flash });
                    if (micefd)
                    {
                        nfds = std::max(nfds, (fd_t)micefd);
                        FD_SET((fd_t)micefd, &socks);
                    }
                    nfds++;
                    if (::select(nfds, &socks, 0, 0, 0) > 0)
                    {
                        if (FD_ISSET(STDIN_FD, &socks))
                        {
                            if (micefd && state.shift(get_kb_state()))
                            {
                                yield.meta_state(state.shift.last);
                                auto data = os::recv(STDIN_FD, buffer.data(), buffer.size());
                                yield.add(data);
                                ipcio.send<faux>(yield);
                                yield.clear();
                            }
                            else
                            {
                                auto data = os::recv(STDIN_FD, buffer.data(), buffer.size());
                                ipcio.send<faux>(data);
                            }
                        }
                        else if (FD_ISSET((fd_t)flash, &socks))
                        {
                            log("xipc: flash fired");
                            flash.flush();
                        }
                        else if (micefd && FD_ISSET((fd_t)micefd, &socks))
                        {
                            auto data = os::recv(micefd, buffer.data(), buffer.size());
                            if (data.size() == 4 /*ImPS only*/)
                            {
                                #if defined(__linux__)
                                    vt_stat vt_state;
                                    ok(::ioctl(STDOUT_FD, VT_GETSTATE, &vt_state));
                                    if (vt_state.v_active == ttynum) // Proceed current active tty only.
                                    {
                                        auto scale = twod{6,12}; //todo magic numbers
                                        auto bttns = data[0] & 7;
                                        mcoor.x += data[1];
                                        mcoor.y -= data[2];
                                        auto wheel = -data[3];
                                        auto limit = _globals<void>::winsz.last * scale;
                                        if (bttns == 0) mcoor = std::clamp(mcoor, dot_00, limit);
                                        state.flags = wheel ? 4 : 0;
                                        if (state.coord(mcoor / scale)
                                        || state.bttns(bttns)
                                        || state.shift(get_kb_state())
                                        || state.flags)
                                        {
                                            yield.w32begin()
                                            .w32mouse(0,
                                                    state.bttns.last,
                                                    state.shift.last,
                                                    state.flags,
                                                    wheel,
                                                    state.coord.last.x,
                                                    state.coord.last.y)
                                            .w32close();
                                            ipcio.send<faux>(view(yield));
                                            yield.clear();
                                        }
                                    }
                                #endif
                            }
                        }
                    }
                }

            #endif

            log(" tty: reader thread completed");
        }

        tty()
        { }

    public:
        static auto proxy(xipc pipe_link)
        {
            _globals<void>::ipcio = pipe_link;
            return tty{};
        }
        bool output(view utf8)
        {
            return os::send<true>(STDOUT_FD, utf8.data(), utf8.size());
        }
        void ignite()
        {
            auto& sig_hndl = _globals<void>::signal_handler;

            #if defined(_WIN32)

                auto& omode = _globals<void>::state[0];
                auto& imode = _globals<void>::state[1];

                ok(GetConsoleMode(STDOUT_FD, &omode), "GetConsoleMode error (stdout)");
                ok(GetConsoleMode(STDIN_FD , &imode), "GetConsoleMode error (stdin)");

                DWORD inpmode = 0
                              | ENABLE_EXTENDED_FLAGS
                              | ENABLE_PROCESSED_INPUT
                              | ENABLE_WINDOW_INPUT
                              | ENABLE_MOUSE_INPUT
                            #ifndef VTM_USE_CLASSICAL_WIN32_INPUT
                              | ENABLE_VIRTUAL_TERMINAL_INPUT
                            #endif
                              ;
                ok(SetConsoleMode(STDIN_FD, inpmode), "SetConsoleMode error (stdin)");

                DWORD outmode = 0
                              | ENABLE_PROCESSED_OUTPUT
                              | ENABLE_VIRTUAL_TERMINAL_PROCESSING
                              | DISABLE_NEWLINE_AUTO_RETURN
                              ;
                ok(SetConsoleMode(STDOUT_FD, outmode), "SetConsoleMode error (stdout)");
                ok(SetConsoleCtrlHandler(sig_hndl, TRUE), "SetConsoleCtrlHandler error");

            #else

                auto& state = _globals<void>::state;
                if (ok(::tcgetattr(STDIN_FD, &state))) // Set stdin raw mode.
                {
                    auto raw_mode = state;
                    ::cfmakeraw(&raw_mode);
                    ok(::tcsetattr(STDIN_FD, TCSANOW, &raw_mode));
                }
                ok(::signal(SIGPIPE , SIG_IGN ));
                ok(::signal(SIGWINCH, sig_hndl));
                ok(::signal(SIGTERM , sig_hndl));
                ok(::signal(SIGHUP  , sig_hndl));

            #endif

            ::atexit(_globals<void>::default_mode);
            _globals<void>::resize_handler();
        }
        void splice(iota mode)
        {
            auto& ipcio = *_globals<void>::ipcio;

            os::set_palette(mode);
            os::vgafont_update(mode);

            auto  input = std::thread{ [&]() { reader(mode); } };

            while (output(ipcio.recv()))
            { }
            os::rst_palette(mode);

            ipcio.reset();
            flash.reset();

            if (input.joinable())
                input.join();
        }
    };

    template<class V> xipc        tty::_globals<V>::ipcio;
    template<class V> conmode     tty::_globals<V>::state;
    template<class V> testy<twod> tty::_globals<V>::winsz;

    class cons
    {
        #if defined(_WIN32)

            HPCON  hPC      { INVALID_FD };
            HANDLE hProcess { INVALID_FD };
            HANDLE hThread  { INVALID_FD };
            HANDLE gameover { INVALID_FD };
            std::thread client_exit_waiter;

        #else

            pid_t pid = 0;

        #endif

        os::ipc                   termlink;
        testy<twod>               termsize;
        std::thread               stdinput;
        std::function<void(view)> receiver;
        std::function<void(iota)> shutdown;

    public:
        ~cons()
        {
            log("cons: dtor started");
            if (termlink) wait_child();
            if (stdinput.joinable())
            {
                log("cons: input thread joining");
                stdinput.join();
            }
            #if defined(_WIN32)
                if (client_exit_waiter.joinable())
                {
                    log("cons: client_exit_waiter thread joining");
                    client_exit_waiter.join();
                }
                log("cons: client_exit_waiter thread joined");
            #endif
        }
        
        operator bool () { return termlink; }

        void start(text cmdline, twod winsz, std::function<void(view)> input_hndl
                                           , std::function<void(iota)> shutdown_hndl)
        {
            receiver = input_hndl;
            shutdown = shutdown_hndl;
            log("cons: new process: ", cmdline);

            #if defined(_WIN32)

                termsize(winsz);
                auto create = [](HPCON& hPC, twod winsz,
                    HANDLE& m_pipe_r, HANDLE& m_pipe_w)
                {
                    HRESULT err_code{ E_UNEXPECTED };
                    HANDLE  s_pipe_r{ INVALID_HANDLE_VALUE };
                    HANDLE  s_pipe_w{ INVALID_HANDLE_VALUE };

                    if (CreatePipe(&m_pipe_r, &s_pipe_w, nullptr, 0) &&
                        CreatePipe(&s_pipe_r, &m_pipe_w, nullptr, 0))
                    {
                        COORD consz;
                        consz.X = winsz.x;
                        consz.Y = winsz.y;
                        err_code = CreatePseudoConsole(consz, s_pipe_r, s_pipe_w, 0, &hPC);
                    }

                    CloseHandle(s_pipe_w);
                    CloseHandle(s_pipe_r);

                    return err_code ? faux : true;
                };
                auto fillup = [](HPCON hPC, auto& attrs_buff, auto& lpAttributeList)
                {
                    SIZE_T attr_size = 0;
                    InitializeProcThreadAttributeList(nullptr, 1, 0, &attr_size);
                    attrs_buff.resize(attr_size);
                    lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrs_buff.data());

                    if (InitializeProcThreadAttributeList(lpAttributeList, 1, 0, &attr_size)
                        &&  UpdateProcThreadAttribute(lpAttributeList,
                                                      0,
                                                      PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                                      hPC,
                                                      sizeof(hPC),
                                                      nullptr,
                                                      nullptr))
                    {
                        return true;
                    }
                    else return faux;
                };

                HANDLE m_pipe_r{ INVALID_HANDLE_VALUE };
                HANDLE m_pipe_w{ INVALID_HANDLE_VALUE };
                STARTUPINFOEX        start_info{ sizeof(STARTUPINFOEX) };
                PROCESS_INFORMATION  procs_info{};
                std::vector<uint8_t> attrs_buff;

                if (   create(hPC, winsz, m_pipe_r, m_pipe_w)
                    && fillup(hPC, attrs_buff, start_info.lpAttributeList)
                    && CreateProcessA(
                        nullptr,                      // lpApplicationName
                        cmdline.data(),               // lpCommandLine
                        nullptr,                      // lpProcessAttributes
                        nullptr,                      // lpThreadAttributes
                        FALSE,                        // bInheritHandles
                        EXTENDED_STARTUPINFO_PRESENT, // dwCreationFlags (override startupInfo type)
                        nullptr,                      // lpCurrentDirectory
                        nullptr,                      // lpEnvironment
                        &start_info.StartupInfo,      // lpStartupInfo (ptr to STARTUPINFOEX)
                        &procs_info))                 // lpProcessInformation
                {
                    hProcess = procs_info.hProcess;
                    hThread  = procs_info.hThread;
                    gameover = CreateEvent(
                        NULL,   // security attributes
                        FALSE,  // auto-reset
                        FALSE,  // initial state
                        NULL);
                    client_exit_waiter = std::thread([&]
                    {
                        HANDLE signals[] = { hProcess, gameover };
                        if (WAIT_OBJECT_0 != WaitForMultipleObjects(2, signals, FALSE, INFINITE))
                        {
                            log("cons: client_exit_waiter error");
                        }
                        log("cons: client_exit_waiter finished");
                        CloseHandle(gameover);
                        if (termlink)
                        {
                            auto exit_code = wait_child();
                            shutdown(exit_code);
                        }
                        log("cons: client_exit_waiter exit");
                    });
                    termlink.set({ m_pipe_r, m_pipe_w }, true);
                    log("cons: conpty created: ", winsz);
                }
                else log("cons: process creation error ", GetLastError());

                //todo workaround for GH#10400 (resolved) https://github.com/microsoft/terminal/issues/10400
                std::this_thread::sleep_for(250ms);

            #else

                auto fdm = ::posix_openpt(O_RDWR | O_NOCTTY); // Get master PTY.
                auto rc1 = ::grantpt     (fdm);               // Grant master PTY file access.
                auto rc2 = ::unlockpt    (fdm);               // Unlock master PTY.
                auto fds = ::open(ptsname(fdm), O_RDWR);      // Open slave PTY via string ptsname(fdm).

                termlink.set({ fdm }, true);
                resize(winsz);

                pid = ::fork();
                if (pid == 0) // Child branch.
                {
                    ::close(fdm);
                    ok(::setsid(), "setsid error"); // Make the current process a new session leader, return process group id.

                    // In order to receive WINCH signal make fds the controlling
                    // terminal of the current process.
                    // Current process must be a session leader (::setsid()) and not have
                    // a controlling terminal already.
                    // arg = 0: 1 - to stole fds from another process, it doesn't matter here.
                    ok(::ioctl(fds, TIOCSCTTY, 0), "cons: assign controlling terminal error");

                    ::signal(SIGINT,  SIG_DFL); // Reset control signals to default values.
                    ::signal(SIGQUIT, SIG_DFL); //
                    ::signal(SIGTSTP, SIG_DFL); //
                    ::signal(SIGTTIN, SIG_DFL); //
                    ::signal(SIGTTOU, SIG_DFL); //
                    ::signal(SIGCHLD, SIG_DFL); //

                    ::dup2 (fds, STDIN_FILENO);  // Assign stdio lines atomically
                    ::dup2 (fds, STDOUT_FILENO); // = close(new);
                    ::dup2 (fds, STDERR_FILENO); // fcntl(old, F_DUPFD, new)
                    ::close(fds);

                    auto args = os::split_cmdline(cmdline);
                    std::vector<char*> argv;
                    for (auto& c : args)
                    {
                        argv.push_back(c.data());
                    }
                    argv.push_back(nullptr);

                    ::setenv("TERM", "xterm-256color", 1); //todo too hacky
                    ok(::execvp(argv.front(), argv.data()), "execvp error");
                    os::exit(1, "cons: exec error ", errno);
                }

                // Parent branch.
                ::close(fds);

            #endif

            stdinput = std::thread([&] { read_socket_thread(); });
        }

        iota wait_child()
        {
            iota exit_code;
            termlink.reset();
            log("cons: wait child process");

            #if defined(_WIN32)

                ClosePseudoConsole(hPC);
                termlink.shut();
                DWORD code = 0;
                if (GetExitCodeProcess(hProcess, &code) == FALSE) log("cons: child GetExitCodeProcess() error: ", GetLastError());
                else if (code == STILL_ACTIVE)                    log("cons: child process still running");
                else                                              log("cons: child process exit code ", code);
                exit_code = code;
                SetEvent(gameover);
                CloseHandle(hProcess);
                CloseHandle(hThread);

            #else

                int status;
                ok(::kill(pid, SIGKILL));
                ok(::waitpid(pid, &status, 0)); // Wait for the child to avoid zombies.
                if (WIFEXITED(status))
                {
                    exit_code = WEXITSTATUS(status);
                    log("cons: child process exit code ", exit_code);
                }
                else
                {
                    exit_code = 0;
                    log("cons: error: child process exit code not detected");
                }

            #endif
            log("cons: wait_child() exit");
            return exit_code;
        }
        void read_socket_thread()
        {
            text flow;
            while (termlink)
            {
                if (auto shot = termlink.recv())
                {
                    flow += shot;
                    auto crop = ansi::purify(flow);
                    receiver(crop);
                    flow.erase(0, crop.size()); // Delete processed data.
                }
                else break;
            }
            if (termlink)
            {
                log("cons: read_socket_thread ended");
                auto exit_code = wait_child();
                shutdown(exit_code);
            }
        }
        void resize(twod const& newsize)
        {
            if (termsize(newsize) && termlink)
            {
                #if defined(_WIN32)

                    COORD winsz;
                    winsz.X = newsize.x;
                    winsz.Y = newsize.y;
                    auto hr = ResizePseudoConsole(hPC, winsz);
                    if (hr != S_OK) log("cons: ResizePseudoConsole error, ", hr);

                #else

                    winsize winsz;
                    winsz.ws_col = newsize.x;
                    winsz.ws_row = newsize.y;
                    ok(::ioctl(termlink.get(), TIOCSWINSZ, &winsz));

                #endif
            }
        }
        void write(view data)
        {
            termlink.send<true>(data);
        }
    };
}

#endif // NETXS_SYSTEM_HPP