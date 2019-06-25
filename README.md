Name
====
NGINX-based HttpDNS Server

Table of Contents
=================
* [Name](#name)
* [Status](#status)
* [Synopsis](#synopsis)
* [Content Handler Directives](#content-handler-directives)
    * [httpdns](#httpdns)
    * [httpdns_arg_ip](#httpdns_arg_ip)
    * [httpdns_arg_host](#httpdns_arg_host)
    * [httpdns_timeout](#httpdns_timeout)
    * [httpdns_ns](#httpdns_ns)

Status
======

This module is test.

Synopsis
========

```nginx

   location /d {
     httpdns forward 114.114.114.114 8.8.8.8;
   }
```
```nginx

   location /d {
     httpdns recursion;
     httpdns_ns pptv.com dns1.pplive.com dns11.pplive.com dns12.pplive.com dns13.pplive.com;

   }
```

[Back to TOC](#table-of-contents)

Content Handler Directives
==========================
httpdns
----
**syntax:** *httpdns [forward|recursion] [forwarders]...*

**default:** *no*

**context:** *location*

**phase:** *content*


```nginx

    httpdns forward 114.114.114.114 8.8.8.8;
```

```nginx

    httpdns recursion;
```

httpdns_arg_ip
----
**syntax:** *httpdns_arg_ip value...*

**default:** *no*

**context:** *location*

**phase:** *content*


```nginx

    httpdns_arg_ip $arg_ip $real_ip $realip_remote_addr;
```

httpdns_arg_host
----
**syntax:** *httpdns_arg_host value...*

**default:** *no*

**context:** *location*

**phase:** *content*


```nginx

    httpdns_arg_host $arg_host;
```
