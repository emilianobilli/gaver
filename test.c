#include "heap.h"
#include "glo.h"
#include "mbuff.h"
#include "mbuff_queue.h"
#include <stdio.h>


int main(void)
{
    struct pkt_queue pktq, other;
    struct pktbuff *p_ptr;
    struct mbuff *m_ptr;
    size_t n;

    pktq.head = NULL;
    pktq.tail = NULL;
    other.head = NULL;
    other.tail = NULL;

    n = alloc_mbuff_chain(&pktq, 20);

    while (pktq.head != NULL)
    {
	p_ptr = pktbuff_dequeue(&pktq);
	m_ptr = p_ptr->p_mbuff;
	free_mbuff(m_ptr);
	free_pkt(p_ptr);
    }
    n = alloc_mbuff_chain(&pktq, 20);
    n += alloc_mbuff_chain(&other, 10);

    pktbuffqcat(&other, &pktq);

    
    printf("%d\n", heap_mem);
    printf("%d\n", n);

    while (other.head != NULL)
    {
	p_ptr = pktbuff_dequeue(&other);
	printf("%x\n", p_ptr);
    }

    return 0;
}
