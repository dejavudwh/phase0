#pragma once

#include <cxxabi.h>
#include <execinfo.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace phase0
{
class FSUtil
{
public:
    static bool Mkdir(const std::string& dirname);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode);
    static std::string Dirname(const std::string& filename);
    static bool Unlink(const std::string& filename, bool exist = false);
};

class StringUtil
{
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);
    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);
    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);
};

// for log/thread/fiber
std::string GetCurThreadName();
void SetCurThreadName(std::string name);
void SetThreadName(std::thread& thread, std::string name);
pid_t GetCurThreadId();
uint64_t GetThreadId(std::thread& thread);

int GetCurFiberId();

uint64_t GetElapsedMS();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

// for debug
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

}  // namespace phase0