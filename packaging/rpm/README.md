# RPM packaging — CONFIGURATION ONLY, NATIVE BUILD UNVERIFIED

No RPM has been produced. There is no Fedora/RPM target environment or rpmbuild
here. Build from a fresh source tree inside the intended Fedora release. Other
RPM distributions require their own native build/dependency/launch verification;
RPM format alone does not establish compatibility. Keep outputs per target.

Fedora build prerequisites: gcc-c++, cmake, make, pkgconf-pkg-config, gtkmm30-devel
and rpm-build. Runtime helpers: systemd, iproute, coreutils and hicolor-icon-theme;
pciutils is optional. CPack/rpmbuild automatic ELF dependency scanning adds actual
library/SONAME and symbol requirements from the native target. gtkmm30 is an
explicit runtime requirement; no Manjaro ABI strings or libraries are copied.

Maintainer: Donald E. Hunt <g_htop@don-hunt.com>.
Copyright © 2026 Donald E. Hunt. License: Apache-2.0.
These owner-approved details are the shared CPack defaults. On the target,
with prerequisites already installed:

```sh
export SOURCE_DATE_EPOCH="$(cat packaging/SOURCE_DATE_EPOCH)"
cmake -S . -B build-rpm -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DG_HTOP_PACKAGE_FORMAT=RPM
cmake --build build-rpm --parallel 2
. /etc/os-release
cpack --config build-rpm/CPackConfig.cmake -G RPM -B "native-dist/$ID-$VERSION_ID"
```

The generated RPM spec includes the maintainer as its Packager header and
Apache-2.0 as its License header.

The package-time guard checks the recorded configure-host family and the current
host family, rejecting a Manjaro-configured build. It does not establish
cross-release ABI compatibility. Compatible RPM-family distributions can attempt
a native build, but package names and older C++17 toolchains must be verified
locally; no RHEL-family compatibility is claimed.

Before marking a target verified, inspect `rpm -qip`, `rpm -qlp`, and `rpm -qpR`,
and perform an installed launch on that target. Files use `/usr/bin/G_htop`, the
standard desktop/icon locations, `/usr/share/doc/g_htop` and
`/usr/share/licenses/g_htop`. Build without root. After review, install with
`sudo dnf install ./g_htop-0.1.0-1.*.rpm` and remove with `sudo dnf remove g_htop`.
No package installation is performed by the build commands.

Reference: https://cmake.org/cmake/help/latest/cpack_gen/rpm.html

Use a fixed native distribution/release and recorded toolchain/dependency versions
for repeatable builds. SOURCE_DATE_EPOCH records the release timestamp; byte-for-byte
reproducibility and native installation have not yet been verified.
