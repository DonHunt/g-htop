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
