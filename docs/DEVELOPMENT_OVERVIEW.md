# G_htop: staged local-LLM development overview

**Project:** G_htop Community 0.1.0  
**Author and copyright holder:** Donald E. Hunt  
**Contact:** g_htop@don-hunt.com  
**Copyright:** © 2026 Donald E. Hunt  
**License:** Apache-2.0; see [LICENSE](../LICENSE).  
**Documentation review date:** October 15, 2026

G_htop is a read-only Linux graphical monitor implemented in C++17 with gtkmm 3,
GTK 3, Cairo, and CMake. Its dashboard combines process and systemd service
information, logical CPU activity, AMD GPU/VRAM telemetry, and recent GPU history.
Detailed views also expose TCP/UDP connections. The application retains native
GTK appearance and ordinary desktop window behavior.

This overview explains how the project was developed through supervised stages
using a local Qwen coding model and Codex coordination. It summarizes reviewed
source, task events, generation receipts, and milestone checkpoints; it is not a
new benchmark or a replay of every historical action. The
[full case study](STAGED_LOCAL_LLM_DEVELOPMENT.md) provides implementation details,
acceptance evidence, limitations, a reusable task template, and research questions.
For operation and installation, use the [README](../README.md).

## Responsibilities and the working loop

Donald E. Hunt defined requirements, priorities, acceptance conditions, and
constraints. Human instructions drove the change from a broad initial assignment
to smaller useful milestones. Human supervision and visual feedback remained
part of development; the owner retained licensing and publication decisions.

Codex acted as a coordinator: inspecting the installed environment and existing
code, preparing focused instructions, invoking local generation, validating
files and replacement anchors, running builds, reviewing failures, and recording
acceptance evidence. The records also show integration corrections and reuse of
locally generated GTK statements with bindings for existing widgets. This was
supervised engineering work, not merely forwarding prompts.

The accepted application stages used `qwen2.5-coder:14b` through Ollama for local
code generation and revision. Early smaller-model/broader-scope attempts had
structured-output and incomplete-code failures. The later workflow generated
bounded source files or fragments through a local API and applied them through
deterministic scripts. Valid output structure did not establish correctness;
the actual compiler and focused GUI checks still determined whether a change
worked. No exclusive line-by-line authorship claim is made for the final source.

The practical loop was human requirements → coordinator instructions → local
model output → validated source changes → compile and targeted correction →
focused functional check → recorded milestone and human decision to advance.
Local application generation does not imply that every surrounding orchestration
service ran locally. This documentation stage also used a bounded local Qwen
prose draft, reviewed and adapted by the coordinator against project evidence.

## How the stages were organized

| Stage | Result and evidence boundary |
| --- | --- |
| 1 — Processes | Built and launched a real process table with names, PIDs, CPU deltas, resident memory, sorting, and automatic updates. Services and GPU work were deferred until this baseline worked. |
| 2 — Services | Added system/user loaded service units, descriptions, states, MainPID, filters, and sorting. Background discovery runs about every five seconds. Activating/deactivating are supported when reported but were not observed during the recorded check. |
| 3 — GPU telemetry | Added discovered amdgpu devices, overall utilization and VRAM from sysfs, and N/A handling. One Radeon Pro W5500 was recorded; multi-GPU operation was not hardware-verified. |
| 4 — CPU threads | Added `/proc/stat` delta-based meters, adaptive wrapping, and specified green/yellow/red bar ranges. Eight logical CPUs and wide/narrow layouts were checked. |
| 5 — Connections | Added real TCP/TCP6/UDP/UDP6 sockets through installed `ss`, endpoints, states, best-effort owners, filters, and numeric port sorting. Ownership and namespace visibility remain limited. |
| 6 — GPU history | Added a resizable GTK/Cairo graph receiving the existing GPU sample stream. A five-minute, bounded in-memory history avoids duplicate telemetry polling and disk logging. |
| 7 — Dashboard | Added three independent dividers, scrolling panels, saved layout, full-screen controls, and a fixed factual status strip. Windowed/full-screen transitions and secondary-monitor operation were recorded. |
| 8 — Polish | Improved theme-aware presentation, one-decimal measurements, numeric alignment/sorting, friendly GPU labels, tooltips, and spacing. Dark/light checks and a demonstrated live-refresh sorting defect led to targeted verification and repair. |
| 9 — Release | Prepared Apache-2.0 notices, clean source distribution, Arch package, checksums, and native DEB/RPM configurations. Arch was built and package-inspected; DEB/RPM remain unbuilt and target-unverified. |
| 10 — Methodology | Created this overview and the detailed case study, linked them from README, and reviewed consistency and preservation without rebuilding application code or packages. |

