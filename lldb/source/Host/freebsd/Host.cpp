//===-- source/Host/freebsd/Host.cpp ------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// C Includes
#include <stdio.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/proc.h>

#include <sys/ptrace.h>
#include <sys/exec.h>
#include <machine/elf.h>

#include <spawn.h>

// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/Core/Error.h"
#include "lldb/Host/Endian.h"
#include "lldb/Host/Host.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/DataExtractor.h"
#include "lldb/Core/StreamFile.h"
#include "lldb/Core/StreamString.h"
#include "lldb/Core/Log.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/Platform.h"

#include "lldb/Core/DataBufferHeap.h"
#include "lldb/Core/DataExtractor.h"
#include "lldb/Utility/CleanUp.h"

#include "llvm/Support/Host.h"


extern "C" {
    extern char **environ;
}

using namespace lldb;
using namespace lldb_private;

class FreeBSDThread
{
public:
    FreeBSDThread(const char *thread_name)
    {
        Host::SetThreadName (LLDB_INVALID_PROCESS_ID, LLDB_INVALID_THREAD_ID, thread_name);
    }
    static void PThreadDestructor (void *v)
    {
        delete (FreeBSDThread*)v;
    }
};

static pthread_once_t g_thread_create_once = PTHREAD_ONCE_INIT;
static pthread_key_t g_thread_create_key = 0;

static void
InitThreadCreated()
{
    ::pthread_key_create (&g_thread_create_key, FreeBSDThread::PThreadDestructor);
}

void
Host::ThreadCreated (const char *thread_name)
{
    ::pthread_once (&g_thread_create_once, InitThreadCreated);
    if (g_thread_create_key)
    {
        ::pthread_setspecific (g_thread_create_key, new FreeBSDThread(thread_name));
    }

    Host::SetShortThreadName (LLDB_INVALID_PROCESS_ID, LLDB_INVALID_THREAD_ID, thread_name, 16);
}

std::string
Host::GetThreadName (lldb::pid_t pid, lldb::tid_t tid)
{
    std::string thread_name;
    return thread_name;
}

void
Host::Backtrace (Stream &strm, uint32_t max_frames)
{
    char backtrace_path[] = "/tmp/lldb-backtrace-tmp-XXXXXX";
    int backtrace_fd = ::mkstemp (backtrace_path);
    if (backtrace_fd != -1)
    {
        std::vector<void *> frame_buffer (max_frames, NULL);
        int count = ::backtrace (&frame_buffer[0], frame_buffer.size());
        ::backtrace_symbols_fd (&frame_buffer[0], count, backtrace_fd);

        const off_t buffer_size = ::lseek(backtrace_fd, 0, SEEK_CUR);

        if (::lseek(backtrace_fd, 0, SEEK_SET) == 0)
        {
            char *buffer = (char *)::malloc (buffer_size);
            if (buffer)
            {
                ssize_t bytes_read = ::read (backtrace_fd, buffer, buffer_size);
                if (bytes_read > 0)
                    strm.Write(buffer, bytes_read);
                ::free (buffer);
            }
        }
        ::close (backtrace_fd);
        ::unlink (backtrace_path);
    }
}

size_t
Host::GetEnvironment (StringList &env)
{
    char *v;
    char **var = environ;
    for (; var != NULL && *var != NULL; ++var)
    {
        v = strchr(*var, (int)'-');
        if (v == NULL)
            continue;
        env.AppendString(v);
    }
    return env.GetSize();
}

bool
Host::GetOSVersion(uint32_t &major,
                   uint32_t &minor,
                   uint32_t &update)
{
    struct utsname un;

    ::memset(&un, 0, sizeof(utsname));
    if (uname(&un) < 0)
        return false;

    int status = sscanf(un.release, "%u.%u", &major, &minor);
    return status == 2;
}

