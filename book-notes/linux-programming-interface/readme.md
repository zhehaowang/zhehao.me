# Concepts

**Operating system**: the central software managing a computer's resources and all of the accompanying standard software tools, such as command-line interpreters, graphical user interfaces, file utilities, etc.

**Kernel**: just the central software managing and allocating CPU, RAM and devices. (i.e. it provides a software layer to manage the limited resources of a computer). Its tasks
* Process scheduling. (Linux being multitask - multiple processes resides in memory at the same time. preemptive - kernel scheduler decides which process to run.)
* Memory management. (Linux uses virtual memory - process memory address is isolated, only part of a process memory is kept in memory.)
* Provision of a file system (crud).
* Process creation and termination (giving a process resources and reclaiming resources after done).
* Access to devices.
* Networking.
* Provision of a system call API.
* Virtual private computer (Linux being multiuser).

The Linux kernel executable is typically `/boot/vmlinuz`. z - compressed, vm - called this way after virtual memory's implemented.

Modern processor allows the CPU to operate in **user mode** and **kernel mode** (switched by hardware instructions).
Areas of virtual memory can also be marked **user space** and **kernel space**: CPU in user mode cannot access kernel address space.

Certain operations can only performed when the processor is in in kernel mode (like initiating IO operation).

* A process typically doesn't know where it is located in RAM,
* doesn't know where on the disk drive the files it accesses are being held,
* can't directly communicate with another process,
* can't itself create a new process or end its own existence (i.e. it requests the kernel to do so).
By contrast,
* The kernel maintains data structures containing information about all running processes and updates these structures as processes are created, change state, and terminate.
* The kernel maintains all of the low-level data structures that enable the filenames used by programs to be translated into physical locations on the disk.
* The kernel also maintains data structures that map the virtual memory of each process into the physical memory of the computer and the swap area(s) on disk.
* All communication between processes is done via mechanisms provided by the kernel.
* The kernel (in particular, device drivers) performs all direct communication with input and output devices, transferring information to and from user processes as required.

A **shell** is a special-purpose program designed to read commands typed by a user and execute appropriate programs in response to those commands.
On UNIX systems, the shell is a user process.
* Bourne shell (sh) featuring io redirection, pipes, globbing, variables, env var manipulation, functions, background command execution, etc
* Bourne again shell (bash) GNU's implementation of Bourne shell.
* C shell, Korn shell

### User & Groups

`/etc/passwd` includes username, uid, (first) gid, home directory, login shell.
`/etc/group` includes groupname, gid, user list.

`superuser` / `root` (uid 0) bypasses all file permissions / can send signal to any process.

Each process has a real uid, gid, an effective uid, gid, and supplementary gids.

### File & directory

The kernel maintains a single hierarchical directory structure to organize all files in the system (regardless of the disk device as Windows would care about).

A directory is a special file whose contents take the form of a table of filenames coupled with references to the corresponding files. (`.` and `..` are special links (i.e. filename-plus-reference association). `/..` is `/`.)

Symlink (to be differentiated from hard link) is a specially marked file containing the name of another file.
When a pathname is specified in a system call, in most circumstances, the kernel automatically follows each symbolic link in the pathname (limited number of depth).

Permission in a directory: read allows listing content, write allows contents (filenames) to be added / removed / changed, and execute allows access to files within the directory and entering that directory.

### File IO model

**Universality** of IO: `open` `read` `write` `close` perform IO on all types of files, including devices.

(UNIX systems have no end-of-file character; the end of a file is detected by a read that returns no data.)

The IO system refers to open files using a file descriptor, obtained by a call to `open()`.
Normally a process inherits three open file descriptors when it's started by the shell. 0 stdin, 1 stdout, 2 stderr (in an interactive shell, these are normally connected to the terminal).

### Memory mapping

