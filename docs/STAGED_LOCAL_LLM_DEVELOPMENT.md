# Staged Autonomous Software Development with Local LLMs: The G_htop Case Study

**Project:** G_htop Community 0.1.0  
**Author and copyright holder:** Donald E. Hunt  
**Contact:** g_htop@don-hunt.com  
**Copyright:** © 2026 Donald E. Hunt  
**License:** Apache-2.0; see [LICENSE](../LICENSE).  
**Documentation review date:** October 15, 2026

## Scope and evidence

G_htop is a read-only Linux desktop monitor built in successive, independently
useful stages. This case study records how human requirements, Codex task
coordination, and a locally hosted Qwen coding model were combined to produce
version 0.1.0. “Autonomous” describes delegated work within defined stages; the
project retained human supervision, corrections, and acceptance decisions.

The account draws on the current source, [user documentation](../README.md),
[release history](../CHANGELOG.md), local generation receipts, task events,
compiler outputs, screenshots, and milestone checkpoints reviewed during this
stage. Private development records are not included in the public source
bundle. Historical observations below are reported from those records, rather
than new monitoring tests performed for this document. Source links identify
implementation evidence; they do not alone prove a historical GUI observation.

No controlled comparison, timing study, model success-rate measurement, or
complete line-by-line authorship audit was conducted. Unknowns are identified
explicitly. Current tool versions are observations at documentation time, not a
reconstruction of every earlier development environment. Nothing in this case
study establishes general performance superiority over other models or methods.

## Background and implementation choices

The objective was a graphical alternative for desktop users who wanted process,
service, CPU, GPU, network, and recent-history information in one application.
The implementation uses C++17, gtkmm 3 over GTK 3, Cairo drawing through GTK, and
CMake. GTK 3 fit the existing Cinnamon desktop and available development
libraries. The application follows the desktop theme rather than supplying its
own theme engine. It has no termination, service-control, firewall, or GPU-tuning
controls.

Development took place on Manjaro Linux, an Arch-family rolling distribution,
with a Cinnamon desktop. The tested hardware included an AMD Radeon Pro W5500
and eight logical CPUs. Those observations describe the development machine;
they are not hardcoded device or CPU-count requirements. Secondary-monitor
placement was subsequently checked on the available extended desktop.

The accepted feature stages used `qwen2.5-coder:14b` through a local Ollama API.
Early attempts with a smaller Qwen coding model and broader task scopes did not
produce an accepted application. The successful approach used bounded local
source or fragment generation, deterministic application of the output, actual
compilation, and GUI inspection. This is a report about that observed workflow,
not a recommendation derived from comparative model testing.

## Development architecture and responsibilities

The working loop was:

```text
Human requirements and acceptance decisions
  → Codex task coordination and bounded instructions
  → Local Qwen generation through Ollama
  → Validated source/file-fragment changes and integration
  → Compilation and targeted correction
  → Focused functional verification
  → Recorded milestone and human decision to advance
```

**Human developer.** Donald E. Hunt set the requirements, priorities, scope,
read-only constraints, and completion conditions. Human supervision included
visual and functional feedback, instructions to simplify failed attempts, and
stage handoffs. The human supplied the copyright holder, license, and maintainer
contact, and retained the publication decision. A recorded tool result was not
a replacement for the owner's authority to accept a milestone.

**Codex.** The coordinator inspected dependencies and existing source, prepared
bounded instructions, invoked the local generation workflow, validated and wrote
files, ran compilers, interpreted failures, coordinated focused checks, and
recorded outcomes. Available scripts also show integration work: rebinding
model-generated statements to existing widget and column names, moving generated
initialization into the correct scope, removing duplicate statements, and
applying targeted fixes. Codex's orchestration is distinct from the local
inference used to generate application code.

