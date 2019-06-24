#include <ngx_config.h>
#include <ngx_core.h>

#ifndef NGX_HTTPDNS_RESPONSE_
#define NGX_HTTPDNS_RESPONSE_

#include <ngx_http.h>
#include "ngx_http_httpdns_module.h"
#include "ngx_http_httpdns_resolver.h"

#define NGX_HTTPDNS_MissingArgument       1    /*  */
#define NGX_HTTPDNS_InvalidHost           2    /*  */
#define NGX_HTTPDNS_MethodNotAllowed      3    /*  */
#define NGX_HTTPDNS_InternalError         5    /*  */

ngx_int_t ngx_httpdns_send_error(ngx_http_request_t *r,
    ngx_http_httpdns_loc_conf_t *hlcf, ngx_uint_t error);
ngx_int_t ngx_httpdns_send_response(ngx_http_request_t *r,
    ngx_http_httpdns_loc_conf_t *hlcf, ngx_httpdns_resolver_ctx_t *ctx);

#endif /* NGX_HTTPDNS_RESPONSE_ */