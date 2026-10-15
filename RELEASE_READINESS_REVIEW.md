# G_htop Community 0.1.0 — release readiness review

**Verdict: NOT READY for publication in its present form.**

Reviewed October 15, 2026. Maintainer/copyright holder: Donald E. Hunt;
contact: g_htop@don-hunt.com; intended license: Apache-2.0;
notice: Copyright © 2026 Donald E. Hunt.

This was a read-only inspection of the actual tree and existing artifacts. Only
this report was created. No code, documentation, packaging, history, settings,
installation, or release archive was changed; nothing was built or published.
Paths below are project-relative. The verdict concerns publication preparation,
not a newly discovered failure of the operational application.

## Confirmed passing checks

- `LICENSE` is byte-for-byte identical to the complete official Apache 2.0 text
  retrieved from https://www.apache.org/licenses/LICENSE-2.0.txt. Both source
  archives and the Arch package carry this same license text.
- Root `NOTICE`, `LICENSES.md`, README, CHANGELOG, methodology documents, Debian
  attribution, and shared CPack metadata identify Donald E. Hunt and Apache-2.0.
  CPack supplies the approved maintainer contact to Debian metadata and the RPM
  Packager header; RPM declares Apache-2.0. Historical notice exceptions follow.
- All **30 local Markdown links** inspected resolve in the working tree.
  `README.md`, `CHANGELOG.md`, `docs/STAGED_LOCAL_LLM_DEVELOPMENT.md`, and
  `docs/DEVELOPMENT_OVERVIEW.md` describe implemented Linux features and disclose
  telemetry limits. They do not advertise implemented Windows/macOS/iOS or Pro
  editions. DEB/RPM are explicitly unbuilt and unverified.
- The README has conventional CMake build/install/uninstall instructions. The
  active source include graph is complete in both source archives, and all 12
  active source/header files match the working tree. No compilation was rerun.
- `data/org.ghtop.Monitor.desktop` passes desktop-file-validate, uses `Exec=G_htop`
  and `Terminal=false`, and references the packaged SVG. The SVG parses and has
  no linked artwork or script. Root and installed-source SVG copies match.
- Both source archives have safe relative member paths, no symlinks, and no
  build caches, model receipts, or development launcher. Package contents use
  conventional `/usr` executable, desktop, icon, documentation, and license paths.
- All **13 entries** in `dist/0.1.0/SHA256SUMS` verify against the files now on disk.
  Both finalized Arch PKGBUILDs pin the preserved original source archive's hash.
- Pattern scans of text and binary data found no credential/private-key/token
  matches or private-network endpoint matches. Local-path findings are listed
  below; no sensitive values are reproduced. Scans are not a guarantee of absence.
  No existing file exceeds 1 MiB; oversized Git objects cannot be assessed because
  no Git repository exists here.

## Blocking publication issues and recommended corrections

### B1 — Uncurated development tree; no ignore policy

**Affected:** missing `.gitignore`; `build/`; `.codex-acceptance.json`;
`G_htop.desktop`; unused `gpu.hpp`, `monitor.hpp`, `process.hpp`, `services.hpp`,
`types.hpp`; duplicate root `G_htop.svg`.

There is no `.git` directory or enclosing discoverable repository. Git status,
tracked/untracked/ignored classifications, branches, tags, remotes, and historical
commit/object scans are therefore unavailable—not clean or approved. No Git
history exists in this inspected directory to review. A different existing
repository, if one will be used, needs its own full-history disclosure review.

The development launcher and task acceptance file contain workstation-specific
paths; generated `build/` files contain local paths and environment details.
Unused draft headers include incomplete implementation material and should not
be mistaken for the application source. Without ignore rules or a curated
initial commit, a blanket add would expose unsuitable files.

**Correction:** before creating an initial commit, add ignore rules for `/build/`,
`/build-*/`, `/native-dist/`, `/dist/`, `/_CPack_Packages/`,
`/.codex-acceptance.json`, the development-only `/G_htop.desktop`, object files,
logs, and editor/temporary files. Exclude the unused drafts and duplicate icon
from the public selection (or deliberately organize them outside shipping
source). Preserve local files/installation; do not delete them merely to publish.
Stage only an explicit allowlist, then inspect staged content and any actual
history before publication. No ignore file or Git repository was created here.

### B2 — No single final source/tag/package snapshot

**Affected:** `dist/0.1.0/G_htop-0.1.0.tar.gz`;
`dist/0.1.0/additional-linux/G_htop-0.1.0.tar.gz`;
`dist/0.1.0/g_htop-0.1.0-1-x86_64.pkg.tar.zst`;
`dist/0.1.0/RELEASE_NOTES.md`; `dist/0.1.0/PKGBUILD`;
`packaging/arch/PKGBUILD`; `docs/`; `README.md`; `CMakeLists.txt`.

Both archives are named `G_htop-0.1.0.tar.gz` but have different contents/hashes.
The original archive lacks the later CPack configuration and full-name notice;
the additional archive has those updates. **Neither archive contains Stage 10
`docs/`**, and its README predates the current methodology links. Active
application code matches, but release metadata/documentation are not one final
snapshot suitable for an unqualified matching version tag.

The original archive and Arch package NOTICE use the earlier abbreviated
copyright attribution; `dist/0.1.0/RELEASE_NOTES.md` also retains that abbreviated
notice. This is inconsistent with the requested final full-name notice. Those
historical notices were previously approved and intentionally preserved; the
finding does not establish infringement or invalidate the license.

