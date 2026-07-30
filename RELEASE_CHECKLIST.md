# WindTrail 1.0 release checklist

## Build

- [x] Clean configure completes without WindTrail-specific warnings
- [x] Both `.so` modules build
- [x] Install goes to the Qt 6 plugin directory
- [x] Install script works from a freshly extracted archive
- [ ] Uninstall script tested from a clean installation

## Runtime

- [x] Effect loads after a clean login
- [x] Wind White preset works
- [x] Crimson Slash preset works
- [x] Ice Blue preset works
- [x] Custom color works
- [x] Apply updates settings without recompilation
- [x] Defaults button restores defaults
- [ ] Full-screen disablement tested across multiple applications
- [ ] Locking/unlocking tested for artifacts
- [ ] Cross-monitor behavior formally recorded
- [x] No visible trail remains after the cursor stops

## Publication

- [ ] Create public repository at `wesleyyach/windtrail`
- [ ] Upload this source tree
- [ ] Create GitHub release `v1.0.0`
- [x] Add two screenshots
- [ ] Capture one or two GIF demonstrations
- [x] Create final `1.0.0` archives and SHA-256 checksums
- [ ] Publish KDE Store entry under KWin Effects
