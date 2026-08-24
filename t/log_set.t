#!/usr/bin/perl

# Tests for ngx_http_log_set_module.

###############################################################################

use warnings;
use strict;

use Test::More;

BEGIN { use FindBin; chdir($FindBin::Bin); }

use Test::Nginx;

###############################################################################

select STDERR; $| = 1;
select STDOUT; $| = 1;

my $t = Test::Nginx->new()->has(qw/http rewrite ngx_condition_module
	ngx_http_log_set_module/)->plan(10);

$t->write_file_expand('nginx.conf', <<'EOF');

%%TEST_GLOBALS%%

daemon off;

events {
}

http {
    %%TEST_GLOBALS_HTTP%%

    log_format values '$uri|$field|$other';

    log_set $field parent-$status;
    log_set $other parent;

    server {
        listen       127.0.0.1:8080;
        server_name  localhost;

        condition special str_eq $arg_mode special;

        access_log %%TESTDIR%%/values.log values;

        location = /inherit {
            return 200 inherit;
        }

        location = /conditional {
            when special {
                log_set $field special-$arg_value;
            }
            log_set $field fallback-$arg_value;
            return 200 conditional;
        }

        location = /override {
            log_set $field local;
            return 200 override;
        }

        location = /response {
            add_header X-Value response;
            log_set $field $status:$sent_http_x_value;
            return 200 response;
        }
    }
}

EOF

$t->run();

###############################################################################

like(http_get('/inherit'), qr/200 OK/, 'inherited rule request');
like(http_get('/conditional?mode=special&value=one'), qr/200 OK/,
	'conditional hit request');
like(http_get('/conditional?mode=other&value=two'), qr/200 OK/,
	'conditional miss request');
like(http_get('/override'), qr/200 OK/, 'local override request');
like(http_get('/response'), qr/200 OK/, 'response variable request');

my $log = $t->read_file('values.log');

like($log, qr{^/inherit\|parent-200\|parent$}m,
	'http rules are inherited');
like($log, qr{^/conditional\|special-one\|parent$}m,
	'first matching definition wins');
like($log, qr{^/conditional\|fallback-two\|parent$}m,
	'condition miss evaluates fallback');
like($log, qr{^/override\|local\|parent$}m,
	'local definition replaces parent for the same variable');
like($log, qr{^/response\|200:response\|parent$}m,
	'log phase can use response variables');

###############################################################################
