# Process

A process is an instance of an executing program.

A program is a file containing a range of information that describes how to construct a process at run time.
Information including:
* Binary format identification. ELF (executable and linking format) is predominant these days.
* Machine-language instructions.
* Program entry-point address.
* Data. Values used to initialize variables, literal constants such as strings.
* Symbol and relocation tables. Location and names of functions and variables within the program. (For debugging and runtime symbol resolution / dynamic linking)
* Shared library and dynamic linking information. (shared libraries the program needs and the paths to load them)
* Other.

From the kernel’s point of view, a process consists of user-space memory containing program code and variables used by that code, and a range of kernel data structures that maintain information about the state of the process (various identifiers, virtual memory tables, table of open file descriptors, signal delivery and handling info, process resource usage and limits, current working directory, etc).

With the exception of a few system processes (e.g. system process `init` uses PID 1) there is no fixed relationship between PID of a process and the program it runs.

Each process has the PID of its parent process, forming a tree-like structure viewable with `pstree` and tracing back to `init` as root.
If a child process becomes orphaned because its birth parent terminates, it becomes adopted by `init`.

### Process memory layout

* Text segment, program code, sharable between many processes running the same program (i.e. mapped into the virtual memory address space of all such processes), read-only
_how does runtime code patching work then?_
* (User) initialized data segment. Global and static variables (including local static) that are explicitly initialized. Values of those vars are read from the executable file when the program is loaded into memory.
* Uninitialized data segment (bss, block started by symbol; zero-initialized segment). Global and static variables (including local static) that are not explicitly initialized. Initialized to 0 before program starts.
  * Reason for separating this from initialized data is these need not be stored in the program. Just storing their location and size in the program is enough, and the area gets allocated by the program loader at runtime.
* Stack frame. One for each function call. Local var, argument, return value.
* Heap. Runtime dynamically allocated memory. Top end of heap is called "program break".

### ABI

An **application binary interface (ABI)** is a set of rules specifying how a binary executable should exchange information with some service (e.g., the kernel or a library) at run time.
Among other things, an ABI specifies which registers and stack locations are used to exchange this information, and what meaning is attached to the exchanged values.

Once compiled for a particular ABI, a binary executable should be able to run on any system presenting the same ABI.
This contrasts with a standardized API (such as SUSv3), which guarantees portability only for applications compiled from source code.

C program environment on most UNIX implementations (including Linux) provides three global symbols: `etext`, `edata`, and `end`. These symbols can be used from within a program to obtain the addresses of the next byte past, respectively, the end of the program text, the end of the initialized data segment, and the end of the uninitialized data segment. Declare these `extern` in your source to use them.

### Virtual memory management

When discussing process memory layout we are talking in terms of **virtual memory**, a technique used to efficiently utilize CPU and RAM by exploiting spatial and temporal locality.
The upshot of locality is we can keep only a subset of a process's address space in RAM in order to run it.

RAM is individed into small fixed size **page**s, by default 4KB ones.
At any one time, only some of the pages of a program need to be resident in physical memory page frames.
These pages form the so-called **resident set**.

Copies of the unused pages of a program are maintained in the **swap** area: a reserved area of disk space used to supplement the computer's RAM and loaded into physical memory only as required.

When a process references a page that is not currently resident in physical memory, a **page fault** occurs, at which point the kernel suspends execution of the process while the page is loaded from disk into memory.

Typically, large ranges of the potential virtual address space are unused, so that it isn't necessary to maintain corresponding page-table entries. If a process tries to access an address for which there is no corresponding page-table entry, it receives a `SIGSEGV` signal.

A process’s range of valid virtual addresses can change over its lifetime, as the kernel allocates and deallocates pages (and page-table entries) for the process.
This can happen when
* stack growing downwards past the previous limit
* heap memory allocated / deallocated (raising the program break using `brk`, `sbrk`) or `malloc` family of functions
* System V shared memory regions are attached using `shmat` and detached using `shmdt`
* memory mappings are mapped/unmapped using `mmap` and `munmap`

Paged memory management unit (PMMU) translates virtual address to physical address, and advises the kernel of a page fault when a particular virtual memory corresponds to a page that is not resident in RAM.

Virtual memory management achieves
* Processes are isolated from one another and from the kernel, so that one pro- cess can’t read or modify the memory of another process or the kernel. (accomplished by having the page-table entries for each process point to dis- tinct sets of physical pages in RAM (or in the swap area).
* Where appropriate, two or more processes can share memory.
  * When multiple processes executing the same program, text segment can be shared. (Or when multiple processes load the same shared library, that code can be shared)
  * Processes can use `shmget`/`mmap` to explicitly request sharing of memory with another process, for inter-process communication.
* Facilitate memory protection (page is readonly, writeable, executable, etc, for a particular process)
* Programmer, compiler and linker needn't be concerned with the physical layout of the program in RAM.
* Because not all memory used by the process need to live in RAM, the program loads and runs faster. (also each process uses less RAM increases the amount of processes that can live in RAM, and at any moment CPU is less likely to be idle.)



