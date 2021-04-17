#include <iostream>
#include <string>
using namespace std;

static std::string gc;
static int i = 0;

struct poc {
    poc(const char *str) { std::cout << str << '\n'; }
};

struct outer {
    static inline poc _0{"1. outer"};

    struct inner {
        static inline poc _1{"2. inner"};
        static inline poc _2{"3. inner"};

        struct in_inner {
            static inline poc _3{"4. in_inner"};
        };

        static inline poc _4{"5. inner"};
    };

    static inline poc _1{"6. outer"};
};

int main(void)
{
    return 0;
}