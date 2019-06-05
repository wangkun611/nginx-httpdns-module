#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <nginx.h>
#include <ngx_http.h>

#include "ngx_http_httpdns_module.h"

static void *
ngx_http_httpdns_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_httpdns_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_httpdns_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    return conf;
}

static char *
ngx_http_httpdns_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    //ngx_http_httpdns_loc_conf_t *prev = parent;
    //ngx_http_httpdns_loc_conf_t *conf = child;

    return NGX_CONF_OK;
}

static ngx_str_t  ngx_http_sample_text = ngx_string(
    "I'm so powerful on stage that I seem to have created a monster.\n" \
    "When I'm performing I'm an extrovert, yet inside I'm a completely different man."
);

static ngx_int_t
ngx_http_httpdns_handler(ngx_http_request_t *r)
{
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t   out;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = ngx_http_sample_text.len;

    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";
    r->headers_out.content_type_len = r->headers_out.content_type.len;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;

    b->start = b->pos = ngx_http_sample_text.data;
    b->end = b->last = ngx_http_sample_text.data + ngx_http_sample_text.len;
    b->memory = 1;
    b->last_buf = 1;
    b->last_in_chain = 1;

    return ngx_http_output_filter(r, &out);
}

static char *
ngx_http_httpdns(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_httpdns_handler;

    return NGX_CONF_OK;
}

static ngx_http_module_t  ngx_http_httpdns_ctx = {
    NULL,                              /* preconfiguration */
    NULL,                              /* postconfiguration */

    NULL,                              /* create main configuration */
    NULL,                              /* init main configuration */

    NULL,                              /* create server configuration */
    NULL,                              /* merge server configuration */

    ngx_http_httpdns_create_loc_conf,  /* create location configuration */
    ngx_http_httpdns_merge_loc_conf    /* merge location configuration */
};

static ngx_command_t  ngx_http_httpdns_commands[] = {
    
    { ngx_string("httpdns"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_ANY|NGX_CONF_NOARGS,
      ngx_http_httpdns,
      0,
      0,
      NULL },

      ngx_null_command
};

ngx_module_t ngx_http_httpdns_module = {
    NGX_MODULE_V1,
    &ngx_http_httpdns_ctx,       /* module context */
    ngx_http_httpdns_commands,   /* module directives */
    NGX_HTTP_MODULE,             /* module type */
    NULL,                        /* init master */
    NULL,                        /* init module */
    NULL,                        /* init process */
    NULL,                        /* init thread */
    NULL,                        /* exit thread */
    NULL,                        /* exit process */
    NULL,                        /* exit master */
    NGX_MODULE_V1_PADDING
};
