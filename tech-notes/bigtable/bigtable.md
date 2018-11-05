### Goal
- PBs of data across thousands of commodity servers (2006)
- Flexible, high performance serving different application use cases (throughput, latency, etc)

### Data model
- A bigtable is a sparse, distributed, persistent multi-dimensional sorted map of
(row: string, column: string, time: int64) --> bytes
Where the tuple of three serves as index, and content is uninterpreted array of bytes

##### Row
- Row key is a string of max 64KB, though typically 10 - 100B
- Every read or write under a single row is atomic, so that it’s easier to reason when there’s possibility of concurrent writes
- BigTable maintains data in lexicographical order by row key
- Row range (tablet) for a table is dynamically partitioned, a tablet is the unit of distribution and load balancing. Client can exploit this property by selecting their row keys so that they get good locality for their accesses.
E.g. in webtable, row key can be reversed domain name so that pages of the same site are grouped together. com.google.www and com.google.maps

##### Column families
- Column keys are grouped into sets called column families
- Column families form the basic unit of access control (application-type based control of read, write, etc)
- A table may have unbounded number of columns, though the number of column families should be fairly small (in the 100s), and families rarely change during operation
- A column key is named as "column:qualifier", such as "anchor:cnn.com" for a href, whose inner text can be the content of this column key

##### Timestamp
- Each cell in BigTable stores multiple versions of the same data, indexed by timestamp that can be assigned by BigTable or client (giving client freedom to use something that's not just a timestamp)
- BigTable can specify garbage-collect and only keep N previous versions / days for each cell

### API
- Read, write by row, regex search in column families, etc
- Single row transactions (atomic read-modify-write), no multi-row transaction support
- Client-supplied custom Sawzall script for data transformation
- Piped with MapReduce framework (as its input / output)

### Build blocks
- GFS for logs and data files
- Cluster management system that schedules jobs, manages resources, dealing with failures and monitoring status
- Google uses SSTable format to store BigTable data, persistent, ordered, **immutable** map from keys to values where both are arbitrary bytes. SSTable is a sequence of 64KB blocks. A block index at the end of table is usesd to locate blocks, and the index is loaded into memory when the SSTable is opened. When performing a read on an opened table, a binary search is done on the index to locate the appropriate block then we read it from disk. SSTable can also be mapped completely to memory
- Chubby (distributed lock service) for ensuring one active master, store bootstrap location, discover tablet servers, store schema information and access control lists. Chubby keeps 5 replicas, among them 1 master to actively serve requests. The service is up when majority of replicas are running and talking to each other. Chubby uses Paxos in case of failure. Chubby provides a namespace of directories and small files. Each directory or file can be used as a lock and read and writes to a file are atomic

### Implementation
- Client library linked into each client, one master server, many tablet servers
- Master assigns tablets to tablet servers, detect addition and expiration of tablet servers, balance tablet-server load, garbage collect, and manages schema, handles schema changes. Clients data does not flow through master servers: they do not rely on master for tablet server location. As a result master is lightly loaded in practice
- A tablet server manages a set of tablets. It handles reads and writes, and also splits tablets that have grown too big

_Confusing sections_

##### Tablet location
- B-tree location hierarchy. A Chubby file that points to a root tablet (does not split), root that points to metadata tablets, and metadata tablets that point to user tables
- Metadata table stores the location of a tablet under a row key that is an encoding of the tablet's table identifier and its end row
- Clients cache tablet location. Cache miss or stale requires extra network roundtrips to Chubby and metadata tablet (or client could do prefetch)

##### Tablet assignment
- Each tablet is assigned to one tablet server at a time
- Master keeps track of the set of live tablet servers, and current assignment of tablets to tablet servers, including unassigned tablets. Bigtable uses Chubby to keep track of tablet servers by monitoring a particular Chubby directory, each file in which corresponds with a tablet server. Master keeps track whether a server is still serving its tablets by asking it for the status of its lock, which if lost or server unreachable, master attempts to acquire the lock and deletes its files.
- 
