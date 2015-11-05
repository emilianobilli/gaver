


struct timeout {
    struct timespec when;
    struct sock     *sk;
    struct mbuff    *mptr;
};


enum {
    CTRL_CLOSE = 1,
    CTRL_BIND,
    CTRL_LISTEN,
    CTRL_CONNECT_REQ,
    CTRL_ACCEPT_REQ,
    CTRL_ESTABLISHED
};



#define DATA_IO_NONE		0x00	/* 0000 */
#define DATA_IO_WAITING_MEM	0x04	/* 0100 */
#define DATA_IO_READ_PENDING	0x01	/* 0001 */
#define DATA_IO_WRITE_PENDING	0x02	/* 0010 */
#define DATA_IO_RW_PENDING	0x03	/* 0011 = IO_READ_PENDING | IO_WRITE_PENDING */

#define MAX_SOCKETS	  256
#define NO_GVPORT         0x0000

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


    int		so_mtu;			/* Mtu for the socket	*/
    u_int64_t   so_upload_speed;
    u_int64_t   so_download_speed;

    double	so_refresh_tokens;
    double	so_vacant_tokens;

    u_int8_t	so_capwin;		/* Congestion Avoidance Percent Window */


    u_int64_t	so_dseq_out;		/* Data seq out         */
    u_int64_t	so_cseq_out;		/* Contrl seq out       */
    u_int64_t	so_dseq_exp;		/* Data seq expected    */
    u_int64_t   so_cseq_exp;		/* Control seq expected */

    struct seq_lost_queue lostq;	/* Queue of lost Trasport Data Unit (mbuff) */

    size_t wmem_size;			/* Write buffer size    */
    size_t rmem_size;			/* Read  buffer size    */
    
    /*

    struct mb_queue wmemq;
    struct mb_queue rmemq;
    struct mb_queue sentq;	In Flyght 
    struct mb_queue roooq;	Reception Out Of Order 

    struct seq_lost_queue lostq;	 Queue of lost Trasport Data Unit (mbuff)

    */

    struct sock *so_next;
};


struct sockqueue {
    size_t size;
    struct sock *head;
    struct sock *tail;
};


struct seqlost {
    u_int64_t	    seq;	/* Seq */
    double          eta;
    struct timespec eta;	/* Estimated arrival Time */
    struct seqlost  *next;	/* Next Node */
}

struct seqlost_queue {
    size_t size;
    struct seqlost *head;
    struct seqlost *tail;
};


