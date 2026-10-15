# G_htop Community 0.1.0

A read-only Linux process, service, and AMD GPU monitor using C++17 and gtkmm 3. It follows the desktop GTK theme.

Launch **G_htop** from the Cinnamon application menu. In **Processes**, click Name, PID, CPU, or Memory to sort; click again to reverse. Process measurements refresh about every two seconds. One fully occupied logical core equals 100% CPU; the first sample initializes counters. Memory is resident memory, including shared pages.

In **Services**, choose All, Active, Inactive, or Failed. Click Description, Unit, Status, or Scope to sort. Both the system manager and your user manager are queried in the background about every five seconds. MainPID is the service's main process when available; N/A is normal for inactive and completed oneshot services. Status displays systemd's reported active state, including activating and deactivating when observed.

Discovery covers loaded service units, including loaded inactive units; installed units that systemd has not loaded are not enumerated. Aliases are deduplicated by canonical unit name within each scope. Query failures appear below the table; no root access is required. There are no service-management or process-termination controls. 

The GPU summary above both tabs refreshes about every two seconds. It detects amdgpu devices through `/sys/class/drm`, showing each device separately with its name (when discoverable), GPU utilization, and used/total/available VRAM in MiB or GiB. Telemetry comes from `gpu_busy_percent`, `mem_info_vram_used`, and `mem_info_vram_total`; available VRAM is total minus used. Unreadable values display N/A. Device discovery and collection run in a background worker; no root access or GPU controls are required. GPU names use the already-installed `lspci` utility, with a generic fallback. Per-process GPU utilization and VRAM attribution are not implemented and display N/A. Restart the application after adding or removing a GPU.

The **CPU Threads** tab shows a wrapping grid of logical CPU meters. It reads `/proc/stat` approximately every two seconds and computes utilization from successive counter differences (idle includes I/O wait; guest time is not double-counted). The first sample and unavailable/reset counters show N/A. Bars are green at 0–33%, yellow above 33% and below 67%, and red at 67–100%; text and other controls keep the current GTK theme. Resize the window to change the number of meter columns; the panel scrolls for large CPU counts.

The **Connections** tab lists TCP, TCP6, UDP, and UDP6 sockets in the current network namespace, including local/remote addresses and ports, socket states, and accessible owners. It queries the installed `ss` utility in a background worker approximately every two seconds without DNS lookups or root access. Filters are All, Listening (TCP LISTEN), Established (ss ESTAB, including connected UDP), and UDP. Click headings to sort; PIDs and ports sort numerically. Unknown/N/A ownership is normal for inaccessible processes and sockets without a live owner. Shared sockets may have one row per distinct owning PID; process/socket exits can race with collection. It has no disconnect, termination, or firewall controls.

The **GPU History** tab plots the same AMD utilization samples used by the summary, approximately every two seconds. Its fixed 0–100% scale spans the last five minutes, with relative time markings from -5m to Now. History starts empty at launch and stays in memory only; unavailable readings show N/A and break the line. The GTK/Cairo `GpuHistory` widget accepts sample packets through `append_sample()` for reuse in a dashboard and has no telemetry timer of its own.

The initial **Dashboard** view combines Processes and Services on the left, with GPU History/VRAM and CPU Threads on the right. Drag the middle divider to resize the columns; each column has its own independent horizontal divider. Process and service tables scroll inside their panels. The original detailed tabs remain available and share the same widgets and sampling workers; filters, sorting, and history carry between views.

Press **F11** or **Full Screen (F11)** to toggle full-screen Dashboard; **Escape** leaves full-screen. Move the normal window to another monitor using Cinnamon's usual window controls before entering full-screen. G_htop does not change monitor settings. Normal window dimensions and divider proportions are saved on close in `~/.config/G_htop/layout`; restoration clamps the layout to available space. **Reset Layout** balances the three dividers. The bottom dashboard strip stays outside the resizable panels and displays `/proc/uptime`, the number of failed services in the current system/user discovery cache, and detected service/GPU/VRAM availability issues. A count of zero is not a general system health assessment.

Presentation follows the active GTK theme, with one-decimal percentages and MiB/GiB values, right-aligned numeric columns, and numeric sorting independent of display formatting. Hover over the compact GPU summary for full device identification. Long process and service descriptions use ellipses and full-name tooltips; Services places state and scope near the description, with further fields available by horizontal scrolling. CPU meters wrap more compactly. The status strip uses native informational/warning icons; a warning indicates a detected monitoring condition, not a general system health assessment.

## Linux requirements

G_htop 0.1.0 supports Linux with readable `/proc`, C++17, CMake 3.16+, pkg-config,
and gtkmm 3 (GTK 3). System/user services use `systemctl` from systemd; network
connections use `ss` from iproute2; bounded helper calls use `timeout` from
coreutils. AMD telemetry requires an accessible amdgpu DRM/sysfs device.
`lspci` from pciutils is optional for descriptive GPU names. No root access is
needed for ordinary monitoring. No Windows, macOS or iOS edition is provided.

