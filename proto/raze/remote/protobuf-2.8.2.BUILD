"""
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""
package(default_visibility = [
  # Public for visibility by "@raze__crate__version//" targets.
  #
  # Prefer access through "//proto/raze", which limits external
  # visibility to explicit Cargo.toml dependencies.
  "//visibility:public",
])

licenses([
  "notice", # "MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_no_worker_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "build-script-build" with type "custom-build" omitted
# Unsupported target "coded_input_stream" with type "bench" omitted
# Unsupported target "coded_output_stream" with type "bench" omitted

rust_no_worker_library(
    name = "protobuf",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__bytes__0_4_12//:bytes",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "2.8.2",
    crate_features = [
        "bytes",
        "with-bytes",
    ],
)

