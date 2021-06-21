# [What every programmer should know about memory](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf)

(Mainstream commodity hardware, as illustrated with Linux, for software developers)

Mass storage and memory subsystem improved slower compared with CPU.

# Common commodity hardware structures

* Northbridge with External Controllers + Southbridge

```
                            CPU CPU
                             |   |
RAM - Memory Controller - Northbridge - Memory Controller - RAM
                               |
       PCI-E, SATA, USB - Southbridge
```

(DDR2: Double Data Rate DRAM 2. Two separate buses to all memory chips, doubling bandwidth)

In this architecture, concurrent memory access patterns reduce delays by simultaneously accessing different memory banks.
The primary limitation is the internal bandwidth of the Northbridge.

* Integrated (into CPU) memory controller

```
                    RAM - CPU - CPU - RAM
                           |     |
                    RAM - CPU - CPU - RAM
                           |
PCI-E, SATA, USB - Southbridge
```
Advantage with this is there are as many memory banks available as processors, without needing a complicated Northbridge with enormous bandwidth.

Downside with this is computer still has to make memory available to all CPUs, so memory access is not uniform anymore. (Hence the name NUMA - non-uniform memory access).

**NUMA factor**: extra time needed to access remote memory.

There can be multiple types of CPU connections as well: in some architectures CPU are organized into nodes, where within a node accessing memory only has a small NUMA factor and connection between nodes can be quite expensive.

### RAM types

* Static RAM (SRAM)
A typical setup uses 6-transistor and represents 1 stable state as long as power is available (no refresh cycles needed). The cell state is available for reading almost immediately once the word access line WL is raised.

CPU caches usually use SRAM.

* Dynamic RAM (DRAM)
A typical setup uses 1-transistor 1-capacitor, and the capacitor to store state and transistor to guard access.
Using a capacitor means reading a cell discharges the capacitor which has to be recharged.

"Leakage": a fully charged capacitor holds a few 10s of thousands of electrons, even with high resistance of the capacitor, it only takes a short time for the capacitor to dissipate.
For most DRAM chips these days refresh has to happen every 64ms.
During refresh no memory reads can be made.

Another problem is the information read is not directly usable, the data line must be connected to a sense amplifier which can distinguish between a 0 and 1.

Every read drains the capacitor and must be followed by an operation to recharge the capacitor.

Charging and draining are not instantaneous: it takes a while before the results of read or write operations are available.

Size and cost are the advantages of DRAM.

Main memory usually uses DRAM.

### DRAM access

Memory cells need to be individually selected to be used.

N address line goes through a demultiplexer to achieve 2^N output lines to select memory cells.

Row address selection (RAS), column address selection (CAS) lines.

* Synchronous DRAM (SDRAM)
Memory controller provides a clock. The frequency of which determines the speed of the Front Side Bus (FSB), the memory controller interface used by SDRAM.

8B * effective bus frequency is the burst memory access speed.

* Double Data Rate DRAM (DDR).

##### DRAM read protocol

The read protocol allows memory controller to specify how much data is to be transimitted (2, 4, 8 words).
This allows filling entire lines in the cache without a new CAS/RAS sequence.

It is also possible for the memory controller to send a new CAS signal without resetting the row selection.
In this way, consecutive memory can be read from and written to faster because the RAS signal does not have to be sent and the row does not have to be deactivated.

Before a new RAS signal can be sent, however, the currently latched row must be deactivated and the new row must be precharged.
This lowers the actual data transfer rate: e.g. only 2 cycles out of 7 can be used for data transfer (depending how many words are accessed).

##### Refreshes

Memory controller schedules refreshes (every 64ms, e.g. if DRAM array has 8192 rows this means memory controller has to issue a refresh every 7.8125us).

How long each refresh takes depends on the DRAM module.

Accessing during refresh further slows down memory access.

### Other main memory users

Other than CPUs, high performance network cards and mass-storage controllers cannot afford to pipe all the data they need or provide through CPU.

Instead they read/write directly into main memory (DMA).

DMA means there is more contention in FSB bandwidth: CPU might stall more than usual when high DMA traffic.

Some systems has graphic systems without dedicated video RAM and they use parts of the main memory as video RAM.
Access to video RAM is frequent, so avoid such systems when performance is a priority.

### Conclusion

Accessing DRAM is not arbitrarily fast.

It is important to keep in mind the differences between CPU and memory frequencies (like 11 cycle of the former can be 1 cycle of the latter).

DRAM access is not always sequential. Non-continuous memory regions are used which means precharging and new RAS signals are needed.
This is when things stall and slow down.