EXTERN struct sock sktable[MAX_SOCKETS];
EXTERN struct sock *sk_gvport[sizeof(u_int16_t)];
EXTERN struct sockqueue so_used;
EXTERN struct sockqueue so_free;


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * makewreq():										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct msg *makewreq (int so_data, struct mb_queue *mbq) 
{
    struct msg *ptr;
    struct mbuff *mbptr;
    size_t len;
    len = 0;
    ptr = alloc_msg_locking();

    if ( ptr != NULL ) {
	ptr->io_opt = IO_OPT_WRITE;
	ptr->io_socket = so_data;
	ptr->io_ret    = 0;
	ptr->io_errno  = 0;
	ptr->discard   = DISCARD_TRUE;

	init_mbuff_queue(&(ptr->mb.mbq));

	while ( (mbptr = mbuff_dequeue(mbq)) != NULL ) {
	    len += mbptr->m_datalen;
	    mbuff_enqueue(&(ptr->mb.mbq));
	}

	ptr->io_req_len = len;
	ptr->io_rep_len = 0;
	ptr->io_chunk_size = 0;
	ptr->p_next = NULL;
    }
    return ptr;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * makerreq():										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct msg *makerreq ( int so_data, size_t len, size_t chunk_size ) 
{
    struct msg *ptr;
    ptr = alloc_msg_locking();

    if (ptr != NULL) {
	ptr->io_opt = IO_OPT_READ;
	ptr->io_socket = so_data;
	ptr->io_ret    = 0;
	ptr->io_errno  = 0;
	ptr->discard   = DISCARD_FALSE;

	init_mbuff_queue(&(ptr->mb.mbq));

	ptr->io_req_len = len;
	ptr->io_rep_len = 0;
	ptr->io_chunk_size = chunk_size;
	ptr->p_next = NULL;
    }
    return ptr;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * mbufftomsg():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct msg *mbufftomsg (struct mbuff *mb, int clone, int discard)
{
    struct msg *mptr;
    struct mbuff *mbnew;

    mptr = alloc_msg_locking();
    if ( mptr != NULL ) 
    {
	mptr->msg_type = MSG_MBUFF_CARRIER;
	if (clone)
	{
	    mbnew = clone_mbuff(mb);
	    mptr->mb.mbp = mbnew;
	}
	else
	    mptr->mb.mbp = mb;
	mptr->discard = discard;
    }
    return mptr;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * mbqtomsgq():										    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t mbqtomsgq (struct msg_queue *msgq, struct mb_queue *mbq, size_t len, int clone, int discard)
{
    struct msg_queue tmp;
    struct msg   *mptr;
    struct mbuff *mbptr, *mbnew;
    size_t ret; 
    size_t sz;
    size_t i;


    init_msg_queue(&tmp);

    sz = ( len > mbq->size )? mbq->size : len;
    
    ret = alloc_msg_chain(&tmp, sz);
    
    if (ret == 0)
	return 0;

    i = 0;
    mbptr = mbq->head;
    mptr  = tmp.head;

    while ( i <= ret-1 ) {
	if (mbptr == NULL)
	    break;

	mptr->msg_type = MSG_BUFF_CARRIER;
	if (clone) {
	    mbnew = clone_mbuff(mbptr);
	    if (mbnew == NULL) {
		/*
		 * Corregir
		 */

		break;
	    }
	    mptr->mb.mbp = mbnew;
	}
	else
	    mptr->mb.mbp = mbptr;

	mptr->discard = discard;
	i++;
	mptr  = mptr->p_next;
	mbptr = mbptr->m_next;
    }
    return i;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * init_seqlost_queue():								    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void init_seqlost_queue(struct seqlost_queue *q)
{
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * getseqlost():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct seqlost *getseqlost(struct seqlost_queue *q, u_int64_t seq)
{
    struct seqlost_queue tmp;
    struct seqlost *tdptr;
    struct seqlost *ret;

    init_seqlost_queue(&tmp);

    while ((tdptr != seqlost_dequeue(q)) != NULL)
    {
	if (tdptr->seq == seq)
	    ret = tdptr;
	else
	    seqlost_enqueue(&tmp, tdptr);
    }
    q->size = tmp.size;
    q->head = tmp.head;
    q->tail = tmp.tail;
    return ret;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * copyseqlost():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void copyseqlost (u_int64_t *buff, struct seqlost_queue *q, size_t max)
{
    struct seqlost *tdptr;

    size_t n = 0;
    tdptr = q->head;
    while (tdptr != NULL && n <= max -1) {
	buff[n] = tdptr->seq;
	n++;
	tdptr = tdptr->next;
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * seqlost_enqueue():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void seqlost_enqueue(struct seqlost_queue *q, struct seqlost *tdptr)
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
 * seqlost_dequeue():									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct seqlost *seqlost_dequeue(struct seqlost_queue *q)
{
    struct seqlost *td;
    
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

    init_sock_queue(&so_used);
    init_sock_queue(&so_free);

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
    memset(soptr,0,sizeof(struct sock));
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
 * bind_gvport(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct sock *bind_gvport(struct sock *sk, u_int16_t gvport)
{
    if ( sk_gvport[gvport] == NULL )
    {
	sk_gvport[gvport] = sk;
	sk->so_local_gvport = gvport;
	return sk;
    }
    return NULL;
}

struct sock *close_gvport_sk (struct sock *sk)
{
    sk_gvport[sk->so_local_gvport] = NULL;
    sk->so_local_gvport = NO_GVPORT;
    return sk;
}

struct sock *close_gvport_gvport(u_int16_t gvport)
{
    struct sock *sk = sk_gvport[gvport];
    if ( sk != NULL )
	return close_gvport_sk(sk);
    return NULL;
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
 * setusedsock(): 									    *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void setusedsock(struct sock *sk)
{
    sock_enqueue(&so_used, sk);
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
