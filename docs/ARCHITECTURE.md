# Architecture

WindTrail is made of two native Qt/KDE plugins:

- `proxyx_windtrail.so`: KWin compositing effect
- `kwin_proxyx_windtrail_config.so`: Plasma configuration module

The effect receives cursor movement from KWin, stores recent position/speed samples, applies Chaikin smoothing, converts the path into variable-width triangle strips, and draws three blended OpenGL passes: halo, body, and bright core.

Settings are stored in `~/.config/kwinrc` under `Effect-proxyx_windtrail`. The configuration module asks KWin to call `reconfigureEffect("proxyx_windtrail")` over D-Bus after Apply, so ordinary setting changes do not require rebuilding or restarting the session.
