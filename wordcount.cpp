#include <emmintrin.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <chrono>

size_t add(__m128i store)
{
    __m128i tmp0, tmp1;
    uint32_t low, high;

    __asm__ volatile(
           "psadbw      %1, %0\n"
           "movd        %0, %2\n"
           "movhlps     %0, %0\n"
           "movd        %0, %3\n"
          :"=x"(tmp0), "=x"(tmp1), "=r"(low), "=r"(high)
          :"0"(_mm_set_epi32(0, 0, 0, 0)), "1"(store)
          :"memory"
                );

    return high + low;
}

size_t naive_count(std::string const& str, size_t sz)
{
    size_t ans = 0;
    bool prev_space = true;
    for (size_t i = 0; i != sz; i++)
    {
        if (prev_space && str[i] != ' ')
            ans++;
        prev_space = (str[i] == ' ');
    }
    return ans;
}


size_t count(std::string const& str, size_t sz)
{
    if (sz == 0)
        return 0;

    char const* s = str.c_str();
    size_t ans = 0;
    __m128i space_reg = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32,    \
                                     32, 32, 32, 32, 32, 32, 32, 32);
    size_t position = 0;
    bool is_space = false;

    if (sz < 64)
        return naive_count(str, sz);

    while (((reinterpret_cast<size_t>(s) + position) % 16 != 0))
    {
        char tmp = *(s + position);
        if (is_space && tmp != ' ')
            ans++;
        is_space = (tmp == ' ');
        position++;

    }

    if(*s != ' ')
        ans++;
    if (position != 0 && *(s + position) != ' ' && is_space)
        ans++;



    __m128i store = _mm_set_epi32(0, 0, 0, 0);
    size_t size = sz - (sz - position) % 16 - 16;

    __m128i cmp, shifted_cmp, tmp;

    __asm__ volatile(
          "movdqa      (%2), %1\n"
          "pcmpeqb     %1, %0\n"
          :"=x"(shifted_cmp), "=x"(tmp)
          :"r"(s + position), "0"(space_reg)
          :"memory"
          );

    for (size_t i = position; i < size; i += 16)
    {
        cmp = shifted_cmp;
        uint32_t mask;
        __m128i tmp0, tmp1, tmp2, tmp3;

        __asm__ volatile(
               "movdqa      (%7), %3\n"
               "pcmpeqb     %3, %0\n"
               "movdqa      %0, %6\n"
               "palignr     $1, %4, %6\n"
               "pandn       %4, %6\n"
               "psubsb      %6, %5\n"
               "paddusb     %5, %1\n"
               "pmovmskb    %1, %2\n"
              :"=x"(shifted_cmp), "=x"(store), "=r"(mask), "=x"(tmp0), "=x"(tmp1), \
                                                           "=x"(tmp2), "=x"(tmp3)
              :"r"(s + i + 16), "0"(space_reg), "1"(store), "4"(cmp),              \
                    "5"(_mm_set_epi32(0, 0, 0, 0))
              :"memory"
                    );

        if (mask != 0)
        {
            ans += add(store);
            store = _mm_set_epi32(0, 0, 0, 0);
        }
    }
    ans += add(store);

    position = size;

    if(*(s + position - 1) == ' ' && *(s + position) != ' ')
        ans--;

    is_space = (*(s + size - 1) == ' ');
    for (size_t i = position; i < sz; i++)
    {
        if (*(s + i) != ' ' && is_space)
            ans++;
        is_space = (*(s + i) == ' ');
    }

    return ans;
}

int main()
{
    std::string s = "This word counter works ! ";
    for (size_t i = 0; i != 15; i++)
        s += s;
    size_t sz = s.size();

    std::cout << naive_count(s, sz) << " words\n";

    auto begin = std::chrono::system_clock::now();
    for (size_t i = 0; i != 1000; i++)
        naive_count(s, sz);
    std::cout << "Naive word counter: " <<
         std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::system_clock::now() - begin).count() << " ms\n";

    std::cout << count(s, sz) << " words\n";

    begin = std::chrono::system_clock::now();
    for (size_t i = 0; i != 1000; i++)
        count(s, sz);
    std::cout << "Fast word counter: " <<
         std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::system_clock::now() - begin).count() << " ms\n";
    return 0;
}