`mmap()` system call creates a new memory mapping in the calling process’s virtual address space.
* A **file mapping** maps a region of a file into the calling process's virtual memory. Once mapped the program can access bytes of the file directly (pages of the mapping are loaded as needed).
* An **anonymous mapping** does not have a corresponding file. The pages of the mapping are initialized to 0.

Memory mapping of a process can be shared with mappings in other processes.
When they share the same page, each process can see others' changes depending on whether the mapping is created as private or shared.
When private, modification to contents of the mapping is not carried through to the underlying file (and not visible to other processes). Shared mapping modifications are carried through to underlying file and visible to other processes.

Memory mappings serve a variety of purposes, including initialization of a process's text segment from the corresponding segment of an executable file, allocation of new (zero filled) memory, memory-mapped file IO and IPC (via a shared mapping).

### Static and shared libraries

Object library (compiled binary code).
Static library / archive: structured bundle of compiled object modules. To use functions from static library, we specify the library in link command used to build the program, and after resolving the various function references from the main program to the modules in the static library, the linker extracts copies of object modules from the library and copies into the executable file.

Drawbacks including each program having a copy wastes disk space. Correspondingly, waste of memory when programs using the same statically linked library function run at the same time.
Additionally if a library requires modification, then after recompiling the library all application needs to be relinked.

Shared libraries don't copy object modules from the library into the executable, the linker just writes a record into the executable to indicate at runtime the executable needs to use that shared library.
When program gets loaded into the memory at runtime, a program called dynamic linker ensures shared libraries required by the program are found and loaded into memory, and performs runtime linking to resolve function calls in the executable to the corresponding definitions in the shared library.

