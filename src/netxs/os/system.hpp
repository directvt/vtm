// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_SYSTEM_HPP
#define NETXS_SYSTEM_HPP

#ifndef VTM_USE_CLASSICAL_WIN32_INPUT
#define VTM_USE_CLASSICAL_WIN32_INPUT // Turns on classical console win32 input mode.
#endif

#if defined(_WIN32)
#elif defined(__linux__) || defined(__APPLE__)
#endif

#include "file_system.hpp"
#include "../text/logger.hpp"
#include "../datetime/quartz.hpp"
#include "../abstract/ptr.hpp"
#include "../console/ansi.hpp"

#include <type_traits>
#include <iostream>		// std::cout

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif

    #pragma warning(disable:4996) // disable std::getenv warnimg

    #pragma comment(lib, "Advapi32.lib")  // GetUserName()

    //#pragma comment(lib, "Wtsapi32.lib")  //WTS users list / WTSEnumerateSessions
    //#pragma comment(lib, "Shlwapi.lib")   // SHDeleteKeyW
    //#pragma comment(lib, "Psapi.lib")  //GetModuleFileNameEx
    //#pragma comment(lib, "Ole32.lib")   //CoCreateInstance  (creating shortcut)
    //#pragma comment(lib, "Shell32.lib")   //CommandLineToArgvW

    //#pragma comment(lib, "libvcruntime.lib")   //__except_handler4

    // /* LINK : warning LNK4098: defaultlib 'libvcruntime.lib' conflicts
    //  *        with use of other libs; use /NODEFAULTLIB:library
    //  * https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=vs-2017
    //  */
    // #if defined(_DEBUG)
    // 	#pragma comment(lib, "libvcruntimed.lib")
    // #else
    // 	#pragma comment(lib, "libvcruntime.lib") //__except_handler4
    // #endif

    #include <Windows.h>
    #include <Psapi.h>		// GetModuleFileNameEx
    #include <Wtsapi32.h>	// get_logged_usres WTSEnumerateSessions

    //Specific
    #include <Shlwapi.h>
    #include <algorithm>
    #include <Wtsapi32.h>
    #include <shobjidl.h>	//IShellLink
    #include <shlguid.h>	//IID_IShellLink
    #include <Shlobj.h>		//create_shortcut: SHGetFolderLocation / (SHGetFolderPathW - vist and later) CLSID_ShellLink
    #include <Psapi.h>		//GetModuleFileNameEx

    #include <DsGetDC.h>	//DsGetDcName
    #include <LMCons.h>		//DsGetDcName
    #include <Lmapibuf.h>	//DsGetDcName

    #include <Sddl.h>		//security_descriptor

#elif defined(__linux__) || defined(__APPLE__)

    //#include <sys/signalfd.h> // ::signalfd()
    //#include <sys/eventfd.h> // ::signalfd()

    #include <errno.h>		// switch(errno)
    #include <string.h>		// ::memset()

    #include <stdlib.h>		// filepath
    #include <spawn.h>		// exec

    #include <unistd.h>		// ::gethostname(), ::getpid(), ::read()
    #include <sys/param.h>	//

    #include <utmp.h>		// get_logged_usres

    #include <sys/types.h>	// getaddrinfo
    #include <sys/socket.h> // ::shutdown() ::socket(2)
    #include <netdb.h>		//

    //extern char **environ;

    //for sockets
    #include <stdio.h>
    #include <unistd.h>		// ::read(), PIPE_BUF
    #include <sys/un.h>
    #include <stdlib.h>

    #include <csignal>		// ::signal(), for window size
    #include <termios.h>   // for raw mode
    #include <sys/ioctl.h> // ioctl, for window size
    #include <sys/wait.h>  // waitpid
    #include <syslog.h>    // syslog for daemonize

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>		// ::splice()

    //#include <ext/stdio_filebuf.h> // Linux specific ifstream from socket descriptor

#endif

namespace netxs::os
{
    using list = std::vector<text>;
    using ui32 = uint32_t;
    using iota = int32_t;
    using namespace std::chrono_literals;
    using namespace netxs::console;

    static text current_module_file();
    static bool exec(text binary, text parameters = {}, int windw_state = 0);
    inline void start_log(view srv_name);
    inline void syslog(text const& data);
    inline bool daemonize(view srv_name);
    static text host_name();
    static bool is_mutex_exists(text&& mutex_name);
    static ui32 process_id();
    static text logged_in_users(view domain_delimiter = "\\", view record_delimiter = "\0");
    //inline auto user();
    //inline void exit(int code, view reason = {});
    inline auto error()
    {
        #if defined(_WIN32)

            return GetLastError();

        #elif defined(__linux__) || defined(__APPLE__)

            return errno;

        #endif
    }
    //inline void disable_sigpipe();

    static bool is_daemon = faux;

    inline auto get_env(text&& var)
    {
        #if defined(_WIN32)

        //#define _CRT_SECURE_NO_WARNINGS 1
            auto val = std::getenv(var.c_str());
            return val ? text{ val }
                       : text{};
        //#undef _CRT_SECURE_NO_WARNINGS

        #elif defined(__linux__) || defined(__APPLE__)

            auto val = std::getenv(var.c_str());
            return val ? text{ val }
                       : text{};

        #endif
    }

    #if defined(_WIN32)

    static text  take_partition(text&& file);
    static text  take_temp(text&& file);
    static text  trusted_domain_name();
    static text  trusted_domain_guid();
    static bool  create_shortcut(text&& path_to_object, text&& path_to_link);
    static text  expand(text&& directory);
    static bool  set_registry_key(text&& path, text&& name, text&& value);
    static text  get_registry_string_value(text&& path, text&& name);
    static list  get_registry_subkeys(text&& path);
    static list  cmdline();
    static bool  delete_registry_tree(text&& path);
    static void  update_process_privileges(void);
    static bool  kill_process(unsigned long proc_id);
    static text  global_startup_dir();

