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
    struct msg *p;
    init_heap();
    itc_init();

    ipt.tv_sec  = 0;
    ipt.tv_nsec = pktime(MBPS_TOBPS(5), 1500); 

/*    itc_block_signal(); */
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


    ifudp = ipv4_udp_socket("0.0.0.0", 5000); 
    if (ifudp == -1) {
	perror("ipv4");
	return -1;
    }
    
    q.size = 0;
    q.head = NULL;
    q.tail = NULL;

    ret = alloc_mbuff_chain(&q, 300);

    thread_table[KERNEL_LAYER_THREAD] = pthread_self();
    pthread_create(&thread_table[NETOUT_LAYER_THREAD], NULL, output, NULL);

    in.sin_family = AF_INET;
    in.sin_port	  = 300;
    inet_aton("8.8.8.8", &in.sin_addr.s_addr);

    in2.sin_family = AF_INET;
    in2.sin_port	  = 300;
    inet_aton("4.4.4.4", &in2.sin_addr.s_addr);

    in3.sin_family = AF_INET;
    in3.sin_port	  = 300;
    inet_aton("4.256.4.4", &in3.sin_addr.s_addr);

    p= q.head;
    for ( i = 0; i <= 300-1; i++ )
    {	
	p->p_mbuff->m_need_ts = 1;
	p->p_mbuff->m_tsoff = 3;
	p->p_mbuff->m_datalen = 1460;
	p->p_mbuff->m_hdrlen  = 8;
	if (( i % 2 ) == 0)
	    memcpy(&(p->p_mbuff->m_outside_addr), &in, sizeof(struct sockaddr_in));
	else
	    memcpy(&(p->p_mbuff->m_outside_addr), &in3, sizeof(struct sockaddr_in));
	
	if (( i % 100) == 0)
	    memcpy(&(p->p_mbuff->m_outside_addr), &in3, sizeof(struct sockaddr_in));
	p = p->p_next;
    }

    printf ("Alloc: %d\n", ret);
    printf ("QSize: %d\n", q.size);
    printf ("Timer: %d\n", output_timer);
    printf ("ITC: %d\n", itc_event);
    printf ("IFUDP: %d\n", ifudp);

    itc_writeto(NETOUT_LAYER_THREAD, &q, PRIO_NOR_QUEUE);

    pthread_join(thread_table[NETOUT_LAYER_THREAD], (void **)&tret);

    printf("RET: %d\n", *tret);
    return 0;
}