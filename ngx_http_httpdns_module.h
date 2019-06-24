#ifndef NGX_HTTPDNS_MODULE_H
#define NGX_HTTPDNS_MODULE_H

#include <ngx_config.h>
#include "ngx_http_httpdns_resolver.h"

extern ngx_module_t  ngx_http_httpdns_module;

/* location config struct */
typedef struct {
    struct ngx_httpdns_resolver_s *resolver;
    ngx_array_t                   *ip_from;
    ngx_msec_t                     resolver_timeout;
    ngx_int_t                      recursion;           /* 启用DNS递归 */
    ngx_array_t                   *ns;                  /* Authoritative nameservers */
 } ngx_http_httpdns_loc_conf_t;

typedef struct {
    ngx_str_t  host;            /* 请求中的参数 host */
    ngx_str_t  ip;              /* 请求中的参数 ip */
} ngx_http_httpdns_ctx_t;

#endif /*  NGX_HTTPDNS_MODULE_H */
