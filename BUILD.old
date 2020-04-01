package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
)

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        ":dosa_filter_config",
        "@envoy//source/exe:envoy_main_lib",
    ],
)

envoy_cc_library(
    name = "dosa_filter_lib",
    srcs = ["http_filter.cc"],
    hdrs = ["http_filter.h"],
    repository = "@envoy",
    deps = [
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/http:filter_interface",
        "@envoy//include/envoy/upstream:cluster_manager_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//source/common/common:enum_to_int",
        "@envoy//source/common/http:message_lib",
        "@envoy//source/common/http:utility_lib",
        "@envoy//source/common/common:utility_lib"
    ],
)

envoy_cc_library(
    name = "dosa_filter_config",
    srcs = ["http_filter_config.cc"],
    hdrs = ["http_filter_config.h"],
    repository = "@envoy",
    deps = [
        ":dosa_filter_lib",
        "@envoy//source/server:configuration_lib",
        "@envoy//source/server/config/network:http_connection_manager_lib"
    ],
)