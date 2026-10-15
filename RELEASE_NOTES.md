# G_htop Community 0.1.0 — final local release candidate

Copyright © 2026 Donald E. Hunt.
Maintainer: Donald E. Hunt <g_htop@don-hunt.com>. License: Apache-2.0.

Read-only Linux monitoring: Processes, system/user Services, CPU Threads,
TCP/UDP Connections, AMD GPU/VRAM, five-minute GPU history, and a resizable
four-panel Dashboard. F11/Escape, theme integration, saved layouts and
secondary-monitor operation are retained. Both Stage 10 development-methodology
documents are included. No application monitoring code was changed for this
finalization.

Final artifacts: G_htop-0.1.0.tar.gz, g_htop-0.1.0-2-x86_64.pkg.tar.zst,
PKGBUILD, RELEASE_NOTES.md and SHA256SUMS. Check integrity with
`sha256sum -c SHA256SUMS`. The package release increment corrects notices and
documentation; the application version remains 0.1.0. The source archive
corresponds to the reviewed Git source export; the finalized PKGBUILD is
separate to avoid hashing itself. GitHub-generated source archives have different
bytes and must not reuse the custom archive checksum.

Arch/Manjaro x86_64: built as an ordinary user and inspected without installation
over the development application. After separate review, install/upgrade with
`sudo pacman -U ./g_htop-0.1.0-2-x86_64.pkg.tar.zst`; remove with
`sudo pacman -R g_htop`. Development executable and launcher remain unchanged.

DEB/RPM: source/configuration only, NOT BUILT or target-verified. Follow each
native packaging README on its intended distribution/release. No Manjaro-built
binary is offered as a Debian/Fedora package. Native dependency and installed
launch checks remain necessary.

Known limits: no per-process GPU/VRAM attribution; some socket owners unavailable;
service discovery covers loaded units; GPU hot-plug requires restart. No universal
system-health or bit-reproducible build claim is made.

Superseded artifacts under dist/historical are backups, not this release's assets.
No public repository, upload, push, package installation or publication has
occurred. Publication requires a separate owner decision.