// The posix_spawn() and posix_spawnp() functions first appeared in FreeBSD 8.0.
static Error
LaunchProcessPosixSpawn (const char *exe_path, ProcessLaunchInfo &launch_info, ::pid_t &pid)
{
    Error error;
    LogSP log(lldb_private::GetLogIfAllCategoriesSet (LIBLLDB_LOG_HOST | LIBLLDB_LOG_PROCESS));

    assert(exe_path);
    assert(!launch_info.GetFlags().Test (eLaunchFlagDebug));

    posix_spawnattr_t attr;

    error.SetError( ::posix_spawnattr_init (&attr), eErrorTypePOSIX);
    error.LogIfError(log.get(), "::posix_spawnattr_init ( &attr )");
    if (error.Fail())
        return error;

    // Make a quick class that will cleanup the posix spawn attributes in case
    // we return in the middle of this function.
    lldb_utility::CleanUp <posix_spawnattr_t *, int> posix_spawnattr_cleanup(&attr, posix_spawnattr_destroy);

    sigset_t no_signals;
    sigset_t all_signals;
    sigemptyset (&no_signals);
    sigfillset (&all_signals);
    ::posix_spawnattr_setsigmask(&attr, &all_signals);
    ::posix_spawnattr_setsigdefault(&attr, &no_signals);

    short flags = POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK;

    error.SetError( ::posix_spawnattr_setflags (&attr, flags), eErrorTypePOSIX);
    error.LogIfError(log.get(), "::posix_spawnattr_setflags ( &attr, flags=0x%8.8x )", flags);
    if (error.Fail())
        return error;

    const size_t num_file_actions = launch_info.GetNumFileActions ();
    posix_spawn_file_actions_t file_actions, *file_action_ptr = NULL;
    // Make a quick class that will cleanup the posix spawn attributes in case
    // we return in the middle of this function.
    lldb_utility::CleanUp <posix_spawn_file_actions_t *, int>
        posix_spawn_file_actions_cleanup (file_action_ptr, NULL, posix_spawn_file_actions_destroy);

    if (num_file_actions > 0)
    {
        error.SetError( ::posix_spawn_file_actions_init (&file_actions), eErrorTypePOSIX);
        error.LogIfError(log.get(), "::posix_spawn_file_actions_init ( &file_actions )");
        if (error.Fail())
            return error;

        file_action_ptr = &file_actions;
        posix_spawn_file_actions_cleanup.set(file_action_ptr);

        for (size_t i = 0; i < num_file_actions; ++i)
        {
            const ProcessLaunchInfo::FileAction *launch_file_action = launch_info.GetFileActionAtIndex(i);
            if (launch_file_action &&
                !ProcessLaunchInfo::FileAction::AddPosixSpawnFileAction (&file_actions,
                                                                         launch_file_action,
                                                                         log.get(),
                                                                         error))
                return error;
        }
    }

    // Change working directory if neccessary.
    char current_dir[PATH_MAX];
    current_dir[0] = '\0';

    const char *working_dir = launch_info.GetWorkingDirectory();
    if (working_dir != NULL)
    {
        if (::getcwd(current_dir, sizeof(current_dir)) == NULL)
        {
            error.SetError(errno, eErrorTypePOSIX);
            error.LogIfError(log.get(), "unable to save the current directory");
            return error;
        }

        if (::chdir(working_dir) == -1)
        {
            error.SetError(errno, eErrorTypePOSIX);
            error.LogIfError(log.get(), "unable to change working directory to %s", working_dir);
            return error;
        }
    }

    const char *tmp_argv[2];
    char * const *argv = (char * const*)launch_info.GetArguments().GetConstArgumentVector();
    char * const *envp = (char * const*)launch_info.GetEnvironmentEntries().GetConstArgumentVector();

    // Prepare minimal argument list if we didn't get it from the launch_info structure.
    // We must pass argv into posix_spawnp and it must contain at least two items -
    // pointer to an executable and NULL.
    if (argv == NULL)
    {
        tmp_argv[0] = exe_path;
        tmp_argv[1] = NULL;
        argv = (char * const*)tmp_argv;
    }

    error.SetError (::posix_spawnp (&pid,
                                    exe_path,
                                    (num_file_actions > 0) ? &file_actions : NULL,
                                    &attr,
                                    argv,
                                    envp),
                    eErrorTypePOSIX);

    error.LogIfError(log.get(), "::posix_spawnp ( pid => %i, path = '%s', file_actions = %p, attr = %p, argv = %p, envp = %p )",
                     pid, exe_path, file_action_ptr, &attr, argv, envp);

    // Change back the current directory.
    // NOTE: do not override previously established error from posix_spawnp.
    if (working_dir != NULL && ::chdir(current_dir) == -1 && error.Success())
    {
        error.SetError(errno, eErrorTypePOSIX);
        error.LogIfError(log.get(), "unable to change current directory back to %s",
                         current_dir);
    }

    return error;
}


