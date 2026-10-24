# G_htop Community 1.1.0 — Windows 11 x64 test candidate

The Windows port retains the four-panel dashboard, detailed monitoring tabs,
resizable panels, saved layout, one graph with separate colored GPU histories,
and the existing About dialog. Community remains free software under Apache
2.0. The About link opens https://don-hunt.com/G_htopPro/ and identifies Pro as
the separately purchased professional edition.

Windows monitoring uses native APIs for processes, read-only services, logical
CPU utilization, automatic DXGI GPU discovery, available WDDM utilization and
dedicated memory counters, TCP/UDP IPv4/IPv6 connections, and uptime. There are
no process termination, service management or GPU configuration controls.

Extract the entire Windows ZIP into a folder you can write to. Run `G_htop.cmd`
or `bin/G_htop.exe`. No MSYS2 installation or administrator account is required.
Keep the executable, DLLs, `lib`, `share` and notices together when moving it.
Verify the supplied SHA256SUMS before launch. The executable is unsigned.

## Validation status

This candidate is prepared for owner testing on native Windows 11. Windows
binaries are not published to GitHub Releases until that testing succeeds and
the owner explicitly approves publication. Existing Linux 1.1.0 assets and the
existing v1.1.0 tag are preserved.

Preliminary checks use MSYS2 UCRT64 Windows GCC/CMake/Ninja under Wine, with an
isolated prefix and no installed host application changes. Linux regression
checks continue against Ubuntu 22.04. Final results and exact source revision
are recorded in WINDOWS_PORT_PROGRESS.md and the candidate build manifest.

## Genuine limitations

- Wine cannot validate physical Windows CPU/GPU counters or driver accuracy.
- First samples, protected processes and unsupported counters show N/A. A real
  zero is preserved. Windows process CPU is percent of one logical core.
- Services come from the Windows SCM Win32 service list, without systemd user
  units. No service control operations are exposed.
- GPU load is the busiest engine after aggregating processes sharing an engine.
  GPU temperature, power and per-process GPU/VRAM attribution remain N/A.
- Dedicated memory usage and capacity depend on driver support. Integrated
  shared memory is not presented as dedicated VRAM. Unsupported data stays N/A.
- UDP remote peers are unavailable from the Windows owner-PID table. Owner names
  may be Unknown when access is denied. IPv6 scope IDs are retained.
- Native Windows font rendering, scaling, multiple monitors and hardware
  accuracy require owner validation. Wine-specific warnings are not proof of
  native Windows defects or native compatibility.

## Owner Windows 11 test checklist

1. Copy the ZIP and SHA256SUMS to Windows. Check `Get-FileHash <zip> -Algorithm
   SHA256` against the supplied digest, then extract the whole ZIP.
2. Run as a standard user on Windows 11 x64, without MSYS2 installed. Confirm
   readable text, icons and the four dashboard panels. Close and relaunch at
   least three times; verify the process exits each time.
3. Move the extracted folder to a path containing spaces and non-ASCII text.
   Launch from a different working directory and confirm resources still load.
4. Resize the window, drag all three dividers, switch through every tab, use
   Reset Layout, F11 and Escape, and confirm normal layout restoration on exit.
5. Open About. Confirm version 1.1.0, Apache-2.0, free Community wording and the
   separate paid Pro label. Click the link and confirm the browser reaches the
   correct Pro site.
6. Compare process names/PIDs, working sets and CPU under idle/load against Task
   Manager. Check service running/stopped states against Services. Protected
   values may be N/A. Confirm CPU count and loaded-core meters after two samples.
7. Compare detected GPUs and utilization with Task Manager during GPU activity.
   On a multi-GPU machine, exercise each GPU and confirm distinct colored lines
   on the single graph. Check available dedicated VRAM; report N/A honestly.
8. Open TCP and UDP traffic, including IPv6 if available. Check Connections,
   numeric sorting, filters and accessible process owners against Windows tools.
9. Report Windows build, GPU/driver versions, CPU, scaling, successes, N/A data
   and any problems. Publication requires explicit owner approval afterward.

Build and redistribution details: [BUILD-WINDOWS.md](BUILD-WINDOWS.md).
