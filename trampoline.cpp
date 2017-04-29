#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>
#include <memory>

template <typename T>
struct trampoline
{
    template <typename F>
    trampoline(F func)
    {}

    T* get() const;
};

template <typename T, typename ... Args>
struct trampoline<T (Args ...)>
{
    template <typename F>
    trampoline(F func)
        :func_obj(new F(std::move(func))), deleter(my_deleter<F>)
    {
        code = mmap(nullptr, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char* pcode = (char*)code;

        //Do NOT work when args > regs
        /*
        //4151              push r9
        *pcode++ = 0x41;
        *pcode++ = 0x51;
        */
/*
        //4883EC08          sub rsp,byte +0x8
        *pcode++ = 0x48;
        *pcode++ = 0x83;
        *pcode++ = 0xec;
        *pcode++ = 0x08;
*/
   /*     //4C89C8            mov rax,r9
        *pcode++ = 0x4c;
        *pcode++ = 0x89;
        *pcode++ = 0xc8;


        //50                push rax
        *pcode++ = 0x50;

*/
        //58                pop rax
        *pcode++ = 0x58;
        //4151              push r9
        *pcode++ = 0x41;
        *pcode++ = 0x51;
        //50                push rax
        *pcode++ = 0x50;
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
        //4883C410          add rsp,byte +0x10
       /* *pcode++ = 0x48;
        *pcode++ = 0x83;
        *pcode++ = 0xc4;
        *pcode++ = 0x10;*/
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
        func_obj = nullptr;
        deleter = nullptr;
        code = nullptr;
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

/*
template <typename R, typename A0>
struct trampoline<R (A0)>
{
    template <typename F>
    trampoline(F const& func)
        : func_obj(new F(func))
        , caller(&do_call<F>)
    {
        code = mmap(nullptr, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char* pcode = (char*)code;
        //4889FE        mov rsi, rdi
        *pcode++ = 0x48;
        *pcode++ = 0x89;
        *pcode++ = 0xfe;
        // 48BF         mov rdi, imm
        *pcode++ = 0x48;
        *pcode++ = 0xbf;
        *(void**)pcode = func_obj;
        pcode += 8;
        // 48B8         mov rax, imm
        *pcode++ = 0x48;
        *pcode++ = 0xb8;
        *(void**)pcode = (void*)&do_call<F>;
        pcode += 8;
        // FFE0         jmp rax
        *pcode++ = 0xFF;
        *pcode++ = 0xE0;
    }

    template <typename F>
    static R do_call(void* obj, A0 a0)
    {
        return (*(F*)obj)(a0);
    }

    R (*get() const)(A0 a0)
    {
        return (R (*)(A0 a0))code;
    }

private:
    void* func_obj;
    void* code;
    R (*caller)(void* obj, A0 a0);
};*/

int main()
{
    int x = 123;
/*
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
*/

    trampoline<int (int, int, int, int, int, int, int)> tr6([&](int a, int b, int c, int d, int e,
                                                               int f, int g)
                                                 { return printf("%d %d %d %d %d %d %d %d\n",
                                                           a, b, c, d, e, x, f, g); });
    auto r = tr6.get();
    r(6, 7, 8, 9, 10, 15, 30);

    trampoline<int (int, int, int, int, int, int, int, char)> tr7([&](int a, int b, int c, int d, int e,
                                                               int f, int g, char ch)
                                                 { return printf("%d %d %d %d %d %d %d %d %c\n",
                                                           a, b, c, d, e, x, f, g, ch); });
    auto q = tr7.get();
    q(6, 7, 8, 9, 10, 15, 30, 'A');

}
//g++ -std=c++14 -Wall -Wextra -Werror trampoline.cpp -o tr







