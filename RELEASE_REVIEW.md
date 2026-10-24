# G_htop Community 1.1.0 — release review

Copyright © 2026 Donald E. Hunt. License: Apache-2.0.
Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.

The owner explicitly authorized committing, pushing and publishing v1.1.0 to
https://github.com/DonHunt/g-htop. The Community license text is unchanged.

Scope: existing Community source plus the reviewed Community GPU work, a conventional
About link to the separately purchased Pro edition, version/package/documentation
updates and a small notebook teardown correction. No commercial Pro application,
licensing infrastructure, credentials, private keys, engineering state, delegation
logs or unrelated workstation files belong in the public source or package.

The source export is curated from tracked files. The final Arch PKGBUILD is
export-ignored to avoid hashing the archive that includes it; its template is
inside the source and the pinned recipe ships beside the archive. The archive,
package, release notes and pinned recipe are covered by SHA256SUMS.

Acceptance requires native regression tests, GPU/About GTK checks, a live desktop
launch, actual isolated package install/upgrade/removal, final diff/content review,
and verification of the published commit/tag and downloaded release assets.
Only the Manjaro-tested x86_64 Arch-format binary is in scope. DEB/RPM source
recipes remain unverified; no AppImage or other platform binary is advertised.
