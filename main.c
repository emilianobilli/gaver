#include "glo.h"
#include "util.h"
#include "gaver.h"
#include "heap.h"
#include "output.h"
#include "mbuff.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>


int main(void)
{
    struct timespec ipt;	/* InterPacket Time */
    struct msg_queue q;
    struct sockaddr_in in, in2, in3;
    int ret, i;
    int *tret;
    u_int64_t seq = 0;
    struct msg *p;
    init_heap();
    itc_init();

    ipt.tv_sec  = 0;
    ipt.tv_nsec = pktime(MBPS_TOBPS(100), 1500); 

    itc_block_signal(); 
    itc_event = itc_signalfd_init();

    if (itc_event == -1) {
	perror("itc_signalfd_init");
	return -1;
    }

    output_timer = event_timer(&ipt); 
    if (output_timer == -1) {
	perror("event_timer");
	return -1;
    }


    ifudp = ipv4_udp_socket("192.168.2.4", 5000); 
    if (ifudp == -1) {
	perror("ipv4");
	return -1;
    }
    
    inet_aton("192.168.2.4", &local_addr);
    q.size = 0;
    q.head = NULL;
    q.tail = NULL;

    ret = alloc_mbuff_chain(&q, 8333);

    thread_table[KERNEL_LAYER_THREAD] = pthread_self();
    pthread_create(&thread_table[NETOUT_LAYER_THREAD], NULL, output, NULL);


    in.sin_family = AF_INET;
    in.sin_port	  = htons(500);
    inet_aton("192.168.2.5", &in.sin_addr);

    p= q.head;
    for ( i = 0; i <= 8333-1; i++ )
    {	
	p->msg_type = MSG_MBUFF_CARRIER; 
	p->mb.mbp->m_need_ts = 1;
	p->mb.mbp->m_tsoff = 3;
	p->mb.mbp->m_datalen = 1460;
	p->mb.mbp->m_hdrlen  = 8;
	p->mb.mbp->m_hdr.type = 5;
	p->mb.mbp->m_hdr.seq  = seq++;
	p->mb.mbp->m_hdr.src_port = htons(20);
	p->mb.mbp->m_hdr.dst_port = htons(50);
	memcpy(&(p->mb.mbp->m_outside_addr), &in, sizeof(struct sockaddr_in));

	p = p->p_next;
    }

    printf ("Alloc: %d\n", ret);
    printf ("QSize: %d\n", q.size);
    printf ("Timer: %d\n", output_timer);
    printf ("ITC: %d\n", itc_event);
    printf ("IFUDP: %d\n", ifudp);

    itc_writeto(NETOUT_LAYER_THREAD, &q, PRIO_NOR_QUEUE);

        
    q.size = 0;
    q.head = NULL;
    q.tail = NULL;

    p = q.head;

    sleep(5);

    ret = alloc_mbuff_chain(&q, 8333);
    printf("Ret1 : %d\n", ret);
    p = q.head;
    for ( i = 0; i <= 8333-1; i++ )
    {	
	p->msg_type = MSG_MBUFF_CARRIER; 
	p->mb.mbp->m_need_ts = 1;
	p->mb.mbp->m_tsoff = 3;
	p->mb.mbp->m_datalen = 1460;
	p->mb.mbp->m_hdrlen  = 8;
	memcpy(&(p->mb.mbp->m_outside_addr), &in, sizeof(struct sockaddr_in));

	p = p->p_next;
    }

    itc_writeto(NETOUT_LAYER_THREAD, &q, PRIO_RET_QUEUE);

    ret = alloc_mbuff_chain(&q, 83333);
    printf("Ret2 : %d\n", ret);
    printf("Qz: %d\n", q.size);
    p = q.head;
    for ( i = 0; i <= 83333-1; i++ )
    {	
	p->msg_type = MSG_MBUFF_CARRIER; 
	p->mb.mbp->m_need_ts = 1;
	p->mb.mbp->m_tsoff = 3;
	p->mb.mbp->m_datalen = 1460;
	p->mb.mbp->m_hdrlen  = 8;
	memcpy(&(p->mb.mbp->m_outside_addr), &in, sizeof(struct sockaddr_in));

	p = p->p_next;
    }

    itc_writeto(NETOUT_LAYER_THREAD, &q, PRIO_RET_QUEUE);

    pthread_join(thread_table[NETOUT_LAYER_THREAD], (void **)&tret);

    printf("RET: %d\n", *tret);
    return 0;
}