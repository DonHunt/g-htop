#!/usr/bin/env python3
"""Finalize relative GTK resources; run on Windows or with --wine on Linux."""

from pathlib import Path, PureWindowsPath
import argparse, subprocess, os, re, json

p = argparse.ArgumentParser()
p.add_argument("stage", type=Path)
p.add_argument("--wine", action="store_true")
a = p.parse_args()
root = a.stage.resolve()
env = os.environ.copy()


def target(path):
    return "Z:" + str(path).replace("/", "\\") if a.wine else str(path)


def run(tool, *args):
    command = (["wine"] if a.wine else []) + [str(root / "bin" / tool), *args]
    return subprocess.run(
        command, env=env, cwd=root, check=True, capture_output=True, text=True
    ).stdout


run("glib-compile-schemas.exe", target(root / "share/glib-2.0/schemas"))
module = Path("lib/gdk-pixbuf-2.0/2.10.0/loaders")
env["GDK_PIXBUF_MODULEDIR"] = target(root / module)
cache = run("gdk-pixbuf-query-loaders.exe")
count = 0
lines = []
for line in cache.splitlines():
    if line.startswith("#"):
        continue
    if re.fullmatch(r'".*\.dll"', line):
        name = PureWindowsPath(json.loads(line)).name
        if not (root / module / name).is_file():
            raise SystemExit("Unexpected loader path: " + line)
        line = '"' + str(module / name).replace("\\", "/") + '"'
        count += 1
    lines.append(line)
if not count:
    raise SystemExit("No image loaders registered")
(root / "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache").write_text("\n".join(lines) + "\n")
print("Registered", count, "relocatable image loaders; schemas compiled.")
