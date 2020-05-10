package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
)

load("@envoy_api//bazel:api_build_system.bzl", "api_proto_package")

envoy_cc_library(
    name = "open_lib",
    # hdrs = glob(["src/google/*.h"]),
    hdrs = glob(["**"]),
    visibility = ["//visibility:public"],
)