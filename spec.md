OS Feature Specification Project profile: General-purpose desktop OS, hybrid kernel (NT-style), from scratch, x86_64, real hardware target, solo developer, C/C++/Rust mixed, Limine bootloader, CLI-first with GUI later, custom package manager, adapted existing filesystem (FAT32/ext2), full TCP/IP + Wi-Fi networking as a priority, standard (not maximal) security posture, GPU-accelerated graphics as an eventual goal. Driver placement: storage, GPU, and network drivers run in kernel mode (critical path); everything else (input, audio, USB HID, etc.) runs in userspace as servers over IPC. Syscall ABI: POSIX-shaped, not POSIX-compliant — familiar shapes (open/read/write/close-style calls) without committing to full POSIX semantics, to make future porting easier without being bound by it. GUI philosophy: flat, saturated, widget-first design — bold rounded-square iconography color-grouped in rows, dark semi-translucent "glass" panels for widgets/dialogs over a photo wallpaper, oversized clock/status typography, and a bottom taskbar with launcher + pinned apps + status tray. Confirmed by user-provided mockups (see Design Language section below) — not a macOS/GNOME clone, closer to a colorful flat/Material-adjacent aesthetic with heavy glass-panel chrome. Multi-user: matters — real accounts, permissions, and switching, not a single-user toy system. Licensing: GPL-style copyleft, intended for eventual public/open-source release. Accessibility: low priority for v1, revisit once the GUI stack exists. This document is meant to be handed to an AI assistant (or used as your own reference) to keep design decisions consistent as the project grows. Items are grouped by subsystem, roughly in the order you'll actually build them, with notes on where your specific choices (hybrid kernel, Rust+C++, Limine, Wi-Fi priority, broad hardware support, GPL) change the "default" answer. Scope reality check — worth reading before anything else "Broad hardware support (many laptop brands/chipsets)" and "solo developer" are in real tension. Linux has thousands of contributor-years behind its hardware support; even narrowing to "modern x86_64 laptops" still means a long tail of Wi-Fi chipsets, touchpad controllers, and ACPI quirks that took the Linux community over a decade to cover. Two honest paths forward: Design for broad support, launch with narrow support. Build driver frameworks (USB, PCI, network) that are architecturally ready to add chipsets, but ship v1 targeting a small, deliberately chosen hardware list (e.g., one Intel Wi-Fi generation, one GPU vendor, your own laptop) — then expand as contributors join post-open-source-release. Lean on standards over vendor-specific drivers where possible — AHCI, NVMe, xHCI, and Intel HDA are all standardized enough that "one driver" covers most vendors. Wi-Fi and GPU are the two areas where this doesn't hold, and they're precisely the two you're both prioritizing — budget the most time and the most willingness to cut early scope there. Recommendation: treat "broad hardware support" as a post-open-source goal, not a v1 requirement — it's exactly the kind of work that benefits from contributors once the project is public, and trying to hit it solo first will slow everything else down. Decided: v1 targets your own current laptop/PC only — exact make, model, CPU, GPU, and Wi-Fi chipset to be filled in below once provided, so every driver section in this document can name the specific chipset instead of a generic category.

Exact v1 hardware target: Chromebook running coreboot/Linux, and high-performance desktop.

Boot & Init
- [x] Limine bootloader (Limine protocol, not legacy Multiboot2, for framebuffer/HHDM/memory map handoff)
- [x] Limine config for multiple boot entries (normal, safe mode, verbose/debug)
- [x] Early serial console logging (before framebuffer console exists)
- [x] Kernel command-line argument parsing (boot flags: debug, nosmp, safe-mode)
- [x] Init system / PID 1 equivalent (own design, not systemd-compatible)
- [x] Staged boot: kernel init → driver init → services → userspace init → shell/GUI
- [x] Boot splash (optional, framebuffer-based, skippable with verbose flag)
- [x] Panic screen with register dump + stack trace, distinct from normal logging

