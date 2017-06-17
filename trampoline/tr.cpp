#include "tr.h"

int main()
{
    int x = 123;
    trampoline<int (int, int, int, int, char)> tr5([&](int a, int b, int c, int d, char e)
                                                    { return printf("%d %d %d %d %c %d\n",
                                                        a, b, c, d, e, x); });
    auto p = tr5.get();
    p(5, 6, 7, 8, 'a');

    x = 124;
    p(6, 7, 8, 9, 'b');


    trampoline<int (int, int)> tr2([&](int a, int b) { return printf("%d %d %d\n", a, b, x); });
    auto q = tr2.get();
    q(15,16);


    trampoline<int (int, int, char, int)> tr4([&](int a, int b, char c, int d)
                                                { return printf("%d %d %c %d %d\n",
                                                                a, b, c, d, x); });
    auto a4 = tr4.get();
    a4(6, 7, 'F', 30);

    /*trampoline<int (__int128)> tr([&](__int128 a) {return printf("%d", a);});
    auto a = tr.get();
    a((__int128)100);*/
    for (int i = 0; i < 10000; i++)
    {
    trampoline<int (int, int, int, float, char, int,\
                    int, int, int, float, char, int)>
                    tr6([&](int a, int b, int c, float d, char e, int f,\
                            int g, int h, int i, float j, char k, int l)
                    { return printf("%d %d %d %f %c %d %d %d %d %f %c %d %d\n",
                                     a, b, c, d, e, f, g, h, i, j, k, l, x); });
    auto qq = tr6.get();
    qq(i, 0, 1, (float)i / 2, static_cast<char>(i), 10,\
       i, 0, 1, (float)2 / i, static_cast<char>(i), 42);
    if (i % 100 == 0)
        printf("\n%d\n\n", i);
    }
    return 0;
}




