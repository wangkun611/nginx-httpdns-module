
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_HTTPDNS_RESOLVER_H_INCLUDED_
#define _NGX_HTTPDNS_RESOLVER_H_INCLUDED_


#define NGX_RESOLVE_A         1   // a host address
#define NGX_RESOLVE_NS        2   // an authoritative name server
#define NGX_RESOLVE_MD        3   // a mail destination (Obsolete - use MX)
#define NGX_RESOLVE_MF        4   // a mail forwarder (Obsolete - use MX)
#define NGX_RESOLVE_CNAME     5   // the canonical name for an alias
#define NGX_RESOLVE_PTR       12  // a domain name pointer
#define NGX_RESOLVE_MX        15  // mail exchange
#define NGX_RESOLVE_TXT       16  // text strings
#if (NGX_HAVE_INET6)
#define NGX_RESOLVE_AAAA      28
#endif
#define NGX_RESOLVE_SRV       33
#define NGX_RESOLVE_DNAME     39

#define NGX_RESOLVE_FORMERR   1
#define NGX_RESOLVE_SERVFAIL  2
#define NGX_RESOLVE_NXDOMAIN  3
#define NGX_RESOLVE_NOTIMP    4
#define NGX_RESOLVE_REFUSED   5
#define NGX_RESOLVE_TIMEDOUT  NGX_ETIMEDOUT


#define NGX_NO_RESOLVER       (void *) -1

#define ngx_httpdns_resolver_MAX_RECURSION    50


typedef struct ngx_httpdns_resolver_s  ngx_httpdns_resolver_t;


typedef struct {
    ngx_connection_t         *udp;
    ngx_connection_t         *tcp;
    struct sockaddr          *sockaddr;
    socklen_t                 socklen;
    ngx_str_t                 server;
    ngx_log_t                 log;
    ngx_buf_t                *read_buf;
    ngx_buf_t                *write_buf;
    ngx_httpdns_resolver_t           *resolver;
} ngx_httpdns_resolver_connection_t;


typedef struct ngx_httpdns_resolver_ctx_s  ngx_httpdns_resolver_ctx_t;

typedef void (*ngx_httpdns_resolver_handler_pt)(ngx_httpdns_resolver_ctx_t *ctx);


typedef struct {
    struct sockaddr          *sockaddr;
    socklen_t                 socklen;
    ngx_str_t                 name;
    u_short                   priority;
    u_short                   weight;
} ngx_httpdns_resolver_addr_t;


typedef struct {
    ngx_str_t                 name;
    u_short                   priority;
    u_short                   weight;
    u_short                   port;
} ngx_httpdns_resolver_srv_t;


typedef struct {
    ngx_str_t                 name;
    u_short                   priority;
    u_short                   weight;
    u_short                   port;

    ngx_httpdns_resolver_ctx_t       *ctx;
    ngx_int_t                 state;

    ngx_uint_t                naddrs;
    ngx_addr_t               *addrs;
} ngx_httpdns_resolver_srv_name_t;


typedef struct {
    ngx_rbtree_node_t         node;
    ngx_queue_t               queue;

    /* PTR: resolved name, A: name to resolve */
    u_char                   *name;

#if (NGX_HAVE_INET6)
    /* PTR: IPv6 address to resolve (IPv4 address is in rbtree node key) */
    struct in6_addr           addr6;
#endif

    u_short                   nlen;
    u_short                   qlen;

    u_char                   *query;
#if (NGX_HAVE_INET6)
    u_char                   *query6;
#endif

    union {
        in_addr_t             addr;
        in_addr_t            *addrs;
        u_char               *cname;
        ngx_httpdns_resolver_srv_t   *srvs;
    } u;

    u_char                    code;
    u_short                   naddrs;
    u_short                   nsrvs;
    u_short                   cnlen;

#if (NGX_HAVE_INET6)
    union {
        struct in6_addr       addr6;
        struct in6_addr      *addrs6;
    } u6;

    u_short                   naddrs6;
#endif

    time_t                    expire;
    time_t                    valid;
    uint32_t                  ttl;

    unsigned                  tcp:1;
#if (NGX_HAVE_INET6)
    unsigned                  tcp6:1;
#endif

    ngx_uint_t                last_connection;

    ngx_httpdns_resolver_ctx_t       *waiting;
} ngx_httpdns_resolver_node_t;


