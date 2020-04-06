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

