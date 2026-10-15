# Arch/Manjaro release build

Use the final PKGBUILD beside G_htop-0.1.0.tar.gz in the release bundle.
PKGBUILD.in is the portable source template; substitute the archive's SHA-256 for
@SOURCE_SHA256@ after creating the archive. Do not put that final PKGBUILD back
inside the archive it hashes. Build with makepkg as an ordinary user, with the
listed build/runtime dependencies already available. No sudo or package install
occurs in build()/package(). The package uses /usr and GNUInstallDirs.

Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.
Copyright © 2026 Donald E. Hunt. License: Apache-2.0.
A public upstream URL has not yet been selected. Nothing has been submitted to the AUR. See the main README for package
installation, upgrade and removal commands and RELEASE_REVIEW.md for the current
publication gate.

The corrected final package is 0.1.0-2. Historical packages are local backups,
not final release assets. The tracked finalized PKGBUILD is excluded by
.gitattributes from the custom source archive to avoid a checksum cycle.
PKGBUILD.in is included; the pinned PKGBUILD is distributed beside the archive.