struct ngx_httpdns_resolver_s {
    ngx_int_t                 recursion_mode;   /* 是否递归模式 */
    /* has to be pointer because of "incomplete type" */
    ngx_event_t              *event;
    void                     *dummy;
    ngx_log_t                *log;

    /* event ident must be after 3 pointers as in ngx_connection_t */
    ngx_int_t                 ident;

    /* simple round robin DNS peers balancer */
    ngx_array_t               connections;
    ngx_uint_t                last_connection;

    ngx_rbtree_t              name_rbtree;
    ngx_rbtree_node_t         name_sentinel;

    ngx_rbtree_t              srv_rbtree;
    ngx_rbtree_node_t         srv_sentinel;

    ngx_rbtree_t              addr_rbtree;
    ngx_rbtree_node_t         addr_sentinel;

    ngx_rbtree_t              ns_rbtree;
    ngx_rbtree_node_t         ns_sentinel;

    ngx_queue_t               name_resend_queue;
    ngx_queue_t               srv_resend_queue;
    ngx_queue_t               addr_resend_queue;

    ngx_queue_t               name_expire_queue;
    ngx_queue_t               srv_expire_queue;
    ngx_queue_t               addr_expire_queue;

#if (NGX_HAVE_INET6)
    ngx_uint_t                ipv6;                 /* unsigned  ipv6:1; */
    ngx_rbtree_t              addr6_rbtree;
    ngx_rbtree_node_t         addr6_sentinel;
    ngx_queue_t               addr6_resend_queue;
    ngx_queue_t               addr6_expire_queue;
#endif

    time_t                    resend_timeout;
    time_t                    tcp_timeout;
    time_t                    expire;
    time_t                    valid;

    ngx_uint_t                log_level;
};

struct ngx_httpdns_resolver_ctx_s {
    ngx_httpdns_resolver_ctx_t       *next;     /* ctx 是一个列表，按照CNAME的解析顺序连接 */
    ngx_httpdns_resolver_t           *resolver;
    ngx_httpdns_resolver_node_t      *node;

    /* event ident must be after 3 pointers as in ngx_connection_t */
    ngx_int_t                 ident;

    ngx_int_t                 state;
    ngx_str_t                 name;
    struct sockaddr           client_addr;       /* 用户的出口IP, edns-client-subnet */
    ngx_str_t                 service;

    time_t                    valid;
    ngx_uint_t                naddrs;
    ngx_httpdns_resolver_addr_t      *addrs;
    ngx_httpdns_resolver_addr_t       addr;
    struct sockaddr_in        sin;

    ngx_uint_t                count;
    ngx_uint_t                nsrvs;
    ngx_httpdns_resolver_srv_name_t  *srvs;

    ngx_httpdns_resolver_handler_pt   handler;
    void                     *data;
    ngx_msec_t                timeout;

    unsigned                  quick:1;
    unsigned                  async:1;
    unsigned                  cancelable:1;
    ngx_uint_t                recursion;      /* 当前递归查询的深度 */
    ngx_event_t              *event;
};

/* 
 * 创建DNS解析器,如果 n > 0 转发模式. 如果 ns不为空,递归模式
 * param: names, n 上级DNS地址
 * param: ns 权威服务器
 */
ngx_httpdns_resolver_t *ngx_httpdns_resolver_create(ngx_conf_t *cf, ngx_str_t *names,
    ngx_uint_t n, ngx_array_t *ns);

ngx_httpdns_resolver_ctx_t *ngx_httpdns_resolve_start(ngx_httpdns_resolver_t *r,
    ngx_httpdns_resolver_ctx_t *temp);
ngx_int_t ngx_httpdns_resolve_name(ngx_httpdns_resolver_ctx_t *ctx);
void ngx_httpdns_resolve_name_done(ngx_httpdns_resolver_ctx_t *ctx);
ngx_int_t ngx_httpdns_resolve_addr(ngx_httpdns_resolver_ctx_t *ctx);
void ngx_httpdns_resolve_addr_done(ngx_httpdns_resolver_ctx_t *ctx);
char *ngx_httpdns_resolver_strerror(ngx_int_t err);


#endif /* _NGX_HTTPDNS_RESOLVER_H_INCLUDED_ */
