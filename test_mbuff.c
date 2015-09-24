
#include "glo.h"
#include "heap.h"
#include "mbuff.h"
#include "mbuff_queue.h"
#include <stdio.h>

void dump_queue(struct mb_queue *q)
{
    struct mbuff *ptr;

    ptr = q->head;
    while (ptr)
    {
	printf ("%ld\t", ptr->m_hdr.seq);
	ptr = ptr->m_next;
    }
    printf ("\n");
}

int main (void)
{
    struct mb_queue q;
    struct mbuff *ptr;

    init_heap();

    init_mbuff_queue(&q);
    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 1;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 0;
    mbuff_insert(&q, ptr);
    dump_queue(&q);


    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 7;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 5;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 4;
    mbuff_insert(&q, ptr);
    dump_queue(&q);


    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 1;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 0;
    mbuff_insert(&q, ptr);
    dump_queue(&q);


    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 7;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 5;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 4;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    ptr = alloc_mbuff();
    ptr->m_hdr.seq = 8;
    mbuff_insert(&q, ptr);
    dump_queue(&q);

    return 0;
}