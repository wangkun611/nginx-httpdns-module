#ifndef NGX_HTTPDNS_MODULE_H
#define NGX_HTTPDNS_MODULE_H
#include <ngx_config.h>
#include "ngx_http_httpdns_resolver.h"

extern ngx_module_t  ngx_http_httpdns_module;

/* location config struct */
typedef struct {
    struct ngx_httpdns_resolver_s *resolver;
} ngx_http_httpdns_loc_conf_t;

typedef struct {
    ngx_msec_t timeout;         /* 超时时间配置,单位毫秒  */
} ngx_http_httpdns_ctx_t;

#endif /*  NGX_HTTPDNS_MODULE_H */