The dashboard and detailed views reuse existing widgets and sampling workers.
Most sampling occurs approximately every two seconds; service discovery has its
own slower interval. GPU history uses the same packets as the summary. The
process scan remains inside the GTK refresh callback, so responsiveness on the
checked host is not a guarantee of latency at arbitrary process counts.

## Challenges that shaped the method

Early attempts encountered invalid-result parsing, incomplete functions, and
incorrect GTK APIs. Narrower tasks and deterministic output application exposed
where a failure occurred, while compiler diagnostics provided concrete repair
targets. The records do not isolate model size from prompt scope or integration
changes, so they cannot establish which factor caused the improvement.

The owner also reported an early completion notification without the executable
or launcher. A recovery record reports an acceptance guard, and later stages
record actual build and launch evidence. The general lesson is to distinguish a
model saying “complete” from deliverables passing acceptance. This documentation
stage did not audit or modify the task worker or notification settings.

Visual inspection mattered. Compilation alone could not establish readable
columns, theme contrast, independent resizing, persistent geometry, or useful
full-screen behavior. A service PID comparator also failed on an empty model
row during refresh; a targeted repair and follow-up check were necessary.
“Focused verification” therefore did not mean forbidding a recheck after a real
failure, nor did it mean every stage succeeded on the first attempt.

## Lessons others can apply

Start with a minimal application that compiles and performs one useful action.
Choose the next bounded feature, state what must remain working, and supply only
the relevant interfaces. Limit the files and operations a generated change may
affect. Compile the real target and correct the demonstrated error rather than
redesigning the whole project after each failure.

Define acceptance in observable terms: an executable launches, real inputs appear,
numeric sorting remains numeric, or a divider preserves its position. Perform a
meaningful acceptance pass, repair demonstrated defects, and retain a known-good
baseline. Record limitations and unresolved blockers before advancing. A
schema-valid response, source checksum, or successful file write proves neither
correct behavior nor readiness to notify completion.

For packaging, separate recipes, built artifacts, and target-installed results.
Use clean source contents, conventional paths, dependency discovery, and
checksums. Build and check each target distribution natively before claiming
compatibility. These principles can be used with other coordinators, models,
runtimes, and build systems; they do not require this project's specific tools.

## Current limits and review status

Per-process GPU utilization/VRAM attribution is unsupported. Some socket owners
are unknown, service discovery covers loaded units, GPU hot-plug needs restart,
and the display is a best-effort snapshot rather than a system health guarantee.
Only the recorded hardware and display arrangement were verified.

The Arch/Manjaro package was built and inspected but not installed over the
working development copy. DEB/RPM have configuration and staging checks only;
target-native builds and installed launches remain outstanding. Byte-identical
reproducibility has not been established. See the
[release review](../RELEASE_REVIEW.md) for the current approval boundary.

Current environment observations and the model identity are recorded in the
[full case study](STAGED_LOCAL_LLM_DEVELOPMENT.md). Complete historical tool
versions, controlled timings, success percentages, and comparative results are
unknown. This single supervised case does not establish general performance
superiority over other models or approaches, and it provides no other-platform
or Pro edition.

These documents are prepared for GitHub review, not automatically published.
Existing packages and source archives were left unchanged during Stage 10.
Subsequent publication finalization includes these documents in the final source
and corrected Arch 0.1.0-2 package; older assets are local historical backups. Future study of task size, context limits,
checkpointing, and acceptance criteria is proposed as research, not presented as
a result established by G_htop.
