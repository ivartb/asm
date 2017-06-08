#ifndef SLAB_H
#define SLAB_H

#include <stdlib.h>
#include <sys/mman.h>

struct slab
{
private:
    void** cur;
    void* mem;
    const int TR_SIZE = 128;

public:
    slab()
    {
        mem = mmap(nullptr, 4096, PROT_EXEC | PROT_READ |
                         PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        cur = (void**) mem;
        void** link = 0;
        for (auto i = 0; i != 4096; i += TR_SIZE)
        {
            *(void**)(static_cast<char*>(mem) + i) = 0;
            if (link != 0)
                *link = static_cast<char*>(mem) + i;
            link = (void**)(static_cast<char*>(mem)+ i);
        }
    }

    ~slab()
    {
        munmap(mem, 4096);
    }

    void* malloc()
    {
        void* ans = cur;
        cur = (void**) cur;
        return ans;
    }

    void free(void* ptr)
    {
        *(void**) ptr = cur;
        cur = (void**) ptr;
    }
};

#endif // SLAB_H




