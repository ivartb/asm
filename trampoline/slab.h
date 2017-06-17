#ifndef SLAB_H
#define SLAB_H

struct slab
{
private:
    void** cur;
    const int TR_SIZE = 128;

public:
    void alloc_new_chunk();

    void* malloc();
    void free(void* ptr);

};

#endif // SLAB_H




