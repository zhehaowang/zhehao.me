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

In an exclusive cache, an eviction from L1d pushes the cache line down into L2, which in turn push to L3 and then main memory.
In an inclusive cache (Intel), each cache line in L1d is also present in L2, therefore we waste a little space but makes eviction faster.

