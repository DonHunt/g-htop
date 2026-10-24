# G_htop Community Edition v1.1.0

Free, open-source Linux monitoring under the Apache License, Version 2.0.
Copyright © 2026 Donald E. Hunt.
Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.

## Changes

- **About** displays version 1.1.0 and links to https://don-hunt.com/G_htopPro/,
  identifying G_htop Pro as the separately purchased professional edition.
  Community features remain free and unrestricted; no advertising pop-up or activation was added.
- Includes the completed Community GPU compatibility work: automatic AMD/NVIDIA/Intel
  identification and optional read-only NVIDIA NVML telemetry.
- One shared five-minute GPU graph uses distinct per-device colors and a compact legend.
  Hover for detailed temperature, power and VRAM telemetry. Identical-model cards remain
  separate, detection reorder preserves history, and missing counters remain N/A.
- Preserves the general desktop dashboard and detailed tabs. Disconnects notebook routing
  during teardown to resolve the previous shutdown warnings.

## Verified assets and installation

Additional v1.1.0 formats: `g-htop_1.1.0-1_amd64.deb` (Ubuntu 22.04),
`g_htop-1.1.0-1.x86_64.rpm` (Fedora 43), and `G_htop-1.1.0-x86_64.AppImage`
(Ubuntu-built; launched on Manjaro, Ubuntu and Fedora). See the attached
`PLATFORMS.md` for installation, exact validation scope and compatibility notes.
Corresponding AppImage runtime sources and notices are supplied. All formats
use the unchanged v1.1.0 application source and include the Pro About link.

The native binary package is **Arch/Manjaro x86_64**:
`g_htop-1.1.0-1-x86_64.pkg.tar.zst`.
It was built on Manjaro using distribution-provided gtkmm 3 and system libraries.
Fresh install, upgrade from 0.1.0-2, installed launch and removal were checked using
actual pacman transactions in disposable user namespaces with the host runtime/display.
These are not separately booted distribution tests or a compatibility guarantee for
older Arch libraries. No installation over the host application was performed.

Download the package and `SHA256SUMS` from this release, then verify before installing:

```sh
# With all listed assets downloaded:
sha256sum -c SHA256SUMS
# Or verify only the downloaded binary package:
grep '  g_htop-1.1.0-1-x86_64.pkg.tar.zst$' SHA256SUMS | sha256sum -c -
sudo pacman -U ./g_htop-1.1.0-1-x86_64.pkg.tar.zst
# Remove using the package manager; user preferences remain:
sudo pacman -R g_htop
```

Also supplied: `G_htop-1.1.0.tar.gz`, a checksum-pinned `PKGBUILD`, these release notes,
and `SHA256SUMS`. Extract the custom source archive next to the PKGBUILD or build
with `makepkg` using the archive without extracting it first. CMake source build
instructions are in README.md. The custom archive's checksum does not apply to
GitHub-generated source ZIP/tar files. Dependencies and optional hwdata are not bundled.

## Validation and limitations

Release build and 12 native GPU regression scenarios; six GTK GPU scenarios; shared-panel
identity/color/history/missing-data checks; About version/license and real link dispatch;
live dual-AMD dashboard, tab navigation, saved layout and warning-free shutdown.
The Pro website responded with HTTP 200 over HTTPS during release preparation.

NVIDIA and Intel hardware testing was previously confirmed by the owner; current
release checks use synthetic fixtures for those vendors and do not establish universal
vendor, driver or GPU model compatibility. Metrics depend on driver support; missing
whole-device counters remain N/A. Per-process GPU/VRAM attribution is unavailable.
Some socket ownership is inaccessible; services list units loaded by systemd.

DEB and RPM passed fresh installation, baseline upgrade, installed launch/About,
navigation, removal and data-preservation acceptance in their native userspaces.
AppImage passed direct FUSE launch on Manjaro and extract-and-run on Ubuntu/Fedora.
Tests share the host kernel/display; they are not independent booted OS tests.
**No other-architecture or non-Linux binaries are published.** Other distributions
and releases require compatible dependencies; format alone is not a guarantee.
No Pro source tree, activation system, credentials, license keys or private signing
material is included. Community LICENSE is unchanged; Pro is a separate purchase
and its commercial terms do not apply to Community.

Additional format publication preserves the original v1.1.0 tag, custom source
archive and Arch package. Current platform notes supersede the initial bundled
Arch-only status; the documentation follow-up does not change application code.
