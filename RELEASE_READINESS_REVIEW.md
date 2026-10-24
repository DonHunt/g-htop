# G_htop Community Edition 1.1.0 — release acceptance

Owner authorization covers source commit/push and the v1.1.0 GitHub release at
https://github.com/DonHunt/g-htop. The release does not depend on a Pro purchase.

## Acceptance scope

- Release CMake build; 12 native GPU regression scenarios with assertions enabled.
- Six GTK GPU fixture scenarios; shared graph same-name identity, reorder,
  add/remove/replacement, stable distinct colors, missing/NaN and empty inventory.
- About version, unchanged Apache-2.0 indication, paid Pro label and real link
  dispatch to https://don-hunt.com/G_htopPro/; HTTPS endpoint returns HTTP 200.
- Live dual-AMD dashboard, six tab switches, saved layout and clean shutdown.
- Fresh pacman installation, 0.1.0-2 upgrade and removal in disposable user
  namespaces using the current host runtime/display. Preserve user configuration
  and unrelated files; inspect all payload hashes, dependencies and desktop metadata.
- Review final diff, source export and binary package for unrelated code, Pro
  modules, credentials/private material, local development state and build output.
- Verify source archive against Git export; pinned recipe source hash; release
  SHA256SUMS; final commit/tag; and downloaded GitHub release assets.

## Assets

`G_htop-1.1.0.tar.gz`, `g_htop-1.1.0-1-x86_64.pkg.tar.zst`, `PKGBUILD`,
`RELEASE_NOTES.md`, `SHA256SUMS`.

LICENSE remains byte-identical to the Apache-2.0 baseline. LICENSES.md and NOTICE
retain attribution and distinguish the free Community edition from commercial Pro.

## Limits

Only Manjaro x86_64 binary acceptance is claimed. User namespaces are not booted
or independent distro tests; older rolling libraries may be incompatible. Owner
reported NVIDIA/Intel hardware testing precedes this release; current fixture
checks are not broad physical certifications. Telemetry depends on driver support.
No per-process GPU/VRAM attribution; socket ownership can be inaccessible and
systemd service discovery covers loaded units. DEB/RPM are source-only; no
AppImage, other-architecture or non-Linux binary is offered. No bit-for-bit
native rebuild or system-health guarantee is made.