Error
Host::LaunchProcess (ProcessLaunchInfo &launch_info)
{
    Error error;
    char exe_path[PATH_MAX];

    PlatformSP host_platform_sp (Platform::GetDefaultPlatform ());

    const ArchSpec &arch_spec = launch_info.GetArchitecture();

    FileSpec exe_spec(launch_info.GetExecutableFile());

    FileSpec::FileType file_type = exe_spec.GetFileType();
    if (file_type != FileSpec::eFileTypeRegular)
    {
        lldb::ModuleSP exe_module_sp;
        error = host_platform_sp->ResolveExecutable (exe_spec,
                                                     arch_spec,
                                                     exe_module_sp,
                                                     NULL);

        if (error.Fail())
    return error;

        if (exe_module_sp)
            exe_spec = exe_module_sp->GetFileSpec();
}

    if (exe_spec.Exists())
    {
        exe_spec.GetPath (exe_path, sizeof(exe_path));
    }
    else
    {
        launch_info.GetExecutableFile().GetPath (exe_path, sizeof(exe_path));
        error.SetErrorStringWithFormat ("executable doesn't exist: '%s'", exe_path);
        return error;
    }

    assert(!launch_info.GetFlags().Test (eLaunchFlagLaunchInTTY));

    ::pid_t pid = LLDB_INVALID_PROCESS_ID;

    error = LaunchProcessPosixSpawn(exe_path, launch_info, pid);

    if (pid != LLDB_INVALID_PROCESS_ID)
    {
        // If all went well, then set the process ID into the launch info
        launch_info.SetProcessID(pid);

        // Make sure we reap any processes we spawn or we will have zombies.
        if (!launch_info.MonitorProcess())
        {
            const bool monitor_signals = false;
            StartMonitoringChildProcess (Process::SetProcessExitStatus,
                                         NULL,
                                         pid,
                                         monitor_signals);
        }
    }
    else
    {
        // Invalid process ID, something didn't go well
        if (error.Success())
            error.SetErrorString ("process launch failed for unknown reasons");
    }
    return error;
}

bool
Host::GetOSBuildString (std::string &s)
{
    int mib[2] = { CTL_KERN, KERN_OSREV };
    char osrev_str[12];
    uint32_t osrev = 0;
    size_t osrev_len = sizeof(osrev);

    if (::sysctl (mib, 2, &osrev, &osrev_len, NULL, 0) == 0)
    {
        ::snprintf(osrev_str, sizeof(osrev_str), "%-8.8u", osrev);
        s.assign (osrev_str);
        return true;
    }

    s.clear();
    return false;
}

bool
Host::GetOSKernelDescription (std::string &s)
{
    struct utsname un;

    ::memset(&un, 0, sizeof(utsname));
    s.clear();

    if (uname(&un) < 0)
    return false;

    s.assign (un.version);

    return true;
}

