#!/usr/bin/env bash
set -Eeuo pipefail

readonly EFFECT_ID="proxyx_windtrail"
readonly CONFIG_ID="kwin_proxyx_windtrail_config"

command -v qtpaths6 >/dev/null 2>&1 || { echo "qtpaths6 not found" >&2; exit 1; }
command -v kwriteconfig6 >/dev/null 2>&1 || { echo "kwriteconfig6 not found" >&2; exit 1; }

purge_settings=false
if [[ "${1:-}" == "--purge-settings" ]]; then
    purge_settings=true
elif [[ $# -gt 0 ]]; then
    echo "Usage: $0 [--purge-settings]" >&2
    exit 2
fi

gdbus call --session \
    --dest org.kde.KWin \
    --object-path /Effects \
    --method org.kde.kwin.Effects.unloadEffect \
    "$EFFECT_ID" >/dev/null 2>&1 || true

qt_plugin_dir="$(qtpaths6 --plugin-dir)"
sudo rm -f \
    "${qt_plugin_dir}/kwin/effects/plugins/${EFFECT_ID}.so" \
    "${qt_plugin_dir}/kwin/effects/configs/${CONFIG_ID}.so"

kwriteconfig6 --file kwinrc --group Plugins \
    --delete "${EFFECT_ID}Enabled" || true

if $purge_settings; then
    for key in Color Thickness Intensity TrailDuration ActivationSpeed Smoothness DisableInFullscreen; do
        kwriteconfig6 --file kwinrc \
            --group "Effect-${EFFECT_ID}" \
            --delete "$key" || true
    done
fi

gdbus call --session \
    --dest org.kde.KWin \
    --object-path /KWin \
    --method org.kde.KWin.reconfigure \
    >/dev/null 2>&1 || true

echo "WindTrail removed."
$purge_settings && echo "Saved settings were removed too."