**Local Qwen.** The local model generated and revised C++ source, headers,
widget fragments, and bounded corrections within the existing project. Later
release work also used local drafts for CMake/packaging metadata. In this
methodology stage it supplied a bounded prose draft that Codex reviewed and
adapted against the records. The use of a local coding model does not mean the
orchestration layer or all surrounding tools ran locally, nor does it establish
that every final source line has exclusive model authorship.

### Output handling and correction

Native agent tool calls and structured results were unreliable in early
attempts. A simpler path called the local inference API and accepted generated
files or narrowly scoped fragments through coordinator-controlled scripts.
Depending on the task, those scripts checked JSON structure, expected filenames,
source framing, or exact replacement anchors before writing. They did not grant
the model arbitrary execution authority or establish that its output was
semantically correct. Compiler feedback remained necessary.

Generation requests were kept small enough to avoid requiring the model to
complete every subsystem at once. Existing working statements could be reused
with deterministic bindings rather than asking for a whole application rewrite.
Actual compilation exposed incorrect APIs, missing definitions, and integration
mistakes that output validation could not detect. Targeted repairs followed those
errors. This involved retries and corrections; it was not a single uninterrupted
successful generation.

### Runtime architecture is a separate concern

The application's collection strategy should not be confused with the coding
workflow. Process and CPU-thread updates read `/proc`; systemd, GPU, and network
collection use background workers, with GUI-side updates. The history graph
consumes existing GPU sample packets and has no independent telemetry poller.
The dashboard reparents existing views, preserving their models, filters, and
history instead of constructing duplicate monitoring pipelines.

The process scan remains in the GTK refresh callback. This implementation was
usable on the checked host, but it is not evidence of bounded GUI latency on
arbitrarily large systems. Service and socket helpers have timeouts; slow or
unavailable data is handled as a monitoring limitation, not fabricated success.

## The ten development stages

### Stage 1 — Processes

**Objective.** Establish a small, compilable, useful graphical process monitor
before adding other subsystems.

**Implementation.** [main.cpp](../main.cpp) enumerates accessible numeric process
directories, reads process CPU counters and resident-memory pages, and populates
a GTK list model with process names and PIDs. CPU percentage uses successive
user/system tick differences divided by elapsed time and clock ticks per
second. Process start time helps distinguish PID reuse. One fully occupied
logical core corresponds to 100%; a multithreaded process can exceed 100%.
Resident memory is converted to MiB. A roughly two-second GTK timer refreshes
the model, with sortable numeric CPU/memory columns and alphabetical names.

**Acceptance and recorded outcome.** The task record reports a successful
build, launcher-based GUI launch, real processes, CPU deltas, resident memory,
name/CPU/numeric-memory sorting, and advancing refresh samples. Services and GPU
work were explicitly deferred until that baseline worked.

**Limitations and lesson.** Processes can exit or become inaccessible during a
scan; resident memory includes shared pages. A new process has no prior CPU
sample. The minimal monitor became a concrete acceptance target after broader
initial attempts failed. Not every draft header remaining in the development
directory is part of the active application; the include graph matters.

### Stage 2 — Services

**Objective.** Add system and user service visibility without changing process
monitoring or exposing service-management actions.

**Implementation.** [service_data.hpp](../service_data.hpp) queries `systemctl
show` in both scopes for canonical ID, description, active/substate, and MainPID.
The [Services view](../services_view.hpp) offers All, Active, Inactive, and Failed
filters and sortable description, unit, and status fields. Canonical unit name
and scope identify rows. A background worker refreshes approximately every five
seconds, rather than forcing service discovery into the two-second process scan.

**Acceptance and recorded outcome.** System and user rows, descriptions, unit
names, PIDs, filters, sorting, periodic refresh, and the preserved process view
were checked. The recorded Failed filter had no failed services to display.
Activating/deactivating are supported as reported states, but no such transition
occurred during the recorded check.

