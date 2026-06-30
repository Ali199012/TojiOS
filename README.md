# Toji OS

A multi-process operating system simulator built for an Operating Systems Lab course. Toji OS provides a GTK-based desktop environment where every application is launched as a **real OS process** via `fork()` + `exec()` — not simulated with function calls — and is scheduled, resource-checked, and managed by a custom kernel layer running entirely in userspace on Linux.

## Why this is more than a typical desktop app

Most "OS simulator" student projects fake process management with function calls. Toji OS doesn't — every one of its 17 applications is spawned as an actual child process, communicates with the kernel over a pipe-based IPC handshake, and is scheduled by a real multi-level queue dispatcher running on its own thread.

## Architecture

Three layers, each with a clear responsibility:

- **Hardware Layer** — simulated CPU cores, RAM, disk, and clock, configurable at boot time
- **Kernel Layer** (kernel mode) — Process Manager, Scheduler, Memory Manager, and IPC module
- **User Layer** (user mode) — all application processes, each running in isolation

```
┌─────────────────────────────────────────┐
│              User Layer                 │
│   17 application processes (fork+exec)  │
├─────────────────────────────────────────┤
│             Kernel Layer                │
│  Process Manager | Scheduler | Memory   │
│            IPC Module                   │
├─────────────────────────────────────────┤
│            Hardware Layer               │
│   CPU cores | RAM | Disk | Clock        │
└─────────────────────────────────────────┘
```

## Core OS Concepts Implemented

- **Process lifecycle** — New → Ready → Running → Blocked → Terminated, with full PCB tracking
- **Multi-Level Queue Scheduler**
  - Level 1 (High) — FCFS, no preemption, reserved for system tasks
  - Level 2 (Normal) — Round Robin, 5ms time quantum
  - Level 3 (Low) — Round Robin, 10ms time quantum
  - Dispatcher always services the highest non-empty level first, preventing starvation
- **Banker's Algorithm** — resource safety check (RAM, disk, CPU cores) before a process is ever admitted
- **Resource request/grant handshake** — a new process requests resources via an IPC pipe; the kernel grants or denies before `exec()` is called
- **POSIX shared memory** (`shm_open` + `mmap`) — the system resource table is shared safely across all processes
- **Synchronization** — mutexes protect shared state; condition variables let the dispatcher thread sleep instead of busy-waiting when queues are empty
- **Kernel/User mode separation** — a Kernel Console app is only accessible in kernel mode, with the ability to forcibly terminate processes and inspect the live resource table

## Applications

System Monitor, Process Manager, File Explorer, Terminal, Settings, Notepad+, Calculator, Calendar, Task Scheduler, File Operations, System Info, Search, Minesweeper, Music Player, Clock & Timer, Snake, Kernel Console — each launched as its own process with its own memory/disk/core budget and priority level.

## Tech Stack

C, GTK3, POSIX threads (pthreads), POSIX shared memory, pipes

## Building & Running

### Prerequisites
- GCC
- GTK3 development libraries

```bash
# Debian/Ubuntu
sudo apt-get install libgtk-3-dev

# Fedora
sudo dnf install gtk3-devel

# macOS (Homebrew)
brew install gtk+3
```

### Build

```bash
make
```

### Run

```bash
./tojios
# or
make run
```

## Project Structure

```
tojios/
├── main.c                 # App registry, process launch logic
├── kernel/
│   ├── boot.c              # Hardware configuration at startup
│   ├── memory.c            # Shared memory, RAM/disk allocation, Banker's algorithm
│   ├── process.c           # Process creation (fork/exec), PCB management
│   └── scheduler.c         # Multi-level queue scheduler & dispatcher thread
├── lib/
│   ├── ipc.c                # Pipe-based grant/deny messaging
│   ├── sync.c                # Mutex/condition variable helpers
│   └── utils.c               # Logging and shared utilities
├── apps/                   # 17 applications, each a standalone process
├── include/minios.h        # Shared types and declarations
└── Makefile
```

## Design Documentation

Full design artifacts — process lifecycle flowchart, scheduling flowchart, system architecture diagram, and resource allocation strategy — are documented separately as part of the Phase 2 design phase of this project.
