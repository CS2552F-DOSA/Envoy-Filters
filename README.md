# Dosa Envoy filter

This project is related to the CS2952F. 

## Building

To build the Envoy static binary in local:

1. `git submodule update --init`
2. `bazel build //:envoy`

To build the docker image:
`$ ./dosa-build.sh`

## How to test

We do not have uint test and integration test. To test or experimentw with this project in docker container:

```
$ git submodule update --init
$ docker run -it --rm -v `pwd`:/source -w /source envoyproxy/envoy-build:latest /bin/bash
$ docker build //:envoy
$ bazel-bin/envoy -c dosa_server.yaml -l info 2>&1
```

On another terminal (better within the container), you can try to send some messages to the Envoy now:

```
$ curl -v 127.0.0.1:9999/service
```

## How to write and use an HTTP filter

- The main task is to write a class that implements the interface
 [`Envoy::Http::StreamDecoderFilter`][StreamDecoderFilter] as in
 [`http_filter.h`](http_filter.h) and [`http_filter.cc`](http_filter.cc),
 which contains functions that handle http headers, data, and trailers.
 Note that this is an example of decoder filters, 
 and to write encoder filters or decoder/encoder filters
 you need to implement 
 [`Envoy::Http::StreamEncoderFilter`][StreamEncoderFilter] or
 [`Envoy::Http::StreamFilter`][StreamFilter] instead.
- You also need a class that implements 
 `Envoy::Server::Configuration::NamedHttpFilterConfigFactory`
 to enable the Envoy binary to find your filter,
 as in [`http_filter_config.cc`](http_filter_config.cc).
 It should be linked to the Envoy binary by modifying [`BUILD`][BUILD] file.
- Finally, you need to modify the Envoy config file to add your filter to the
 filter chain for a particular HTTP route configuration. For instance, if you
 wanted to change [the front-proxy example][front-envoy.yaml] to chain our
 `sample` filter, you'd need to modify its config to look like

```yaml
http_filters:
- name: sample          # before envoy.router because order matters!
  config:
    key: via
    val: sample-filter
- name: envoy.router
  config: {}
```
 

[StreamDecoderFilter]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/include/envoy/http/filter.h#L300
[StreamEncoderFilter]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/include/envoy/http/filter.h#L413
[StreamFilter]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/include/envoy/http/filter.h#L462
[BUILD]: https://github.com/envoyproxy/envoy-filter-example/blob/d76d3096c4cbd647d26b44b3f801c3afbc81d3e2/http-filter-example/BUILD#L15-L18
[front-envoy.yaml]: https://github.com/envoyproxy/envoy/blob/b2610c84aeb1f75c804d67effcb40592d790e0f1/examples/front-proxy/front-envoy.yaml#L28
