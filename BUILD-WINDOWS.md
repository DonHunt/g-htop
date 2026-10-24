# Build G_htop Community 1.1.0 for Windows

Target: Windows 11 x64. The shared C++17/gtkmm3 dashboard uses native Win32
monitoring collectors. The Linux build instructions remain in README.md.
Windows packages are test candidates until the owner completes native testing.

## Native MSYS2 UCRT64 build

Install MSYS2 from https://www.msys2.org/. Open **MSYS2 UCRT64**, update it using
`pacman -Syu`, and reopen that shell if requested. Use official MSYS2 packages:

```sh
pacman -S --needed git mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-python \
  mingw-w64-ucrt-x86_64-gtkmm3 mingw-w64-ucrt-x86_64-librsvg \
  mingw-w64-ucrt-x86_64-adwaita-icon-theme \
  mingw-w64-ucrt-x86_64-hicolor-icon-theme \
  mingw-w64-ucrt-x86_64-shared-mime-info
git clone --branch windows/community-1.1.0 https://github.com/DonHunt/g-htop.git
cd g-htop
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-windows --parallel
ctest --test-dir build-windows --output-on-failure
```

Do not mix MINGW64/MSVCRT headers, UCRT64 DLLs, or different C++ toolchains.
The preparation environment uses the official UCRT64 Windows compiler,
CMake and Ninja under an isolated Wine prefix on Linux. This performs a Windows
build; Wine is not evidence of Windows hardware telemetry accuracy.
Exact dependency versions and archive SHA256 values accompany the candidate.
The checked preparation used GCC 16.2.0, gtkmm 3.24.11, GTK 3.24.52 and the
versions pinned in `packaging/windows/dependencies.json`. Use that manifest to
retrieve the same official binary/source archives when exact reproduction is
needed; a future `pacman -Syu` may select newer versions.

## Portable staging

The packaging helper downloads nothing. Point it at the installed UCRT64 prefix
and the compiled executable. Also extract the supplied, checksum-verified
`G_htop-Community-1.1.0-Windows-runtime-sources.tar.xz` into `runtime-materials/`
so that it contains `runtime-sources/`, `rust-notices/` and the Rust vendor tree.
This supplies the matching source manifests and notices; the helper refuses to
stage a package without them. Use a fresh output directory:

```sh
/ucrt64/bin/python packaging/windows/portable.py \
  --prefix C:/msys64/ucrt64 --exe build-windows/G_htop.exe \
  --output dist/G_htop-Community-1.1.0-Windows-x64 \
  --runtime-materials runtime-materials
```

The helper recursively resolves PE imports, fails on missing non-system DLLs,
and copies image decoder modules, schemas, MIME data, icons and license texts.
Finalize the caches using the included script:

```sh
/ucrt64/bin/python packaging/windows/finalize.py \
  dist/G_htop-Community-1.1.0-Windows-x64
```

On Linux with Wine, set an isolated `WINEPREFIX` and add `--wine`. This runs only
the packaged resource tools. The script compiles schemas and rewrites loader
paths relative to the package root. GdkPixbuf resolves those paths from its DLL
installation prefix. Verify the package after moving it to a different folder.
The application locates GTK resources relative to `bin/G_htop.exe`; the working
directory is irrelevant. Zip the complete staging directory, preserving its
`bin`, `lib`, `share`, notices and instructions. In PowerShell:

```powershell
Compress-Archive -Path dist/G_htop-Community-1.1.0-Windows-x64 -DestinationPath G_htop-Community-1.1.0-Windows-x64.zip
Get-FileHash G_htop-Community-1.1.0-Windows-x64.zip -Algorithm SHA256
```

Extract the complete ZIP and run `G_htop.cmd` or `bin/G_htop.exe`. Keep `bin`,
`lib` and `share` together. Administrator privileges and MSYS2 are unnecessary.
The package is unsigned; confirm its published SHA256 before launching.

Archive the exact reviewed source commit with `git archive --format=zip`, and
record that commit alongside the Windows build. Preserve dependency archives,
hashes, upstream sources, patches, package recipes and license notices for
redistribution. Dynamically linked LGPL libraries can be replaced in `bin`.
Do not publish binaries until matching runtime source materials are available.
Do not replace existing Linux release assets or their SHA256SUMS.

## Monitoring differences

| Data | Windows implementation | Limitations |
| --- | --- | --- |
| Processes | Toolhelp32, GetProcessTimes, GetProcessMemoryInfo | Protected processes may show N/A. CPU is percent of one logical core and may exceed 100%. |
| Services | EnumServicesStatusExW | Read-only SCM Win32 services; no systemd user scope or service controls. |
| CPU | PDH Processor Information instances, processor groups | First/unsupported samples N/A; supports more than one processor group. |
| GPU discovery | DXGI hardware adapters and LUIDs | Software renderers excluded; discovery retries. |
| GPU load | WDDM GPU Engine PDH counters | Busiest engine after summing processes on each engine; unsupported counters N/A. |
| VRAM | GPU Adapter Memory dedicated usage, DXGI dedicated capacity | Integrated/shared-only memory and unavailable driver counters may show N/A. |
| Network | IP Helper TCP/UDP owner-PID tables, IPv4/IPv6 | UDP remote endpoints unavailable; protected owner names may be Unknown. |
| Uptime | GetTickCount64 | Windows boot uptime, including sleep. |

Temperature, device power and per-process GPU/VRAM attribution are N/A. These
metrics have no universal native adapter interface in this implementation.
No vendor utility, PowerShell, WMIC or manual manufacturer selection is needed.

## Troubleshooting and validation

- A DLL error means extraction is incomplete or incompatible DLLs were mixed.
  Re-extract the entire package; do not download DLLs from third-party sites.
- Missing icons/images: retain `share`, `lib`, the relative loaders cache and all
  imported DLLs. Confirm the package still works after moving it to a folder
  containing spaces and non-ASCII characters, launched from a different cwd.
- Wine: the EXE detects Wine and automatically selects Fontconfig with the
  packaged `etc/fonts/fonts-wine.conf`. This avoids Wine's damaged Win32 glyphs
  for both EXE and CMD launches, without external font environment settings.
  The configuration uses Wine/Windows font folders and standard Linux font
  directories through Wine's Z: drive. Keep `etc` with the extracted package.
  Native Windows retains its normal Win32 font backend.
- Wine 11.17 returned `PDH_CSTATUS_NO_COUNTER` for the CPU and GPU performance
  counter paths. GPU discovery can still name an adapter, but there are no
  utilization measurements to plot. The chart explicitly identifies that
  unavailable state. Validate real CPU/GPU telemetry on native Windows 11;
  the application does not replace missing measurements with invented zeros.
- The focused GTK test tools capture screenshots by default. On older Cairo
  backends that fail while capturing a window, set `G_HTOP_TEST_NO_SCREENSHOT=1`
  and use an external capture tool; all functional assertions still run.
- Windows PDH counters may be unavailable with particular drivers or privileges.
  Wait for two samples. Record N/A rather than substituting an invented value.
- Test standard-user Windows 11 startup, shutdown, repeated launches, resize,
  navigation, About/link, CPU/process load, service states, network tables and
  each physical GPU, including stable distinct colors on the shared graph.

API references: [process times](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes),
[SCM enumeration](https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-enumservicesstatusexw),
[TCP tables](https://learn.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getextendedtcptable),
[language-neutral PDH](https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhaddenglishcounterw),
[Windows GPU aggregation](https://devblogs.microsoft.com/directx/gpus-in-the-task-manager/),
[MSYS2 CMake](https://www.msys2.org/docs/cmake/).
