# Name

`ngx_http_log_set_module` allows setting the variable to the given value before access log writing.

# Table of Content

- [Name](#name)
- [Table of Content](#table-of-content)
- [Status](#status)
- [Synopsis](#synopsis)
- [Installation](#installation)
- [Conditional syntax](#conditional-syntax)
- [Directives](#directives)
  - [log\_var\_set](#log_set)
- [Author](#author)
- [License](#license)

# Status

This Nginx module is currently considered experimental. Issues and PRs are welcome if you encounter any problems.

# Synopsis

```nginx
log_format main '$remote_addr - $remote_user [$time_local] '
                    '"$request" $status $body_bytes_sent '
                    '"$http_referer" "$http_user_agent" '
                    '"$log_field1" "$log_field2"';
access_log /spool/logs/nginx-access.log;

server {
    listen 127.0.0.1:80;
    server_name localhost;

    location / {
        log_set $log_field1 $upstream_http_custom_header1;
        condition has_field2 is_not_empty $upstream_http_custom_header2;
        when has_field2 {
            log_set $log_field2 $upstream_http_custom_header2;
        }
        proxy_pass http://example.upstream.com;
    }
}
```

# Installation

To use theses modules, configure your nginx branch with `--add-module=/path/to/ngx_http_log_set_module`.

To enable named conditions, build `ngx_condition_module` and this module statically in the same nginx configuration.

# Conditional syntax

Conditional syntax is selected at compile time. With `ngx_condition_module`, place `log_set` inside an `http`, `server`, or `location` `when` block; `if=` and `if!=` are rejected. Without it, `when` is unavailable and legacy `if=`/`if!=` remain supported. A rule whose condition does not match is skipped so the next definition of the same variable can be evaluated.

# Directives

## log_set

**Syntax:** *log_set $variable value;*

**Default:** *-*

**Context:** *http, server, location, http when, server when, location when*

Sets the request variable to the given value before access log writing. The value may contain variables from request or response, such as $upstream_http_*.
These directives are inherited from the previous configuration level only when there is no directive for the same variable defined at the current level.

# Author

Hanada im@hanada.info

# License

This Nginx module is licensed under [BSD 2-Clause License](LICENSE).