**Limitations and lesson.** Discovery covers loaded service units, including
loaded inactive ones; it is not an inventory of every installed but unloaded
unit file. A user-manager query can fail without root being an appropriate
remedy. Distinguish a filter working with an empty result from having observed
every possible state.

### Stage 3 — AMD GPU telemetry

**Objective.** Add accurate overall GPU/VRAM information with explicit missing
values and no invented process attribution.

**Implementation.** [gpu_data.hpp](../gpu_data.hpp) discovers amdgpu devices under
`/sys/class/drm` instead of assuming a fixed card index. It reads
`gpu_busy_percent`, `mem_info_vram_total`, and `mem_info_vram_used`; available VRAM
is total minus used when valid. Optional `lspci` output supplies identification.
The [summary worker](../gpu_summary.hpp) refreshes approximately every two
seconds. Unreadable values remain N/A.

**Acceptance and recorded outcome.** The records identify one Radeon Pro W5500
and report live utilization/VRAM readings, visible alongside both existing views,
while process and service checks still passed. No additional GPU was recorded.

**Limitations and lesson.** Multiple-device discovery exists, but multi-GPU
hardware operation was not verified. Per-process GPU utilization and VRAM
attribution are not implemented. Devices are discovered at startup, so hot-plug
requires restarting. Overall telemetry must not be allocated to processes by
assumption.

### Stage 4 — CPU threads

**Objective.** Make logical CPU activity readable at different window sizes.

**Implementation.** [cpu_panel.hpp](../cpu_panel.hpp) reads successive per-CPU
counters from `/proc/stat`, uses total/idle deltas, includes I/O wait in idle,
and avoids double-counting guest time. It creates labeled meters for discovered
logical CPUs and wraps their layout as width changes. Only the utilization
indicator uses specified colors: green from 0–33%, yellow above 33% and below
67%, and red from 67–100%.

**Acceptance and recorded outcome.** Eight meters, CPU 0 through CPU 7, displayed
updating values. The recorded wide/narrow layout check changed from three
columns to one while preserving earlier views and GPU reporting.

**Limitations and lesson.** First samples and invalid/reset deltas cannot yield
valid utilization and display N/A. Eight threads were observed; behavior at much
larger CPU counts was not established by a benchmark. Adaptive layout is useful
without claiming exhaustive hardware coverage.

### Stage 5 — Network monitoring

**Objective.** Add sockets, endpoints, states, and accessible process ownership
without intrusive network actions.

**Implementation.** [connection_data.hpp](../connection_data.hpp) uses the
installed `ss` utility with numeric IPv4/IPv6 queries. This was the permitted
alternative to implementing direct `/proc/net` decoding and inode-to-process
scanning. A [background view worker](../connections_view.hpp) collects TCP,
TCP6, UDP, and UDP6 rows approximately every two seconds. Sort bindings cover
application, PID, protocol, local/remote ports, and state; PIDs and ports have
numeric sort keys.

**Acceptance and recorded outcome.** The records report real sockets, IPv4/IPv6
endpoints, accessible and unknown owners, All/Listening/Established/UDP filters,
port sorting, and continued operation of the earlier tabs.

**Limitations and lesson.** Visibility is limited to the current network
namespace and accessible ownership data. Shared sockets can have one row per
owning PID. Listening selects TCP LISTEN; unconnected UDP remains UNCONN under
the UDP filter. Established follows `ss` ESTAB, which can include connected UDP.
Do not describe this implementation as a custom `/proc/net` parser or infer
traffic direction from a socket snapshot.

### Stage 6 — GPU history

**Objective.** Add a reusable graph without a second GPU sampling system or
continuous disk logging.

**Implementation.** [gpu_history.hpp](../gpu_history.hpp) implements a GTK
DrawingArea/Cairo graph. `append_sample()` receives the existing GPU readings,
stamps them with a monotonic time, and retains a five-minute rolling window.
Time-based expiry and a 152-packet cap bound memory use at the nominal two-second
cadence. The graph scales with its allocation, uses a fixed 0–100% vertical axis,
and labels relative time from five minutes ago to now. Unavailable samples
interrupt the trace.

