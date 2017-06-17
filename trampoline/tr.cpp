#include "arg_types.h"
#include "slab.h"
#include <stdio.h>
#include <unistd.h>
#include <cstdio>
#include <memory>
#include <cassert>
#include <stdlib.h>


template <typename T>
struct trampoline
{
    template <typename F>
    trampoline(F func)
    {}

    T* get() const;
};

slab slb;
template <typename T, typename ... Args>
struct trampoline<T (Args ...)>
{
    template <typename F>
    trampoline(F func)
        :func_obj(new F(std::move(func))), deleter(my_deleter<F>)
    {
        args_types<Args ...> args;
        int INTEGER_ARGS = args.INTEGER_CLASS;
        int SSE_ARGS = args.SSE_CLASS;

        code = slb.malloc();
        //code = mmap(nullptr, 4096, PROT_EXEC | PROT_READ |\
                    PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char* pcode = (char*)code;

        if (INTEGER_ARGS < 6)
        {
            switch (INTEGER_ARGS)
            {
            case 5:
            {
                //4D89C1            mov r9,r8
                *pcode++ = 0x4d;
                *pcode++ = 0x89;
                *pcode++ = 0xc1;
                INTEGER_ARGS--;
            }
            case 4:
            {
                //4989C8            mov r8,rcx
                *pcode++ = 0x49;
                *pcode++ = 0x89;
                *pcode++ = 0xc8;
                INTEGER_ARGS--;
            }
            case 3:
            {
                //4889D1            mov rcx,rdx
                *pcode++ = 0x48;
                *pcode++ = 0x89;
                *pcode++ = 0xd1;
                INTEGER_ARGS--;
            }
            case 2:
            {
                //4889F2            mov rdx,rsi
                *pcode++ = 0x48;
                *pcode++ = 0x89;
                *pcode++ = 0xf2;
                INTEGER_ARGS--;
            }
            case 1:
            {
                //4889FE            mov rsi, rdi
                *pcode++ = 0x48;
                *pcode++ = 0x89;
                *pcode++ = 0xfe;
            }
            }
            // 48BF             mov rdi, imm
            *pcode++ = 0x48;
            *pcode++ = 0xbf;
            *(void**)pcode = func_obj;
            pcode += 8;
            // 48B8             mov rax, imm
            *pcode++ = 0x48;
            *pcode++ = 0xb8;
            *(void**)pcode = (void*)&do_call<F>;
            pcode += 8;
            // FFE0             jmp rax
            *pcode++ = 0xFF;
            *pcode++ = 0xE0;
        }
        else
        {
            /*
            // return address to r11
            //415B              pop r11
            *pcode++ = 0x41;
            *pcode++ = 0x5B;
            // free place on stack
            //50                push rax
            *pcode++ = 0x50;
            */

            //4C8B1C24          mov r11,[rsp]
            *pcode++ = 0x4c;
            *pcode++ = 0x8b;
            *pcode++ = 0x1c;
            *pcode++ = 0x24;

            // 6th arg on stack
            //4151              push r9
            *pcode++ = 0x41;
            *pcode++ = 0x51;
            //4D89C1            mov r9,r8
            *pcode++ = 0x4d;
            *pcode++ = 0x89;
            *pcode++ = 0xc1;
            //4989C8            mov r8,rcx
            *pcode++ = 0x49;
            *pcode++ = 0x89;
            *pcode++ = 0xc8;
            //4889D1            mov rcx,rdx
            *pcode++ = 0x48;
            *pcode++ = 0x89;
            *pcode++ = 0xd1;
            //4889F2            mov rdx,rsi
            *pcode++ = 0x48;
            *pcode++ = 0x89;
            *pcode++ = 0xf2;
            //4889FE            mov rsi, rdi
            *pcode++ = 0x48;
            *pcode++ = 0x89;
            *pcode++ = 0xfe;

            // 8 SSE args go to xmm0 - xmm7. Others go on stack
            int sz = 8 * (INTEGER_ARGS - 6 + std::max(SSE_ARGS - 8, 0));

            // rax on top of stack
            //4889E0            mov rax,rsp
            *pcode++ = 0x48;
            *pcode++ = 0x89;
            *pcode++ = 0xe0;

            // rax on first-1 arg on stack
            //4805            add rax, ...
            *pcode++ = 0x48;
            *pcode++ = 0x05;
            *(int32_t*)pcode = sz + 8;
            pcode += 4;
            // rsp on place for arg
            //4881C4            add rsp, ...
            *pcode++ = 0x48;
            *pcode++ = 0x81;
            *pcode++ = 0xc4;
            *(int32_t*)pcode = 8;
            pcode += 4;

            char* lbl = pcode;
            // if no more args on stack then goto lbl2, else shift all args by 1 place
            //4839E0            cmp rax,rsp
            *pcode++ = 0x48;
            *pcode++ = 0x39;
            *pcode++ = 0xe0;
            //74                je
            *pcode++ = 0x74;

            char* lbl2 = pcode;
            pcode++;


            // shift all args by 1 place
                // rsp from free place to arg for shift
                //4881C4            add rsp, ...
                *pcode++ = 0x48;
                *pcode++ = 0x81;
                *pcode++ = 0xc4;
                *(int32_t*)pcode = 8;
                pcode += 4;
                // save arg for shift in rdi
                //488B3C24          mov rdi,[rsp]
                *pcode++ = 0x48;
                *pcode++ = 0x8b;
                *pcode++ = 0x3c;
                *pcode++ = 0x24;
                // push saved arg to free place
                //48897C24F8        mov [rsp-0x8],rdi
                *pcode++ = 0x48;
                *pcode++ = 0x89;
                *pcode++ = 0x7c;
                *pcode++ = 0x24;
                *pcode++ = 0xf8;
                // do that in loop
                //EB                jmp
                *pcode++ = 0xeb;

                *pcode = lbl - pcode - 1;
                pcode++;

            *lbl2 = pcode - lbl2 - 1;

            // return address (saved in r11) on bottom of stack
            //4C891C24          mov [rsp],r11
            *pcode++ = 0x4c;
            *pcode++ = 0x89;
            *pcode++ = 0x1c;
            *pcode++ = 0x24;

            // rsp on 1st arg
            //4881EC            sub rsp,...
            *pcode++ = 0x48;
            *pcode++ = 0x81;
            *pcode++ = 0xec;
            *(int32_t*)pcode = sz + 8;
            pcode += 4;

            // 48BF             mov rdi, imm
            *pcode++ = 0x48;
            *pcode++ = 0xbf;
            *(void**)pcode = func_obj;
            pcode += 8;

            //48B8              mov rax, imm
            *pcode++ = 0x48;
            *pcode++ = 0xb8;
            *(void**)pcode = (void*)&do_call<F>;
            pcode += 8;

            // func call
            //FFD0            call rax
            *pcode++ = 0xFF;
            *pcode++ = 0xd0;

            // remove 6th arg from stack
            //4159              pop r9
            *pcode++ = 0x41;
            *pcode++ = 0x59;

            /*
            // rax on ret addr
            //4889E0            mov rax,rsp
            *pcode++ = 0x48;
            *pcode++ = 0x89;
            *pcode++ = 0xe0;
            //4881C0            add rax, ...
            *pcode++ = 0x48;
            *pcode++ = 0x81;
            *pcode++ = 0xc0;
            *(int32_t*)pcode = sz;
            pcode += 4;


            // save ret addr to r11
            //4C8B18            mov r11,[rax]
            *pcode++ = 0x4c;
            *pcode++ = 0x8b;
            *pcode++ = 0x18;
            */

            //4C8B9C24const   mov r11,[rsp+const]
            *pcode++ = 0x4c;
            *pcode++ = 0x8b;
            *pcode++ = 0x9c;
            *pcode++ = 0x24;
            *(int32_t*)pcode = sz;
            pcode += 4;


            /*
            // ret addr on top of stack, as it was before the call
            //4159              pop r9
            *pcode++ = 0x41;
            *pcode++ = 0x59;
            //4153              push r11
            *pcode++ = 0x41;
            *pcode++ = 0x53;
            */

            //4C891C24          mov [rsp],r11
            *pcode++ = 0x4c;
            *pcode++ = 0x89;
            *pcode++ = 0x1c;
            *pcode++ = 0x24;


            //C3                ret
            *pcode++ = 0xc3;
       }
    }

    template <typename F>
    static T do_call(void* obj, Args ... args)
    {
        return (*(F*)obj)(args ...);
    }

    T (*get() const)(Args ... args)
    {
        return (T (*)(Args ... args))code;
    }

    ~trampoline()
    {
        if (func_obj) {
            deleter(func_obj);
        }
        slb.free(code);
    }

    private:
    void* func_obj;
    void* code;
    void (*deleter)(void*);

    template <typename F>
    static void my_deleter(void* func_obj)
    {
        delete static_cast<F*>(func_obj);
    }
};

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
    for (int i = 0; i < 1000; i++)
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




