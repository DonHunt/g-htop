# Arch/Manjaro release build

Use the final PKGBUILD beside `G_htop-1.1.0.tar.gz` from the v1.1.0 GitHub Release:
https://github.com/DonHunt/g-htop/releases/tag/v1.1.0

Verify `SHA256SUMS`, then run `makepkg` as an ordinary user with the declared
build/runtime dependencies already installed. No sudo or package installation
occurs in build(), check() or package(). Native regression checks run in check().
The package is `g_htop-1.1.0-1-x86_64.pkg.tar.zst` and uses /usr and GNUInstallDirs.

`PKGBUILD.in` is the portable template; replace @SOURCE_SHA256@ after creating
the custom source archive. The pinned `packaging/arch/PKGBUILD` is excluded from
Git source exports to avoid a checksum cycle. The finalized recipe is a separate
release asset; do not reuse its hash against GitHub-generated source archives.
Nothing has been submitted to the AUR. See README.md for installation/removal.

Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.
Copyright © 2026 Donald E. Hunt. License: Apache-2.0.
