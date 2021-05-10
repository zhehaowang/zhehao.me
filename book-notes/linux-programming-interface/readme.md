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
* Initialized data segment. Global and static variables that are explicitly initialized. Values of those vars are read from the executable file when the program is loaded into memory.
* Uninitialized data segment (bss, block started by symbol). Global and static variables that are not explicitly initialized. Initialized to 0 before program starts.
  * Reason for separating this from initialized data is these need not be stored in the program. Just storing their location and size in the program is enough, and the area gets allocated by the program loader at runtime.
* Stack frame. One for each function call. Local var, argument, return value.
* Heap. Runtime dynamically allocated memory. Top end of heap is called "program break".