    static int64_t	get_host_id();
    static void	save_host_id(int64_t id);
    static void	set_dns_suffix(text&& dns_suffix);

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

    #endif // Windows specific

    inline void exit(int code)
    {
        #if defined(_WIN32)

            ExitProcess(code);

        #elif defined(__linux__) || defined(__APPLE__)

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

    text current_module_file()
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

        #elif defined(__linux__) || defined(__APPLE__)

        char* resolved = realpath("/proc/self/exe", NULL);
        if (resolved)
        {
            result = std::string(resolved);
            free(resolved);
        }
        #endif

        return result;
    }

    auto split_cmdline(view cmdline)
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
    bool exec(text binary, text params, int window_state)
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

        #elif defined(__linux__) || defined(__APPLE__)

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
    void start_log(view srv_name)
    {
        #if defined(_WIN32)

            //todo inplement

        #elif defined(__linux__) || defined(__APPLE__)

            ::openlog(srv_name.data(), LOG_NOWAIT | LOG_PID, LOG_USER);
            is_daemon = true;

        #endif
    }
    void syslog(text const& data)
    {
        #if defined(_WIN32)

            std::cout << data << std::flush;

        #elif defined(__linux__) || defined(__APPLE__)

            if (os::is_daemon) ::syslog(LOG_NOTICE, "%s", data.c_str());
            else               std::cout << data << std::flush;

        #endif
    }
    //todo unify with exec()
    bool daemonize(view srv_name)
    {
        #if defined(_WIN32)
            return true;
        #elif defined(__linux__) || defined(__APPLE__)

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
                //is_daemon = true;

                umask(0);

                // Open system logs for the child process.
                start_log(srv_name);
                //::openlog(srv_name.data(), LOG_NOWAIT | LOG_PID, LOG_USER);

                // A daemon cannot use the terminal, so close standard file descriptors for security reasons
                ::close(STDIN_FILENO);
                ::close(STDOUT_FILENO);
                ::close(STDERR_FILENO);

                return true;
            }