**Acceptance and recorded outcome.** The checkpoint records evolving real
samples agreeing with the summary, readable axes, resizing, and access to all
existing views after a successful build and launcher launch.

**Limitations and lesson.** History starts empty and is lost at exit. Scheduled
refresh is approximate, so it is not a guaranteed uniformly sampled measurement
instrument. Reusing the sample stream avoided adding another hardware poller.

### Stage 7 — Dashboard and display modes

**Objective.** Bring four views together while retaining detailed tabs and
normal desktop behavior.

**Implementation.** [dashboard.hpp](../dashboard.hpp) provides a left/right
split and one upper/lower split per column. Processes and Services occupy the
left; GPU history/VRAM and CPU meters occupy the right. The views scroll or adapt
inside their panels. Normal window dimensions and divider proportions persist
in a user configuration file, with clamping and Reset Layout. F11 toggles
full-screen Dashboard; Escape returns to windowed mode. A fixed status strip
uses `/proc/uptime`, cached failed-service counts, and detected collection issues.

**Acceptance and recorded outcome.** The checkpoint records independent dragging
of all three dividers, restored size/proportions, Reset Layout, F11/Escape,
retained detailed views/history, and a visible bottom strip. Secondary-monitor
movement, full-screen operation at 1920×1080, and return to a window on that
monitor were recorded. Eight live CPU meters were also visible.

**Limitations and lesson.** Placement uses the window manager; the application
does not configure displays. One tested extended-desktop arrangement does not
prove every monitor or scaling combination. A zero failed-service count is a
statement about discovered units, not overall system health.

### Stage 8 — Interface polish

**Objective.** Improve readability without replacing the collectors or losing
the accepted dashboard behavior.

**Implementation.** [presentation.hpp](../presentation.hpp) centralizes display
formatting and related GTK presentation helpers. Percentages and MiB/GiB values
use one decimal; numeric columns align right while numeric model values continue
to determine sorting. Friendly GPU labels retain detailed tooltip information.
Spacing, ellipsizing, tooltips, CPU wrapping, graph contrast, and the
[status strip](../status_strip.hpp) were adjusted to use theme-compatible
presentation and factual warnings.

**Acceptance and recorded outcome.** Checkpoints report readability at
1100×750 and 1920×1080, dark and process-local light-theme checks, working
numeric sorting, all detailed views, a real GPU tooltip, and the installed
launcher. An empty-row service-PID comparator exception was demonstrated and
repaired with nonthrowing conversion; sorted refresh then worked.

**Limitations and lesson.** Very narrow panels still scroll or ellipsize long
names. Theme checks cover the recorded appearances, not all GTK themes. A
successful compile did not catch the live-refresh comparator defect; focused
interaction was necessary.

### Stage 9 — Release preparation

**Objective.** Prepare a clean Linux Community release for owner review without
publication or replacement of the working installation.

**Implementation.** CMake install rules, desktop entry, SVG icon, complete
Apache-2.0 license, NOTICE, attribution review, README, CHANGELOG, source archive,
Arch recipe/package, and SHA-256 checksums were prepared. The owner confirmed
Donald E. Hunt as holder and maintainer and `g_htop@don-hunt.com` as contact.
Additional [CPack configurations](../packaging/cpack/Packaging.cmake) provide
DEB/RPM metadata and native-host guards. Native ELF dependency scans are enabled;
helper dependencies use distribution-specific names.

**Acceptance and recorded outcome.** The Arch/Manjaro source archive was built
with makepkg, its package was constructed and inspected, and paths, desktop
metadata, icon, licenses, resolved shared libraries, and checksums were checked.
It was not installed over the development application. Additional CPack
configuration and staged-path checks passed, including rejection of incompatible
Manjaro-configured DEB/RPM packaging. The later metadata update was checked
without rebuilding the application.

