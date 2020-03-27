#!/bin/sh

# /source - source directory with this script in it
# /source/dist - where the binary should end up

set -ex

cd /source
bazel --batch build --strategy=Genrule=standalone --spawn_strategy=standalone -c dbg //:envoy
mv bazel-bin/envoy /source/dist/envoy
rm bazel-*
