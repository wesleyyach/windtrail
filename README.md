# WindTrail

**WindTrail** is a native KWin effect that draws a smooth, speed-reactive wind ribbon behind the mouse cursor on KDE Plasma Wayland.

Unlike classic mouse trails that repeat cursor images, WindTrail generates a continuous curved ribbon whose width and lifetime respond to movement speed.


## Features

- Continuous curved ribbon instead of repeated cursor images
- Width and lifetime react to cursor speed
- Crimson Slash, Wind White, Ice Blue, and custom-color presets
- Adjustable thickness, intensity, duration, activation speed, and smoothing
- Native preview and configuration button in Plasma's Desktop Effects page
- Optional automatic disablement in full-screen apps and games
- Multi-monitor support
- No network access, telemetry, or background service

## Compatibility

- KDE Plasma / KWin **6.7 or newer**
- Wayland session
- OpenGL compositing
- Linux

Version 1.0.0 was validated on **Fedora 44, Plasma 6.7.3, KWin 6.7.3, Wayland, x86_64**. Native KWin plugins are linked against KWin and may need recompilation after KWin upgrades.

## Install from source

Download and extract the release archive, then run:

```bash
cd windtrail-1.0.0
./scripts/install.sh
```

On Fedora 44, install the required development packages with:

```bash
sudo dnf install \
  gcc-c++ cmake extra-cmake-modules ninja-build \
  qt6-qtbase-devel \
  kf6-kcoreaddons-devel kf6-kconfig-devel kf6-kcmutils-devel \
  kwin-devel libepoxy-devel libdrm-devel
```

The installer builds the effect, installs both native plugin modules into the Qt 6 plugin directory, enables WindTrail, and reports whether a logout/login is required.

## Configure

Open:

**System Settings → Apps & Windows → Window Management → Desktop Effects → WindTrail**

Then click the configuration button.

## Verify installation

```bash
./scripts/verify.sh
```

## Uninstall

```bash
./scripts/uninstall.sh
```

To remove saved settings too:

```bash
./scripts/uninstall.sh --purge-settings
```

## Building manually

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j"$(nproc)"
sudo cmake --install build
```

## Reporting bugs

Open an issue at <https://github.com/wesleyyach/windtrail/issues> and include the output of:

```bash
./scripts/verify.sh
journalctl --user -b --no-pager | grep -iE 'windtrail|proxyx_windtrail|kwin'
```

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
