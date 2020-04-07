package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
)

load("@envoy_api//bazel:api_build_system.bzl", "api_proto_package")

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        ":dosa_filter_config",
        "@envoy//source/exe:envoy_main_entry_lib",
    ],
)

api_proto_package()

envoy_cc_library(
    name = "dosa_filter_lib",
    srcs = ["http_filter.cc"],
    hdrs = ["http_filter.h"],
    repository = "@envoy",
    deps = [
        ":pkg_cc_proto",
        "@envoy//source/extensions/filters/network/common/redis:codec_interface",
        "@envoy//source/exe:envoy_common_lib",
    ],
)

envoy_cc_library(
    name = "dosa_filter_config",
    srcs = ["http_filter_config.cc"],
    hdrs = ["http_filter_config.h"],
    repository = "@envoy",
    deps = [
        ":dosa_filter_lib",
        "@envoy//include/envoy/server:filter_config_interface",
        "@envoy//source/extensions/filters/network/common/redis:codec_lib",
    ],
)