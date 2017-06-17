#include <stdlib.h>
#include <sys/mman.h>
#include "slab.h"

namespace
{
    void** cur = nullptr;
    const int TR_SIZE = 128;

    void alloc_new_chunk()
    {
        void* mem = mmap(nullptr, 4096, PROT_EXEC | PROT_READ |
                         PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        cur = (void**) mem;
        if (mem != nullptr)
        {
            for (auto i = 0; i < 4096; i += TR_SIZE)
            {
                auto tmp = static_cast<char*>(mem) + i;
                *(void**)tmp = 0;
                if (i != 0)
                    *(void**)(tmp - TR_SIZE) = tmp;
            }
        }
    }
}

void* slab::malloc()
{
    if (cur == nullptr)
    {
        alloc_new_chunk();
        if (cur == nullptr)
            return nullptr;
    }
    void* ans = cur;
    cur = (void**)*cur;
    return ans;
}

void slab::free(void* ptr)
{
    *(void**) ptr = cur;
    cur = (void**) ptr;
}





