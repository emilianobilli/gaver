#include "queue.h"
#include "bbtp_output.h"
#include <pthreads.h>

#define TRUE  1
#define FALSE 0

#define SIG_OUTPUT_TIMER


/* File descriptor ready to receive signals */
extern int sigfd;

/* Timer to send packets */
extern timer_t *output_timer;

/* Multiplier factor: for each timer how many packets needs to send */
extern int output_mlt;

/* Socket to send packages */
extern int udp_socket;

extern share_queue_t outq;


void *bbtp_output_thread(void *arg)
{
    int ovrun;
    int signo;
    ssize_t len;
    queue_t tx_queue;

    tx_queue.head = NULL;
    tx_queue.tail = NULL;    

    while (1)
    {
	ovrun = TRUE;
	if ( tx_queue.head == NULL && tx_queue.tail == NULL )
	{
	    /*	
	     * TX QUEUE is empty
             */

	    /*
             * Enter to critical region
             */
	    pthread_mutex_lock(&(outq.mutex));
	    if (outq.head == NULL && outq.tail == NULL)
		/*
                 * No packages to process. Wait more packages
                 */
		pthread_cond_wait(&(outq.cond),&(outq.mutex));
	
	    tx_queue.head = outq.q.head;
	    tx_queue.tail = outq.q.tail;
	    outq.q.head = NULL;
	    outq.q.tail = NULL;

	    /*
	     * Exit to critical region
	     */	
	    pthread_mutex_unlock(&(outq.mutex));

	    ovrun = FALSE;
	}
	signo = wait_signal(sigfd);
	if (signo == SIG_OUTPUT_TIMER)
	{
	    if ( ovrun == FALSE )
	    {
		snd_count = 0;
		ovrun = TRUE;
	    }
	    else
		snd_count = timer_getoverrun(output_timer);
	}
	snd_count = (snd_count == 0) ? output_mlt : snd_count * mlt;
	while ( snd_count > 0 )
	{
	    pkt = dequeue(&tx_queue);
	    if (pkt == NULL)
		/*
                 * Empty
		 */
		break;
	    
	    len = bbtp_finish_output(udp_socket,
				     pkt->data,
				     pkt->lendata,
				     pkt->hdr,
				     pkt->lenhdr,
				     pkt->dst,
				     NULL);		     
    

	    snd_count--;
	}
    }
    return NULL;
}