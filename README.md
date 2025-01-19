# Name

`ngx_http_log_var_set_module` allows setting the variable to the given value before access log writing.

# Table of Content

- [Name](#name)
- [Table of Content](#table-of-content)
- [Status](#status)
- [Synopsis](#synopsis)
- [Installation](#installation)
- [Directives](#directives)
	- [log\_var\_set](#log_var_set)
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
        log_var_set $log_field1 $upstream_http_custom_header1;
        log_var_set $log_field2 $upstream_http_custom_header2;
        proxy_pass http://example.upstream.com;
    }
}
```

# Installation

To use theses modules, configure your nginx branch with `--add-module=/path/to/ngx_http_log_var_set_module`.

# Directives

## log_var_set

**Syntax:** *log_var_set $variable value;*

**Default:** *-*

**Context:** *http, server, location*

Sets the request variable to the given value before access log writing. The value may contain variables from request or response, such as $upstream_http_*.
These directives are inherited from the previous configuration level only when there is no directive for the same variable defined at the current level.

# Author

Hanada im@hanada.info

# License

This Nginx module is licensed under [BSD 2-Clause License](LICENSE).
