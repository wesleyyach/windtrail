# Troubleshooting

## `loadEffect` returns `(false,)`

Confirm both modules were installed under the directory printed by:

```bash
qtpaths6 --plugin-dir
```

Then run `./scripts/verify.sh`.

## Changes compile but the old appearance remains

Qt can keep a native plugin library loaded inside KWin even after unloading the effect. Log out and back in once after replacing the `.so` file. Normal settings changes do not require a logout.

## Ninja says `manifest 'build.ninja' still dirty after 100 tries`

The archive may contain timestamps newer than the local clock. The installer normalizes timestamps automatically. For a manual build:

```bash
find . -path ./build -prune -o -type f -exec touch {} +
rm -rf build
```

## The settings button does not appear

Check that `kwin_proxyx_windtrail_config.so` exists in:

```text
$(qtpaths6 --plugin-dir)/kwin/effects/configs/
```

Then log out and back in.

## Build dependency error

On Fedora 44:

```bash
sudo dnf install \
  gcc-c++ cmake extra-cmake-modules ninja-build \
  qt6-qtbase-devel \
  kf6-kcoreaddons-devel kf6-kconfig-devel kf6-kcmutils-devel \
  kwin-devel libepoxy-devel libdrm-devel
```
