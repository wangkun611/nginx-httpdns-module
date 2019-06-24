#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <nginx.h>
#include <ngx_http.h>

#include "ngx_http_httpdns_module.h"
#include "ngx_http_httpdns_response.h"

static ngx_http_httpdns_ctx_t *ngx_http_httpdns_create_ctx(ngx_http_request_t *r);
static void ngx_http_httpdns_resolve_handler(ngx_httpdns_resolver_ctx_t *ctx);

static void *
ngx_http_httpdns_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_httpdns_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_httpdns_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->ip_from = NGX_CONF_UNSET_PTR;
    conf->resolver_timeout = NGX_CONF_UNSET_MSEC;
    conf->recursion  = 1;
    return conf;
}

static char *
ngx_http_httpdns_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_httpdns_loc_conf_t *prev = parent;
    ngx_http_httpdns_loc_conf_t *conf = child;

    ngx_conf_merge_ptr_value(conf->ip_from, prev->ip_from, NGX_CONF_UNSET_PTR);
    ngx_conf_merge_msec_value(conf->resolver_timeout,
                              prev->resolver_timeout, 30000);

    return NGX_CONF_OK;
}

static ngx_http_httpdns_ctx_t *
ngx_http_httpdns_create_ctx(ngx_http_request_t *r)
{
    ngx_http_httpdns_ctx_t *ctx;

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_httpdns_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }

    return ctx;
}

static ngx_int_t
ngx_http_httpdns_handler(ngx_http_request_t *r)
{
    ngx_http_httpdns_loc_conf_t  *hlcf;
    ngx_http_httpdns_ctx_t       *ctx;
    ngx_str_t    host;
    ngx_str_t    ip;
    ngx_httpdns_resolver_ctx_t       *rctx, temp;

    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_httpdns_module);

    if (ngx_http_arg(r, (u_char *) "host", 4, &host) != NGX_OK) {
        return ngx_httpdns_send_error(r, hlcf, NGX_HTTPDNS_MissingArgument); 
    }
    if (ngx_http_arg(r, (u_char *) "ip", 2, &ip) != NGX_OK) {

    }
    
    ctx = ngx_http_get_module_ctx(r, ngx_http_httpdns_module);
    if (ctx == NULL) {
        ctx = ngx_http_httpdns_create_ctx(r);
        if (ctx == NULL) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        ngx_http_set_ctx(r, ctx, ngx_http_httpdns_module);
    }

    temp.name = host;

    rctx = ngx_httpdns_resolve_start(hlcf->resolver, &temp);
    if (rctx == NULL) {
        return ngx_httpdns_send_error(r, hlcf, NGX_HTTPDNS_InternalError);
    }

    if (rctx == NGX_NO_RESOLVER) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                        "no resolver defined to resolve %V", &host);

        return ngx_httpdns_send_error(r, hlcf, NGX_HTTPDNS_InternalError);
    }

    r->count ++;
    rctx->name = host;
    rctx->handler = ngx_http_httpdns_resolve_handler;
    rctx->data = r;
    rctx->timeout = hlcf->resolver_timeout;

    if (ngx_httpdns_resolve_name(rctx) != NGX_OK) {
        return ngx_httpdns_send_error(r, hlcf, NGX_HTTPDNS_InternalError);
    }

    return NGX_DONE;
}

static void
ngx_http_httpdns_resolve_handler(ngx_httpdns_resolver_ctx_t *ctx)
{
    ngx_connection_t              *c;
    ngx_http_request_t            *r;
    ngx_http_httpdns_loc_conf_t   *hlcf;

    r = ctx->data;
    c = r->connection;
    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_httpdns_module);

    if (ctx->state)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "%V could not be resolved (%i: %s)",
                      &ctx->name, ctx->state,
                      ngx_httpdns_resolver_strerror(ctx->state));
        if (ngx_httpdns_send_error(r, hlcf, NGX_HTTPDNS_InternalError) == NGX_ERROR) {
            ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        } else {
            ngx_http_finalize_request(r, NGX_OK);
        }
    } else {
        if (ngx_httpdns_send_response(r, hlcf, ctx) == NGX_ERROR) {
            ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        } else {
            ngx_http_finalize_request(r, NGX_OK);
        }
    }
    ngx_httpdns_resolve_name_done(ctx);
}

static char *
ngx_http_httpdns(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_httpdns_loc_conf_t *hlcf;
    ngx_http_core_loc_conf_t    *clcf;
    ngx_str_t                   *value;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_httpdns_handler;

    hlcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_httpdns_module);
    if (cf->args->nelts > 1) {
        value = cf->args->elts;
        if (ngx_strncasecmp(value[1].data, (u_char *) "forward", 7) == 0)
        {
            hlcf->resolver = ngx_httpdns_resolver_create(cf, &value[2], cf->args->nelts - 2, NULL);
            if (hlcf->resolver == NULL) {
                return NGX_CONF_ERROR;
            }
        } else if (ngx_strncasecmp(value[1].data, (u_char *) "recursion", 7) == 0)  {
        } else {
            return "is not supportted mode";
        }
    }
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
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_1MORE,
      ngx_http_httpdns,
      0,
      0,
      NULL },
    { ngx_string("httpdns_ip"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_1MORE,
      ngx_conf_set_str_array_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_httpdns_loc_conf_t, ip_from),
      NULL},

    { ngx_string("httpdns_timeout"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_httpdns_loc_conf_t, resolver_timeout),
      NULL },

    { ngx_string("httpdns_ns"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
      ngx_conf_set_keyval_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_httpdns_loc_conf_t, ns),
      NULL},

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
