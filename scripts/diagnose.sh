#!/usr/bin/env bash
set -u

echo "WindTrail diagnostics"
echo "====================="
echo "Date: $(date --iso-8601=seconds 2>/dev/null || date)"
echo "OS: $(grep -E '^PRETTY_NAME=' /etc/os-release 2>/dev/null | cut -d= -f2- | tr -d '"' || echo unknown)"
echo "Architecture: $(uname -m)"
echo "Kernel: $(uname -r)"
echo "Session: ${XDG_SESSION_TYPE:-unknown}"
echo "Desktop: ${XDG_CURRENT_DESKTOP:-unknown}"
echo "Plasma: $(plasmashell --version 2>/dev/null || echo unavailable)"
echo "KWin: $(kwin_wayland --version 2>/dev/null || echo unavailable)"
echo "Qt: $(qtpaths6 --qt-version 2>/dev/null || echo unavailable)"
echo "Qt plugin dir: $(qtpaths6 --plugin-dir 2>/dev/null || echo unavailable)"
echo
echo "Displays:"
kscreen-doctor -o 2>/dev/null || echo "kscreen-doctor unavailable"
echo
echo "WindTrail verification:"
"$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)/verify.sh" || true
echo
echo "Recent KWin/WindTrail logs:"
journalctl --user -b --no-pager 2>/dev/null | grep -iE 'windtrail|proxyx_windtrail|kwin' | tail -120 || true
