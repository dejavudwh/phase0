#include <iostream>
#include <vector>

using namespace std;

struct AnyType {
    template <typename T>
    operator T();
};

// template <typename T>
// consteval size_t CountMember(auto&&... Args) {
//     if constexpr (! requires { T{ Args... }; }) { // (1)
//         return sizeof...(Args) - 1;
//     } else {
//         return CountMember<T>(Args..., AnyType{}); // (2)
//     }
// }

// int main(int argc, char** argv) {
//     struct Test { int a; int b; int c; int d; };
//     printf("%zu\n", CountMember<Test>());
// }

template <typename T, typename = void, typename ...Ts>
struct CountMember {
    constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename ...Ts>
struct CountMember<T, std::void_t<decltype(T{Ts{}...})>, Ts...> {
    constexpr static size_t value = CountMember<T, void, Ts..., AnyType>::value;
};

int main(int argc, char** argv) {
    struct Test { int a; int b; int c; int d; };
    printf("%zu\n", CountMember<Test>::value);
}
