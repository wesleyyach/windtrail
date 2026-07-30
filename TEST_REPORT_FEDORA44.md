# WindTrail 1.0.0 validation report

- Date: 2026-07-30
- Distribution: Fedora 44
- Architecture: x86_64
- Plasma version: 6.7.3
- KWin version: 6.7.3
- Session type: Wayland
- GPU: AMD Radeon 680M
- Monitor setup: three displays

## Verified

- Clean CMake configuration: pass
- Both native modules build: pass
- Fresh archive installer: pass
- Correct Qt 6 plugin paths: pass
- Effect enabled in `kwinrc`: pass
- Effect loaded by KWin: pass
- Settings module opens: pass
- Settings apply without recompilation: pass
- Preset and custom-color controls: pass
- Logout/login upgrade flow: pass

## Not formally recorded in this report

- Uninstall and reinstall cycle
- Full-screen suppression across multiple games
- Lock/unlock artifact test
- Long-duration performance profiling
- Driver version