Hardware and software prefetching can be used to create more overlap in the timing and reduce the stall.
(also shifts memory operation in time so that there's less contention at later times when the data is actually needed)

# CPU caches

CPU frequency used to be like memory bus frequency and memory access is slightly slower than registers.
In the 90s this changed: CPU frequency increased by a lot but memory bus not as much, for cost reasons.

A slower but bigger DRAM outperforms a smaller but faster SRAM in that swaps with disk is much slower.

We instead use both in a storage hierarchy.
One possible alternative design is giving the OS or user control of a separate SRAM region, but that makes administering the resources harder (e.g. virtual address to physical address mapping).
Instead SRAM is used to make temporary copies of data in memory and serve as a cache (more relevant as program code and data have spatial and temporal locality. Think loops, hot members, etc).

### Cache in the big picture

Early systems with CPU cache do things like this
```
Main memory
     |
-------------- Bus
  |
Cache - CPU core
```
where the core is no longer connected to memory directly: all loads go through the cache, and CPU-cache uses a fast/special connection.

Modern architecture splits data and code caches, because the access pattern are pretty different and the memory regions needed are pretty different.

Soon after the introduction of cache the speed discrepancy between cache and main memory got bigger, and another layer of cache was introduced. (only increasing the cache size is not feasible for economic reasons)

Each processor has multiple cores, each core with its own copy of (almost) all hardware resources (e.g. L1 caches. Not necessarily L2 caches. Some high end trading machines do have L2 caches per core. Check the manual!).

Each core has multiple threads, and they share the core's hardware resources (Intel's implementation of threads has only separate registers for the threads and even that is limited, some registers are shared).

Multiple processors can be grouped into a package, and in some architectures, all cores in the same package can share L3 cache.

### Cache operation

Caches are tagged using the address of the data word in memory, such that when fetching that data word from memory (or writing to it), we can search cache for the matching tag. (this address can be virtual or physical, depending on the implementation of the cache)

**When memory content is needed by the processor the entire cache line is loaded into the L1d, typically 64B in size these days.**

In practice an address is split into 3 parts, and a 32-bit address can look like
```
Tag - Cache Set - Offset
```
where Offset can be 6 bits (`log_2(cache_line_size)`), and there are `2^(tag + cache_set)` cache lines organized into sets.

It is not possible for a cache to hold partial cache lines.
A cache line which has been written to and which has not been written back to main memory is said to be "dirty".
Once it is written the dirty flag is cleared.

In an **exclusive** cache, an eviction from L1d pushes the cache line down into L2, which in turn push to L3 and then main memory.
In an **inclusive** cache (Intel), each cache line in L1d is also present in L2, therefore we waste a little space but makes eviction faster.

The CPUs are allowed to manage the caches as they like as long as the memory model defined for the processor architecture is not changed.
(E.g. take advantage of non-busy bus line and write dirty cache lines back to memory)

**Cache coherency**: in symmetric multi-processor systems, caches of all CPUs cannot work independently from each other: all processors are supposed to see the same memory content all the time. (cache line dirtied by other processors in their caches)

Cache coherency protocols generally achieve: a dirty cache line is not present in any other processor's cache, and clean copies of the same cache line can reside in arbitrarily many caches.
All the processors need to do is to monitor each others' write accesses and compare the addresses with those in their local caches.

When possible, we want to start the memory/cache read early such that by the time the CPU pipeline needs it, we have the data ready and the cost of reading memory can be hidden.
This is often possible for L1d and for processors with long pipelines possible for L2 as well.

### CPU cache implementation

**Fully associative** cache: any cache line can hold a copy of any memory location. (length of Cache-Set in the address is 0)
We do a linear walk of cache and compare Tag with requested address to decide if in cache or not.

These are practical for some small caches, e.g. TLB caches on some Intel processors (a few dozen entries).

**Direct mapped** cache does the completely opposite thing (length of Tag in the address is 0).
This unsurprisingly creates hot spots in cache lines.

A **set associative** cache combines the above two approaches.
E.g. an 8-way associative cache (4MB/64B line, 16 bits) where 13 bits of tags are used to address the set, and within that set 3 bits (8) tags has to be compared.

##### Write behavior

Need to achieve coherency transparent to user code.

Write policies:

* Write-through cache: if the cache line is written to, processor also writes the cache line to main memory. simple but not very fast (think a program writing a local var again and again, this does not need to generate traffic on the bus).

* Write-back cache: the cache line is only marked dirty and not immediately written back, when the cache line is dropped from the cache having dirty bit set would cause it to be written back to memory.
More prevalent since more efficient. Processors can also proactively write back (and unset dirty bit) when FSB is not busy.

* Write combining: for special regions not backed by RAM, when writing back is expensive such as in video RAM. Instead of transferring a line when just one word is written, we transfer the line when all words are written.

