#include <cstdint>
#include <string>
#include <time.h>

namespace phase0
{
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSeconds);
    static Timestamp now();
    std::string toString() const;
    int64_t microSeconds() const;

private:
    int64_t microSeconds_;
};
}  // namespace phase0
