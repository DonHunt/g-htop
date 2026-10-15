# G_htop Community 0.1.0 — final publication readiness

**Verdict: READY for owner review and a separate GitHub publication decision.**

Finalized October 15, 2026. Copyright © 2026 Donald E. Hunt.
Maintainer: Donald E. Hunt <g_htop@don-hunt.com>. License: Apache-2.0.
No remote, push, upload, tag or publication is created by this finalization.

## Resolution of the four publication blockers

1. **Repository hygiene:** `.gitignore` excludes build output, caches, logs, local
   task state, the development launcher and generated release directories. Required
   application source, CMake, icons, documentation and packaging configurations
   remain eligible for tracking. Unused draft headers and the duplicate development
   icon were retained outside the shipping tree; no monitoring code was edited.
   The staged public files were scanned for credentials, private endpoints, local
   paths and binary/oversize content before the initial commit.
2. **Local Git baseline:** branch `main` was initialized with a reviewed 38-file
   source/assets commit. Initial commit: `d2175d781388eae01634af0cc564f94c60784881`.
   Release metadata is finalized in a subsequent local commit; use `git rev-parse
   HEAD` for its identity. This avoids embedding a commit's own hash in its source.
   Final acceptance requires a clean working tree, no remote, and a review of all
   blobs reachable from both commits, rather than checking only the current tree.
3. **Source snapshot:** the single final source archive includes both Stage 10
   documents, all active headers, CMake install rules, build helper, licenses,
   desktop/icon assets, packaging configurations and release documentation.
   The archive must match the final Git export byte-for-byte. The tracked pinned
   `packaging/arch/PKGBUILD` has a documented export-ignore rule because it hashes
   the archive; its template is included and the pinned recipe is a separate
   release asset. No release binaries or archives are tracked in Git.
4. **Copyright consistency:** final project-owned release notices use
   Copyright © 2026 Donald E. Hunt. The complete Apache license and third-party
   attribution remain unchanged. A fresh Arch package is built from the corrected
   source; an old binary package is not edited internally. Package release 2
   distinguishes it from the superseded build; application version stays 0.1.0.

## Final release asset set

- Source: `dist/0.1.0/G_htop-0.1.0.tar.gz`.
- Corrected Arch/Manjaro x86_64 package:
  `dist/0.1.0/g_htop-0.1.0-2-x86_64.pkg.tar.zst`.
- Source-pinned build recipe: `dist/0.1.0/PKGBUILD`.
- Release notes: `dist/0.1.0/RELEASE_NOTES.md` (same content as root release notes).
- Integrity manifest: `dist/0.1.0/SHA256SUMS`, covering the four assets above.

Superseded archives/packages and their old manifests are preserved under
`dist/historical/0.1.0-before-finalization/`. Their abbreviated historical notices
are not represented as current release notices. Do not upload that backup
directory or confuse its checksums with the final manifest.

## Focused final acceptance gates

The completion report is issued only after these gates succeed:

- Reviewed initial and final Git contents; no tracked credentials, workstation
  configuration, caches, build output, archives or binary packages. Every
  reachable historical blob is scanned; local Markdown links resolve.
- Initial and final source snapshots retain all active application files. Final
  working tree is clean on main, and no remote exists. No history was rewritten.
- Final custom source archive equals the final Git export with its documented
  export rule and fixed release timestamp; Stage 10 documents are present.
- Arch package is rebuilt as an ordinary user in a separate build tree. Package
  metadata, source checksum, executable payload, standard paths, desktop entry,
  icon, installed methodology documents, Apache license and full-name NOTICE are
  inspected without installing it.
- SHA256SUMS verifies every final asset. Working executable, installed Cinnamon
  launcher and all active monitoring sources retain their pre-finalization hashes.

The integrity manifest stays outside Git/source to avoid circular hashes.
GitHub-generated source archives have different bytes from the custom archive.
Neither the custom checksum nor its pinned recipe should be reused against them.

## Packaging and remaining limitations

Arch/Manjaro: corrected package built and structurally inspected; no installation
was performed over the working development copy. This does not establish binary
compatibility with older Arch runtimes or other distributions.

DEB (`packaging/deb/`) and RPM (`packaging/rpm/`), using `packaging/cpack/`, remain
**configuration-only, unbuilt and unverified on native targets**. Maintainer
metadata is confirmed; native build, actual dependency checks and installed
launches remain required per target distribution/release. Host-family guards
do not prove cross-release compatibility. No new environments or tools were
installed. Repeatable recipes do not establish byte-identical build reproducibility.

The source/install rules now include Stage 10 documents and a documentation copy
of LICENSE. Source-code links inside installed methodology documents are intended
for source checkouts; packages do not ship C++ source. The build helper now quotes
checkout paths. An optional reviewed screenshot is still absent; README says so.

Linux monitoring limitations are unchanged: per-process GPU/VRAM is unsupported,
some socket owners are unavailable, services cover loaded units, and GPU hot-plug
requires restart. No system-health guarantee or non-Linux/Pro release is implied.

## Publication decision

The repository contains source, documentation, installation assets, packaging
recipes, license notices and this report. The final four assets and checksum
manifest belong in GitHub Releases, outside the source repository. Historical
backups and development state remain local. The original audit remains available
in the initial commit; this report records its resolution.

No additional identity or Community licensing confirmation is requested. Existing
provenance qualifications in LICENSES.md remain; no new third-party permission
or ownership guarantee is asserted. Choose a repository URL and explicitly
approve any remote/publication separately. An annotated v0.1.0 tag may later
identify the final reviewed commit; no tag is created here. Any later source or
release-document changes require refreshing the affected assets/checksums.