* Uncacheable: special addresses not backed by RAM, such as memory mapped cards attached to PCI-E, and cannot be brought into the cache.

##### Multi-processor coherency

Biggest problem with write-back is multiple processors. One processor dirtying cache means others cannot just load from RAM.

It is completely impractical to provide direct access from one processor to the cache of another processor. The connection is simply not fast enough.

What developed instead is the MESI 4-state cache coherence protocol.
* Modified: local processor modified this line, which is also the only copy in any cache.
* Exclusive: not modified, but also the only copy in any cache.
* Shared: not modified and also exists in other caches.
* Invalid: unused.

State transition looks something like (rr - remote read, lw - local write):
```
E -- rr --> S -- rr/lr --> S -- rw --> I
S -- lw --> M -- lw/lr --> M -- rw --> I
E -- rw --> I -- rw --> I -- lr --> E/S
I -- lw --> M
```

Certain operations a processor performs are announced on external pins and thus make the processor’s cache handling visible to the outside.

One processor hearing remote write invalidates all other cache's copy, and this is the rather expensive Request-For-Ownership (RFO).

Having the Exclusive state (as opposed to not differentiating from Shared) means local writes in the former case does not have to be annouced.
It is an optimization.

A MESI transition cannot happen until it is clear that all the processors in the system have had a chance to reply to the message.
(Collisions on the bus, latency in NUMA systems can all further slow things down.)

Concurrency is severely limited by the finite bandwidth available for the implementation of the necessary synchronization.
Programs need to be carefully designed to minimize accesses from different processors and cores to the same memory locations.

**Hyper threads** (symmetric multi-threading) share the same processor resources except for the register set.
The advantage is that the CPU can schedule another hyperthread and take advantage of the available resources such as arithmetic logic units when the currently running hyperthread is delayed e.g. by memory access.

One thing to worry about two hyperthreads share the L1d L1i caches, so if they are running completely different code, effective size of the cache is halved for both meaning more cache misses.
For two threads to actually bring down total run time, certain cache hit rate has to be guaranteed in comparison to the single threaded situation.

##### Virtual address or physical address for tagging

Virtual addresses can refer to different physical addresses over time, and the same address in different programs also likely refers to different physical addresses.

The downside with using physical addresses for tagging is virtual-to-physical address translation (through the MMU) takes time, meaning physical address is only available later on in the pipeline.

Processor designers are currently using virtual addresses for the first level cache (given its low latency).

For larger caches we can tag with physical address: they have higher latency and the virtual -> physical address translation can finish in time (and also takes longer to load in new content).

##### Replacement strategy

LRU is a good default.

It might be cache lines in all logical pages are mapped to the same cache sets, leaving much of the cache unused, it is the job of the OS or VM to ensure this deficiency in physical address assignment does not happen too much.

The best a programmer can do is to a) use logical memory pages completely and b) use page sizes as large as meaningful to diversify the physical addresses as much as possible.

### Instruction cache

Instruction cache is usually less problematic than data caches, more predictable flow / access pattern help with prefetching, and spatial / temporal locality.

Pipeline stalls happen, for instance, if the location of the next instruction cannot be correctly predicted or if it takes too long to load the next instruction (e.g. from memory).

In CISC architecture such as x86, x86-64 instruction decoding also takes time.
Processor L1i caches in recent years cache the decoded instruction and not the raw instruction bytes.

To achieve the best performance with instruction cache,
* generate code which is as small as possible (unless the overhead of doing so is too high).
* help the processor make good prefetching decisions (done through code layout or explicit prefetching).

Compiler code generation tries to keep both in mind.

##### Self modifying code

Self-modifying code should generally be avoided.
They are usually correctly executed except in edge cases and can create performance problems if not done correctly.

Code which is changed cannot be kept in decoded instructions cache, and if an upcoming instruction in the pipeline is changed then CPU has to throw away a lot of work.

Because in the vast majority of cases program code is not self-modifying, L1i cache does not use MESI for simplicity, and if changes are made a lot of pessimistic assumptions has to be made.

If they have to be used, SMC write operation should bypass cache.

On Linux, the programmer has to perform significant magic at link time to create an executable where the code pages are writable.

### Cache miss factors

A set of experiments to produce `(working set size - read/write bytes/cycle)` on various processors with various cache sizes and policies, hyperthreaded vs single-thread.

Critical workload: memory to cache transfers 8B each time, typically 8 transfers to fill the cache, and in burst mode DRAM chips can transfer 64B without further commands from the memory controller. (Each 8B block would still arrive a couple of cycles later than the previous)

The word inside the cache line which is required for the program to continue might not be the first word in the cache line, in which case memory controller is free to request blocks in different order by communicating which **critical word** the program is waiting on which can be served first.
This technique is called Critical Word First & Early Restart
