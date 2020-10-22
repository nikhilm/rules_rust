
# Rust Persistent Worker

The Rust Persistent Worker is itself implemented in Rust. It is built via Cargo and distributed as binaries. Source and release binaries are maintained at the [rustc-worker project](https://github.com/nikhilm/rustc-worker). Contributions should be submitted there and then the version of the binaries updated in `worker/repositories.bzl`.

## Why isn't this built via Bazel?

Bootstrapping the worker using these same rules (e.g. `rust_binary`) may be possible, but is not supported right now. There are a couple of challenges:
1. Since the worker has dependencies on various crates, it uses cargo-raze, which generates relevant rules. This means "don't use the worker to build this target" is transitive and such information needs to be propagated down the tree in a way that works with restrictions in Bazel. Initial experiments repeatedly encountered failures due to Bazel treating the rule dependency on the worker executable target as a cycle, even when building in non-worker mode. This may be user error or a restriction in Bazel. Until that is addressed, the easiest way to fix this is to change cargo-raze to customize what rules are used, and provide `rust_no_worker_binary` and `rust_no_worker_library` rules that do not have this cycle.
2. Figuring out a good strategy for dependencies. Since Bazel doesn't really have transitive dependencies, attempts to build this worker from source necessarily require users of these rules to register all the external repositories for the worker in their `WORKSPACE`. This could cause collisions with other dependencies. In addition, if the `_no_worker_` approach above is adopted, users will lose the benefits of workers for those dependencies (like `protobuf`) shared between the worker and their code. There is no satisfactory solution for this right now.

## How about rewriting the worker in C++?

That is certainly an option!