**Package status.** Arch/Manjaro: built and package-inspected, not an installed
package test. DEB and RPM: configuration only; neither has been natively built,
installed, or verified on a target distribution. No suitable target environment
was available. They must not be represented as completed compatible packages.

**Limitations and lesson.** Source rebuilds, dependency inspection, and installed
launches remain necessary per intended DEB/RPM distribution and release. Fixed
archive timestamps and SOURCE_DATE_EPOCH assist repeatable recipes, but
byte-for-byte reproducibility was not established. Original verified Arch assets
and their matching pinned source/recipe were preserved separately from the newer
additional source/configuration bundle. Historical notices differ from the later
full-name notice; those preserved artifacts were not silently rewritten.
Publication remains a separate owner decision. See
[release review](../RELEASE_REVIEW.md) and [license audit](../LICENSES.md).

### Stage 10 — Methodology documentation

**Objective.** Record a practical account of the method, supported outcomes,
limitations, and a procedure others can adapt.

**Implementation.** This case study and the shorter
[development overview](DEVELOPMENT_OVERVIEW.md) were prepared from source and
local records, with a local Qwen prose draft and coordinator review. README links
were added while retaining feature and installation instructions.

**Acceptance and outcome.** Documentation review covers the ten stages, evidence
qualification, relative links, author/version/license consistency, package
status, and accidental disclosure. Preservation hashes are compared for source,
packaging assets, the executable, release artifacts, and the existing launcher.
This stage does not run application builds or fresh monitoring acceptance tests.

**Limitations and lesson.** The narrative is a retrospective account with
incomplete experimental controls, not a formal evaluation. Publication approval
is still required. Missing historical measurements remain unknown rather than
being filled with plausible numbers.

## Why bounded stages helped in this project

A stage reduced the number of simultaneous interfaces and assumptions presented
to the model. Stage 1 could be judged by a compiled process table; a missing GPU
backend no longer prevented acceptance of that baseline. Later stages reused
working GTK patterns and collected data, reducing the need to regenerate known
behavior. Explicit exclusions made it easier to reject unrelated controls,
dependency expansion, or a rewrite disguised as a repair.

Acceptance criteria tied progress to observable artifacts: a real executable,
rows populated from Linux, sorting that survived refresh, and a launcher with a
valid path. Preserving source snapshots and checksums made a proposed change
reviewable against a known state. When a later feature failed, earlier accepted
functionality remained useful instead of disappearing inside an unfinished
replacement.

The project preference was a focused acceptance check followed by repairs of
actual defects, rather than repeated unchanged experiments or broad performance
testing. That preference does not mean every stage needed exactly one compiler
invocation or one GUI run. Some errors required targeted follow-up checks.
Incremental work supported recovery here; it does not mathematically guarantee
correctness, limit effort, or outperform larger tasks on other projects.

## Challenges and recovery

**Structured output and tool calls.** Early task events include invalid-result
parsing failures and uncertain tool outcomes. Local draft records include
incomplete source and undefined sampler functions. Switching to bounded API
output with deterministic writers made the failure boundary clearer: generation,
file application, compilation, and acceptance could be examined separately. The
exact causal contribution of model size versus prompt scope versus integration
changes is unknown.

**False completion.** The owner reported an early completion notification without
an executable or launcher. A later recovery task record states that a completion
acceptance guard was added and checked, and subsequent milestones include
artifact and launch evidence. This case study does not independently audit that
worker or reproduce the earlier notification failure. The transferable lesson
is to keep model prose separate from task success: missing artifacts mean
incomplete or needs-review, and a success notification belongs after acceptance.
No worker or notification configuration was changed during Stage 10.

