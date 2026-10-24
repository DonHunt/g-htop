# G_htop Community 1.1.0 — Windows port progress

## Checkpoint 2026-10-24T04:11:27-04:00

- Branch: `windows/community-1.1.0`, based on Linux/documentation main `9362905`.
- State: initial inspection complete; Windows development has not yet been built.
- Existing Linux v1.1.0 release assets and tag are preserved. No production-branch push or Windows publication is authorized by a checkpoint.
- Publication gate: STOP at owner Windows 11 native validation handoff. Do not upload Windows binaries to GitHub Releases until the owner confirms native testing and approves publication.

### Completed

- Inspected C++17/CMake/gtkmm3 UI and Linux-specific process, CPU, GPU, service, network and uptime collectors.
- Confirmed host Wine is available; no MinGW cross-compiler is installed on the host.
- Established this separate review worktree.

### Current task

Establish an isolated, reproducible MinGW/official MSYS2 GTK build before implementing native monitoring behind the existing UI.

### Remaining

1. Resolve Windows gtkmm/runtime packages and install cross-tools inside the existing isolated Ubuntu build userspace.
2. Separate shared models/UI from Linux monitoring implementations; preserve Linux behavior.
3. Implement Win32 process/SCM/CPU/DXGI+PDH/network/uptime backends with unavailable metrics represented honestly.
4. Build and regression-test both platforms.
5. Bundle DLLs, GTK resources, icons, Apache/third-party notices and source archive without developer paths.
6. Test the portable build under an isolated Wine prefix; record native-only validation requirements.
7. Finish BUILD-WINDOWS.md and WINDOWS_RELEASE_NOTES.md, review diff/security/licensing, prepare review branch/commit and native-testing handoff.

### Latest successful build/test

Inherited Linux v1.1.0 release previously passed native and GTK tests; no Windows build/test yet.

### Known limitations/blockers

No native Windows 11 test system is connected. Wine can validate UI/package loading, not physical Windows GPU/CPU telemetry accuracy. Native testing is an explicit final handoff gate.

### Next checkpoint

Due no later than 2026-10-24T04:31:27-04:00 during active work. Save all source/docs, latest tests, current step and blockers; create a coherent Git checkpoint where appropriate.

### Resume instruction

Read this file, git status/log, and `../evidence/` results first. Continue from the recorded current task; do not restart completed work or publish Windows binaries. Preserve Linux assets and existing background integrations.

## Checkpoint 2026-10-24T04:29:57-04:00

- Completed: separated shared models/UI from Linux collectors; added native Windows Toolhelp/process times/memory, SCM services, IP Helper TCP/UDP IPv4/6, processor-group/PDH CPU, DXGI+PDH GPU, uptime and portable GTK runtime setup.
- Successful: Linux application and all existing tools built; all 12 Linux CTest cases passed. Official matching MSYS2 UCRT64 GCC 16.2 / gtkmm 3.24.11 Windows application and 3 test tools built under isolated Wine. Windows collector smoke passed, including a controlled TCP listener and UDP socket, process self-enumeration/memory/CPU, services and GPU identity parsing.
- Portable staging: `../evidence/candidate-03` (~74 MiB), complete PE import closure and 14 relative image loader modules. Application launches without development WINEPATH. Not a final artifact yet.
- Current task: investigate Wine text rendering; complete shared graph/About tests, UI navigation/resizing/shutdown and moved-folder tests. Collect corresponding MSYS2 runtime sources and finish docs/package manifests/checksums.
- Known issue: first Wine Win32 font screenshot has damaged glyph rendering. Testing the supported Pango fontconfig backend inside isolated Wine. Native Windows graphics and telemetry remain unvalidated.
- Toolchain decision: Ubuntu GCC 10/MSVCRT cross-tool trial rejected due header/runtime ABI mismatch. Final Windows builds use matching official MSYS2 UCRT64 Windows GCC/CMake/Ninja under Wine. Trial artifacts are outside Git and will not ship.
- Remaining: finish UI verification and source/license audit, Linux GTK regression, final clean build/package, source archive tied to review commit, final native Win11 handoff. No production push, tag mutation or Windows binary publication.
- Evidence: `../evidence/native-windows-build.log`, `native-collectors-network.log`, `linux-build.log`, `staging.log`, `finalize.log`, `windows-dashboard.png`; reproducible local environment helpers in `../evidence/`.
- Resume: read the latest checkpoint, git status/log and evidence. Continue the current task; do not redo completed toolchain setup. Do not publish Windows binaries before owner native test approval.
- Next checkpoint due no later than 2026-10-24T04:49:57-04:00 during active work.

