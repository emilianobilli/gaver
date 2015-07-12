


struct timeout {
    struct timespec when;
    struct sock     *sk;
    struct mbuff    *mptr;

};



struct sock {
    int 	so_lodata;
    int 	so_lodata_state;
    int 	so_loctrl;
    int 	so_loctrl_state;
    in_addr	so_host_addr;
    u_int16_t	so_host_port;
    u_int16_t   so_host_gvport;
    u_int16_t	so_local_gvport;
    int		so_state;


    u_int64_t	so_dseq_out;
    u_int64_t	so_cseq_out;
    u_int64_t	so_dseq_exp;
    u_int64_t   so_cseq_exp;

    struct tpdu_lost_queue lostq;

    
    struct mb_queue wmemq;
    struct mb_queue rmemq;
    struct mb_queue sentq;	/* In Flyght */
    struct mb_queue roooq;	/* Reception Out Of Order */

    struct sock *so_next;
    struct sock *so_prev;
};



struct sockqueue {
    size_t size;
    struct sock *head;
    struct sock *tail;
};


struct tpdu_lost {
    u_int64_t	    seq;
    struct timespec eta;

    struct tpdu_lost *next;
}

struct tpdu_lost_queue {
    size_t size;
    struct tpdu_lost *head;
    struct tpdu_lost *tail;
};

struct sockqueue so_used;
struct sockqueue so_free;


struct sock *ports[sizeof(u_int16_t)];

struct sock *getbyport(u_int16_t port)
{
    return sk_table[port];
} 
struct sock *getbysd