Kernel Core (Hybrid Architecture)
- [x] Hybrid kernel with a decided split: storage, GPU, and network drivers run in kernel mode for performance on the critical path; everything else (input/HID, audio, USB non-storage classes, etc.) runs in userspace as servers communicating over your IPC mechanism
- [x] Formal driver classification list — as new device classes come up, decide "kernel or userspace" against this rule up front rather than case-by-case, so the boundary stays consistent
- [x] GDT/TSS setup, per-CPU TSS for SMP
- [x] IDT with full exception handlers (not just page fault — GP fault, divide error, invalid opcode, etc., all need real handlers, not just "halt")
- [x] APIC (Local APIC + I/O APIC) instead of legacy PIC
- [ ] SMP bring-up (AP trampoline code, per-core stacks, per-core GDT/TSS)
- [x] Preemptive scheduler, per-core run queues
- [x] Priority-based or CFS-style scheduling (round-robin is a fine v1)
- [x] Full context switch including FPU/SSE/AVX state (XSAVE/XRSTOR)
- [x] Kernel threads before user processes (validate scheduler in isolation)
- [x] syscall/sysret fast path (not legacy int 0x80)
- [x] Per-process page tables (not shared address space)
- [x] Spinlocks, mutexes, semaphores, condition variables as kernel primitives
- [x] Kernel heap allocator (slab/slob-style) separate from physical frame allocator
- [x] IPC mechanism — decided: hybrid IPC, message-passing for control/small messages plus shared memory for bulk data transfer. This matches your hybrid kernel model (it's what Windows NT's LPC + shared sections do, and what modern microkernels like seL4/ Zircon do) and avoids the two failure modes of a pure approach: pure message-passing is too slow for framebuffer/large-file data; pure shared memory lacks synchronization/safety for control signals.
- [x] Message-passing primitive: synchronous call/reply (client blocks until server replies) as the default, since it's simpler to reason about; consider async/queued messages only for specific cases (e.g., input events, notifications) once the sync path is solid
- [x] Shared memory primitive: kernel-mediated shared memory regions (client and server both map the same physical pages) — used for framebuffer data, large file reads/writes between VFS and storage drivers, and GPU command buffers
- [x] Capability/handle-based access to IPC endpoints (a process needs an explicit handle to talk to a server, not a global namespace) — this keeps your "standard security posture" intact without extra design work later
- [x] Designed early because retrofitting IPC into a hybrid kernel later is painful — every kernel-mode driver (storage/GPU/network) and every userspace server (HID, audio, etc.) depends on this from day one
- [x] Kernel panic/crash dump collection with symbol resolution

Rust/C/C++ Interop (specific to your language mix)
- [ ] Decide subsystem boundaries by language now, not later — e.g., C for low-level boot/arch code, Rust for drivers and safety-critical subsystems, C++ for higher-level services/userspace libraries
- [ ] Stable extern "C" ABI boundary between all cross-language modules
- [ ] #![no_std] Rust kernel crates with a custom allocator shim
- [ ] Shared build system that handles all three toolchains (Meson/CMake + Cargo, or a custom build orchestrator — pick one early)
- [ ] Bindgen/cbindgen (or hand-written headers) to keep C/C++/Rust struct layouts in sync — this breaks silently and often, worth automating
- [ ] Panic handler strategy for Rust code running in kernel context (no unwinding — abort/halt semantics)
- [ ] Decide early whether C++ exceptions are allowed in kernel space (usually: no)

Memory Management (building on what you have)
- [x] Physical frame allocator (buddy allocator recommended at this stage)
- [x] Virtual memory manager with per-process address spaces
- [ ] Demand paging
- [ ] Copy-on-write for fork()-equivalent process creation
- [ ] Memory-mapped files (mmap-equivalent)
- [x] Guard pages for stack overflow detection
- [ ] NX bit enforcement (W^X policy) from day one — cheap now, painful to retrofit
- [ ] Basic KASLR (kernel base randomization)
- [ ] Swap/pagefile support (can be late-stage)
- [ ] OOM handling policy (what happens when memory is exhausted)

Process & Thread Model
- [x] Process creation/termination primitives
- [x] Thread creation within a process, shared address space
- [x] Process states (running, ready, blocked, zombie) and a real state machine
- [ ] Signals or an equivalent async-notification mechanism
- [ ] Process groups / job control (for shell job control later)
- [ ] Resource limits per process (memory, CPU time, open handles)
- [x] Handle/descriptor table per process (files, sockets, IPC endpoints unified or separate — decide now)
- [ ] Zombie reaping / wait() equivalent

