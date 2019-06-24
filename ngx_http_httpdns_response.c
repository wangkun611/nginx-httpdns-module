#include <ngx_config.h>
#include "ngx_http_httpdns_response.h"

static ngx_str_t ngx_httpdns_error_MissingArgument = ngx_string("{\"code\": \"MissingArgument\"}");
static ngx_str_t ngx_httpdns_error_InvalidHost = ngx_string("{\"code\": \"InvalidHost\"}");
static ngx_str_t ngx_httpdns_error_MethodNotAllowed = ngx_string("{\"code\": \"MethodNotAllowed\"}");
static ngx_str_t ngx_httpdns_error_InternalError = ngx_string("{\"code\": \"InternalError\"}");
static ngx_str_t ngx_httpdns_error_Unknown = ngx_string("{\"code\": \"Unknown\"}");

static ngx_int_t 
ngx_httpdns_send_header(ngx_http_request_t *r, ngx_int_t status, ngx_int_t content_length) {
    r->headers_out.status = status;
    r->headers_out.content_length_n = content_length;
    r->headers_out.content_type_len = sizeof("application/json;charset=UTF-8") - 1;
    ngx_str_set(&r->headers_out.content_type, "application/json;charset=UTF-8");
    r->headers_out.content_type_lowcase = NULL;

    return ngx_http_send_header(r);
}
ngx_int_t ngx_httpdns_send_error(ngx_http_request_t *r,
    ngx_http_httpdns_loc_conf_t *hlcf, ngx_uint_t error)
{
    const ngx_str_t  *error_desc;
    ngx_uint_t        http_code;
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t   out[1];

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                "http special response: %i, \"%V?%V\"",
                error, &r->uri, &r->args);

    switch (error)
    {
    case NGX_HTTPDNS_MissingArgument:
        error_desc = &ngx_httpdns_error_MissingArgument;
        http_code = NGX_HTTP_BAD_REQUEST;
        break;
    case NGX_HTTPDNS_InvalidHost:
        error_desc = &ngx_httpdns_error_InvalidHost;
        http_code = NGX_HTTP_BAD_REQUEST;
        break;
    case NGX_HTTPDNS_MethodNotAllowed:
        error_desc = &ngx_httpdns_error_MethodNotAllowed;
        http_code = NGX_HTTP_NOT_ALLOWED;
        break;
    case NGX_HTTPDNS_InternalError:
        error_desc = &ngx_httpdns_error_InternalError;
        http_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
        break;
    default:
        error_desc = &ngx_httpdns_error_Unknown;
        http_code = NGX_HTTP_BAD_REQUEST;
        break;
    }
    rc = ngx_httpdns_send_header(r, http_code, error_desc->len);

    if (rc == NGX_ERROR || r->header_only) {
        return rc;
    }

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->memory = 1;
    b->pos = error_desc->data;
    b->last = error_desc->data + error_desc->len;
    b->last_buf = 1;
    b->last_in_chain = 1;

    out[0].buf = b;
    out[0].next = NULL;

    return ngx_http_output_filter(r, &out[0]);
}

ngx_int_t ngx_httpdns_send_response(ngx_http_request_t *r,
    ngx_http_httpdns_loc_conf_t *hlcf, ngx_httpdns_resolver_ctx_t *ctx)
{
    u_char    *start, *end, *p, *p1;
    ngx_uint_t  i;
    struct sockaddr_in   *sin;
    ngx_chain_t          out[1];
    ngx_buf_t           *b;
    ngx_int_t            rc;

    p =  ngx_palloc(r->pool, 4096);
    if (p == NULL) {
        return NGX_ERROR;
    }
    start = p;
    end = p + 4096;
    p = ngx_slprintf(p, end, "{\"host\": \"%V\",\"ips\": [", &ctx->name);

    for (i = 0; i < ctx->naddrs; i++)
    {
        sin = (struct sockaddr_in*)ctx->addrs[i].sockaddr;
        p1 = (u_char *) &sin->sin_addr;
        if (sin->sin_family == AF_INET) {
            if (i == 0) {
                p = ngx_slprintf(p, end, "\"%ud.%ud.%ud.%ud\"", p1[0], p1[1], p1[2], p1[3]);
            } else {
                p = ngx_slprintf(p, end, ",\"%ud.%ud.%ud.%ud\"", p1[0], p1[1], p1[2], p1[3]);
            }
        }
    }

    p = ngx_slprintf(p, end, "],\"ttl\": 57,\"origin_ttl\": 120}");

    rc = ngx_httpdns_send_header(r, NGX_HTTP_OK, p - start);

    if (rc == NGX_ERROR || r->header_only) {
        return rc;
    }

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->memory = 1;
    b->pos = start;
    b->last = p;
    b->last_buf = 1;
    b->last_in_chain = 1;

    out[0].buf = b;
    out[0].next = NULL;
    return ngx_http_output_filter(r, &out[0]);
}