# G_htop Community 0.1.0 — publication review

Copyright © 2026 Donald E. Hunt.
Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.
License: Apache-2.0; LICENSE is the complete unmodified license text.

The owner confirmed identity, contact, copyright and Community licensing.
NOTICE and LICENSES.md record attribution, provenance and its limits; upstream
dependency licenses remain unchanged. A separately developed future Pro edition
and non-Linux implementations are outside this release. No CLA or trademark
policy is introduced.

## Final local release

The final source snapshot includes both Stage 10 documents, standard CMake
installation rules, desktop/icon assets, notices and Arch/DEB/RPM configurations.
The corrected Arch package is 0.1.0-2; the application version remains 0.1.0.
Final assets live in dist/0.1.0. Superseded packages and archives are retained in
dist/historical and must not be presented as the final release.

DEB and RPM remain configuration-only: no native build, package installation or
target launch has been verified. Their automatic dependency scans and host-family
guards do not establish cross-release compatibility. No target environments or
extra toolchains have been installed.

The existing development executable and Cinnamon launcher are preserved.
Release packaging is staged in a separate build tree and is not installed over
the working application. See RELEASE_READINESS_REVIEW.md for acceptance gates.

## Remaining publication decision

The local repository has no remote and nothing has been uploaded or published.
Select the intended public repository location and explicitly approve publication
separately. Any future edits require refreshing the affected assets/checksums.
No byte-identical native rebuild claim is made.
