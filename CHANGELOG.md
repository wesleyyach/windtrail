# Changelog

## 1.0.0 — 2026-07-30

First stable public release.

### Added

- Continuous, speed-reactive wind-ribbon cursor trail
- Native KWin effect and native Plasma configuration module
- Crimson Slash, Wind White, Ice Blue, and custom-color presets
- Configurable thickness, intensity, duration, activation speed, and smoothing
- Full-screen suppression option
- English public UI and Portuguese documentation
- Multi-monitor support
- Install, uninstall, verification, and diagnostics scripts
- KDE Store listing draft, screenshots, testing report, and troubleshooting guide

### Fixed

- Native modules install into the Qt 6 plugin directory used by KWin
- Installer normalizes archive timestamps to prevent Ninja regeneration loops
- Color alpha is normalized because opacity is controlled by Intensity
- CMake automoc recognizes `KWIN_EFFECT_FACTORY`

### Compatibility

- Minimum KWin version: 6.7
- Validated on Fedora 44 / Plasma 6.7.3 / Wayland / x86_64

## 1.0.0-rc1 — 2026-07-30

Release candidate validated before the stable release.
