load("//worker:toolchain.bzl", "worker_toolchain")

toolchain_type(name = "toolchain_type")

worker_toolchain(
    name = "worker_linux_x86_64",
    worker_binary = "@rust_worker_linux_x86_64//file",
)

worker_toolchain(name = "worker_dummy")

toolchain(
    name = "linux_x86_64",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    toolchain = ":worker_linux_x86_64",
    toolchain_type = ":toolchain_type",
)

toolchain(
    name = "dummy",
    toolchain = ":worker_dummy",
    toolchain_type = ":toolchain_type",
)
