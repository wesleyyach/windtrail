# Contributing

Contributions are welcome, especially compatibility reports for other Plasma 6 versions and Linux distributions.

## Before submitting a change

1. Build with `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`.
2. Run `cmake --build build -j"$(nproc)"`.
3. Test on a Wayland session.
4. Verify the effect can be enabled, configured, disabled, and uninstalled.
5. Keep source files covered by the existing GPL-3.0-or-later SPDX headers.

Native KWin plugins can be ABI-sensitive. Include your distribution, architecture, Plasma version, KWin version, Qt version, GPU, and session type in bug reports.
