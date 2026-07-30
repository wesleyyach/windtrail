# Testing report template

- Distribution:
- Architecture:
- Plasma version:
- KWin version:
- Qt version:
- Session type:
- GPU:
- Driver:
- Monitor count / layout:

## Results

- Build: pass / fail
- Install: pass / fail
- Effect loads: pass / fail
- Settings module opens: pass / fail
- Settings apply live: pass / fail
- Multi-monitor movement: pass / fail
- Full-screen suppression: pass / fail
- Screen lock/unlock: pass / fail
- Uninstall: pass / fail

## Notes / logs

Use:

```bash
./scripts/verify.sh
journalctl --user -b --no-pager | grep -iE 'windtrail|proxyx_windtrail|kwin'
```