Filesystem (adapting an existing FS, per your choice)
- [x] VFS abstraction layer — build this before wiring up FAT32/ext2, so you're not hardcoding one filesystem's assumptions into every syscall
- [x] FAT32 driver (simplest, good first real filesystem, useful for interop with USB drives / other OSes)
- [ ] ext2 driver (if you want Linux image interop / familiar semantics) — read support first, write support second
- [x] Block device abstraction layer beneath the VFS (so FS code doesn't touch hardware directly)
- [x] Buffer cache / page cache for block devices
- [x] Mount/unmount support, multiple filesystems mounted simultaneously
- [x] Path resolution with proper ./../symlink handling
- [x] File permissions model (even a simple owner/read/write/execute model)
- [x] Device files / special files (/dev-equivalent) exposed through VFS

Storage Drivers
- [x] AHCI driver (SATA) — needed for real hardware
- [x] NVMe driver — increasingly necessary for modern laptops, prioritize after AHCI is stable
- [x] USB mass storage driver (depends on your USB stack — see below)
- [x] Partition table parsing (GPT primarily, MBR for compatibility)
- [x] Disk I/O scheduler (even a simple one — matters once you have real HDD/SSD mixes)

USB Stack (needed for real hardware — Wi-Fi dongles, storage, input)
- [x] xHCI controller driver — since USB mass storage feeds your kernel-mode storage stack, the xHCI host controller driver itself likely belongs in-kernel even though HID/other USB classes are userspace
- [x] USB device enumeration and descriptor parsing (kernel-mode, since it's the layer both kernel storage and userspace HID depend on)
- [x] HID class driver (keyboards, mice) — userspace, per your driver placement model; talks to the kernel-mode USB core over IPC
- [x] Mass storage class driver — kernel-mode, feeds directly into your storage stack (section 7)
- [x] USB hub support (nested hubs, real laptops have several)

Networking (full stack — priority per your answer)
- [x] Network interface abstraction layer (NIC drivers register here)
- [x] Ethernet driver(s) for common chipsets (Intel e1000 is the classic "easy" first real NIC driver, also what QEMU emulates by default)
- [x] Wi-Fi driver stack — this is the hard part you flagged as important:
  - [x] 802.11 frame handling layer (separate from Ethernet framing)
  - [x] A mac80211-equivalent abstraction so individual chipset drivers don't each reimplement association/authentication logic
  - [x] Start with one well-documented chipset family (Intel iwlwifi-class cards are the most realistically achievable for a solo dev — Broadcom and Realtek are notoriously harder due to closed firmware blobs)
  - [x] WPA2/WPA3 supplicant (handshake logic) — this alone is a significant sub-project, consider whether to port wpa_supplicant logic conceptually or write minimal WPA2-only support first
  - [x] Firmware blob loading mechanism (most Wi-Fi chips require loading vendor firmware at runtime — plan storage/loading for this early)
- [x] ARP
- [x] IPv4 stack (fragmentation, routing table, ICMP)
- [ ] IPv6 stack (can follow after IPv4 is solid)
- [x] UDP
- [x] TCP (congestion control, retransmission, window scaling — this is a multi-month project on its own if done properly)
- [x] DHCP client
- [x] DNS resolver (stub resolver is fine initially)
- [x] Socket API (BSD-socket-style is the pragmatic choice even in a custom OS, since every network-aware program you port will expect it)
- [x] Loopback interface
- [x] Firewall/packet filter hooks (even basic allow/deny rules)
- [x] TLS library port or integration (needed the moment you want HTTPS anything)

Graphics (GPU-accelerated, eventual goal)
- [x] Stage 1: Limine framebuffer console (linear framebuffer, software text rendering)
- [x] Stage 2: Generic framebuffer graphics API (put-pixel, blit, simple 2D primitives)
- [ ] Stage 3: Mode-setting driver — decide GOP/UEFI framebuffer vs writing a real KMS-style mode-setting driver per-GPU
- [ ] Stage 4: GPU driver — this is a major scope decision:
  - [ ] Realistic solo path: target one GPU vendor's open-source-documented path (Intel integrated graphics has the best public documentation; AMD has open register specs; Nvidia is the hardest due to closed firmware)
  - [ ] Command submission / ring buffer management
  - [ ] Memory management unit for GPU (separate from CPU MMU — GPU has its own VA space)
  - [ ] 2D acceleration first (blitting, compositing) before 3D
  - [ ] 3D: decide whether to implement a Vulkan-subset driver (modern, harder) or a legacy GL-subset (more tutorials exist, less "modern")
- [x] Display compositor (your own, since GUI is planned) — window buffer compositing, damage tracking, vsync-aware presentation
- [ ] Multi-monitor support (can be deferred well past v1)

GUI / Desktop Environment (later phase, per your roadmap)
Design language (from your mockups — treat as the actual spec, not a placeholder)
- [x] Iconography: solid, saturated, flat-colored rounded squares (large corner radius, no gradients/shadows beyond a subtle drop shadow) — icons color-grouped in rows/categories rather than alphabetically or by install date
- [x] Panels/dialogs: dark, semi-translucent "glass" rounded rectangles floating over the wallpaper — this is your dialog/widget chrome throughout the OS (login box, app widgets, popups), not just one screen
- [x] Typography: oversized serif or semi-serif display font for the clock specifically; smaller clean sans-serif for body/UI text — establish this as two deliberate type tracks (display vs UI) rather than one font doing both jobs
- [x] Desktop model: widget-first, not icon-grid-first — the "recent apps" / "updates" panel is a persistent desktop widget with pagination arrows, meaning your compositor needs a widget layer distinct from both the app grid and the taskbar
- [x] Wallpaper: full-bleed photographic wallpaper is a first-class background element that panels float over (not a flat color) — panels need real alpha blending against arbitrary imagery, not just a solid theme color
- [x] Taskbar: persistent bottom bar with three zones — hamburger/start menu (left), pinned app shortcuts (left-center), status tray (right: updates, Bluetooth, Wi-Fi icons)
- [x] Clock/status area: always-visible time + timezone label (including UTC offset display, per the second mockup) — this is a persistent desktop element, not just a lock-screen feature

Desktop UX behavior (exact decisions — no ambiguity left)
- [x] Wallpaper: static image, user-changeable via Settings (no dynamic time-of-day wallpaper cycling — that's explicitly out of scope for now)
- [x] App grid organization: fixed order only — install order or alphabetical (pick one and document it; no manual drag-to-reorder, no auto-categorization by app type)
- [x] App grid overflow: paginate to additional pages, and support a swipe-down gesture to reveal the full app drawer (Android-style) — both interaction paths need to work, not just one
- [x] Status tray: fully user-customizable — build a tray-icon registration API that apps/services can register into, plus a Settings panel where the user toggles which registered icons are visible; don't hardcode a fixed icon set
- [x] Window management: hybrid — windows float and are draggable/resizable by default, with an optional tiling mode (e.g., a keybind or edge-snap that auto-arranges) — both modes need to coexist, not be mutually exclusive
- [x] Taskbar pinned apps: user-customizable — add, remove, and reorder pinned shortcuts (drag-and-drop onto/off the taskbar)
- [x] Notifications: a notification-center panel (opened via swipe or click) that retains history — not transient popup-only banners; popups can still appear briefly on arrival, but must also land in the history panel
- [x] Light/dark mode: both supported, user-toggleable in Settings (not automatic/time-based — explicit user choice)
- [x] Global search: one entry point (likely from the taskbar/launcher) that searches apps, files, and settings simultaneously — needs a unified search index/service, not three separate search boxes
- [x] Accent color theming: user picks an accent color in Settings that retints UI elements (buttons, toggles, highlights, focus rings) across the whole toolkit — build accent color as a toolkit-wide theme variable from the start, not hardcoded per-widget colors
- [x] Animation speed: user-configurable (a Settings slider/toggle, e.g. off/fast/normal) — the compositor/toolkit's animation system needs a global speed multiplier baked in from day one, not added retroactively
- [x] Bundled apps (v1 full suite, not deferred to package manager):
  - [x] File manager
  - [x] Settings app
  - [x] Terminal emulator
  - [x] Web browser — decided: port an existing rendering engine core (WebKit/Blink/Servo-class) rather than write one from scratch. This is the single largest subsystem in the entire OS by likely engineering time — treat it as its own project with its own milestones, separate from your desktop environment timeline:
    - Evaluate porting difficulty per engine: Servo (Rust-based, may align well with your Rust usage, but historically less complete/stable as a standalone embeddable target), WebKit (mature, embeddable via WebKitGTK/WPE-style ports, large C++ codebase), Blink/Chromium (most capable, also by far the heaviest port — assumes a large POSIX-like surface, a real process model, GPU compositing, and a sandboxing model)
    - This is exactly where your POSIX-shaped (not compliant) syscall decision pays off — ported engines assume POSIX-ish primitives (mmap, threads, sockets, file I/O); budget time to build a compatibility shim layer translating your syscalls to what the engine's portability layer expects
    - Sandboxing/process-per-tab model — modern engines assume this for security; decide whether you replicate it (recommended, given "standard security posture") or run single-process (simpler, less safe)
    - GPU compositing path for the renderer — connects directly to your GPU driver work in section 10; the browser will likely be your GPU driver's most demanding client
  - [x] Media player (audio + video playback, ties into the audio driver and eventually GPU video decode)
  - [x] Text editor
- [x] Settings structure — two separate apps, not one:
  - [x] Simple Settings app (Windows Settings-style): common day-to-day options — Wi-Fi, Bluetooth, display, accounts, wallpaper/theme, accent color, notifications, basic power settings — card/category based navigation, minimal jargon
  - [x] Advanced/Administrator Settings app (Windows Control Panel-style): deeper system configuration — device manager equivalent, advanced network config, user/group management beyond basic accounts, driver management, disk/partition tools, advanced power plans, system diagnostics — denser, more technical UI, may require elevated/admin privileges to open certain panels
  - [x] Decide the boundary between the two now (which settings live where) and document it, so future settings additions have a clear home
- [x] Keyboard shortcuts: globally customizable — user can rebind any system shortcut (GNOME-style), which means:
  - [x] A central keybinding registry/service, not shortcuts hardcoded per-app or per-component
  - [x] Conflict detection when a user tries to rebind to an already-used combination
  - [x] A default shortcut set shipped out of the box, fully overridable
  - [x] Shortcut configuration lives in the Simple Settings app (per the structure above) since it's a common user-facing customization
- [x] Full-bleed wallpaper background, no letterboxing
- [x] Large clock top-left with timezone name/offset beneath it
- [x] Power button (top-right, circular, icon-only)
- [x] Centered-offset glass panel: "welcome back, {user}" greeting, circular user avatar, password field, and a "not you?" button that opens user switching — build this as a real component (avatar + password + switch-user) since multi-user requires this to work, not just look right
- [x] User-switch flow: "not you?" needs to lead to an account picker, not just a dead button — tie this directly into the multi-user session manager from section 17

Core components
- [x] Windowing system core (window creation, z-ordering, focus management)
- [x] Custom widget toolkit (buttons, text fields, lists, and a distinct "glass panel" container component used for both dialogs and desktop widgets)
- [x] Event system (input events routed to focused/hovered window)
- [x] Font rendering — needs to support at least two distinct type styles (display/clock font + UI font) cleanly; start bitmap, move to TrueType/FreeType-style rendering once basics work
- [x] Compositor with damage-region tracking and real alpha blending (glass panels over photographic wallpaper need proper compositing, not just opaque rectangles)
- [x] App launcher grid (collapsible, per the chevron in your mockup) — color-grouped rows of rounded-square icons
- [x] Desktop widget layer (recent apps, updates, and future widgets — build this as an extensible widget system, not one hardcoded panel)
- [x] Taskbar (hamburger menu, pinned apps, status tray icons for update-available, Bluetooth, Wi-Fi at minimum)
- [x] Settings app (network, display, users/accounts, since multi-user matters)
- [x] File manager
- [x] Terminal emulator (you'll want this even with a GUI, for your own dev workflow)
- [x] Clipboard support (system-wide, cross-window)
- [x] Drag-and-drop between windows
- [x] Accessibility hooks deferred but not foreclosed: keep text rendering and widget focus order clean now so screen-reader support and high-contrast theming can be added later without a toolkit rewrite

12. Input
- [x] PS/2 keyboard/mouse driver (useful fallback even on modern hardware, some VMs use it)
- [x] USB HID driver (primary path for real laptops)
- [x] Keyboard layout system (even just US QWERTY + one or two others to start)
- [x] Touchpad driver (precision touchpad protocol on modern laptops — this is its own can of worms, budget real time for it)

13. Audio
- [ ] Intel HDA (HD Audio) driver — userspace, per your driver placement model (audio isn't storage/GPU/network)
- [ ] Audio mixer service (userspace, talks to apps and the HDA driver over IPC)
- [ ] PCM playback/capture API for userspace apps

14. Package Management (custom, per your answer)
- [ ] Package format spec (metadata: name, version, dependencies, install paths)
- [ ] Dependency resolver (even a simple topological-sort resolver is fine at first)
- [ ] Local package database (installed packages, versions, file ownership for clean uninstall)
- [ ] Package build/creation tool (so you and others can package software for your OS)
- [ ] Repository server format (how packages are hosted/fetched — can start as a flat file server before anything fancy)
- [ ] Signature verification for packages (even basic, before you have real users)
- [ ] CLI package manager tool (install, remove, update, search, info)
- [ ] Update mechanism with rollback (don't let a bad package update brick the system)

15. Userspace / Libc
- [x] Minimal custom libc (or port newlib/musl as a base and extend) — string, memory, math, stdio equivalents
- [x] Syscall wrapper library (userspace-facing API over your raw syscalls)
- [ ] Dynamic linker/loader if you want shared libraries (can defer — static linking is a reasonable v1 simplification)
- [x] ELF loader in the kernel (parse, map segments, set up entry point)
- [x] fork/exec-equivalent or your own process creation API (doesn't have to be POSIX fork — decide deliberately since POSIX compat is "nice to have" not required)
- [x] Environment variables, argument passing to new processes

16. Shell & CLI Tools
- [x] Interactive shell (your own — command parsing, piping, redirection)
- [x] Core utilities: ls, cat, cp, mv, rm, mkdir, ps, kill, grep-equivalent, echo
- [x] Job control (background processes, &, ctrl-Z equivalent — ties into process groups above)
- [x] Shell scripting support (even basic — sequencing, conditionals)
- [x] Command history, tab completion

17. Security & Multi-User Accounts (standard posture, multi-user matters)
- [x] Real UID/GID model — not a token single-user stub, since multi-user support is a stated requirement
- [x] User account database (usernames, hashed passwords, home directories, groups)
- [x] Login manager — text-mode login prompt first, graphical login screen once the GUI exists
- [x] Fast user switching (multiple logged-in sessions, switch without logging out — a real desktop OS expectation you flagged)
- [x] Per-user home directory convention and default permissions
- [x] File permission enforcement at VFS layer (owner/group/other, at minimum)
- [x] Process privilege separation — root/admin vs standard user, with a sudo/su-equivalent elevation mechanism
- [x] Session management (track which processes belong to which login session)
- [x] ASLR for userspace binaries
- [x] Stack canaries (compiler-level, cheap to enable, worth doing now)
- [x] Password hashing via a well-known KDF (Argon2 or bcrypt — don't roll your own)
- [x] Secure random number source (needed for ASLR, crypto, TCP sequence numbers)
- [x] Account lockout / login rate-limiting (basic brute-force protection)

18. Diagnostics & Dev Tools
- [x] Kernel log ring buffer + dmesg-equivalent viewer
- [x] Serial debug output toggle
- [x] Kernel debugger hooks (GDB stub over serial is the standard hobby-OS approach)
- [x] Crash/panic report format (register state, stack trace, loaded modules)
- [x] Basic profiling/perf counters (even just scheduler tick counts, page fault counts)
- [x] Unit test framework for kernel code that can run host-side (test logic outside QEMU where possible)

19. Licensing & Governance (public GPL/LGPL release, planned)
- [x] License split — decided: LGPL for libraries (your libc, widget toolkit, any shared runtime libraries apps link against), GPL for everything else (kernel, drivers, servers, bundled applications, package manager). This lets third-party apps link your libraries without being forced GPL themselves, while keeping the OS core and first-party apps copyleft.
- [x] Decide GPLv2 vs GPLv3 specifically for the GPL portion (v3 adds patent/tivoization protections relevant if you ever worry about locked-down hardware running your OS; v2 is simpler and what Linux itself uses) — still open, worth deciding before public release
- [x] Decide LGPLv2.1 vs LGPLv3 for the library portion — same v2-vs-v3 tradeoff applies
- [x] Draw the exact library/non-library line now: is your widget toolkit LGPL even though GUI apps built with it are part of the "everything else" GPL bucket? Document this explicitly so future contributors know which license header goes on which new file
- [x] License headers in every source file from the start (retrofitting this across a whole codebase later is tedious and easy to miss files)
- [x] CONTRIBUTING.md and a basic code-of-conduct before going public — reduces friction for first-time contributors
- [x] Decide on a Contributor License Agreement (CLA) or Developer Certificate of Origin (DCO) policy before accepting external patches
- [x] Public issue tracker / roadmap (even a simple one) so "broad hardware support" work can be crowdsourced post-launch, per the scope note above
- [x] Trademark/naming decision for the OS itself, if you plan to protect the name

20. Build & QA Infrastructure
- [x] Reproducible build system (single command from clean checkout to bootable ISO)
- [x] QEMU test harness (automated boot test — does it reach a shell prompt without hanging?)
- [x] CI pipeline (even solo, catch regressions automatically)
- [x] Real-hardware test checklist (since real hardware is your target, QEMU passing isn't sufficient — track a list of physical machines/hardware you test on)
