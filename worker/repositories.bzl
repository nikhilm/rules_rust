# buildifier: disable=module-docstring
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_file")

def rust_worker_repositories():
    """Registers rustc-worker binary archives."""
    http_file(
        name = "rust_worker_linux_x86_64",
        executable = True,
        sha256 = "c7ee178d658a9ff9c9b10f7acce48af57227170d454741072aff5fabf923f8fb",
        urls = ["https://github.com/nikhilm/rustc-worker/releases/download/v0.2.0/rustc-worker-0.2.0-linux-x86_64"],
    )

# buildifier: disable=unnamed-macro
def rust_worker_toolchains():
    """Registers worker toolchains for supported platforms."""
    native.register_toolchains(
        "@io_bazel_rules_rust//worker:linux_x86_64",
    )
