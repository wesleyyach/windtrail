#!/usr/bin/env bash
set -Eeuo pipefail

readonly EFFECT_ID="proxyx_windtrail"
readonly CONFIG_ID="kwin_proxyx_windtrail_config"
readonly ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly BUILD_DIR="${ROOT_DIR}/build"

die() {
    printf 'WindTrail installer: %s\n' "$*" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "missing command: $1"
}

for command_name in cmake ninja qtpaths6 kwriteconfig6 gdbus sudo; do
    require_command "$command_name"
done

printf '== WindTrail 1.0.0 ==\n\n'

plasma_version="$(plasmashell --version 2>/dev/null | awk '{print $2}' || true)"
kwin_version="$(kwin_wayland --version 2>/dev/null | awk '{print $2}' || true)"
printf 'Plasma: %s\n' "${plasma_version:-unknown}"
printf 'KWin:   %s\n\n' "${kwin_version:-unknown}"

# Release archives can preserve timestamps from another machine. Normalizing
# them prevents Ninja's "manifest still dirty" regeneration loop.
find "$ROOT_DIR" -path "$BUILD_DIR" -prune -o -type f -exec touch {} +
rm -rf "$BUILD_DIR"

printf 'Configuring...\n'
if ! cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr; then
    cat >&2 <<'EOF'

CMake could not find all development dependencies.

Fedora 44:
  sudo dnf install gcc-c++ cmake extra-cmake-modules ninja-build \
    qt6-qtbase-devel kf6-kcoreaddons-devel kf6-kconfig-devel \
    kf6-kcmutils-devel kwin-devel libepoxy-devel libdrm-devel
EOF
    exit 1
fi

printf '\nBuilding...\n'
cmake --build "$BUILD_DIR" -j"$(nproc)"

qt_plugin_dir="$(qtpaths6 --plugin-dir)"
effect_path="${qt_plugin_dir}/kwin/effects/plugins/${EFFECT_ID}.so"
config_path="${qt_plugin_dir}/kwin/effects/configs/${CONFIG_ID}.so"
backup_dir="${HOME}/.local/share/windtrail/backups/$(date +%Y%m%d-%H%M%S)"

if [[ -f "$effect_path" || -f "$config_path" ]]; then
    mkdir -p "$backup_dir"
    [[ -f "$effect_path" ]] && cp -a "$effect_path" "$backup_dir/"
    [[ -f "$config_path" ]] && cp -a "$config_path" "$backup_dir/"
    printf '\nBacked up previous modules to:\n  %s\n' "$backup_dir"
fi

loaded_before="$(gdbus call --session \
    --dest org.kde.KWin \
    --object-path /Effects \
    --method org.kde.kwin.Effects.isEffectLoaded \
    "$EFFECT_ID" 2>/dev/null || true)"

printf '\nInstalling...\n'
sudo cmake --install "$BUILD_DIR"

[[ -f "$effect_path" ]] || die "effect module was not installed at $effect_path"
[[ -f "$config_path" ]] || die "configuration module was not installed at $config_path"

kwriteconfig6 --file kwinrc --group Plugins \
    --key "${EFFECT_ID}Enabled" true

gdbus call --session \
    --dest org.kde.KWin \
    --object-path /KWin \
    --method org.kde.KWin.reconfigure \
    >/dev/null 2>&1 || true

printf '\nInstalled files:\n  %s\n  %s\n' "$effect_path" "$config_path"

if [[ "$loaded_before" == "(true,)" ]]; then
    cat <<'EOF'

WindTrail was already loaded before the upgrade.
Log out and back in once so KWin loads the new native library.
Your current session can continue safely until then.
EOF
else
    load_result="$(gdbus call --session \
        --dest org.kde.KWin \
        --object-path /Effects \
        --method org.kde.kwin.Effects.loadEffect \
        "$EFFECT_ID" 2>/dev/null || true)"
    printf '\nKWin load result: %s\n' "${load_result:-unavailable}"
    if [[ "$load_result" != "(true,)" ]]; then
        printf 'Log out and back in once to finish loading WindTrail.\n'
    fi
fi

printf '\nDone. Run ./scripts/verify.sh to inspect the installation.\n'
