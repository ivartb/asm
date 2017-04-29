#include <vector>
#include <iostream>
#include <chrono>
#include <cstring>
#include <emmintrin.h>
#include <cassert>

bool is_aligned (size_t ptr, size_t alignment)
{
 return (ptr % alignment) == 0;
}

void memcpy_char(void* dst, void const* src, size_t N)
{
 for (size_t i = 0; i != N; i++)
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
}

void memcpy_asm(void* dst, void const* src, size_t N)
{
 /*
 size_t i = 0;
 while ((N > 0) && !is_aligned(reinterpret_cast<size_t>(dst) + i, 8))
 {
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
     i++;
     N--;
 }
 */

 size_t tmp;
 __asm__ volatile(
        "sub    $8, %2\n"
    "1:"
        //"add    $8, %2\n"
        "mov    (%0), %3\n"
        "mov    %3, (%1)\n"
        "add    $8, %0\n"
        "add    $8, %1\n"
        "sub    $8, %2\n"
        //"sub    $8, %2\n"
        "jnl    1b\n"
        "add    $8, %2\n"
        : "=r"(src), "=r"(dst), "=r"(N), "=r"(tmp)
        : "0"(reinterpret_cast<size_t>(src)),
          "1"(reinterpret_cast<size_t>(dst)), "2"(N)
        : "memory"
             );

 /*
 for (size_t i = 0; i != N; i++)
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
 */
}

void memcpy_asm_xmm(void* dst, void const* src, size_t N)
{
 size_t i = 0;
 while ((N > 0) && (!is_aligned(reinterpret_cast<size_t>(dst) + i, 16)))
 {
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
     i++;
     N--;
 }

 __m128i tmp;

 __asm__ volatile(
        "sub    $16, %2\n"
    "1:"
        //"add    $16, %2\n"
        "movdqu (%0), %3\n"
        "movdqa %3, (%1)\n"
        "add    $16, %0\n"
        "add    $16, %1\n"
        "sub    $16, %2\n"
        //"sub    $16, %2\n"
        "jnl    1b\n"
        "add    $16, %2\n"
        : "=r"(src), "=r"(dst), "=r"(N), "=x"(tmp)
        : "0"(reinterpret_cast<size_t>(src) + i),
          "1"(reinterpret_cast<size_t>(dst) + i), "2"(N)
        : "memory"
             );
 for (size_t i = 0; i != N; i++)
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
}

void memcpy_asm_xmm_nt(void* dst, void const* src, size_t N)
{
 size_t i = 0;
 while ((N > 0) && (!is_aligned(reinterpret_cast<size_t>(dst) + i, 16)))
 {
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
     i++;
     N--;
 }

 __m128i tmp;

 __asm__ volatile(
        "sub    $16, %2\n"
    "1:"
        //"add    $16, %2\n"
        "movdqu (%0), %3\n"
        "movntdq %3, (%1)\n"
        "add    $16, %0\n"
        "add    $16, %1\n"
        "sub    $16, %2\n"
        //"sub    $16, %2\n"
        "jnl    1b\n"
        "add    $16, %2\n"
	"sfence"
        : "=r"(src), "=r"(dst), "=r"(N), "=x"(tmp)
        : "0"(reinterpret_cast<size_t>(src) + i),
          "1"(reinterpret_cast<size_t>(dst) + i), "2"(N)
        : "memory"
             );

 for (size_t i = 0; i != N; i++)
     static_cast<char*>(dst)[i] = static_cast<char const*>(src)[i];
}

int main()
{
 size_t const N = 123456789;
 size_t const M = 10;

 std::vector<char> a(N), b(N);

 auto fst = b.data() + 3;
 auto snd = a.data() + 5;
 auto sz = N - 7;

 auto begin = std::chrono::system_clock::now();
 for (size_t i = 0; i != M; i++)
     memcpy(fst, snd, sz);
 std::cout << "standard memcpy: " <<
      std::chrono::duration_cast<std::chrono::milliseconds>
           (std::chrono::system_clock::now() - begin).count() << "ms\n";

 begin = std::chrono::system_clock::now();
 for (size_t i = 0; i != M; i++)
     memcpy_char(fst, snd, sz);
 std::cout << "8-bit memcpy: " <<
      std::chrono::duration_cast<std::chrono::milliseconds>
           (std::chrono::system_clock::now() - begin).count() << "ms\n";

 begin = std::chrono::system_clock::now();
 for (size_t i = 0; i != M; i++)
     memcpy_asm(fst, snd, sz);
 std::cout << "64-bit asm memcpy: " <<
      std::chrono::duration_cast<std::chrono::milliseconds>
           (std::chrono::system_clock::now() - begin).count() << "ms\n";

 begin = std::chrono::system_clock::now();
 for (size_t i = 0; i != M; i++)
     memcpy_asm_xmm(fst, snd, sz);
 std::cout << "128-bit asm memcpy: " <<
      std::chrono::duration_cast<std::chrono::milliseconds>
           (std::chrono::system_clock::now() - begin).count() << "ms\n";

 begin = std::chrono::system_clock::now();
 for (size_t i = 0; i != M; i++)
     memcpy_asm_xmm_nt(fst, snd, sz);
 std::cout << "128-bit non-temporary asm memcpy: " <<
      std::chrono::duration_cast<std::chrono::milliseconds>
           (std::chrono::system_clock::now() - begin).count() << "ms\n";

 return 0;
}
//g++ -std=c++14 -Wall -Wextra -Werror memcpy.cpp