static bool
GetFreeBSDProcessArgs (const ProcessInstanceInfoMatch *match_info_ptr,
                      ProcessInstanceInfo &process_info)
{
    if (process_info.ProcessIDIsValid())
    {
        int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ARGS, (int)process_info.GetProcessID() };

        char arg_data[8192];
        size_t arg_data_size = sizeof(arg_data);
        if (::sysctl (mib, 4, arg_data, &arg_data_size , NULL, 0) == 0)
        {
            DataExtractor data (arg_data, arg_data_size, lldb::endian::InlHostByteOrder(), sizeof(void *));
            lldb::offset_t offset = 0;
            const char *cstr;

            cstr = data.GetCStr (&offset);
            if (cstr)
            {
                process_info.GetExecutableFile().SetFile(cstr, false);

                if (!(match_info_ptr == NULL ||
                    NameMatches (process_info.GetExecutableFile().GetFilename().GetCString(),
                                 match_info_ptr->GetNameMatchType(),
                                 match_info_ptr->GetProcessInfo().GetName())))
                    return false;

                Args &proc_args = process_info.GetArguments();
                while (1)
                {
                    const uint8_t *p = data.PeekData(offset, 1);
                    while ((p != NULL) && (*p == '\0') && offset < arg_data_size)
                    {
                        ++offset;
                        p = data.PeekData(offset, 1);
                    }
                    if (p == NULL || offset >= arg_data_size)
                        return true;

                    cstr = data.GetCStr(&offset);
                    if (cstr)
                        proc_args.AppendArgument(cstr);
                    else
                        return true;
                }
            }
        }
    }
    return false;
}

static bool
GetFreeBSDProcessCPUType (ProcessInstanceInfo &process_info)
{
    if (process_info.ProcessIDIsValid())
    {
        process_info.GetArchitecture() = Host::GetArchitecture (Host::eSystemDefaultArchitecture);
        return true;
    }
    process_info.GetArchitecture().Clear();
    return false;
}

static bool
GetFreeBSDProcessUserAndGroup(ProcessInstanceInfo &process_info)
{
    struct kinfo_proc proc_kinfo;
    size_t proc_kinfo_size;

    if (process_info.ProcessIDIsValid())
    {
        int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID,
            (int)process_info.GetProcessID() };
        proc_kinfo_size = sizeof(struct kinfo_proc);

        if (::sysctl (mib, 4, &proc_kinfo, &proc_kinfo_size, NULL, 0) == 0)
        {
            if (proc_kinfo_size > 0)
            {
                process_info.SetParentProcessID (proc_kinfo.ki_ppid);
                process_info.SetUserID (proc_kinfo.ki_ruid);
                process_info.SetGroupID (proc_kinfo.ki_rgid);
                process_info.SetEffectiveUserID (proc_kinfo.ki_uid);
                if (proc_kinfo.ki_ngroups > 0)
                    process_info.SetEffectiveGroupID (proc_kinfo.ki_groups[0]);
                else
                    process_info.SetEffectiveGroupID (UINT32_MAX);
                return true;
            }
        }
    }
    process_info.SetParentProcessID (LLDB_INVALID_PROCESS_ID);
    process_info.SetUserID (UINT32_MAX);
    process_info.SetGroupID (UINT32_MAX);
    process_info.SetEffectiveUserID (UINT32_MAX);
    process_info.SetEffectiveGroupID (UINT32_MAX);
    return false;
}

