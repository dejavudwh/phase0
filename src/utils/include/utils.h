#pragma once

#include <cxxabi.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>
#include <execinfo.h>

namespace phase0
{
class FSUtil
{
public:
    static bool Mkdir(const std::string& dirname);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode);
    static std::string Dirname(const std::string& filename);
};

// for log/thread/fiber
std::string GetCurThreadName();
void SetCurThreadName(std::string name);
pid_t GetCurThreadId();

int GetCurFiberId();

// for debug
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

}  // namespace phase0