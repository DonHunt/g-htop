# G_htop Community changelog

## 1.1.0 — 2026-10-24

- Added a conventional About dialog with the Community version, Apache-2.0 license,
  and a G_htop Pro website link identifying the separately purchased professional edition.
- Retained all free Community features, desktop layout, filters, sorting and detailed tabs.
- Included the completed Community AMD/NVIDIA/Intel discovery and optional NVML work.
- Consolidated GPU utilization into one graph with distinct device colors and a compact
  legend; full telemetry remains on hover. Same-model cards keep separate histories.
- Preserved unaffected GPU histories across detection reorder and card add/remove;
  missing readings stay N/A and replaced cards do not acquire previous history.
- Disconnected notebook routing before child teardown to stop the existing shutdown warnings.
- Updated source/package version metadata and current release documentation to 1.1.0.
- Preserved the Apache License, Version 2.0. No Pro application or licensing modules ship.

### Additional v1.1.0 package formats

- Added independently built Ubuntu 22.04 DEB and Fedora 43 RPM binaries.
- Added a GTK/gtkmm AppImage built on Ubuntu 22.04 and tested on Manjaro, Ubuntu and Fedora.
- Verified native fresh installs, baseline upgrades, installed launches and removals;
  retained the original application tag/source and Arch binary unchanged.
- Added platform notes, corresponding AppImage runtime sources and extended checksums.
- Updated current download documentation; other architectures/non-Linux remain unsupported.

## 0.1.0 — initial release

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
