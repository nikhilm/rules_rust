load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "worker",
    srcs = [
        "worker.cc",
        "//util/process_wrapper:system.h",
        "//util/process_wrapper:utils.h",
        "//util/process_wrapper:utils.cc",
    ] + select({
        "@bazel_tools//src/conditions:host_windows": [
            "//util/process_wrapper:system_windows.cc",
        ],
        "//conditions:default": [
            "//util/process_wrapper:system_posix.cc",
        ],
    }),
    defines = [] + select({
        "@bazel_tools//src/conditions:host_windows": [
            "UNICODE",
            "_UNICODE",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
