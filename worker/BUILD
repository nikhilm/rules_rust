# load("@rules_proto//proto:defs.bzl", "proto_library")
# load("@io_bazel_rules_rust//proto:proto.bzl", "rust_proto_library")
# load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS", "rust_proto_toolchain")
load("//rust:rust.bzl", "rust_no_worker_library", "rust_no_worker_binary")

rust_no_worker_library(
    name = "rustc_worker",
    edition = "2018",
    srcs = [
        "src/lib.rs",
        "src/worker_protocol.rs",
    ],
    deps = [
        "//proto/raze:protobuf",
    ],
)

rust_no_worker_binary(
    name = "rustc-worker",
    edition = "2018",
    srcs = ["src/main.rs"],
    deps = [
        ":rustc_worker",
        "//proto/raze:protobuf",
    ],
    visibility = ["//visibility:public"],
)

# The rust_* rules are required to build the worker
# They shouldn't _use_ the worker when building the worker OR any of its dependencies
# Ideally we want some transitive switch applier (like a custom transition?) that will enforce this across the entire dependency graph.
# What does that look like?
# Can we use transitions? Is that supported in Bazel 0.27?
# Alternatively can we use a config-setting?
# In that case, how do we enforce the config setting when building the worker?
# Alternatively, we would need to make raze emit customizable rule names, then have it run in the custom mode to generate the BUILD files for the protobuf bits.

# On the C++ side, the protobuf repo does provide a "protobuf" and "protobuf-lite" target to link the worker against, that will allow us to use protobufs easily.

# we would need to write code to repeatedly execute the child process and capture stdout.
# similarly, we would need to write code to parse args (actually that exists). 
# we would need some equivalent of canonicalize (realpath on unix, GetFinalPathNameByHandle on windows), 
# response file handling and reading
# hashing the inputs
# mkdir_all

# otherwise edit the raze files...