            os::exit(0); // SUCCESS (This child is reaped below with waitpid()).
        }

        // Reap the child, leaving the grandchild to be inherited by init.
        int Stat;
        ::waitpid(pid, &Stat, 0);
        if (WIFEXITED(Stat) && (WEXITSTATUS(Stat) == 0))
        {
            os::exit(0); // Child forked and exited successfully.
        }
        else
        {
            return faux;
        }

        return faux;

        #endif
    }
    text host_name()
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

        #elif defined(__linux__) || defined(__APPLE__)


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
    bool is_mutex_exists(text&& mutex_name)
    {
        bool result = false;

        #if defined(_WIN32)
            HANDLE mutex = CreateMutex(0, 0, mutex_name.c_str());
            result = GetLastError() == ERROR_ALREADY_EXISTS;
            CloseHandle(mutex);

            return result;
        #elif defined(__linux__) || defined(__APPLE__)
            //todo linux implementation
            return true;
        #endif
    }
    ui32 process_id()
    {
        uint32_t result;

        #if defined(_WIN32)

            result = GetCurrentProcessId();

        #elif defined(__linux__) || defined(__APPLE__)

            result = getpid();

        #endif
        return result;
    }
    text logged_in_users(view domain_delimiter, view record_delimiter)
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

        #elif defined(__linux__) || defined(__APPLE__)

        #define NAME_WIDTH  8

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
    auto user()
    {
        #if defined(_WIN32)

            static constexpr auto INFO_BUFFER_SIZE = 32767UL;
            //TCHAR  infoBuf[INFO_BUFFER_SIZE];
            char   infoBuf[INFO_BUFFER_SIZE];
            DWORD  bufCharCount = INFO_BUFFER_SIZE;

            if (!GetUserName(infoBuf, &bufCharCount))
                log("error GetUserName");

            return text(infoBuf, bufCharCount);

        #elif defined(__linux__) || defined(__APPLE__)

            uid_t id;
            id = ::geteuid();
            return id;

        #endif
    }

    //void disable_sigpipe()
    //{
    //	if (::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    //	{
    //		throw;
    //	}
    //}

    #if defined(_WIN32)
    // Windows specific functions

    text take_partition(text&& file)
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
    text take_temp(text&& file)
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
    text trusted_domain_name()
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
    text trusted_domain_guid()
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
    bool create_shortcut(text&& path_to_object, text&& path_to_link)
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
    text expand(text&& directory)
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
    bool set_registry_key(text&& key_path, text&& parameter_name, text&& value)
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
    text get_registry_string_value(text&& key_path, text&& parameter_name)
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
    list get_registry_subkeys(text&& key_path)
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
    list cmdline()
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
    bool delete_registry_tree(text&& path)
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
    void update_process_privileges(void)
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
    bool kill_process(unsigned long proc_id)
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
    text global_startup_dir()
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


    enum role { client, server };

    using xipc = std::shared_ptr<class ipc>;

    class ipc
    {
        const bool IS_TTY = faux;

    public:
        using vect = std::vector<char>;

        #if defined(_WIN32)

            using type = HANDLE; // typename std::invoke_result<::socket, int, int, int>::type;
            static constexpr type INVALID_FD = INVALID_HANDLE_VALUE;
            static constexpr iota PIPE_BUF = 65536;
            static constexpr iota STDIN_BUF = 1024;

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

    private:
            text pipepath;
            type r_handle; // sock: socket file descriptor
            type w_handle; // sock: socket file descriptor

        #elif defined(__linux__) || defined(__APPLE__)

            using type = int; // typename std::invoke_result<::socket, int, int, int>::type;
            static constexpr type INVALID_FD = -1;
            type handle; // sock: socket file descriptor
            type charge_w; // sock: pipe for events (instead of eventfd)

            //using lock = std::recursive_mutex;
            //lock      mutex; // pipe: Thread sync mutex.

        #endif

        vect buffer; // sock: receive buffer
        type charge; // sock: descriptor for reading interrupt
        bool sealed; // sock: provide autoclosing
        text path; // sock: Socket path (in order to unlink)

        void init(iota buff_size = PIPE_BUF) { buffer.resize(buff_size); }

        struct nothing
        {
            template<class T>
            operator T () { return T{}; }
        };
        template<class ...Args>
        static auto fail(Args&&... msg)
        {
            log("xipc: ", msg..., " (", os::error(), ") ");
            return nothing{};
        };

    public:
        #if defined(_WIN32)
            ipc(type r_socket = INVALID_FD, type w_socket = INVALID_FD, bool sealed = faux)
                :	r_handle{ r_socket },
                    w_handle{ w_socket },
                    sealed{ sealed }
            {
                charge = CreateEvent(NULL, TRUE, TRUE, NULL);
                if (charge == INVALID_FD) log("xipc - signalfd error");
                //log("xipc: control descriptor ", charge);
                if (*this) init();
            }
            ~ipc()
            {
                log("xipc: closing ", *this);
                if (sealed)
                {
                    if (r_handle != INVALID_FD) CloseHandle(r_handle);
                    if (w_handle != INVALID_FD) CloseHandle(w_handle);
                }
                CloseHandle(charge);
            }
            operator bool () { return r_handle != INVALID_FD && w_handle != INVALID_FD; }
        #elif defined(__linux__) || defined(__APPLE__)
            ipc(type socket = INVALID_FD, bool sealed = faux, bool IS_TTY = faux)
                : handle{ socket },
                  sealed{ sealed },
                  IS_TTY{ IS_TTY }
            {
                //todo ::signal(SIGPIPE, SIG_IGN) does not work
                if (::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
                {
                    throw;
                }

                // instead of ::signal(SIGPIPE, SIG_IGN)
                //static struct sigaction sigign = { SIG_IGN };
                //::sigaction(SIGPIPE, &sigign, nullptr);

                //charge = ::eventfd(0, 0);
                //if (charge == INVALID_FD) log("xipc - signalfd error");

                type eventfd[2];
                auto isok = ::pipe(eventfd);
                charge   = eventfd[0];
                charge_w = eventfd[1];
                if (isok < 0) log("xipc - signalfd error");

                log("xipc: control descriptor ", charge);
                if (*this) init();
            }
            ~ipc()
            {
                log("xipc: closing ", *this);
                if (*this && sealed) ::close(handle);
                ::close(charge);
                ::close(charge_w);

                #if defined(__APPLE__)

                // cleanup file system unix domain socket
                if (path.length())
                {
                    ::unlink(path.c_str());
                }

                #endif
            }
            operator bool () { return handle != INVALID_FD; }
        #endif

        #if defined(_WIN32)
            void set(type r_h, type w_h, bool s)
            {
                r_handle = r_h;
                w_handle = w_h;
                sealed = s;
                if (*this) init();
            }
        #elif defined(__linux__) || defined(__APPLE__)
            void set(type h, bool s)
            {
                handle = h;
                sealed = s;
                if (*this) init();
            }
        #endif
        auto get()
        {
            #if defined(_WIN32)

                return std::pair{ r_handle, w_handle };

            #elif defined(__linux__) || defined(__APPLE__)

                return handle;

            #endif
        }
        //todo implement xplat
        template<class T>
        auto cred(T id) const // get peer cred
        {
            #if defined(_WIN32)

            //todo implement for win32

            //#elif defined(__linux__) || defined(__APPLE__)
            #elif defined(__linux__)

            //struct result
            //{
            //	long pid, uid, gid;
            //};

            struct ucred cred = {};
            unsigned size = sizeof(cred);
            auto error = -1 == ::getsockopt(handle, SOL_SOCKET, SO_PEERCRED, &cred, &size);

            //return success ? std::optional{ result{ cred.pid, cred.uid, cred.gid } }
            //               : std::nullopt;

            if (error)
            {
                log("sock: getsockopt error ", errno, ", abort");
                return faux;
            }
            if (cred.uid && id != cred.uid) // Deny foreign users except root
            {
                log("sock: other users are not allowed to the session, abort");
                return faux;
            }

            log("sock: creds from SO_PEERCRED",
                ":  pid=", cred.pid,
                ", euid=", cred.uid,
                ", egid=", cred.gid);

            #elif defined(__APPLE__)

            uid_t euid;
            gid_t egid;
            auto error = -1 == ::getpeereid(handle, &euid, &egid);

            if (error || (euid && id != euid)) // Deny foreign users except root
            {
                //log("sock: other users are not allowed to the session, abort");
                return faux;
            }

            log("sock: socket owner: ", id);
            log("sock: peer creds from getpeereid",
                ", euid=", euid,
                ", egid=", egid);

            #endif

            return true;
        }
        auto meet()
            -> std::shared_ptr<ipc>
        {
            #if defined(_WIN32)

            //security_descriptor pipe_acl(security_descriptor_string);

                auto sock_ptr = std::make_shared<ipc>(r_handle, w_handle, true);
                //log("------- waiting clients on ", sock_ptr);

                auto to_server = "\\\\.\\pipe\\r_" + pipepath;
                auto to_client = "\\\\.\\pipe\\w_" + pipepath;

                auto r_fConnected = ConnectNamedPipe(r_handle, NULL)
                    ? true
                    : (GetLastError() == ERROR_PIPE_CONNECTED);
                //log(r_handle, " is r_fConnected = ", r_fConnected ? "true" : "faux");

                // recreate the waiting point for the next client
                r_handle = CreateNamedPipe(
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

                //log("created a new r_handle = ", r_handle);
                if (r_handle == INVALID_FD)
                {
                    r_handle = sock_ptr->r_handle;
                    return fail("CreateNamedPipe error (read)");
                }

                auto w_fConnected = ConnectNamedPipe(w_handle, NULL)
                    ? true
                    : (GetLastError() == ERROR_PIPE_CONNECTED);
                //log(w_handle, " is w_fConnected = ", w_fConnected ? "true" : "faux");

                w_handle = CreateNamedPipe(
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
                    //pipe_acl);                // DACL

                //log("created a new w_handle = ", w_handle);
                if (w_handle == INVALID_FD)
                {
                    CloseHandle(r_handle);
                    r_handle = sock_ptr->r_handle;
                    w_handle = sock_ptr->w_handle;
                    return fail("CreateNamedPipe error (write)");
                }

                return sock_ptr;

            #elif defined(__linux__) || defined(__APPLE__)

                auto sock_ptr
                    = std::make_shared<ipc>(::accept(handle, 0, 0), true);

                return *sock_ptr ? sock_ptr
                                 : nullptr;

                //if (*sock_ptr)
                //{
                //	//to prevent SIGPIPE on the socket
                //	//int set = 1;
                //	//auto fd = sock_ptr->get();
                //	//log ("set MSG_NOSIGNAL for ", handle);
                //	//log ("set MSG_NOSIGNAL for ", fd);
                //	//auto p = ::setsockopt(handle, SOL_SOCKET, MSG_NOSIGNAL, (void*)&set, sizeof(int));
                //	//if (p == -1) fail(" error set MSG_NOSIGNAL for ", handle);
                //	//auto p= ::setsockopt(fd, SOL_SOCKET, MSG_NOSIGNAL, (void*)&set, sizeof(int));
                //	//if (p == -1) fail(" error set MSG_NOSIGNAL for ", fd);
                //	return sock_ptr;
                //}
                //else return nullptr;

            #endif

        }
        template<class SIZE_T>
        auto recv(char* buff, SIZE_T size) const
        {
            #if defined(_WIN32)

                bool fSuccess = faux;
                DWORD count;

                //HANDLE waits[2] = { r_handle, charge };
                //if (WAIT_OBJECT_0 == WaitForMultipleObjects(2, waits, FALSE, INFINITE))
                fSuccess = ReadFile(
                    r_handle,    // pipe handle
                    buff,        // buffer to receive reply
                    (DWORD)size, // size of buffer
                    &count,      // number of bytes read
                    NULL);       // not overlapped

                if (fSuccess) return qiew{ buff, count };
                else          return qiew{};

            #elif defined(__linux__) || defined(__APPLE__)

                auto count = ::read(handle, buff, size);
                if (count > 0) return qiew{ buff, count };
                else           return qiew{};
                //else
                //{
                //	if (count == -1)
                //	{
                //		log("recv: socket read error ", errno);
                //		return qiew{};
                //	}
                //	return qiew{ buff, 1 };
                //}

            #endif
        }
        auto recv() // It is not thread safe!
        {
            return recv(buffer.data(), buffer.size());
        }

        #if defined(__linux__) || defined(__APPLE__)
        auto pick() // interruptable read from stdin
        {
            fd_set socks;

            FD_ZERO(&socks);
            FD_SET(handle, &socks);
            FD_SET(charge, &socks);
            auto nfds = std::max(handle, charge) + 1;

            if (::select(nfds, &socks, 0, 0, 0) > 0)
            {
                if (FD_ISSET(charge, &socks))
                {
                    log("xipc: stop fired on ", *this, " by ctrl=", charge);
                    auto x = 0UL;
                    auto n = ::read(charge, &x, sizeof(x));
                }
                else if (FD_ISSET(handle, &socks))
                {
                    return recv();
                }
            }
            return qiew{};
        }
        #endif

        auto fire() // interrupt reading operation
        {
            #if defined(_WIN32)

                SetEvent(charge);

            #elif defined(__linux__) || defined(__APPLE__)

                auto x = 1UL;
                if (-1 == ::write(charge_w, &x, sizeof(x)))
                    log("xipc: stop write error on ", *this);

            #endif
        }
        //bool alive = true;
        auto send(char const* data, size_t size)// const
        {
            while (size)
            {

            #if defined(_WIN32)
                DWORD count;
                auto fSuccess = WriteFile(
                    w_handle,    // pipe handle
                    data,        // message
                    (DWORD)size, // message length
                    &count,      // bytes written
                    NULL);       // not overlapped
                if (!fSuccess) count = 0;

            #elif defined(__linux__) || defined(__APPLE__)

                //todo revise ::write (::signal(SIGPIPE, SIG_IGN) not work)
                //     or     ::send + , MSG_NOSIGNAL)

                //::signal(SIGPIPE, SIG_IGN);

                // std::lock_guard guard{ mutex }; // socket shutdown while writing cause SIGPIPE

                //auto count = ::write(handle, data, size);

                auto count = IS_TTY ? ::write(handle, data, size)
                                    : ::send (handle, data, size, MSG_NOSIGNAL); // not work with open_pty
                                                                                 // recursive connection causes sigpipe on destroy when using write(2)
                //auto count = ::send(handle, data, size, MSG_NOSIGNAL);

                // send(2)does not work with file descriptors, only sockets
                // write(2) works with fds as well as sockets
                //auto count = ::send(handle, data, size, MSG_NOSIGNAL);
                //fail("writing error");

            #endif

                if (count != size)
                {
                    if (count > 0)
                    {
                        log("xipc: partial writing: socket=", *this,
                            " total=", size, ", written=", count);
                        data += count;
                        size -= count;
                    }
                    else
                    {
                        fail("xipc: error write to socket=", *this);
                        return faux;
                    }
                }
                else return true;
            }
            return faux;
        }
        template<class T>
        auto send(T const& buff)// const
        {
            auto data = buff.data();
            auto size = buff.size();
            return send(data, size);
        }
        auto send(char c)// const
        {
            return send(&c, 1);
        }
        auto line(char delim) const /*Read line*/
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
            #if defined(_WIN32)

                if (sealed)
                { // Disconnection order does matter
                    if (w_handle != INVALID_FD) DisconnectNamedPipe(w_handle);
                    if (r_handle != INVALID_FD) DisconnectNamedPipe(r_handle);
                }
                return true;

            #elif defined(__linux__) || defined(__APPLE__)

                //an important conceptual reason to want
                //to use shutdown:
                //             to signal EOF to the peer
                //             and still be able to receive
                //             pending data the peer sent.
                //"shutdown() doesn't actually close the file descriptor
                //            — it just changes its usability.
                //To free a socket descriptor, you need to use close()."

                fail("closing handle ", handle);
                auto errcode = ::shutdown(handle, SHUT_RDWR); // Further sends and receives are disallowed
                if (errcode == -1)
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
        static auto open(text path, datetime::period retry_time = {}, P retry_proc = P())
            -> std::shared_ptr<ipc>
        {
            #if defined(_WIN32)

            //security_descriptor pipe_acl(security_descriptor_string);
            //log("pipe: DACL=", pipe_acl.security_string);

                auto sock_ptr = std::make_shared<ipc>(INVALID_FD, INVALID_FD, true);

                auto& r_sock = sock_ptr->r_handle;
                auto& w_sock = sock_ptr->w_handle;

                auto& pipepath = sock_ptr->pipepath;
                pipepath = path;
                auto to_server = "\\\\.\\pipe\\r_" + path;
                auto to_client = "\\\\.\\pipe\\w_" + path;

                if constexpr (ROLE == role::server)
                {
                    r_sock = CreateNamedPipe(
                        to_server.c_str(),        // pipe name
                        PIPE_ACCESS_INBOUND,      // read/write access
                        PIPE_TYPE_BYTE |          // message type pipe
                        PIPE_READMODE_BYTE |      // message-read mode
                        PIPE_WAIT,                // blocking mode
                        PIPE_UNLIMITED_INSTANCES, // max instances
                        PIPE_BUF,                 // output buffer size
                        PIPE_BUF,                 // input buffer size
                        0,                        // client time-out
                        NULL);                // DACL
                        //pipe_acl);                // DACL
                    if (r_sock == INVALID_FD)
                        return fail("CreateNamedPipe error (read)");

                    w_sock = CreateNamedPipe(
                        to_client.c_str(),        // pipe name
                        PIPE_ACCESS_OUTBOUND,     // read/write access
                        PIPE_TYPE_BYTE |          // message type pipe
                        PIPE_READMODE_BYTE |      // message-read mode
                        PIPE_WAIT,                // blocking mode
                        PIPE_UNLIMITED_INSTANCES, // max instances
                        PIPE_BUF,                 // output buffer size
                        PIPE_BUF,                 // input buffer size
                        0,                        // client time-out
                        NULL);                // DACL
                        //pipe_acl);                // DACL
                    if (w_sock == INVALID_FD)
                    {
                        CloseHandle(r_sock);
                        return fail("CreateNamedPipe error (write)");
                    }
                }
                else if constexpr (ROLE == role::client)
                {
                    auto play = [&]() -> bool {

                        w_sock = CreateFile(
                            to_server.c_str(), // pipe name
                            GENERIC_WRITE,
                            0,                // no sharing
                            NULL,             // default security attributes
                            OPEN_EXISTING,    // opens existing pipe
                            0,                // default attributes
                            NULL);            // no template file

                        if (w_sock == INVALID_FD)
                            return faux; // fail("could not open to_server link");

                        r_sock = CreateFile(
                            to_client.c_str(), // pipe name
                            GENERIC_READ,
                            0,                // no sharing
                            NULL,             // default security attributes
                            OPEN_EXISTING,    // opens existing pipe
                            0,                // default attributes
                            NULL);            // no template file

                        if (r_sock == INVALID_FD)
                        {
                            CloseHandle(w_sock);
                            return faux; // fail("could not open to_client link");
                        }
                        else return true; };

                    auto done = play();
                    if (!done)
                    {
                        if (!retry_proc())
                            return fail("failed to start server");

                        auto stop = datetime::tempus::now() + retry_time;
                        do
                        {
                            std::this_thread::sleep_for(100ms);
                            done = play();
                        }
                        while (!done && stop > datetime::tempus::now());

                        if (!done)
                            return fail("connection error");
                    }
                }

            #elif defined(__linux__) || defined(__APPLE__)

                auto sock_ptr = std::make_shared<ipc>(INVALID_FD, true);
                auto& sock = sock_ptr->handle;

                #if defined(__APPLE__)
                //todo unify see vtmd.cpp:1564, file system socket
                path = "/tmp/" + path + ".sock";
                //path = path + ".sock";
                #endif

                if (path.size() > sizeof(sockaddr_un::sun_path) - 2)
                    return fail("socket path too long");

                if ((sock = ::socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_FD)
                    return fail("open unix domain socket error");

                struct sockaddr_un addr = {};
                addr.sun_family = AF_UNIX;

            #if defined(__linux__)

                // abstract namespace socket (begins with zero)
                // The abstract socket namespace is a nonportable Linux extension.
                std::copy(path.begin(), path.end(), addr.sun_path + 1);

            #elif defined(__APPLE__)

                // file system unix domain socket
                std::copy(path.begin(), path.end(), addr.sun_path);

            #endif

                if constexpr (ROLE == role::server)
                {
                    #if defined(__APPLE__)
                    // cleanup file system socket.
                    ::unlink(path.c_str());
                    #endif

                    // For unlink on exit (file system socket).
                    sock_ptr->path = path;

                    if (::bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
                        return fail("error unix socket bind for ", path);

                    if (::listen(sock, 5) == -1)
                        return fail("error listen socket for ", path);
                }
                else if constexpr (ROLE == role::client)
                {
                    auto play = [&]() {
                        return -1 != ::connect(sock, (struct sockaddr*)&addr, sizeof(addr)); };

                    auto done = play();
                    if (!done)
                    {
                        if (!retry_proc())
                            return fail("failed to start server");

                        auto stop = datetime::tempus::now() + retry_time;
                        do
                        {
                            std::this_thread::sleep_for(100ms);
                            done = play();
                        }
                        while (!done && stop > datetime::tempus::now());

                        if (!done)
                            return fail("connection error");
                    }

                    //int val = 1;
                    //auto error = -1 == ::setsockopt(sock, SOL_SOCKET, SO_PASSCRED, &val, sizeof(val));
                }

            #endif

            sock_ptr->init();
            return sock_ptr;
        }

        friend auto& operator << (std::ostream& s, netxs::os::ipc const& sock)
        {
            #if defined(_WIN32)
            return s << "{ xipc: " << sock.r_handle << "," << sock.w_handle << " }";
            #elif defined(__linux__) || defined(__APPLE__)
            return s << "{ xipc: " << sock.handle << " }";
            #endif
        }
        friend auto& operator << (std::ostream& s, netxs::os::xipc const& sock)
        {
            return s << *sock;
        }
    };

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

    class tty
    {
        template<class V>
        struct _globals
        {
            static xipc sock;

            #if defined(_WIN32)

                static DWORD       omode;
                static DWORD       imode;
                static HANDLE      board;
                static HANDLE      input;
                static HANDLE      reset;
                static testy<twod> winsz;

            #elif defined(__linux__) || defined(__APPLE__)

                static ::termios mode;

                static void resize_handler()
                {
                    struct winsize size;
                    ::ioctl(1, TIOCGWINSZ, &size);
                    sock->send(console::ansi::win({ size.ws_col, size.ws_row }));
                }
                static void signal_handler(int signal)
                {
                    switch (signal)
                    {
                        case SIGWINCH:
                            resize_handler();
                            break;
                        default:
                            break;
                    }
                }
                static void default_mode()
                {
                    ::tcsetattr(0, TCSANOW, &mode);
                }

            #endif
        };

        #if defined(_WIN32)

            //ipc in_fd { STD_INPUT_HANDLE  };
            //ipc out_fd{ STD_OUTPUT_HANDLE };
            static auto get_size()
            {
                auto& board = _globals<void>::board;
                twod yield;
                CONSOLE_SCREEN_BUFFER_INFO cinfo;
                if(ok(GetConsoleScreenBufferInfo(board, &cinfo)))
                {
                    yield = twod{ cinfo.srWindow.Right - cinfo.srWindow.Left + 1,
                                  cinfo.srWindow.Bottom - cinfo.srWindow.Top + 1 };
                }
                return yield;
            }


        #elif defined(__linux__) || defined(__APPLE__)

            ipc in_fd { STDIN_FILENO  , faux, true };
            ipc out_fd{ STDOUT_FILENO , faux, true };

        #endif

        static void defeat(text msg = "")
        {
            log("proxy: platform specific error: ", msg, " ",
                #if defined(_WIN32)
                    GetLastError()
                #elif defined(__linux__) || defined(__APPLE__)
                    errno
                #endif
            );
        }
        template<class T>
        static bool ok(T error_condition, text msg = "")
        {
            if(
                #if defined(_WIN32)
                    error_condition == 0
                #elif defined(__linux__) || defined(__APPLE__)
                    error_condition == (T)-1
                #endif
            )
            {
                defeat(msg);
                return faux;
            }
            else return true;

        }
        void reader()
        {
            #if defined(_WIN32)

            auto& board = _globals<void>::board;
            auto& input = _globals<void>::input;
            auto& omode = _globals<void>::omode;
            auto& imode = _globals<void>::imode;
            auto& reset = _globals<void>::reset;
            auto& winsz = _globals<void>::winsz;

            auto& sock = *_globals<void>::sock;

            // the input codepage to UTF-8 is severely
            //          broken in all Windows versions
            //          ReadFile and ReadConsoleA either replace
            //          non-ASCII characters with NUL or return 0 bytes read.
            std::vector<INPUT_RECORD> reply(1);
            DWORD                     count;
            HANDLE                    waits[2] = { input, reset };

            #ifdef VTM_USE_CLASSICAL_WIN32_INPUT

            ansi::esc            yield;
            std::vector<wchar_t> slide(ipc::STDIN_BUF);
            while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, waits, FALSE, INFINITE))
            {
                if (!GetNumberOfConsoleInputEvents(input, &count))
                {
                    // ERROR_PIPE_NOT_CONNECTED
                    // 233 (0xE9)
                    // No process is on the other end of the pipe.
                    //defeat("GetNumberOfConsoleInputEvents error");
                    os::exit(-1, "GetNumberOfConsoleInputEvents error ", GetLastError());
                    break;
                }
                else if (count)
                {
                    if (count > reply.size()) reply.resize(count);

                    if (!ReadConsoleInputW(input, reply.data(), (DWORD)reply.size(), &count))
                    {
                        //ERROR_PIPE_NOT_CONNECTED = 0xE9 - it's means that the console is gone/crashed
                        //defeat("ReadConsoleInput error");
                        os::exit(-1, "ReadConsoleInput error ", GetLastError());
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
                                    if (winsz(get_size()))
                                    {
                                        yield.w32winsz(winsz.last);
                                    }
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
                        sock.send(yield);
                    }
                }
            }

            #else

            text                      yield;
            std::vector<wchar_t>      slide(ipc::STDIN_BUF);
            while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, waits, FALSE, INFINITE))
            {
                if (!GetNumberOfConsoleInputEvents(input, &count))
                {
                    defeat("GetNumberOfConsoleInputEvents error");
                }
                else if (count)
                {
                    if (count > reply.size()) reply.resize(count);

                    if (!PeekConsoleInput(input, reply.data(), (DWORD)reply.size(), &count))
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
                                slide.data(),
                                (DWORD)slide.size(),
                                &count,
                                &state);

                            //todo forward key ctrl state too
                            yield += utf::to_utf(slide.data(), count);
                        }
                        else FlushConsoleInputBuffer(input);

                        sock.send(yield);
                    }
                }
            }

            #endif // USE_WIN32_INPUT

            #elif defined(__linux__) || defined(__APPLE__)

                auto& sock = *_globals<void>::sock;

                while (sock.send(in_fd.pick())) { }

            #endif
        }
        void stop()
        {
            #if defined(_WIN32)

                auto& reset = _globals<void>::reset;
                ok(SetEvent(reset), "SetEvent error");

            #elif defined(__linux__) || defined(__APPLE__)

                in_fd.fire(); // Unblock reading thread.

            #endif
        }

        tty(xipc link)
        {
            _globals<void>::sock = link;
        }

    public:
        static auto proxy(xipc link)
        {
            return tty{ link };
        }
        bool output(view text)
        {
            #if defined(_WIN32)
                //todo unify, use fd
                if (text.size())
                {
                    //todo use sock(STDOUT_FD)->send(WriteFile...)
                    std::cout << text << std::flush;
                    return true;
                }
                else return faux;

            #elif defined(__linux__) || defined(__APPLE__)

                return out_fd.send(text);

            #endif
        }
        void ignite()
        {
            #if defined(_WIN32)

                auto& board = _globals<void>::board;
                auto& input = _globals<void>::input;
                auto& omode = _globals<void>::omode;
                auto& imode = _globals<void>::imode;
                auto& reset = _globals<void>::reset;
                auto& winsz = _globals<void>::winsz;
                auto& sock = *_globals<void>::sock;

                if ((board = GetStdHandle(STD_OUTPUT_HANDLE))== INVALID_HANDLE_VALUE)
                    defeat("GetStdHandle error (output)");
                if ((input = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
                    defeat("GetStdHandle error (input)");

                ok(reset = CreateEvent(NULL, TRUE, FALSE, NULL), "CreateEvent error");

                ok(GetConsoleMode(board, &omode), "GetConsoleMode error (omode)");
                ok(GetConsoleMode(input, &imode), "GetConsoleMode error (imode)");

                DWORD sources = 0
                              | ENABLE_EXTENDED_FLAGS
                              | ENABLE_PROCESSED_INPUT
                              | ENABLE_WINDOW_INPUT
                              | ENABLE_MOUSE_INPUT
                            #ifndef VTM_USE_CLASSICAL_WIN32_INPUT
                              | ENABLE_VIRTUAL_TERMINAL_INPUT
                            #endif
                              ;
                ok(SetConsoleMode(input, sources), "SetConsoleMode error (input)");

                DWORD mode = 0
                           | ENABLE_PROCESSED_OUTPUT
                           | ENABLE_VIRTUAL_TERMINAL_PROCESSING
                           | DISABLE_NEWLINE_AUTO_RETURN
                           ;
                ok(SetConsoleMode(board, mode), "SetConsoleMode error (board)");

                auto ctrlHandler = [](DWORD signal)->BOOL
                {
                    auto& sock = *_globals<void>::sock;
                    switch (signal)
                    {
                        case CTRL_C_EVENT:
                            sock.send(ansi::C0_ETX);
                            break;
                        case CTRL_BREAK_EVENT:
                            sock.send(ansi::C0_ETX);
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
                };
                SetConsoleCtrlHandler(ctrlHandler, TRUE);

                // Get current terminal window size.
                ansi::esc yield;
                winsz(get_size());
                yield.w32begin();
                yield.w32winsz(winsz.last);
                yield.w32close();
                sock.send(yield);

            #elif defined(__linux__) || defined(__APPLE__)

                auto& cur_mode = _globals<void>::mode;
                auto& sig_hndl = _globals<void>::signal_handler;
                auto& def_mode = _globals<void>::default_mode;

                if (ok(::tcgetattr(0, &cur_mode))) // Set stdin raw mode.
                {
                    ::termios raw_mode = cur_mode;
                    ::cfmakeraw(&raw_mode);
                    if (ok(::tcsetattr(0, TCSANOW, &raw_mode)))
                        ok(::atexit(def_mode));
                }
                ok(::signal(SIGPIPE,  SIG_IGN )); // Disable sigpipe.
                ok(::signal(SIGWINCH, sig_hndl)); // Set resize handler.
                ok(::raise (SIGWINCH));           // Get current terminal window size.

            #endif
        }
        void splice()
        {
            auto& sock = *_globals<void>::sock;
            auto input = std::thread{ [&]() { reader(); } };

            while (output(sock.recv()))
            { }

            stop(); // Unblock reading thread.

            if (input.joinable())
                input.join();
        }
        void revert()
        {
            #if defined(_WIN32)

                ok(SetConsoleMode(_globals<void>::board, _globals<void>::omode), "SetConsoleMode error (revert_o)");
                ok(SetConsoleMode(_globals<void>::input, _globals<void>::imode), "SetConsoleMode error (revert_i)");

            #elif defined(__linux__) || defined(__APPLE__)

            #endif
        }
    };

    #if defined(_WIN32)

    template<class V> DWORD       tty::_globals<V>::omode;
    template<class V> DWORD       tty::_globals<V>::imode;
    template<class V> HANDLE      tty::_globals<V>::board;
    template<class V> HANDLE      tty::_globals<V>::input;
    template<class V> HANDLE      tty::_globals<V>::reset;
    template<class V> testy<twod> tty::_globals<V>::winsz;

    #elif defined(__linux__) || defined(__APPLE__)

    template<class V> ::termios tty::_globals<V>::mode;

    #endif

    template<class V> xipc tty::_globals<V>::sock;

    class cons
    {
        testy<twod> consize;

        #if defined(_WIN32)

            os::ipc socket{ 0, 0, true };
            //todo make it as an os::pty class with os::ipc socket;
            HPCON  hPC     { INVALID_HANDLE_VALUE };
            HANDLE hProcess{ INVALID_HANDLE_VALUE };
            HANDLE hThread { INVALID_HANDLE_VALUE };

        #elif defined(__linux__) || defined(__APPLE__)

            os::ipc socket{ 0, faux, true };
            pid_t pid = 0;

        #endif

        text        stdin_text;
        text        ready_text;
        std::thread std_input;
        bool        alive; // cons: Read input loop state.

        //todo may be a list of functions?
        std::function<void(view)> receiver;

    public:
        ~cons()
        {
            close();
        }
        
        operator bool () { return alive; }

        void start(text cmdline, twod winsz, std::function<void(view)> input_hndl)
        {
            receiver = input_hndl;
            log("cons: new process: ", cmdline);

            #if defined(_WIN32)

                auto create = [](HPCON& hPC, twod winsz,
                    HANDLE& m_pipe_r, HANDLE& m_pipe_w)
                {
                    HRESULT err_code{ E_UNEXPECTED };
                    HANDLE  s_pipe_r{ INVALID_HANDLE_VALUE };
                    HANDLE  s_pipe_w{ INVALID_HANDLE_VALUE };

                    if (CreatePipe(&m_pipe_r, &s_pipe_w, nullptr, 0) &&
                        CreatePipe(&s_pipe_r, &m_pipe_w, nullptr, 0))
                    {
                        COORD consoleSize{ static_cast<SHORT>(winsz.x),
                                           static_cast<SHORT>(winsz.y)};
                        err_code =
                            CreatePseudoConsole(consoleSize, s_pipe_r, s_pipe_w, 0, &hPC);
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
                    lpAttributeList =
                        reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrs_buff.data());

                    if (InitializeProcThreadAttributeList(lpAttributeList, 1, 0, &attr_size)
                        &&  UpdateProcThreadAttribute(lpAttributeList,
                                                      0,
                                                      PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                                      hPC,
                                                      sizeof(HPCON),
                                                      nullptr,
                                                      nullptr))
                    {
                        return true;
                    }
                    else return faux;
                };

                HANDLE m_pipe_r{ INVALID_HANDLE_VALUE };
                HANDLE m_pipe_w{ INVALID_HANDLE_VALUE };
                STARTUPINFOEX       start_info{ sizeof(STARTUPINFOEX) };
                PROCESS_INFORMATION procs_info{};
                std::vector<char>   attrs_buff;

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

                    socket.set(m_pipe_r, m_pipe_w, true);
                    alive = true;
                }
                else log("cons: process creation error ", GetLastError());

            #elif defined(__linux__) || defined(__APPLE__)

                auto fdm = ::posix_openpt(O_RDWR | O_NOCTTY); // get master pty
                auto rc1 = ::grantpt     (fdm);          // grant master pty file access
                auto rc2 = ::unlockpt    (fdm);          // unlock master pty
                auto fds = ::open(ptsname(fdm), O_RDWR); // open slave pty via string ptsname(fdm)

                pid = ::fork();
                if (pid == 0) // Child branch
                {
                    ::close(fdm);

                    auto rc = ::setsid(); // Make the current process a new session leader, return process group id

                    // In order to receive WINCH signal make fds the controlling
                    // terminal of the current process.
                    // Current process must be a session leader (::setsid()) and not have
                    // a controlling terminal already.
                    // arg = 0: 1 - to stole fds from another process,
                    // it doesn't matter here
                    //if (::ioctl(fds, TIOCSCTTY, 0) == -1)
                    if (::ioctl(fds, TIOCSCTTY, 0) == -1)
                        log("cons: assign controlling terminal error ", errno);

                    struct winsize wsize{
                        static_cast<unsigned short>(winsz.y),
                        static_cast<unsigned short>(winsz.x) };

                    if (::ioctl(fds, TIOCSWINSZ, &wsize) == -1) // Preset slave tty size
                        log("cons: ioctl set winsize error ", errno);

                    ::signal(SIGINT,  SIG_DFL); // Reset control signals to the default.
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

                    ::execvp(argv.front(), argv.data());
                    os::exit(1, "cons: exec error ", errno);
                }

                // Parent branch
                ::close(fds);

                socket.set(fdm, true);
                alive = true;

            #endif

            std_input = std::thread([&] { read_socket_thread(); });
        }

        void read_socket_thread ()
        {
            text content;
            qiew data;

            //log("cons: read_socket_thread started");

            while (alive && (data = socket.recv()))
            {
                content += data;
                auto shadow = ansi::purify(content);
                receiver(shadow);

                content.erase(0, shadow.size()); // Delete processed data.
            }
            //log("cons: read_socket_thread ended");
        }
        void close()
        {
            alive = faux;

            #if defined(_WIN32)

                ClosePseudoConsole(hPC);
                socket.shut();
                CloseHandle(hProcess);
                CloseHandle(hThread);

            #elif defined(__linux__) || defined(__APPLE__)

                socket.shut();
                ::kill(pid, SIGKILL);
                ::waitpid (pid, 0, 0); // Wait for the child to avoid zombies.

            #endif

            if (std_input.joinable())
            {
                log("cons: input thread joining");
                std_input.join();
            }
            log("cons: input thread joined");
        }
        void resize(twod newsize)
        {
            if (consize(newsize))
            {
                #if defined(_WIN32)

                    COORD size;
                    size.X = newsize.x;
                    size.Y = newsize.y;
                    auto hr = ResizePseudoConsole(hPC, size);

                #elif defined(__linux__) || defined(__APPLE__)

                    struct winsize winsz;
                    winsz.ws_col = newsize.x;
                    winsz.ws_row = newsize.y;
                    ::ioctl(socket.get(), TIOCSWINSZ, &winsz);

                #endif
            }
        }
        void write(view data)
        {
            //if (alive) socket.send(data);
            socket.send(data);
        }
    };
}

#endif // NETXS_SYSTEM_HPP