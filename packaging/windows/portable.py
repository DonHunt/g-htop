#!/usr/bin/env python3
"""Stage a portable UCRT64 GTK runtime; never downloads dependencies.

Run with MSYS2 UCRT64 Python or Linux Python, an installed official UCRT64
prefix, and an already built G_htop.exe. All DLL imports must resolve either
inside that prefix or from documented Windows system libraries.
"""

from pathlib import Path
import argparse, subprocess, re, shutil, json, hashlib, os

p = argparse.ArgumentParser()
p.add_argument("--prefix", required=True, type=Path)
p.add_argument("--exe", required=True, type=Path)
p.add_argument("--output", required=True, type=Path)
p.add_argument("--objdump", default="objdump")
p.add_argument("--runtime-materials", required=True, type=Path)
a = p.parse_args()
materials = a.runtime_materials.resolve()
for required in [
    "runtime-sources/MANIFEST.json",
    "runtime-sources/librsvg-build/crates.json",
    "rust-notices",
]:
    if not (materials / required).exists():
        raise SystemExit("Missing corresponding runtime materials: " + required)
source = Path(__file__).resolve().parents[2]
prefix = a.prefix.resolve()
out = a.output.resolve()
if out.exists():
    raise SystemExit("Output directory already exists; choose a new staging directory.")
(out / "bin").mkdir(parents=True)
shutil.copy2(a.exe, out / "bin/G_htop.exe")
# Explicit system allowlist: missing non-system libraries are a packaging error.
system = {
    "gdiplus.dll",
    "xmllite.dll",
    "bcryptprimitives.dll",
    "dwrite.dll",
    "msctf.dll",
    "d2d1.dll",
    "kernel32.dll",
    "user32.dll",
    "gdi32.dll",
    "advapi32.dll",
    "shell32.dll",
    "ole32.dll",
    "oleaut32.dll",
    "ws2_32.dll",
    "winmm.dll",
    "imm32.dll",
    "comdlg32.dll",
    "comctl32.dll",
    "shlwapi.dll",
    "version.dll",
    "setupapi.dll",
    "cfgmgr32.dll",
    "dwmapi.dll",
    "uxtheme.dll",
    "msimg32.dll",
    "winspool.drv",
    "rpcrt4.dll",
    "secur32.dll",
    "crypt32.dll",
    "bcrypt.dll",
    "ncrypt.dll",
    "dnsapi.dll",
    "iphlpapi.dll",
    "psapi.dll",
    "pdh.dll",
    "dxgi.dll",
    "d3d11.dll",
    "d3d9.dll",
    "dxguid.dll",
    "opengl32.dll",
    "glu32.dll",
    "usp10.dll",
    "hid.dll",
    "normaliz.dll",
    "userenv.dll",
    "netapi32.dll",
    "ntdll.dll",
    "wtsapi32.dll",
    "ucrtbase.dll",
    "msvcrt.dll",
    "dbghelp.dll",
    "wldap32.dll",
    "winhttp.dll",
    "wininet.dll",
    "authz.dll",
}
index = {f.name.lower(): f for f in (prefix / "bin").glob("*.dll")}
pending = [out / "bin/G_htop.exe"]
seen = set()
imports = {}
for tool in ["gdk-pixbuf-query-loaders.exe", "glib-compile-schemas.exe"]:
    shutil.copy2(prefix / "bin" / tool, out / "bin" / tool)
    pending.append(out / "bin" / tool)


def include(path):
    target = out / "bin" / path.name
    if not target.exists():
        shutil.copy2(path, target)
        pending.append(target)


# Runtime-loadable image decoders are outside the PE import closure.
module = Path("lib/gdk-pixbuf-2.0/2.10.0/loaders")
(out / module).mkdir(parents=True)
for f in (prefix / module).glob("*.dll"):
    shutil.copy2(f, out / module / f.name)
    pending.append(out / module / f.name)
while pending:
    file = pending.pop()
    key = str(file.relative_to(out))
    if key in seen:
        continue
    seen.add(key)
    result = subprocess.run(
        [a.objdump, "-p", str(file)], check=True, capture_output=True, text=True
    )
    names = re.findall(r"DLL Name:\s*(\S+)", result.stdout)
    imports[key] = names
    for name in names:
        lower = name.lower()
        if lower in index:
            include(index[lower])
        elif lower not in system and not lower.startswith(
            ("api-ms-win-", "ext-ms-win-")
        ):
            raise SystemExit(f"Unresolved import {name} from {key}")
for relative in [
    "share/glib-2.0/schemas",
    "share/icons/Adwaita",
    "share/icons/AdwaitaLegacy",
    "share/icons/hicolor",
    "share/mime",
    "share/licenses",
]:
    if (prefix / relative).exists():
        shutil.copytree(prefix / relative, out / relative, dirs_exist_ok=True)
# License texts are retained for the complete dependency closure.
for name in [
    "LICENSE",
    "NOTICE",
    "LICENSES.md",
    "BUILD-WINDOWS.md",
    "WINDOWS_RELEASE_NOTES.md",
]:
    if (source / name).exists():
        shutil.copy2(source / name, out / name)
icon = out / "share/icons/hicolor/scalable/apps"
icon.mkdir(parents=True, exist_ok=True)
shutil.copy2(source / "data/icons/G_htop.svg", icon / "G_htop.svg")
# Native query-loaders is used after staging to produce a relocatable cache.
(out / "G_htop.cmd").write_text('@echo off\r\nstart "" "%~dp0bin\\G_htop.exe"\r\n')
(out / "README-WINDOWS.txt").write_text(
    "G_htop Community Edition 1.1.0\n\nExtract the entire ZIP to a folder. Double-click G_htop.cmd or bin\\G_htop.exe.\nKeep bin, lib and share together. No MSYS2 or administrator account required.\nThis is a native Windows 11 test candidate; see WINDOWS_RELEASE_NOTES.md.\nCommunity is free under Apache-2.0. G_htop Pro is purchased separately:\nhttps://don-hunt.com/G_htopPro/\n"
)
shutil.copytree(
    materials / "rust-notices", out / "share/licenses/librsvg-rust", dirs_exist_ok=True
)
shutil.copy2(materials / "runtime-sources/MANIFEST.json", out / "RUNTIME-MANIFEST.json")
shutil.copy2(
    materials / "runtime-sources/librsvg-build/crates.json", out / "RUST-CRATES.json"
)
shutil.copy2(
    source / "packaging/windows/THIRD-PARTY-NOTICES.txt",
    out / "THIRD-PARTY-NOTICES.txt",
)
(out / "DLL-IMPORTS.json").write_text(json.dumps(imports, indent=2) + "\n")
print(out)

fonts = out / "etc/fonts"
fonts.mkdir(parents=True, exist_ok=True)
shutil.copy2(source / "packaging/windows/fonts-wine.conf", fonts / "fonts-wine.conf")