**Compilation and integration.** Invalid GTK APIs, incomplete definitions,
duplicate bindings, and misplaced initialization required correction. Limiting
repair scope to the actual compiler diagnostic helped avoid restarting whole
subsystems. A valid output schema or successful file write could not substitute
for a compile.

**Visual and live-data behavior.** Theme contrast, table widths, numeric order,
independent dividers, full-screen transitions, and live refresh needed inspection
beyond compiler success. Human feedback and coordinator-assisted GUI observation
were both part of acceptance; the records do not attribute every inspection to
a particular observer. The service comparator defect illustrates why testing
one relevant interaction can matter more than a long list of superficial checks.

**Telemetry honesty.** Missing ownership, unloaded services, absent metrics,
first samples, and failed collection are distinct from zero usage. Showing N/A
and factual warnings preserved that distinction. Monitoring is best effort and
non-atomic across subsystems; the display is not a universal health assessment.

## Reusable staged procedure

1. **Establish a minimal compilable application.** Inspect the installed tools
   and APIs; make the smallest useful program build and launch before adding
   optional subsystems.
2. **Divide features into useful milestones.** Give each stage a result a person
   can use independently and define the behavior it must preserve.
3. **Specify requirements and constraints.** State the allowed scope, data
   sources, privilege limits, dependencies, and excluded actions.
4. **Generate one bounded change.** Supply the relevant interfaces and nearby
   code. Validate expected filenames or replacement anchors and reject incomplete
   or ambiguous output before applying it.
5. **Compile and repair targeted failures.** Send the actual diagnostic and
   relevant code back for correction. Change the approach when an unchanged
   failed attempt would merely repeat the same result.
6. **Perform one meaningful acceptance pass.** Exercise the stage's defining
   behavior with real data and a relevant preservation check. Repair demonstrated
   defects and recheck those repairs rather than expanding into unrelated tests.
7. **Preserve a known-good state.** Keep a reviewed diff, snapshot, or version
   control checkpoint; retain the existing executable or installation until its
   replacement has been accepted. Hashes identify content but do not prove it
   correct.
8. **Record and advance.** Record observed evidence, unresolved limitations, and
   what remains incomplete. Have the responsible human decide the next stage.
9. **Prepare repeatable packaging.** Use clean source lists, conventional paths,
   declared dependencies, target-native builds, and checksums. Separate recipes
   from packages and build success from installed compatibility.
10. **Document the workflow.** Describe roles, failures, corrections, provenance,
    and evidence limits. Require a separate decision before publication.

These steps can use any suitable coordinator, local inference runtime, coding
model, compiler, or GUI framework. Their useful property is an explicit boundary
between proposed changes, applied files, checked behavior, and accepted work.

### Reusable task template

```text
Project/version:
Known-good baseline and how to recover it:
Stage objective: one independently useful behavior
Relevant source/interfaces and allowed files:
Required behavior and data sources:
Preserve: existing features, installation, and user data
Constraints: dependencies, privileges, read-only actions, local generation policy
Excluded work: unrelated refactors, services/configuration changes, publication

Execution:
  Inspect the baseline and installed APIs.
  Generate a bounded change; validate its structure and write only allowed files.
  Compile the actual target; correct demonstrated errors with narrow changes.
  Run one focused acceptance pass; repair and recheck demonstrated failures.

Acceptance evidence:
  Required artifact exists and launches where applicable.
  Defining behavior works on real inputs, including one relevant failure case.
  Earlier affected behavior remains accessible.
  Record commands/results and limitations without secrets.

Needs-review conditions:
  Missing artifact, truncated/ambiguous output, unavailable required environment,
  unresolved compiler/functional failure, or an action outside authorized scope.
  Preserve partial work and the exact blocker; do not report success.

Completion:
  Record the accepted baseline and remaining limitations.
  Ask the responsible human to decide the next milestone or publication.
```

## Reproducibility and limitations

### Observed environment, not a frozen historical toolchain