Reduced copies in memory and easier to make sure a program uses the latest version.
(_A header from so change would still require a recompile of executables using it, usually such should be identified by the version of the so? What if an so masquerades as a particular version? that would be how dll injection works? but what if .h doesn't match? Probably undefined behavior, like the stack is a couple bytes short?_)

### Signals

Signals are sent to a process by the kernel, by another process or by the process itself.
The kernel may send a signal when
* user types interrupt character on the keyboard
* one of process's children terminated
* a timer set by the process has expired
* the process attempted to access an invalid memory address
* shell `kill` command / `kill()` system call

The program can install custom signal handlers.
Normally a signal is delivered as soon as the receiving process is next scheduled to run, or immediately if the process is already running.
However it's also possible to block a signal temporarily by adding it to the process's signal mask. They get blocked until removed from the signal mask.

### Threads

Each thread is executing the same program code and shares the same data area and heap. However, each thread has it own stack containing local variables and function call linkage information.

The primary advantages of using threads are that they make it easy to share data (via global variables) between cooperating threads and that some algorithms transpose more naturally to a multithreaded implementation than to a multiprocess implementation.
Furthermore, a multithreaded application can transparently take advantage of the possibilities for parallel processing on multiprocessor hardware.

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
If a child process becomes orphaned because its birth parent terminates, it becomes adopted by `init` (`init` is loaded from program file `/sbin/init`, and cannot be killed).

Each process has a current working directory, and it is from this directory that relative pathnames are interpreted for the process.
A process inherits its current working directory from its parent process.

A **daemon** process is usually long lived and runs in the background, like `syslogd` which records messages in the system log.

Each process can be limited in resources such as open files, memory and CPU time by `setrlimit()`. (`fork`ed program inherits copies of parents' resource limit settings.)

The resource limits of the shell can be adjusted using the `ulimit` command, which is also inherited by child processes created by this shell.

### Realtime OS

Response is guaranteed to be delivered within a certain deadline time after the triggering event.
This requires support from the underlying system, and such requirements can conflict with that of a multiuser time- sharing operating systems.
Realtime variants of Linux have also been created.

### The `/proc` file system

Linux (like Unix) provides the **/proc** file system is a virtual file system that provides an interface to kernel data structures in a form that looks like files and directories on a file system.

### Process creation & termination

A process can create a new process calling `fork`.
The forked child inherits copies of the parent's data, stack, and heap segments (note that the read-only text is shared between the two), which it may then modify independent of the parent's copies.

The child can go on and execute same code as parent, or use `execve` system call to load and execute a different program.
An `execve()` call destroys the existing text, data, stack, and heap segments, replacing them with new segments based on the code of the new program.

A program can request its own termination using `_exit()` system call, or do so by the delivery of a signal.
Either case the program yields a termination status (either the param to `_exit`, or the type of signal) which is available for the parent process to inspect using `wait()` system call.

### Date and time

Two types of time:
* **Real time** - wall clock since epoch.
* **CPU time** - total amount of CPU time that a process has used since starting. System time - CPU time spent in kernel mode (syscalls and kernel services performed on behalf of the process), user time - CPU time spent in user mode.

### Process group, session

Each program executed by the shell is started in a new process.
For example, the shell creates three processes to execute the following pipeline `ls -l | sort -k5n | less`

A **session** is a collection of process groups (jobs).
Sessions usually have an associated controlling terminal.
At any point in time, one process group in a session is the foreground process group (foreground job), which may read input from the terminal and send output to it.
A session can have any number of background jobs.

### Process memory layout

* Text segment, program code, sharable between many processes running the same program (i.e. mapped into the virtual memory address space of all such processes), read-only
_how does runtime code patching work then? Probably private filename mmap, then the text segment would need to be duplicated anyway?_
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
These pages form the **resident set**.

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
* Processes are isolated from one another and from the kernel, so that one process can’t read or modify the memory of another process or the kernel. (accomplished by having the page-table entries for each process point to distinct sets of physical pages in RAM (or in the swap area).
* Where appropriate, two or more processes can share memory.
  * When multiple processes executing the same program, text segment can be shared. (Or when multiple processes load the same shared library, that code can be shared)
  * Processes can use `shmget`/`mmap` to explicitly request sharing of memory with another process, for inter-process communication.
* Facilitate memory protection (page is readonly, writeable, executable, etc, for a particular process)
* Programmer, compiler and linker needn't be concerned with the physical layout of the program in RAM.
* Because not all memory used by the process need to live in RAM, the program loads and runs faster. (also each process uses less RAM increases the amount of processes that can live in RAM, and at any moment CPU is less likely to be idle.)

### Stack

The stack pointer tracks the current top of the stack.
Each time a function is called, an additional frame is allocated on the stack, and this frame is removed when the function returns.

The term **user stack** is used to distinguish the stack here from the **kernel stack**.
The kernel stack is a per-process memory region maintained in kernel memory that is used as the stack for execution of the functions called internally during the execution of a system call. (The kernel can't employ the user stack for this purpose since it resides in unprotected user memory.)

Each (user) stack frame contains the following information:
* Function arguments and local variables (automatic variables)
* Call linkage information: each function uses certain CPU registers. Each time one function calls another, a copy of these registers is saved in the called function's stack frame so that when the function returns, the appropriate register values can be restored for the calling function. 

### `argc`, `argv`, environ

`argv[0]` contains the name used to invoke the program. (`gzip`, `zcat`, `gunzip` points to the same binary, and uses this invoked name technique to identify what code to run).
They live in a memory area above the program's stack, and that area is limited in memory (usually by `ARG_MAX` from `limits.h`).

GNU C library exposes these global vars `program_invocation_name` and `program_invocation_short_name`.


Each process has an associated array of strings called the environment.
When a new process is created, it inherits a copy of its parent's environment.

A common use of environment variables is in the shell.
`export` (`unset`), `setenv` (`unsetenv`) permanently add a value to the shell’s environment and this environment is then inherited by all child processes that the shell creates. (`printenv`, `/proc/PID/environ` file to see).
Bourne shell and its descendants allow setting single command environment values by prepending `NAME=value` to the command run.

The environment variables are available to C as a global `char **environ` or `char* getenv(const char*)`. (to set, `setenv(name, value, override)` / `putenv(char*)`. note that the latter doesn't make a copy in environment but rather let a pointer in the environment linked list point to your given string! `clearenv` sets `environ` head to null.)

### setjmp, longjmp

`setjmp()`, `longjmp()`: non-local go-to. (target location is somewhere outside the current executing function.)
They should be avoided like `goto`.

(`goto` from C can't be used to jump between functions in C, because all functions reside at the same scope level, thus given two functions `x` and `y`, the compiler has no way of knowing whether a stack frame for `x` might be present when `y` is invoked (and thus whether a `goto` from `y` to `x` is possible). Having nested function can make this possible. Inner function can goto the outer scope.)

Calling `setjmp(jmp_buf env)` establishes a target for a later jump performed by `longjmp(jmp_buf env, int val)`.
From a programming point of view, after the `longjmp()`, it looks exactly as though we have just returned from the `setjmp()` call for a second time.
The way in which we distinguish the second `return` from the initial return is by the integer value returned by `setjmp()`: first call returns `0`, second call returns what `longjmp` is called with.
`setjmp` saves various information about the current process environment (including `PC` and `SP`) into its argument and `longjmp` needs to call with the same env.

It is permitted to call setjmp() only inside expressions simple enough not to require temporary storage. (assignment from `setjmp` is not allowed).
`setjmp(global)` --> `return` --> `longjmp(global, 1)` is wrong in that the `longjmp` would set the stack pointer to a frame that no longer exists.

`longjmp` may interact poorly with compiler optimization: local vars optimized onto a register may not have their state correctly restored.

# Memory allocation

### Heap allocation

The current limit of the heap is referred to as the **program break**.

To allocate memory, C programs normally use the `malloc` family of functions, which uses `brk` / `sbrk`.

`brk` / `sbrk` resizes the heap by telling the kernel to adjust where the process's program break is.

After the program break is increased, the program may access any address in the newly allocated area, but no physical memory pages are allocated yet.
The kernel automatically allocates new physical pages on the first attempt by the process to access addresses in those pages.

```cpp
#include <unistd.h>

// Sets program break to the specified location (rounded up to the next page boundary)
// Returns 0 on success, or –1 on error.
// Adjusting the program break below the initial boundary (end, as the end of user
// initialized data segment) is usually undefined behavior.
int brk(void *end_data_segment);

// Adjusts the program break by adding increment (of integer data type) to it.
// Returns previous program break (begin of newly allocated area) on success, or (void *) –1 on error.
// sbrk(0) returns the current setting of the program break without changing it.
void *sbrk(intptr_t increment);
```

Compared with `sbrk` / `brk`, `malloc` / `free`:
* are standardized as part of the C language
* are easier to use in threaded programs
* provide a simple interface that allows memory to be allocated in small units
* allow us to arbitrarily deallocate blocks of memory, which are maintained on a free list and recycled in future calls to allocate memory

```cpp
#include <stdlib.h>

// Returns pointer to allocated memory on success, or NULL and sets errno on error.
// Returned memory is aligned on a byte boundary suitable for any type of C data structure.
// 
// malloc scans the list of memory blocks previously released by free() in order to find
// one whose size is larger than or equal to its requirements (a best-fit or first-fit
// strategy e.g.) and split if the empty block is larger. If no block on the free list is
// large enough, then malloc() calls sbrk() to allocate more memory (usually some
// multiple of the virtual memory page size).
void *malloc(size_t size);

// deallocates the block of memory pointed to by its ptr argument, which should be an
// address previously returned by malloc()
// 
// In general, free() doesn't lower the program break, but instead adds the block of
// memory to a list of free blocks that are recycled by future calls to malloc(), because
// - free may not happen at the end of program's heap
// - minimizes the number of sbrk calls (system calls have a small but significant overhead
//
// free(NULL) does nothing and is not an error.
// Double-free and use-after-free are.
// glibc free() calls sbrk() to lower the program break only when the free block at the top
// end is "sufficiently" large.
// 
// How does free know the size to free?
// When malloc() allocates the block, it allocates extra bytes to hold an integer containing
// the size of the block and return pointer to memory after that.
void free(void *ptr);

// mtrace() and muntrace() allow a program to turn tracing of memory allocation calls on and
// off. (They write to MALLOC_TRACE env var, name of a file).
// mcheck(), mprobe(), MALLOC_CHECK_ env var require linking in a malloc debugging library.
// mallopt() and mallinfo() (non-standard) controls / reports params for malloc / free.

// Allocate memory for an array of identical items. same return convention as malloc.
// Unlike malloc(), calloc() initializes the allocated memory to 0.
void *calloc(size_t numitems, size_t size);

// Resize (usually enlarge) a block of memory previously allocated by a malloc-like function.
// Returns pointer to allocated memory on success (which may be different from its location
// before the call, in which case realloc copies over all existing data), or NULL on error
// and ptr is untouched. When increasing size, realloc does not initialize the additional
// bytes.
void *realloc(void *ptr, size_t size);

// memory allocated with calloc and realloc should also be free'ed.
// realloc-copy is often and expensive, in general this should be minimized.
// 
// to account for possibility of returned memory being different, we want to do
nptr = realloc(ptr, newsize);
if (nptr == NULL) {
  /* Handle error. note setting ptr in this case would mean we lose reference to the
     currently allocated object! */
} else {
  /* realloc() succeeded. Note that any pointers to locations inside the previous block
     (if realloc moves this) are now invalid. */
  ptr = nptr;
}
```

```cpp
#include <malloc.h>

// Allocates size bytes starting at an address aligned to a multiple of boundary.
// Returns pointer to allocated memory on success, or NULL on error.
void *memalign(size_t boundary, size_t size);
// an alternative posix_memalign (as specified in SUSv3) may be provided instead.
// should also be free'ed when done.
```

### Stack allocation

```cpp
#include <alloca.h>

// Obtains memory from the stack by bumping stack pointer. Need not be free'd.
// Returns pointer to allocated block of memory.
// We can’t use alloca() within a function argument list.
void *alloca(size_t size);
```

# IPC

### Taxonomy

Functional categories of IPC:
* commuication
  * data transfer (one process writes to kernel memory buffer, others read from it. Multiple readers are possible. Compared with shm reads are destructive. Synchronization is automatic, i.e. reader blocks on waiting for data)
    * byte stream (undelimited, arbitrary number of bytes): pipe, fifo, stream socket (internet domain, UNIX domain)
    * message (each read is one whole message): System V message queue, POSIX message queue, datagram socket
    * pseudoterminal
  * shared memory (same RAM page made available to two processes. No syscall or transfer to kernel memory required when communicating, hence very fast, but may need manual synchronization.): System V shm, POSIX shm, memory mapping (anonymous mapping, mapped file)
* synchronization
  * semaphore (kernel-maintained integer that processes can increment / decrement, never drops below 0 and blocks on decrease instead): System V. POSIX named / unnamed.
  * file lock (read / shared and write / exclusive locks): `fcntl`
  * mutex, conditional variable
* signal: standard, realtime

These mechanisms also differ in accessibility, persistence (process, kernel (before reboot) and file system level)

Here we refrain from making performance comparison on different IPC mechanisms, since performance may vary across Linux kernel versions, and performance will wary depending on the manner and environment they are used.
Benchmark individual applications if performance is a concern. Hide them behind an abstraction layer such that substitution is easy.

# System programming concepts

### System calls

A **system call** is a controlled entry point into the kernel, allowing a process to request that the kernel perform some action on the process’s behalf.
* changes the processor state from user mode to kernel mode, so that the CPU can access protected kernel memory.
* the set of system calls is fixed. Each system call is identified by a unique number.
* each system call may have a set of arguments that specify information to be transferred from user space (i.e., the process's virtual address space) to kernel space and vice versa.

From a program, invoking a syscall looks like calling a C function, but what happens underneath the hood is (e.g. on x86-32)
* invoking wrapper function in the C library
* wrapper function makes all of the syscall arguments available (passed to the wrapper C function via stack but kernel expects them to be in certain registers, so the wrapper copies them onto registers)
* all system calls enter kernel in the same way and the kernel needs some way to identify the syscall. To do this the wrapper copies syscall number into register `eax`
* wrapper function executes a `trap` / `sysenter` machine instruction, which causes the processor to switch to kernel mode and execute code pointed to by location 0x80 of system's trap vector.
* In response to trap to location 0x80, the kernel invokes its `system_call` routine to handle the trap. This
  * saves register values onto kernel stack
  * checks validity of syscall number
  * invokes the appropriate syscall routine (lookup via syscall number), which performs the task and may involve transferring data between kernel and user space memory (e.g. in IO operation) and returns a result status to `system_call` routine. 
  * restores register value from the kernel stack and places system call return value on the stack.
  * returns to wrapper, and returning processor to user mode.
* if the syscall return value is error, the wrapper function sets global var `errno` (return value of the syscall) and returns an int to the user to indicate success / fail. (syscall returns non-negative when success, and negative when error, whose value negated is used to set errno. The wrapper function returns -1 to indicate error.)

(Even for a simple system call like `getppid`, a considerable amount of work needs to happen! These can have significant overhead over a user-space function call due to mode switch, verifying args, transferring data between system and user memory, etc.)

`strace` calls can be used to trace syscalls made by a program.

### Library functions

A **library function** is a function in the standard C library, many of which don't involve syscalls underneath while others do.
(e.g. library function `fopen` uses syscall `open` underneath the hood, `printf` uses syscall `write` underneath the hood, and does output buffering, formatting, on top of the latter which just outputs a block of bytes. Similarly `malloc` and `free` over the underlying `brk`.)

### GNU C library (glibc)

There are different standard C library implementations, with the most common one on Linux being GNU C library.

Running `ldd` (list dynamic dependencies) can usually find the path of glibc shared library.

### Handling syscall and library function errors

Always check the return of syscalls / library functions. (except on a few syscalls that never fail: `getpid`, `_exit`, etc) (`exit()` vs `_exit()`, former flushes stdio buffer and calls exit handlers while latter does not)

Successful system calls and library functions never reset `errno` to 0, so it can have a value from the previous failed syscall.
(plus rarely a successful syscall can set `errno`, so always check retvalue before checking `errno`.)

(also rarely a system call can return `-1` on success, like `getpriority`, in this case user code can set `errno` before the call, and check it afterwards.)

`errno` to (locale-sensitive) string is available via `perror()` and `strerror()` library functions (the string returned from the latter can be statically allocated and overwritten by later calls to it).

(In C terminology, an lvalue is an expression referring to a region of storage.)

### Portability

Various implementation data types are represented using standard C types (like pids, uids, file offsets).

One could use C fundamental types like `long` and `int` but this reduces portability as size of these can vary across platform.
Instead, use `pid_t` (usually `typedef`'ed from `int`), `uid_t`, `off_t`, `size_t` (size of an object in bytes).

Note that library header files, order of members in library-defined structs, macros declared can vary across platforms as well.

# Universal file IO model

All system calls for performing I/O refer to open files using a **file descriptor**, a (usually small) nonnegative integer.
File descriptors are used to refer to all types of open files, including pipes, FIFOs, sockets, terminals, devices, and regular files.
Each process has its own set of file descriptors.

Three standard file descriptors (stdin, stdout, stderr) are inherited from copies of the shell's file descriptors.

Key system calls for performing IO is `open`, `read`, `write`, `close`.

### Universality

The above system calls perform IO on all types of files, including devices such as terminals (`/dev/tty`)

Universality of I/O is achieved by ensuring that each file system and device driver implements the same set of I/O system calls.

