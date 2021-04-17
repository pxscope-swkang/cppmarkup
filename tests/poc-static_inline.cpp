#include <iostream>
#include <string>
using namespace std;

static std::string gc;

struct poca {
    poca() { gc = "A", cout << "hell, world! " << gc << "\n"; }
};

struct pocb {
    pocb() { gc = "B", cout << "hell, world! " << gc << "\n"; }
};

struct upper {
    int a, b;
};

struct lower {
    upper a;
    int c, d;
};

struct bottom {
    lower g;
    int r;
};

struct ANNONYMOUS {
    static inline poca a;
    static inline pocb b;
};

int main(void)
{
    bottom c = {
        .g = {
            .a = {
                .a = 1,
                .b = 2}}};

    return 0;
}