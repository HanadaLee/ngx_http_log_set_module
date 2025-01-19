
/*
 * Copyright (C) Hanada
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_int_t                 index;
    ngx_http_complex_value_t  value;
    ngx_http_set_variable_pt  set_handler;
} ngx_http_log_var_set_variable_t;


typedef struct {
    ngx_array_t              *vars;
} ngx_http_log_var_set_loc_conf_t;


static ngx_int_t ngx_http_log_var_set_handler(ngx_http_request_t *r);
static char * ngx_http_log_var_set(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static ngx_int_t ngx_http_log_var_set_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static void *ngx_http_log_var_set_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_log_var_set_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_http_log_var_set_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_log_var_set_commands[] = {

    { ngx_string("log_var_set"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
      ngx_http_log_var_set,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_log_var_set_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_http_log_var_set_init,              /* postconfiguration */

    NULL,                                   /* create main conf */
    NULL,                                   /* init main conf */

    NULL,                                   /* create srv conf */
    NULL,                                   /* merge srv conf */

    ngx_http_log_var_set_create_loc_conf,   /* create loc conf */
    ngx_http_log_var_set_merge_loc_conf     /* merge loc conf */
};


ngx_module_t  ngx_http_log_var_set_module = {
    NGX_MODULE_V1,
    &ngx_http_log_var_set_module_ctx,       /* module context */
    ngx_http_log_var_set_commands,          /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_log_var_set_handler(ngx_http_request_t *r)
{
    ngx_str_t                          val;
    ngx_http_variable_t               *v;
    ngx_http_variable_value_t         *vv;
    ngx_http_log_var_set_loc_conf_t   *llcf;
    ngx_http_log_var_set_variable_t   *lv, *last;
    ngx_http_core_main_conf_t         *cmcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "log var set handler");

    llcf = ngx_http_get_module_loc_conf(r, ngx_http_log_var_set_module);

    if (llcf->vars == NULL) {
        return NGX_OK;
    }

    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);
    v = cmcf->variables.elts;

    lv = llcf->vars->elts;
    last = lv + llcf->vars->nelts;

    while (lv < last) {
        /*
         * explicitly set new value to make sure it will be available after
         * internal redirects
         */

        vv = &r->variables[lv->index];

        if (ngx_http_complex_value(r, &lv->value, &val) != NGX_OK) {
            return NGX_ERROR;
        }

        vv->valid = 1;
        vv->not_found = 0;
        vv->data = val.data;
        vv->len = val.len;

        if (lv->set_handler) {
            /*
             * set_handler only available in cmcf->variables_keys, so we store
             * it explicitly
             */

            lv->set_handler(r, vv, v[lv->index].data);
        }

        lv++;
    }

    return NGX_OK;
}


static char *
ngx_http_log_var_set(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_log_var_set_loc_conf_t *llcf = conf;

    ngx_str_t                         *value;
    ngx_http_variable_t               *v;
    ngx_http_log_var_set_variable_t   *lv;
    ngx_http_compile_complex_value_t   ccv;

    value = cf->args->elts;

    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    if (llcf->vars == NGX_CONF_UNSET_PTR) {
        llcf->vars = ngx_array_create(cf->pool, 1,
                                      sizeof(ngx_http_log_var_set_variable_t));
        if (llcf->vars == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    lv = ngx_array_push(llcf->vars);
    if (lv == NULL) {
        return NGX_CONF_ERROR;
    }

    v = ngx_http_add_variable(cf, &value[1], NGX_HTTP_VAR_CHANGEABLE);
    if (v == NULL) {
        return NGX_CONF_ERROR;
    }

    lv->index = ngx_http_get_variable_index(cf, &value[1]);
    if (lv->index == NGX_ERROR) {
        return NGX_CONF_ERROR;
    }

    if (v->get_handler == NULL) {
        v->get_handler = ngx_http_log_var_set_variable;
        v->data = (uintptr_t) lv;
    }

    lv->set_handler = v->set_handler;

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = &lv->value;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_log_var_set_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "log var set variable");

    v->not_found = 1;

    return NGX_OK;
}


static void *
ngx_http_log_var_set_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_log_var_set_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_log_var_set_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->vars = NGX_CONF_UNSET_PTR;

    return conf;
}


static char *
ngx_http_log_var_set_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_log_var_set_loc_conf_t *prev = parent;
    ngx_http_log_var_set_loc_conf_t *conf = child;

    ngx_conf_merge_ptr_value(conf->vars, prev->vars, NULL);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_log_var_set_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_log_var_set_handler;

    return NGX_OK;
}
