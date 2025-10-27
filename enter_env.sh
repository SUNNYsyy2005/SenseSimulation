#!/usr/bin/env bash
# SenseSimulation environment loader
# 仅加载当前子工作区的 overlay，并在加载前移除根 overlay 路径。

# 防止重复加载
if [[ -n "$SIM_SENSEBEETLE_ENV" ]]; then
  return 0
fi

# Resolve dirs
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ROOT_OVERLAY="$ROOT_DIR/install"

info() { echo -e "[simulation-env] $*"; }

strip_path_contains() {
  local var_name="$1"; shift
  local needle="$1"; shift
  local current="${!var_name}"
  local IFS=':'; local out=()
  for p in $current; do
    if [[ -n "$p" && "$p" != *"$needle"* ]]; then
      out+=("$p")
    fi
  done
  IFS=':'; eval export "$var_name=\"${out[*]}\""
}

for v in AMENT_PREFIX_PATH CMAKE_PREFIX_PATH COLCON_PREFIX_PATH LD_LIBRARY_PATH PYTHONPATH PKG_CONFIG_PATH PATH; do
  strip_path_contains "$v" "$ROOT_OVERLAY"
done

if [[ -f "$SCRIPT_DIR/install/setup.bash" ]]; then
  info "sourcing SenseSimulation/install/setup.bash"
  # shellcheck disable=SC1090
  source "$SCRIPT_DIR/install/setup.bash"
else
  info "skip SenseSimulation/install/setup.bash (not found)"
fi

export SIM_SENSEBEETLE_ENV=1
info "environment ready (SenseSimulation only)"
