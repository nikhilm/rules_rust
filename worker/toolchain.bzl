def _worker_toolchain_impl(ctx):
    toolchain_info = platform_common.ToolchainInfo(
        worker_binary = ctx.executable.worker_binary,
    )
    return [toolchain_info]

worker_toolchain = rule(
    implementation = _worker_toolchain_impl,
    attrs = {
        "worker_binary": attr.label(allow_single_file = True, executable = True, cfg = "exec"),
    },
)
