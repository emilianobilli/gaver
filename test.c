#include "heap.h"
#include "glo.h"
#include "mbuff.h"
#include "mbuff_queue.h"
#include <stdio.h>


int main(void)
{
    struct msg_queue msgq, other;
    struct msg *p_ptr;
    struct mbuff *m_ptr;
    size_t n;

    msgq.head = NULL;
    msgq.tail = NULL;
    other.head = NULL;
    other.tail = NULL;

    init_heap();

    n = alloc_mbuff_chain(&msgq, 20);

    while (msgq.head != NULL)
    {
	p_ptr = msg_dequeue(&msgq);
	m_ptr = p_ptr->p_mbuff;
	free_mbuff(m_ptr);
	free_msg(p_ptr);
    }
    n = alloc_mbuff_chain(&msgq, 20);
    n += alloc_mbuff_chain(&other, 10);

    msgqcat(&other, &msgq);

    
    printf("%d\n", heap_mem);
    printf("%d\n", n);

    while (other.head != NULL)
    {
	p_ptr = msg_dequeue(&other);
	printf("%x\n", p_ptr);
    }

    return 0;
}
