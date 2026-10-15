# DEB packaging — CONFIGURATION ONLY, NATIVE BUILD UNVERIFIED

No DEB has been produced. This workstation is Manjaro; installed dpkg tools do
not provide a Debian/Ubuntu runtime or prove compatibility. Build from a fresh
source tree **inside each intended Debian, Ubuntu or Linux Mint release**.
A Debian-built binary is not automatically compatible with every Ubuntu/Mint
release. Keep outputs in separate distribution/version directories.

Build prerequisites on the target: build-essential, cmake, pkg-config,
libgtkmm-3.0-dev, dpkg-dev. Runtime helpers are systemd, iproute2, coreutils and
hicolor-icon-theme. pciutils is optional for descriptive GPU names. CPack's
`dpkg-shlibdeps` scan derives actual gtkmm/GTK, glibc and C++ runtime dependencies
from the target build; it handles release-specific runtime package names rather
than hardcoding Manjaro library requirements. No library bundles are supplied.

Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.
Copyright © 2026 Donald E. Hunt. License: Apache-2.0.
These owner-approved details are the shared CPack defaults. On the target,
with prerequisites already installed:

```sh
export SOURCE_DATE_EPOCH="$(cat packaging/SOURCE_DATE_EPOCH)"
cmake -S . -B build-deb -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DG_HTOP_PACKAGE_FORMAT=DEB
cmake --build build-deb --parallel 2
. /etc/os-release
cpack --config build-deb/CPackConfig.cmake -G DEB -B "native-dist/$ID-$VERSION_ID"
```

Use a clean build directory on each target. The package-time guard rejects a
build configured on Manjaro or a mismatched distribution family, even if the
build directory is later copied to another host. The configured maintainer email
is validated before packaging. The guard is not a substitute for target-version
testing.

Before marking a target verified, inspect `dpkg-deb --info` and `dpkg-deb --contents`,
check the installed dependency list and launch on that target. The package uses
`/usr/bin/G_htop`, a freedesktop desktop entry/icon, documentation and licenses;
it also installs `/usr/share/doc/g-htop/copyright`. No root is needed to build.
After review, install with `sudo apt install ./g-htop_0.1.0-1_*.deb` and remove
with `sudo apt remove g-htop`. No installation is performed by these build commands.

Reference: https://cmake.org/cmake/help/latest/cpack_gen/deb.html

Use a fixed native distribution/release and recorded toolchain/dependency versions
for repeatable builds. SOURCE_DATE_EPOCH records the release timestamp; byte-for-byte
reproducibility and native installation have not yet been verified.
