# Licensing and attribution audit — 1.1.0

## Project materials

G_htop Community is licensed under Apache-2.0; the complete unmodified license
is in LICENSE. Copyright holder: Donald E. Hunt. The requested attribution is
“Copyright © 2026 Donald E. Hunt.” The project owner explicitly supplied this holder
identity, attribution and license instruction for the Community release. They
were not inferred from an account name, filesystem path or model name. No CLA or
trademark policy is introduced. The owner explicitly authorized the v1.1.0 Community GitHub release.

The audit covers main.cpp, its recursively included project headers, CMake and
packaging files, documentation, desktop metadata and data/icons/G_htop.svg.
Unused draft headers, development receipts and build artifacts are excluded.
Available development records identify local Qwen-generated source plus
coordinator bindings and corrections. The SVG exactly matches the original local
model generation receipt; it has geometric SVG paths and no linked images,
fonts, scripts or other embedded external assets. The source contains no existing
third-party copyright headers or source-license notices that were removed.

This is a provenance and included-file review, not a global similarity search or
a guarantee that generated output has no third-party obligations. No model
weights, inference runtime, copied theme icons, vendored libraries or external
image assets are bundled. The project owner has instructed Apache-2.0 licensing
for this Community distribution; no independent third-party rights guarantee is
asserted by this audit.

## System dependencies (not bundled)

G_htop links dynamically to distribution-provided gtkmm/GTK and their supporting
libraries. Installed package metadata identifies GTK, gtkmm, glibmm and pangomm
as LGPL-2.1-or-later, and cairomm as LGPL-2.0-or-later. Other supporting libraries
and compiler runtimes retain their upstream terms, including applicable runtime
exceptions. The Arch package declares dependencies; it does not ship their
libraries or replace their license files. Distribution package metadata remains
the authority for the specific library versions installed by a recipient.

systemctl (systemd), ss (iproute2), timeout (coreutils), and optional NVIDIA NVML
(driver library) are supplied by the host, not copied into this release. Native GTK theme and symbolic icons remain supplied by the desktop.

## Separate Pro edition

G_htop Pro is a separately purchased professional edition linked from About.
No Pro source tree, subscription/activation modules, entitlement format, customer
license material or private signing keys are included. This release adds the
reviewed Community GPU adapter and tests under the same Community Apache-2.0
license. Optional NVML declarations describe the public read-only ABI; NVIDIA
libraries, SDKs, headers and drivers are not redistributed. Their upstream terms
remain separate. The About link does not impose commercial terms on Community.
No non-Linux edition is implemented.

References: https://www.apache.org/licenses/LICENSE-2.0.txt