**Correction:** select and document the authoritative v0.1.0 source snapshot and
one unambiguous source asset name. For a consistent final release, authorize a
later archive/package refresh including Stage 10 and the full-name notice,
update the Arch source pin and release notes, and regenerate checksums only
once the final bytes are selected. Alternatively explicitly approve retaining
and labeling the historical package/source pair as such. Do not silently overwrite
those preserved artifacts or reuse their checksum against the newer archive.
The present manifest proves integrity of the current 13 assets, **not** that they
represent the final intended source/tag contents. It does not cover Stage 10.

## Package verification status

| Format / exact path | What is established | Outstanding |
| --- | --- | --- |
| Arch: `dist/0.1.0/g_htop-0.1.0-1-x86_64.pkg.tar.zst` | Existing built x86_64 package; prior build verification recorded. This audit inspected ELF payload, 0.1.0-1 metadata, Apache-2.0, dependency declarations, `/usr` paths, launcher/icon/docs/licenses, and checksum. | Not installed or launched as an installed release package during this audit; final-snapshot/notice decision above. |
| Original source: `dist/0.1.0/G_htop-0.1.0.tar.gz` | Preserved source paired with Arch recipe/package; complete active sources and matching source pin. | Predates additional packaging and Stage 10; abbreviated notices. |
| Additional source: `dist/0.1.0/additional-linux/G_htop-0.1.0.tar.gz` | Full-name notices and approved CPack contact; matches loose additional packaging copies. | No Stage 10; not interchangeable with original Arch source pin. |
| DEB: `packaging/deb/`, `packaging/cpack/` | Configuration only; native shlib dependency discovery enabled, Debian helper names declared. | No `.deb`; native build, generated dependencies, package inspection and installed launch remain unverified. |
| RPM: `packaging/rpm/`, `packaging/cpack/` | Configuration only; ELF dependency scanning enabled, RPM helper names and license/contact declared. | No `.rpm`; native build/spec, dependencies and installed launch remain unverified. |

CPack host-family guards reject unsupported/mismatched families; they do not
prove compatibility across releases within a family. Do not attach Manjaro
binaries as Debian/Fedora releases. No native target environment was added.

## Nonblocking improvements

- **Installed documentation links:** `CMakeLists.txt` installs README but not
  `docs/`. A future package built from the current tree would install README
  links without their local targets. Install both documents under the matching
  relative `docs/` location or use approved public repository links once selected.
- **Helper portability:** `build.sh` leaves directory expansions unquoted, so a
  checkout path containing spaces can fail. Prefer documented CMake commands or
  later quote the helper's path handling; do not characterize this as an observed
  application compile failure.
- **Screenshots:** `README.md` has no screenshots/placeholders and no screenshots
  exist in this tree. Add an optional dashboard image after reviewing process
  names, endpoints, usernames, and other visible data; do not reuse raw private
  workstation screenshots automatically.
- **Reproducibility:** recipes and SOURCE_DATE_EPOCH exist; no locked distribution
  snapshot/toolchain or byte-identical rebuild result exists. Describe repeatable
  recipes separately from proven bit reproducibility.
- **Dependencies/artwork:** no bundled third-party libraries/model weights or
  linked artwork were found. Dynamic dependencies and external helpers retain
  upstream licenses. `LICENSES.md` appropriately qualifies its provenance review;
  it cannot establish originality or third-party permissions through a global
  similarity audit. No unsupported guarantee should be added.

## Suggested initial repository and v0.1.0 release

**Repository allowlist:** `main.cpp`, `connection_data.hpp`,
`connections_view.hpp`, `cpu_panel.hpp`, `dashboard.hpp`, `gpu_data.hpp`,
`gpu_history.hpp`, `gpu_summary.hpp`, `presentation.hpp`, `service_data.hpp`,
`services_view.hpp`, `status_strip.hpp`; `CMakeLists.txt`; `data/`; `packaging/`;
`docs/`; README, CHANGELOG, LICENSE, NOTICE, LICENSES, RELEASE_REVIEW, this report,
and a reviewed `.gitignore`. Include `build.sh` only after deciding the helper
correction. Keep the finalized Arch checksum-pinned recipe explicitly paired to
its selected source. No binaries, caches, task receipts, generated CPack builds,
model drafts, or workstation launcher belong in the initial commit.

**GitHub Releases only:** the selected source archive, reviewed Arch package,
matching checksum-pinned PKGBUILD, release notes, and final SHA256SUMS. DEB/RPM
recipes already belong in source; do not list nonexistent packages as binaries.
Keep superseded archives local or explicitly label historical assets to avoid
same-name ambiguity. GitHub-generated source snapshots are different bytes from
the custom archive and must not inherit its checksum.

**Tag plan:** after approved cleanup and final asset selection, create a reviewed
initial commit containing all ten stages' intended source/docs, then an annotated
`v0.1.0` tag pointing to that exact commit. Build later authorized refreshed assets
from that snapshot and inspect the final manifest before a separate publication
approval. No commit, tag, remote, or publication was created by this review.

## Maintainer confirmations

Confirm the authoritative final archive/tag contents and whether historical
abbreviated-notice artifacts may be publicly retained or should be superseded.
Confirm that no known external code/artwork or contributor rights are omitted
from `LICENSES.md`; identity, contact and Community Apache-2.0 instructions are
already confirmed and need not be re-authorized. Select a public repository
location and approve publication separately. Native DEB/RPM compatibility is
still an evidence gap, not a fact to confirm by assertion.
