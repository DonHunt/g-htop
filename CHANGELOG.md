# G_htop Community changelog

## 0.1.0 — unpublished release candidate

- Read-only process CPU/RAM, system/user systemd services, and TCP/UDP sockets.
- Logical CPU meters and AMD GPU/VRAM telemetry with five-minute GPU history.
- Four-panel resizable dashboard, detailed tabs, sorting and filtering.
- F11/Escape display modes, secondary monitors and persistent divider layouts.
- Native GTK theme, readable numeric values, device tooltips and status strip.
- CMake installation, freedesktop launcher/icon and Arch/Manjaro packaging.
- Apache-2.0 Community release; copyright holder Donald E. Hunt, with the
  requested notice Copyright © 2026 Donald E. Hunt.

Known limits: per-process GPU/VRAM attribution is unavailable; socket ownership
can be inaccessible; service discovery covers units loaded by systemd; GPU device
hot-plug requires restarting. This release supports Linux only.

### Additional Linux packaging preparation

- Added native-build CPack DEB/RPM configurations with automatic library
  dependency scanning and host-family checks; target builds remain unverified.
- Updated new distribution notices to Copyright © 2026 Donald E. Hunt.
- Preserved the original verified Arch package and matching source archive.

### Publication finalization

- Added repository hygiene and a curated local main-branch source snapshot.
- Included Stage 10 documentation in source and installed documentation.
- Standardized G_htop notices and confirmed maintainer contact.
- Corrected Arch packaging release to 0.1.0-2 without changing application code.
- Consolidated final source/assets; retained superseded artifacts as local backups.
- DEB/RPM remain unbuilt and target-unverified. No publication has occurred.
