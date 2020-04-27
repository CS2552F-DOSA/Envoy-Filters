FROM ubuntu:14.04
ADD dist/envoy /usr/local/bin/envoy
ADD dosa_server.yaml /dosa_server.yaml
ENTRYPOINT /usr/local/bin/envoy -c /dosa_server.yaml