uint32_t
Host::FindProcesses (const ProcessInstanceInfoMatch &match_info, ProcessInstanceInfoList &process_infos)
{
    std::vector<struct kinfo_proc> kinfos;

    int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };

    size_t pid_data_size = 0;
    if (::sysctl (mib, 3, NULL, &pid_data_size, NULL, 0) != 0)
        return 0;

    // Add a few extra in case a few more show up
    const size_t estimated_pid_count = (pid_data_size / sizeof(struct kinfo_proc)) + 10;

    kinfos.resize (estimated_pid_count);
    pid_data_size = kinfos.size() * sizeof(struct kinfo_proc);

    if (::sysctl (mib, 3, &kinfos[0], &pid_data_size, NULL, 0) != 0)
        return 0;

    const size_t actual_pid_count = (pid_data_size / sizeof(struct kinfo_proc));

    bool all_users = match_info.GetMatchAllUsers();
    const lldb::pid_t our_pid = getpid();
    const uid_t our_uid = getuid();
    for (int i = 0; i < actual_pid_count; i++)
    {
        const struct kinfo_proc &kinfo = kinfos[i];
        const bool kinfo_user_matches = (all_users ||
                                         (kinfo.ki_ruid == our_uid) ||
                                         // Special case, if lldb is being run as root we can attach to anything.
                                         (our_uid == 0)
                                         );

        if (kinfo_user_matches == false      || // Make sure the user is acceptable
            kinfo.ki_pid == our_pid          || // Skip this process
            kinfo.ki_pid == 0                || // Skip kernel (kernel pid is zero)
            kinfo.ki_stat == SZOMB    || // Zombies are bad, they like brains...
            kinfo.ki_flag & P_TRACED  || // Being debugged?
            kinfo.ki_flag & P_WEXIT)     // Working on exiting
            continue;

        // Every thread is a process in FreeBSD, but all the threads of a single process
        // have the same pid. Do not store the process info in the result list if a process
        // with given identifier is already registered there.
        bool already_registered = false;
        for (uint32_t pi = 0;
             !already_registered &&
             (const int)kinfo.ki_numthreads > 1 &&
             pi < (const uint32_t)process_infos.GetSize(); pi++)
            already_registered = (process_infos.GetProcessIDAtIndex(pi) == (uint32_t)kinfo.ki_pid);

        if (already_registered)
            continue;

        ProcessInstanceInfo process_info;
        process_info.SetProcessID (kinfo.ki_pid);
        process_info.SetParentProcessID (kinfo.ki_ppid);
        process_info.SetUserID (kinfo.ki_ruid);
        process_info.SetGroupID (kinfo.ki_rgid);
        process_info.SetEffectiveUserID (kinfo.ki_svuid);
        process_info.SetEffectiveGroupID (kinfo.ki_svgid);

        // Make sure our info matches before we go fetch the name and cpu type
        if (match_info.Matches (process_info) &&
            GetFreeBSDProcessArgs (&match_info, process_info))
        {
            GetFreeBSDProcessCPUType (process_info);
            if (match_info.Matches (process_info))
                process_infos.Append (process_info);
        }
    }

    return process_infos.GetSize();
}

bool
Host::GetProcessInfo (lldb::pid_t pid, ProcessInstanceInfo &process_info)
{
    process_info.SetProcessID(pid);

    if (GetFreeBSDProcessArgs(NULL, process_info))
    {
        // should use libprocstat instead of going right into sysctl?
        GetFreeBSDProcessCPUType(process_info);
        GetFreeBSDProcessUserAndGroup(process_info);
        return true;
    }

    process_info.Clear();
    return false;
}

lldb::DataBufferSP
Host::GetAuxvData(lldb_private::Process *process)
{
   int mib[2] = { CTL_KERN, KERN_PS_STRINGS };
   void *ps_strings_addr, *auxv_addr;
   size_t ps_strings_size = sizeof(void *);
   Elf_Auxinfo aux_info[AT_COUNT];
   struct ps_strings ps_strings;
   struct ptrace_io_desc pid;
   DataBufferSP buf_sp;
   std::unique_ptr<DataBufferHeap> buf_ap(new DataBufferHeap(1024, 0));

   if (::sysctl(mib, 2, &ps_strings_addr, &ps_strings_size, NULL, 0) == 0) {
           pid.piod_op = PIOD_READ_D;
           pid.piod_addr = &ps_strings;
           pid.piod_offs = ps_strings_addr;
           pid.piod_len = sizeof(ps_strings);
           if (::ptrace(PT_IO, process->GetID(), (caddr_t)&pid, 0)) {
                   perror("failed to fetch ps_strings");
                   buf_ap.release();
                   goto done;
           }

           auxv_addr = ps_strings.ps_envstr + ps_strings.ps_nenvstr + 1;

           pid.piod_addr = aux_info;
           pid.piod_offs = auxv_addr;
           pid.piod_len = sizeof(aux_info);
           if (::ptrace(PT_IO, process->GetID(), (caddr_t)&pid, 0)) {
                   perror("failed to fetch aux_info");
                   buf_ap.release();
                   goto done;
           }
           memcpy(buf_ap->GetBytes(), aux_info, pid.piod_len);
           buf_sp.reset(buf_ap.release());
   } else {
           perror("sysctl failed on ps_strings");
   }

   done:
   return buf_sp;
}