The following versions were read from the development environment during this
October 15, 2026 documentation stage. Earlier stage-by-stage tool versions,
complete dependency lockfiles, and the exact version of the Codex orchestration
model were not reconstructed and remain unknown.

| Component | Verified observation |
| --- | --- |
| OS/desktop | Manjaro Linux, Arch-family rolling release; Cinnamon desktop |
| Language/build contract | C++17; CMake minimum 3.16 declared in CMakeLists.txt |
| Compiler currently installed | GCC 16.2.1, build date 20260810 |
| CMake currently installed | 4.4.3 |
| gtkmm / GTK currently installed | 3.24.11 / 3.24.52 via pkg-config |
| Local inference runtime currently installed | Ollama 0.32.1 |
| Local model tag | qwen2.5-coder:14b |
| Model metadata currently reported | GGUF, 14.8B, Q4_K_M |
| Tested telemetry hardware | AMD Radeon Pro W5500; eight logical CPUs observed |
| Display check in records | Windowed 1100×750 and secondary-monitor 1920×1080 full-screen |

The locally reported model digest at documentation time is
`9ec8897f747e246e970bc5cfdda85d22f1123dc2e3d34978a010a75968716849`.
This identifies the installed model blob; it does not prove every historical
request used identical runtime settings. A reviewed release-drafting helper used
an 8192-token context setting, temperature 0.1, and a bounded output allowance.
Those helper settings are not asserted for all earlier generations, and no
universal token count, latency, or memory requirement was measured.

### What can and cannot be reproduced from this repository

The repository supplies application source, build/install rules, user guidance,
license notices, packaging recipes, and this method. Private raw prompts, task
IDs, screenshots of the workstation, credentials, and internal endpoint details
are intentionally not part of this document. Readers can reproduce a source
build in a suitable Linux environment and inspect the implementation, but cannot
replay an exact, controlled development experiment from the public files alone.

Use the [README build instructions](../README.md) and native
[DEB](../packaging/deb/README.md) or [RPM](../packaging/rpm/README.md) instructions
as appropriate. Matching a model tag and prompt does not guarantee identical
output; runtime versions, context, inference settings, and integration decisions
also matter. A checksum verifies file identity, not safety or compatibility.

Linux `/proc`, systemd availability, installed helper utilities, permissions,
network namespaces, and accessible amdgpu sysfs affect monitoring coverage.
Per-process GPU/VRAM attribution remains unsupported; some socket owners remain
unknown; GPU hot-plug requires restart; service inventory is loaded units only.
Only one recorded GPU and one extended-display arrangement were verified.
DEB/RPM native compatibility and byte-identical rebuilds remain unverified.

This is one supervised development case. It does not establish general accuracy,
reliability, cost, speed, privacy of every surrounding service, or superiority
over cloud models, other local models, or conventional development. No Windows,
macOS, iOS, or future Pro implementation is provided by this work.

## Possible future research — questions, not findings

A separate evaluation could study how milestone size affects completion
reliability; whether explicit acceptance criteria change defect detection; how
available context relates to implementation complexity; whether checkpoints
improve recovery; and how orchestration changes the usability of local coding
models. Such work would need defined tasks, comparable baselines, repeated runs,
recorded environments, and a distinction between model output and human or
coordinator corrections. No such experiment was performed for G_htop, and these
questions are not results of this case study.

## Publication status

This document is prepared for repository review under the G_htop Community
Apache-2.0 license. It introduces no contributor agreement or new policy.
The later publication-finalization source snapshot includes Stage 10, and the
corrected Arch package is 0.1.0-2 with the full-name notice. Earlier artifacts
are local historical backups. The initial Stage 9 observations above remain a
historical account; DEB/RPM still have no native build verification.
Implementation-source links in installed copies are intended for source
checkouts; packages do not install the C++ source files. Nothing has been published or uploaded. The shorter
[overview](DEVELOPMENT_OVERVIEW.md) provides a practical entry point.
