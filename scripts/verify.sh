#!/usr/bin/env bash
set -u

readonly EFFECT_ID="proxyx_windtrail"
readonly CONFIG_ID="kwin_proxyx_windtrail_config"
status=0

qt_plugin_dir="$(qtpaths6 --plugin-dir 2>/dev/null || true)"
effect_path="${qt_plugin_dir}/kwin/effects/plugins/${EFFECT_ID}.so"
config_path="${qt_plugin_dir}/kwin/effects/configs/${CONFIG_ID}.so"

echo "WindTrail verification"
echo "======================"
echo "Plasma: $(plasmashell --version 2>/dev/null || echo unavailable)"
echo "KWin:   $(kwin_wayland --version 2>/dev/null || echo unavailable)"
echo "Session: ${XDG_SESSION_TYPE:-unknown}"
echo "Qt: $(qtpaths6 --qt-version 2>/dev/null || echo unavailable)"
echo "Qt plugin directory: ${qt_plugin_dir:-unavailable}"
echo

if [[ -f "$effect_path" ]]; then
    echo "[OK] effect module: $effect_path"
else
    echo "[MISSING] effect module: $effect_path"
    status=1
fi

if [[ -f "$config_path" ]]; then
    echo "[OK] config module: $config_path"
else
    echo "[MISSING] config module: $config_path"
    status=1
fi

enabled="$(kreadconfig6 --file kwinrc --group Plugins --key "${EFFECT_ID}Enabled" 2>/dev/null || true)"
echo "Enabled in kwinrc: ${enabled:-not set}"
[[ "$enabled" == "true" ]] || status=1

loaded="$(gdbus call --session \
    --dest org.kde.KWin \
    --object-path /Effects \
    --method org.kde.kwin.Effects.isEffectLoaded \
    "$EFFECT_ID" 2>/dev/null || true)"
echo "Loaded by KWin: ${loaded:-unavailable}"
[[ "$loaded" == "(true,)" ]] || status=1

exit "$status"