On Arch/Manjaro, install build prerequisites through your normal package manager:
`base-devel cmake pkgconf gtkmm3`. Runtime package dependencies are declared in
`packaging/arch/PKGBUILD.in`; pciutils is optional. The desktop uses its existing
GTK theme and icon infrastructure.

## Build and install from source

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --parallel 2
./build/G_htop
# Optional installation, performed only when you choose to install:
sudo cmake --install build
```

The installation prefix is configurable. Standard installs place the executable
in `bin`, the launcher in `share/applications/org.ghtop.Monitor.desktop`, the
project icon in `share/icons/hicolor/scalable/apps`, documentation in
`share/doc/G_htop`, and LICENSE/NOTICE in `share/licenses/g_htop`, all under the
chosen prefix. Add a nonstandard prefix's `bin` directory to PATH before using
the launcher. `DESTDIR` staging is supported without root.

To uninstall a manual CMake installation, retain that build directory and review
its `build/install_manifest.txt`, then remove exactly the recorded files:

```sh
sudo xargs -r -d '\n' rm -f -- < build/install_manifest.txt
```

This preserves parent directories and your `~/.config/G_htop/layout`. Do not use
a manifest from a different installation. Prefer the package manager for packaged
installations.

## Arch/Manjaro package

The local release bundle contains the source archive, an installable x86_64
package, a checksum-pinned PKGBUILD, SHA256SUMS and release notes. From that
bundle, verify checksums and optionally rebuild without root or automatic
dependency installation:

```sh
sha256sum -c SHA256SUMS
makepkg
```

The source archive includes the PKGBUILD template. Its `@SOURCE_SHA256@` token is
replaced with the completed archive's hash in the adjacent final PKGBUILD; the
final PKGBUILD is outside the source archive to avoid a self-referential hash.
The source is a local file; no unpublished repository URL is invented.

Installation and upgrades (not performed during release preparation):

```sh
sudo pacman -U ./g_htop-0.1.0-2-x86_64.pkg.tar.zst
# Future upgrade: use pacman -U with the newer reviewed package.
# Removal:
sudo pacman -R g_htop
```

Do not manually delete package-managed files. User layout preferences are kept on
removal. The packaged desktop ID is `org.ghtop.Monitor`; an existing development
launcher with another ID is not overwritten. Both menu entries may be named
G_htop. Close a running development instance before launching an installed copy,
since GTK's application ID is shared. This preparation does not install over the
development copy or change its launcher.

## License and release review

Copyright © 2026 Donald E. Hunt. Copyright holder: Donald E. Hunt.

G_htop Community is licensed under Apache License, Version 2.0; see LICENSE and
NOTICE. Dependencies retain their own licenses (LICENSES.md). The copyright
holder and Community license were explicitly confirmed by the project owner.
This is an unpublished local review build; publication requires separate
approval (RELEASE_REVIEW.md). A future separately developed G_htop Pro edition is
not included, and no license for future Pro code is specified by this release.
No other-platform edition is included.

## Additional DEB/RPM release preparation

Native DEB and RPM configurations and build instructions are in `packaging/cpack`,
`packaging/deb` and `packaging/rpm`. These targets are **not built or verified**.
No target-distribution environment is available on the development workstation.
Maintainer: Donald E. Hunt <g_htop@don-hunt.com> (the shared CPack default).
Build on each intended distribution/release;
a CPack guard rejects packaging a Manjaro-configured binary as DEB/RPM. Native
ELF scans determine actual library dependencies. No containers/toolchains have
been installed and no package has been installed over the development app.

The final release assets are in `dist/0.1.0`: one source archive, the corrected
Arch/Manjaro 0.1.0-2 package, its checksum-pinned PKGBUILD, release notes and
SHA256SUMS. Source contains both Stage 10 documents and all native packaging
configurations. Superseded artifacts are preserved locally under
`dist/historical/` and are not intended for publication.

The Git repository contains source and packaging recipes, not generated release
archives or binaries. `.gitattributes` excludes the finalized Arch PKGBUILD from
the source export because it hashes that archive; its template remains inside.
Use the pinned recipe supplied beside the final archive to reproduce the package.

## Development methodology

The [development overview](docs/DEVELOPMENT_OVERVIEW.md) summarizes the supervised,
staged local-Qwen workflow used for G_htop Community 0.1.0. The
[full technical case study](docs/STAGED_LOCAL_LLM_DEVELOPMENT.md) covers all ten
stages, implementation evidence, acceptance criteria, corrections, limitations,
and a reusable task template. These documents describe the observed process;
they do not establish comparative model performance.

## Screenshots

A reviewed dashboard screenshot may be added before publication. No screenshot
is bundled yet; raw workstation captures should be checked for private data.
