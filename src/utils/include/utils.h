#pragma once

#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h> 

namespace phase0
{
class FSUtil
{
public:
    static bool Mkdir(const std::string& dirname);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode);
    static std::string Dirname(const std::string& filename);
};

std::string GetCurThreadName();
void SetCurThreadName(std::string name);
pid_t GetCurThreadId();

int GetCurFiberId();

}  // namespace phase0