## Native-validation handoff checkpoint 2026-10-24T04:47:07-04:00

### Completed and verified

- Native Windows UCRT64 application and all focused test executables build successfully with official MSYS2 GCC 16.2.0, gtkmm 3.24.11 and GTK 3.24.52. Dependency binary/source hashes and Rust metadata are committed in packaging/windows/dependencies.json.
- All three focused Windows tests pass from the staged runtime with development WINEPATH removed: native collectors/controlled TCP+UDP/GPU aggregation; shared multi-GPU graph identity, colors, hot replacement, N/A and zero; About v1.1.0, Apache text, paid-Pro distinction and actual link activation URI.
- Three final launches and normal window-manager shutdowns pass from a moved folder containing spaces and Édition, with /tmp as cwd. Dashboard, divider drag/reset, all tabs, resizing, F11 fullscreen toggle and About are visually checked. Relative image cache and all DLL imports verified; 55 runtime DLLs and 14 image modules staged.
- Pro URL returned HTTP 200 and correct Pro page title.
- Linux Release build and all 12 collector CTest cases pass; focused shared graph and About/link GTK regression also pass. LICENSE SHA256 is unchanged from Linux main.
- Matching MSYS2 runtime source packages (47 binaries, 46 source archives), locked vendor sources for 361 librsvg Rust crates, notices and source hashes are prepared in the separate Windows-runtime-sources archive. No compiler, MSYS runtime DLL, proprietary Pro code, credentials or signing keys are shipped.
- Diff and generated-artifact/credential checks completed. Current source is review-only; main, v1.1.0 and all 11 Linux release assets remain unchanged.

### Artifacts and evidence

Final deliverables are under ../artifacts/: Windows x64 ZIP, exact-source ZIP, Windows runtime-sources tar.xz, BUILD-WINDOWS.md, WINDOWS_RELEASE_NOTES.md, BUILD-INFO-WINDOWS.json and SHA256SUMS. The build manifest records the exact review commit; the source ZIP is git-archived from that commit. Git review branch: windows/community-1.1.0. A draft review may be opened, without merging or binary publication.

Evidence under ../evidence/: native-windows-final-build.log, final-tests.json and final-*.log, ui-results.json, final-ui-qa.log, qa-*.png, linux-final-build.log, linux-gtk-final.log, runtime-manifest.json and source/vendor logs. Local isolated build helpers and complete verified package caches remain there for continuation.

### Remaining / native-only gate

Owner must test on Windows 11 x64 using WINDOWS_RELEASE_NOTES.md checklist: fonts/scaling, real CPU/process/service/network accuracy, each physical GPU and distinct lines under load, driver VRAM availability, multiple monitors and native keyboard/close behavior. Wine cannot establish those results. Wine Win32 font rendering required the documented test-only fontconfig setting; simulated Escape/Alt+F4 was unreliable, while normal window-manager closing passed.

STOP HERE when artifacts/checksums and review branch are verified. No Windows binary has been uploaded to GitHub Releases. Do not merge main, move the existing v1.1.0 tag, or publish the Windows package until native owner testing succeeds and publication is explicitly approved. A Windows-specific source tag and release-asset update remain gated; keep the Linux tag/assets intact.

### Resume instruction

Read this latest checkpoint and BUILD-INFO-WINDOWS.json, check Git status/log and SHA256SUMS, then incorporate the owner's native Windows results. Reuse the prepared artifacts and matching build environment; do not restart the port. Fix reported defects, rerun relevant checks, and preserve the publication gate. No timer checkpoint is required while stopped at the native testing handoff; if implementation resumes, checkpoint at least every 20 minutes of active work.

## Final archive/review verification 2026-10-24T04:50:15-04:00

Draft PR: https://github.com/DonHunt/g-htop/pull/1 (open, unmerged). The review branch is pushed; production main and the original v1.1.0 tag remain at their original commits. Windows artifacts remain local under ../artifacts/. All six SHA256SUMS entries, both ZIP integrity checks, runtime-source xz integrity, packaged executable hash, exact archived source, required resources, relative loader paths and exclusion of compiler/MSYS/test executables passed verification. BUILD-INFO-WINDOWS.json records the exact archive commit; refresh it and SHA256SUMS for this documentation-only final checkpoint. No application code changed after the successful final tests. Work is stopped at owner native Windows 11 validation; the full checklist is in WINDOWS_RELEASE_NOTES.md.
