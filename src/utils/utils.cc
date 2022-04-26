#include "utils.h"

#include <cstring>
#include <sstream>
#include <system_error>
#include <vector>

#include "fiber.h"

namespace phase0
{
static int __mkdir(const char* dirname)
{
    if (access(dirname, F_OK) == 0)
    {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

static int __lstat(const char* file, struct stat* st = nullptr)
{
    struct stat lst;
    int ret = lstat(file, &lst);
    if (st)
    {
        *st = lst;
    }
    return ret;
}

bool FSUtil::Mkdir(const std::string& dirname)
{
    if (__lstat(dirname.c_str()) == 0)
    {
        return true;
    }
    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');
    do
    {
        for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/'))
        {
            *ptr = '\0';
            if (__mkdir(path) != 0)
            {
                break;
            }
        }
        if (ptr != nullptr)
        {
            break;
        }
        else if (__mkdir(path) != 0)
        {
            break;
        }
        free(path);
        return true;
    } while (0);

    free(path);
    return false;
}

std::string FSUtil::Dirname(const std::string& filename)
{
    if (filename.empty())
    {
        return ".";
    }
    auto pos = filename.rfind('/');
    if (pos == 0)
    {
        return "/";
    }
    else if (pos == std::string::npos)
    {
        return ".";
    }
    else
    {
        return filename.substr(0, pos);
    }
}

bool FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode)
{
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open())
    {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}

/////////////////////////////////////////////////////////////////////////////
// for log/thread/fiber

std::string GetCurThreadName()
{
    char* tname = new char[16];
    prctl(PR_GET_NAME, tname);
    return std::move(tname);
}

void SetCurThreadName(std::string name) { prctl(PR_SET_NAME, name.c_str()); }

pid_t GetCurThreadId() { return syscall(SYS_gettid); }

int GetCurFiberId() { return phase0::Fiber::GetFiberId(); }

/////////////////////////////////////////////////////////////////////////////
// for debug
static std::string demangle(const char* str)
{
    size_t size = 0;
    int status = 0;
    std::string rt;
    rt.resize(256);
    if (1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0]))
    {
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        if (v)
        {
            std::string result(v);
            free(v);
            return result;
        }
    }
    if (1 == sscanf(str, "%255s", &rt[0]))
    {
        return rt;
    }
    return str;
}

void Backtrace(std::vector<std::string>& bt, int size, int skip)
{
    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if (strings == NULL)
    {
        fprintf(stderr, "backtrace_synbols error\n");
        return;
    }

    for (size_t i = skip; i < s; ++i)
    {
        bt.push_back(demangle(strings[i]));
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix)
{
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i)
    {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

}  // namespace phase0