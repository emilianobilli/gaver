


struct timeout {
    struct timespec when;
    struct sock     *sk;
    struct mbuff    *mptr;
};

#define IO_NONE		  0x00  /* 0000 */
#define IO_READ_PENDING   0x01	/* 0001 */
#define IO_WRITE_PENDING  0x02  /* 0010 */
#define IO_RW_PENDING     0x03  /* 0011 = IO_READ_PENDING | IO_WRITE_PENDING */

#define MAX_SOCKETS	  255

struct sock {
    int 	so_lodata;
    int 	so_lodata_state;	/* DATAIO_READ_PENDING | DATAIO_WRITE_PENDING */
    int 	so_loctrl;
    int 	so_loctrl_state;
    in_addr	so_host_addr;
    u_int16_t	so_host_port;
    u_int16_t   so_host_gvport;
    u_int16_t	so_local_gvport;
    int		so_state;


    u_int64_t	so_dseq_out;		/* Data seq out         */
    u_int64_t	so_cseq_out;		/* Contrl seq out       */
    u_int64_t	so_dseq_exp;		/* Data seq expected    */
    u_int64_t   so_cseq_exp;		/* Control seq expected */

    struct tdu_lost_queue lostq;	/* Queue of lost Trasport Data Unit (mbuff) */

    size_t wmem_size;			/* Write buffer size    */
    size_t rmem_size;			/* Read  buffer size    */
    struct mb_queue wmemq;
    struct mb_queue rmemq;
    struct mb_queue sentq;	/* In Flyght */
    struct mb_queue roooq;	/* Reception Out Of Order */

    struct sock *so_next;
};


struct sockqueue {
    size_t size;
    struct sock *head;
    struct sock *tail;
};


struct tdulost {
    u_int64_t	    seq;	/* Seq */
    struct timespec eta;	/* Estimated arrival Time */
    struct tdulost  *next;	/* Next Node */
}

struct tdulost_queue {
    size_t size;
    struct tdulost *head;
    struct tdulost *tail;
};



EXTERN struct sock sktable[MAX_SOCKETS];
EXTERN struct sock *sk_gvport[sizeof(u_int16_t)];
EXTERN struct sockqueue so_used;
EXTERN struct sockqueue so_free;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_tdulost_queue():								    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_tdulost_queue(struct tdulost_queue *q)
{
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * gettdulost():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct tdulost *gettdulost(struct tdulost_queue *q, u_int64_t seq)
{
    struct tdulost_queue tmp;
    struct tdulost *tdptr;
    struct tdulost *ret;

    init_tdulost_queue(&tmp);

    while ((tdptr != tdulost_dequeue(q)) != NULL)
    {
	if (tdptr->seq == seq)
	    ret = tdptr;
	else
	    tdulost_enqueue(&tmp, tdptr);
    }
    q->size = tmp.size;
    q->head = tmp.head;
    q->tail = tmp.tail;
    return ret;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * copytdulost():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void copytdulost (u_int64_t *buff, struct tdulost_queue *q, size_t max)
{
    struct tdulost *tdptr;

    size_t n = 0;
    tdptr = q->head;
    while (tdptr != NULL && n <= max -1) {
	buff[n] = tdptr->seq;
	n++;
	tdptr = tdptr->next;
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * tdulost_enqueue():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void tdulost_enqueue(struct tdulost_queue *q, struct tdulost *tdptr)
{
    tdptr->next = NULL;
    if (q->tail == NULL && q->head == NULL)
        q->head = tdptr;
    else
        q->tail->next = tdptr;	
    q->tail = tdptr;
    q->size++;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * tdulost_dequeue():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct tdulost *tdulost_dequeue(struct tdulost_queue *q)
{
    struct tdulost *td;
    
    td = q->head;
    if (td != NULL) {
	q->head = td->next;
	if (q->head == NULL)
	    q->tail = NULL;
	q->size--;
	td->next = NULL;
    }
    return td;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock_table(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_sock_table(void)
{
    int n;

    init_sock_queue(so_used);
    init_sock_queue(so_free);

    for ( n = 0; n <= MAX_SOCKETS-1; n++ )
    {
	init_sock(&sktable[n]);
	sock_enqueue(&so_free, &sktable[n]);
    }
    return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_sock(struct sock *soptr)
{
    /*
     * 
     * Codigo para inicializar la estructura sock
     *
     */
    return;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_sock_queue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_sock_queue(struct sockqueue *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbygvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getsockbygvport(u_int16_t gvport)
{
    return sk_gvport[gvport];
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbysodata(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getsockbysodata(int so_data)
{
    struct sock *ptr;
    
    ptr = so_used.head;
    while (ptr != NULL)
    {
	if (ptr->so_lodata == so_data)
	    return ptr;
	ptr = ptr->so_next;
    }
    return NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getsockbysoctrl(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getsockbysoctrl(int so_ctrl)
{
    struct sock *ptr;

    ptr = so_used.head;
    while (ptr != NULL)
    {
	if (ptr->so_loctrl == so_ctrl)
	    return ptr;
	ptr = ptr->so_next;
    }
    return NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getfreesock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *getfreesock(void)
{
    return sock_dequeue(&so_free);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_dequeue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *sock_dequeue(struct sockqueue *q)
{
    struct sock *so;
    
    so = q->head;
    if (so != NULL) {
	q->head = so->so_next;
	if (q->head == NULL)
	    q->tail = NULL;
	q->size--;
	so->so_next = NULL;
    }
    return so;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * sock_enqueue(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void sock_enqueue (struct sockqueue *q, struct sock *soptr)
{
    soptr->so_next = NULL;
    if (q->tail == NULL && q->head == NULL)
        q->head = soptr;
    else
        q->tail->so_next = soptr;	
    q->tail = soptr;
    q->size++;
}





