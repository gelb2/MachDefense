#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CONFIG="$ROOT/.local/compatibility.json"
read_config() { /opt/homebrew/bin/python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))[sys.argv[2]])' "$CONFIG" "$1"; }
export CARGO_HOME="$(read_config cargo_home)" RUSTUP_HOME="$(read_config rustup_home)"
export PATH="$CARGO_HOME/bin:/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin"
export CARGO_TARGET_DIR="$(read_config build_root)/target"
export IPHONEOS_DEPLOYMENT_TARGET=15.0 CMAKE_POLICY_VERSION_MINIMUM=3.5
export GIT_CONFIG_GLOBAL=/dev/null CARGO_NET_GIT_FETCH_WITH_CLI=true
cd "$(read_config runtime_source)"
cargo +1.99.1 build --locked --release --target "${1:-aarch64-apple-ios-sim}" --no-default-features --features static,cpu_interpreter --bin touchHLE
