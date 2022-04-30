#include "utils.h"

#include <cstring>
#include <sstream>
#include <system_error>
#include <vector>

#include "LogMarco.h"
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

bool FSUtil::Unlink(const std::string& filename, bool exist)
{
    if (!exist && __lstat(filename.c_str()))
    {
        return true;
    }
    return ::unlink(filename.c_str()) == 0;
}
/////////////////////////////////////////////////////////////////////////////

std::string StringUtil::Format(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    auto v = Formatv(fmt, ap);
    va_end(ap);
    return v;
}

std::string StringUtil::Formatv(const char* fmt, va_list ap)
{
    char* buf = nullptr;
    auto len = vasprintf(&buf, fmt, ap);
    if (len == -1)
    {
        return "";
    }
    std::string ret(buf, len);
    free(buf);
    return ret;
}

static const char uri_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char xdigit_chars[256] = {
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

//-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
std::string StringUtil::UrlEncode(const std::string& str, bool space_as_plus)
{
    static const char* hexdigits = "0123456789ABCDEF";
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c)
    {
        if (!CHAR_IS_UNRESERVED(*c))
        {
            if (!ss)
            {
                ss = new std::string;
                ss->reserve(str.size() * 1.2);
                ss->append(str.c_str(), c - str.c_str());
            }
            if (*c == ' ' && space_as_plus)
            {
                ss->append(1, '+');
            }
            else
            {
                ss->append(1, '%');
                ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                ss->append(1, hexdigits[*c & 0xf]);
            }
        }
        else if (ss)
        {
            ss->append(1, *c);
        }
    }
    if (!ss)
    {
        return str;
    }
    else
    {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus)
{
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c)
    {
        if (*c == '+' && space_as_plus)
        {
            if (!ss)
            {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, ' ');
        }
        else if (*c == '%' && (c + 2) < end && isxdigit(*(c + 1)) && isxdigit(*(c + 2)))
        {
            if (!ss)
            {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
            c += 2;
        }
        else if (ss)
        {
            ss->append(1, *c);
        }
    }
    if (!ss)
    {
        return str;
    }
    else
    {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::Trim(const std::string& str, const std::string& delimit)
{
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos)
    {
        return "";
    }
    auto end = str.find_last_not_of(delimit);
    return str.substr(begin, end - begin + 1);
}

std::string StringUtil::TrimLeft(const std::string& str, const std::string& delimit)
{
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos)
    {
        return "";
    }
    return str.substr(begin);
}

std::string StringUtil::TrimRight(const std::string& str, const std::string& delimit)
{
    auto end = str.find_last_not_of(delimit);
    if (end == std::string::npos)
    {
        return "";
    }
    return str.substr(0, end);
}

std::string StringUtil::WStringToString(const std::wstring& ws)
{
    std::string str_locale = setlocale(LC_ALL, "");
    const wchar_t* wch_src = ws.c_str();
    size_t n_dest_size = wcstombs(NULL, wch_src, 0) + 1;
    char* ch_dest = new char[n_dest_size];
    memset(ch_dest, 0, n_dest_size);
    wcstombs(ch_dest, wch_src, n_dest_size);
    std::string str_result = ch_dest;
    delete[] ch_dest;
    setlocale(LC_ALL, str_locale.c_str());
    return str_result;
}

std::wstring StringUtil::StringToWString(const std::string& s)
{
    std::string str_locale = setlocale(LC_ALL, "");
    const char* chSrc = s.c_str();
    size_t n_dest_size = mbstowcs(NULL, chSrc, 0) + 1;
    wchar_t* wch_dest = new wchar_t[n_dest_size];
    wmemset(wch_dest, 0, n_dest_size);
    mbstowcs(wch_dest, chSrc, n_dest_size);
    std::wstring wstr_result = wch_dest;
    delete[] wch_dest;
    setlocale(LC_ALL, str_locale.c_str());
    return wstr_result;
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

void SetThreadName(std::thread& thread, std::string name)
{
    pthread_setname_np(thread.native_handle(), name.c_str());
}

pid_t GetCurThreadId() { return syscall(SYS_gettid); }

uint64_t GetThreadId(std::thread& thread)
{
    // fuck hack
    auto id = thread.get_id();
    std::stringstream ss;
    ss << id;
    return std::stoull(ss.str());
}

int GetCurFiberId() { return phase0::Fiber::GetFiberId(); }

uint64_t GetElapsedMS()
{
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

std::string Time2Str(time_t ts, const std::string& format)
{
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

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