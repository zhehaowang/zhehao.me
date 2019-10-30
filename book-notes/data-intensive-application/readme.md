# Designing Data Intensive Applications

### Chap 1. Reliability, scalability and maintainability

Data intensive and compute intensive

##### Reliability

* Fault and failure
* Hardware error, usually uncorrelated and handled by redundancy
* Software error, usually happens when certain assumptions about the system don't hold any more. Correlated and cascading and harder to debug
* Human error

##### Scalability

* Defining load

**The Twitter example: to handle large fanout large read workload problem**
* Relational query approach
* Fanout queue approach

* Measuring performance
  * Percentiles
  * Load parameters
  * Head of line blocking
  * Tail latency amplification

* Conventional wisdom for scaling databases or stateful data systems is to scale up (grow tall, as opposed to scale out / grow wide) until you hit a limit (Think ComDB2)
* Elasticity

##### Maintainability

* Operability, simplicity, extendibility / agility of a system
* Complexity as native to the use case / required functionality, or accidental: debt and hacks accumulated over time, fitting a circle into a square, etc. Good abstractions, albeit hard to come by, help bring down accidental complexity.
* Non-functional requirements

### Chap 2. Data model and query languages

* Relational data model, SQL.
  * Data is organized by relation into tables of rows (unordered collection of tuples)
  * Comes from business data processing: business transactions and batch processing
  * Back in the days network model and hierarchical model (e.g. xml DB) were the main alternatives

* NoSQL describes a variety of non-relational approach to data modeling
  * Driven by these concerns
    * Scalability: very large datasets, very high write throughput
    * Open free software over commercial software
    * Specialized queries not well supported in SQL
    * Restrictiveness of a relational schema: need more dynamic and expressiveness

* In the near future we'll see a polyglot persistance: relational approach coexists with NoSQL

* Object-relational mismatch: matching SQL tables / rows / columns and OOP objects takes effort. Object-Relational Mapping tools are introduced to address this.

* If we are to model one-to-many with SQL, the options are
  * separate table with foreign key
  * latest SQL has structured data type support (e.g. xml / json). This comes with indexing / query support inside. This is almost like a blend between document-based and relational
  * encode as json / xml and store encoded. Won't be able to index / query in this case
  * The resume use case (self-contained document; one person to many jobs)
    * One json object a row makes sense: document-oriented DB like MongoDB comes in handy
    * This features a lack of schema, flexibility, and better locality

* **normalization** in DB refers deduplicating human-meaningful information using one level of indirection: e.g. the neighborhood a person lives in should not be represented with a raw string, but rather an enum with another map from enum to neighborhood name string.

* This brings the problem of modeling many-to-many, where
  * document-oriented storage can get awkward: we'd need app-level joins on several documents.
  * network model uses a tree-like structure with multiple parents, and software traverses the structure following along "access paths" like traversing a linked list. This makes it difficult to change data model and the software.
  * the relational model addresses this with separate tables and supporting arbitrary select and join conditions. Query optimization is abstracted away from app-level code.
  * In dealing with many-to-many relationship, relational and document databases aren't fundamentally different: a unique id to identify the external record, called a foreign key and a document reference respectively.

* Relational vs document databases today
  * Schema flexibility, better performance due to locality, closer to the data structure of the application for document databases.
  * Better support for join, many-to-one and many-to-many for relationship databases.
  * Relational can employ the technique of shredding (splitting a document-like structure into multiple tables linked by a foreign key) to achieve a document-like structure, which is cumbersome to schema and complicates application code.
  * Document cannot refer directly to a nested item within a document, instead you say the 2nd item in the list of positions for user 256 (like an access path).
  * To model many-to-many in document, you can do denormalization but the application code need to do additional work to keep denormalized data consistent.

* Schema-on-read (the reading application interprets / expects certain schema, like runtime type checking) and schema-on-write (the database enforcing some schema upon writing data, like static type checking in a compiled language). In general there is no right answer in terms of which is preferable. The difference is particularly noticeable when performing a schema change. Changing application code or migrating DB.

* Managing locality. When you need large parts of data in the same document. Or in relational model, Spanner's interleaved table rows within a parent table. Similarly, column-family in Bigtable / Cassandra / HBase data model. 

* When the relational model is introduced, SQL, a declarative way to query the data came with it, whereas others of its time queried databases primarily imperatively. The declarative approach follows relational algebra notions more closely, while the imperative way is more like telling the computer step by step what to do. Higher-level abstractions are often preferable as they are easier, more expressive, and the application query is decoupled from underlying database performance improvement.

* Higher level query languages like SQL can be implemented as a pipeline of MapReduce operations, and there is nothing that constraints a SQL query to run on a single machine.

* Graph-like data models (social graph, web graph, road networks, etc: where nodes are homogeneous; E.g. Facebook's social graph can have nodes as people, locations, events, etc)
  * Property graphs
    * Each node has a unique_id, incoming_vertices, outgoing_vertices, and a collection of key-value pairs
    * Each vertice has a unique_id, head_vertex, tail_vertex, label, and a collection of key-value pairs
    * Can store flexible information and good for extension
    * Described in a relational schema it looks like the following
```
CREATE TABLE vertices (
  vertex_id   integer PRIMARY KEY,
  properties  json
);
CREATE TABLE edges (
  edge_id     integer PRIMARY KEY,
  tail_vertex integer REFERENCES vertices (vertex_id),
  head_vertex integer REFERENCES vertices (vertex_id),
  label       text,
  properties  json
);
CREATE INDEX edge_tails ON edges (tail_vertex);
CREATE INDEX edge_heads ON edges (head_vertex);

; any can connect with any; with the indexes we can efficiently find head and
; tail edges of a given vertex thus traversing the graph

; we then insert entries like (in Cypher graph query language)

CREATE
  (USA:location    {name:'United States', type:'country'}),
  (Europe:location {name:'Europe',        type:'continent'}),
  (France:location {name:'France',        type:'country'}),
  (Lucy:person     {name:'Lucy'}),
  (France) -[:WITHIN]->  (Europe),
  (Lucy)   -[:BORN_IN]-> (USA),
  (Lucy)   -[:LIVE_IN]-> (France);

; and the query of all people emigrated to Europe from the US looks like
MATCH
  (person) -[:BORN_IN]-> () -[:WITHIN*0]-> (us:location {name:'United States'}),
  (person) -[:LIVE_IN]-> () -[:WITHIN*0]-> (eu:location {name:'Europe'})
RETURN
  person.name

; find any vertex (call it person) that meets both conditions:
;   has an outgoing BORN_IN edge to some vertex, from there you can follow any
;   number of WITHIN edges until reaching a node of type location whose name
;   property is "United States". Similar for the "Europe" analysis.
;
; the query optimizer then chooses the optimal way to execute this query (from
; all persons, or the two regions, e.g. depending on where you have index)
```
    * Although it's possible to put a graph database in a relational database, supporting queries in such can be difficult as the number of joins is not known beforehand. (SQL supports `WITH RECURSIVE`, but is very clumsy)
  * Triple-store graph model
    * all information stored in three-part statement `(subject, predicate, object)`, where an object can be a value or another node. In case of value this means a property, in case of another node this means an edge.
```
(Turtle/N3)
_:France a         :location;
         :name     :"France";
         :type     :country.
_:Lucy   a         :person;
         :name     :"Lucy";
         :live_in _:France.
...
; this can alternatively be expressed in XML
```
  * Semantic web (independent from triple-store) proposes a Resource Description Framework under which websites publish data in a consistent format thus allowing different websites to be automatically combined into a web of data; like a graph database of the entire web.
    * SPARQL query language operates on trip-stores using the RDF data model. Cypher's pattern matching above is borrowed from SPARQL. (difference being triple-store does not differentiate properties and edges thus both can be queried using the same syntax)

* Are graph databases the second coming of network model (CODASYL)?
  * No. CODASYL schema specifies what record types can be nested within which other record types. Graph databases allow any vertex to connect to any other. In CODASYL the only way to reach a node is via an access path, graph database allows query by / index on unique_id as well. CODASYL children of a record are an ordered set, graph databases have no such constraint on nodes and edges. In CODASYL all queries are imperative and easily broken by change in schema. Cypher or SPARQL queries are high-level.

* Datalog. Foundation for later query languages.
  * Similar data model as triple store, generalized to `predicate(subject, object)`. We define rules that depend on other rules or themselves.
```
Datalog / Prolog
within_recursive(Location, Name) :- name(Location, Name).
within_recursive(Location, Name) :- within(Location, via),
                                    within_recursive(via, Name).
migrated(Name, BornIn, LivingIn) :- name(person, Name),
                                    born_in(person, bornloc),
                                    within_recursive(bornloc, BornIn),
                                    live_in(person, liveloc),
                                    within_recursive(liveloc, LivingIn)
?- migrated(Who, 'United States', 'Europe')
```
  * The Datalog approach is powerful in that rules can be combined and reused in different queries.

##### Summary

Data models.
Hierarchical historically, relational came along to tackle many-to-many relation.

Recent applications whose data model does not fit either, NoSQL, mostly diverged in **document** and **graph** databases.
These don't usually enforce a schema on data they write, but has assumptions about data when reading.
Each model comes with their own sets of query languages, SQL, MapReduce, Cypher, Datalog, MongoDB's pipeline, etc.

Relational, document and graph are all widely used today, and one model can be emulated using another though the result is often awkward.

Some ongoing research about data models try to find suitable ones for sequence similarity searches (genome data); PBs amount of data (particle physics); and full-text search indexes. 

# Chap 3. Storage and retrieval

Database is all about storage and query.

Log-based storage engines (an append-only sequence of records) and page-oriented storage engines.

An index is an additional structure that is derived from the primary data. Adding and removing indexes does not affect the contents of the database, only performance of queries.
Any kind of index usually slows down writes because the index also needs to be updated on-write.
Thus databases typically let the application developer choose what to index on since they know the access pattern of the database best.

### Log-structured indexes

##### In-memory hash index

A simple approach could be a log-structured storage where each record is `<key, value>` and a hash index `<hash(key), byte_offset_into_log>` is stored in main memory.
Bitcask is one example does this, and is well suited for cases where key cardinality is low (fits in memory) but updated often (hence requiring fast writes).

* To avoid only growing disk usage, we can chunk the log into segments of a certain size and when closing off on writing one segment we perform **compaction** where we only keep the most recent update to each key. We can also merge several segments together when performing compaction. While compaction is being performed we can continue to serve read and write requests using the old segment files. (compaction never modifies an existing segment files: the result is written to a new segment files). After compaction is done we switch read and write requests to using the newly produced segment file, and delete the old segments.

* With a hash index each segment now needs its own hashmap index in memory, and when looking for a key we first search in the hashmap of the most recent segment, and if not found we go to the hashmap of the next most recent segment.

* We usually store the log in binary format, append a special tombstone to indicate deletion of a record. To recover from a crash one could read all the segments to repopulate the hashmap index, which is slow. Bitcask stores a snapshot of each segment's hashmap on disk, which can be loaded into memory for faster recovery. To handle partially written records (e.g. crash while writing) Bitcask uses a checksum. For concurrency control we can allow only one writer thread to append to the current segment. Multiple threads can read at the same time.

* Append-only log may seem wasteful, but as opposed to updating the record, append-only uses sequential write operations which are generally much faster than random writes, especially on magnetic spinning disk hard drives. Concurrency and crash recovery are also made much simpler if segment files are append-only or immutable. Merging old segments avoids the problem of data files getting fragmented over time.

* Hash-based index should fit in memory: you could maintain a hashmap on disk but on-disk hashmap query requires a lot of random access IO, is expensive to grow when it becomes full, and hash collisions require fiddly logic. Hash-based index is also efficient when doing range queries. 

##### SSTables and LSM tree

Previously our stored records (key, value) pairs appear in the sequence they are written, **SSTable** (Sorted String Table) require that the sequence of (key, value) pairs is sorted by key.

SSTables has several advantages over log segments with hash indexes:
* Merging segments is simple and efficient even for file bigger than the available memory: starting at the head of all segments, each time copy over the lowest ordered-key over to the new file. This produces a merged segment file also sorted by key. If the same key appears in multiple input segments, we only need to keep the value from the most recent segment.
* To find a particular key, we don't need to keep an index of all the keys in memory. Instead you can have an in-memory sparse index to tell you the offsets of some keys, and you can scan a range of two offsets to find if the key you are looking for is in.
* Blocks of entries between every two sparse indices can be compressed, which reduces IO bandwidth.

Constructing and maintaining SSTables:
* We maintain an in-memory stored structure (**memtable**), say, a red-black tree, and write requests gets added to this tree.
* When tree gets big enough we flush it into disk. While this tree is being flushed we can keep writing to a new memtable.
* When querying we first look inside the memtable, then the most recent on-disk segment, then the next recent, etc.
* From time to time run merge and compaction to combine segment files and to discard overwritten or deleted values.

To recover from memtable crashes, as we write to memtable we also write to a log. This log doesn't need to be sorted by key as all it serves is crash recovery and can be discarded when a memtable gets flushed to disk.

LevelDB and RocksDB use the algorithm described above.
Similar storage engines are used in Cassandra and HBase, both were inspired by Bigtable paper which introduced the terms SSTable and memtable.
Lucene (indexing engine for full-text search used by Elasticsearch and Solr) uses a similar method for storing its term dictionary.

This merging and compacting indexing structure originally builds upon Log Structured Merge trees which was built upon earlier work on log-structured file systems.

Optimization
* Query can be slow: first memtable then segment-by-segment lookups. We can add a bloom filter to definitively tell if a key does not exist.
* Size-tiered and level-tiered compaction: RocksDB and LevelDB use level-tier where key range is split into smaller SSTables and older data is moved into separate levels (_?_), HBase uses size-tier where newer and smaller SSTables get merged into older and larger ones. Cassandra supports both.

The basic idea of LSM-trees / keeping a cascade of SSTables that are merged in the background is simple and effective. It scales well when data is much bigger than memory, supports range query well, and because all disk writes are sequential the LSM-tree can support remarkably high write throughput.

### B-tree indexes

The most widely used indexing structure is B-tree. They remain the standard implementation in almost all relational databases, and many non-relational databases use them, too.

Similar as SSTables, B-tree also sorts by key, but that's where the similarity ends.
Log-structured indexes break the database down into variable-size segments and always write a segment sequentially, while B-trees break the database down to fixed-size blocks or pages, traditionally 4KB in size, and read / write one page at a time. This corresponds closely to the underlying hardware.

One page is designated as root of the tree, root points to child pages where each child is responsible for a continuous range of keys. A leaf page can either contain the (key, value) inline or contains references to pages where the values can be found.
Typically branching factor of a B-tree is several hundred.

To update an existing value for an existing key, find the leaf page, change the page and write the page back to disk.
Adding a new key may split an existing page into two. The split algorithm keeps the B-tree balanced.
Most databases can fit into a B-tree that is three or four levels deep. (4KB pages 4 levels branching factor of 500 can store up to 256TB)

The basic write operation of a B-tree is to overwrite a page on disk with new data, and it is assumed that the overwrite does not change the location of the page i.e. all references to the page remain intact when the page is overwritten.
This is in stark contrast with LSM-trees where files are append-only and deleted but never modified in-place.

A write causing a split will cause two children pages and parent page to be overwritten.
This is a dangerous operation as a crash after some pages have been written leaves you with a corrupted index.
In order to make B-tree resilient to crashes it includes a **write-ahead log**, an append-only file to which every B-tree modification must be written before it can be applied to the pages of the trees itself. This log is used to restore B-tree to a consistent state after crash.

Concurrency control is also more complicated: a reader may see a tree in an inconsistent state without concurrency control. B-trees typically use **latches** (lightweight locks).
Log-structured approaches are simpler in this regard, as all merging are done in the background without interfering with incoming queries; and atomically swap old segments for new segments from time to time.

B-tree optimizations have been introduced over the years, e.g. copy-on-write pages where a modified page is written to a different location, and a new version of the parent pages in the tree is created pointing at the new location. Abbreviating keys in interior pages. Optimized storage layout such that leaf pages appear in sequential order on disk. Adding additional pointers such as left and right siblings. Fractal trees borrow some log-structured ideas to reduce disk seeks.

### Comparison: B-tree and LSM-tree

As a rule of thumb, LSM-trees are typically faster in write, whereas B-trees are thought to be faster in reads (LSM-trees are thought as slow in this regard as potentially several SSTables at different stages of compression has to be checked).
However benchmarks are generally inconclusive and sensitive to the details of workload.

**Write amplification**: one write to the database would result in multiple writes to the disk over te course of the data's lifetime. E.g. compaction in LSM trees, B-trees overwriting an entire page even if only a few bytes changed. This is a concern for SSD as blocks can only be overwritten a limited number of times before wearing out.

In high-write-workload scenarios, bottleneck might be the rate at which the database can write to disk, in which case write amplification directly affects performance.

LSM-trees can typically sustain higher write workload than B-trees, because of lower write amplification (depends on workload and storage engine configuration), and also writing compact SSTable structures sequentially as opposed to overwriting several pages in the tree. This is particularly important for magnetic hard drives where random writes are far slower than sequential writes.

LSM-trees typically have lower storage overhead. B-tree leaves some disk space unused due to fragmentation: e.g. when splitting. LSM trees are not page based and periodically compact SSTables to remove fragmentation, giving them typically lower storage overhead.

Some SSDs internally use a log-structured algorithm to turn random writes into sequential writes hence nullifying the downsides of B-tree's random writes, but LSM's typically lower write amplification and reduced fragmentation still matters for throughput.

One downside of LSM tree is expensive compactions affecting the performance of read/writes as a user request may need to wait for disk to finish a compaction. The impact on throughput and response is usually small, but at higher percentiles the response time of LSM-trees can be less predictable than that of B-trees.

Also as compaction threads and the logging / flushing memtable to disk thread shares the write bandwith, the larger the database gets, the more write bandwidth compaction threads might use.
If compaction cannot keep up with new writes, tbe number of unmerged segments grow until you run out of disk space, reads would become slow as well. This is a situation you want to monitor.

B-trees also have a key at one specific location only while LSM trees can have the same key stored in multiple places. The former also made offering strong transactional semantics easier as what they can do is to lock the key range and attach those locks directly to the tree.

B-trees are very ingrained in the database architecture of today's and aren't going away any time soon. LSM-trees are getting popular but you should assess given your workload to decide which is more suitable.

### Other indexes

##### Seconary indexes

The above discusses (key, value) indexes like a primary key in the relational model (unique identifier).
Secondary indexes are very common (not unique) and both LSM-trees and B-trees can be used as secondary indexes.

When storing records for each key we can store the value itself or a reference to somewhere else (known as a **heap file**), which uses no particular order, can be append-only or keep track of deleted entries and overwrite them.
The heap file approach is common to deduplicate the actual data.

When not changing the key, overwriting with a smaller value is easy but overwriting with a larger value will cause the value to be relocated, thus needing to update all references as well, or leave a forwarding pointer in its old heap location.

Sometimes the extra hop from index to heap file is too much of a performance penalty for reads, so it can be desirable to store the indexed row directly within an index.
This is called a **clustered index**, which the primary key of a table in MySQL's InnoDB storage engine is always a clustered index, and secondary indexes refer to the primary key rather than a heap file location.
A compromise between a clustered index and a nonclustered (storing only references to the data within the index) is a **covering index** where some of a table's columns are stored with the index, allowing some queries to be answered by using the index alone.
Covering and clustered index can speed up reads but introduce extra storage and overhead to writes. Transactional guarantee also becomes harder because of duplication.

##### Multi-column indexes

**Multi-column index** can be a **concatenated index** where it's concatenating a few keys, and it would allow querying by a number of prefix keys of the concatenated key.

E.g., to support 2D geospatial data search, one option is to translate the 2 dimensions into a single number using a space-filling curve then use a B-tree, or more commonly specialized indexes such as R-trees are used.

Another case is when needing to support filtering by multiple columns at the same time.

##### Full-text searches and fuzzy indexes

Fuzzy search within a certain edit distance, ignore the grammatical variations of the searched keyword.

Lucene supports such and internally it uses an SSTable-like structure where the in-memory index is a finite state automaton over the characters in the keys, similar to a trie.
This automaton can then be transformed to a Levenshtein automaton which supports efficient search for words within a given edit distance.

##### Keeping everything in memory

Compared with memory disks are awkward to deal with: you have to lay out data carefully if you want good performance on reads and writes.

With memory getting cheaper keeping entire databases in memory becomes possible.
memcached in-memory caching provides a solution when data durability is not required.

When durability is required, an in-memory database can write a log to disk, writing peiodic snapshots to disk or replicating the in-memory state to other machines.
Despite the disk interaction it's still considered an in-memory database because disk is only used as an append-only log for durability and reads are served entirely from memory.

Writing to disk also has operational advantages where it's easier to backed up, inspected and analyzed by external utilities.
Redis provides weak durability by writing to disk asynchronously.

Counter-intuitively, the performance advantage of in-memory databases is not due to the fact they don't need to read from disks. Even a disk-based storage engine may never need to read from disk if you have enough memory as the OS caches recently used disk blocks anyway.
Rather they can be faster as they avoid the overheads of encoding in-memory data structures in a form that can be written to disk.

In-memory databases also provides data models that are difficult to implement with disk-based indexes, e.g. Redis offers a DB-like interface to various data structures such as priority queues and sets. Keeping all data in memory makes its implementation comparatively simple.

In-memory databases can store data larger than memory without bringing back the overheads of using a disk, the **anti-caching** approach works by evicting the least recently used data to disk when there is not enough memory and loading it back in when accessed.
This is similar to what OS does with swaps and virtual memory, but with more granularity e.g. individual records as opposed to memory pages, hence more efficient than what the OS does.
This approach still requires the entire index to fit in memory

### Transaction processing or analytics

The word **transaction** traces back to databases' early days for recording money changing hands, now it refers to a group of reads and writes that form a logical unit.
A transaction needn't necessarily have **ACID** (atomicity, consistency, isolation and durability) properties, transaction processing just means allowing clients to make low-latency reads and writes as opposed to batch processing jobs which run only periodically.

Over time we see two major query patterns for databases,
* look up a small number of records by some key, using an index. Records are then inserted or updated based on the user's input. These application are usually interactive and became known as online transaction processing (OLTP).
* scan over a huge number of records, reading only a few columns per record and calculates aggregate statistics (sum, avg, etc) rather than returning the raw data to user. These are known as online analytics processing (OLAP).

Relational DBs started out working fine for both OLTP and OLAP, over time some companies switched over to **data warehouse** for their OLAP workload.
In some setups, the OLTP systems being latency sensitive and mission critical, does not serve analytics requests, a read-only copy of data extracted from the OLTP system transformed into an analysis friendly schema and cleaned up is loaded into the data warehouse. This process of loading OLTP data into OLAP warehouse is known as Extract-Transform-Load (ETL).

Having a separate OLAP system allows optimization specific to its query pattern. 

The data model of a data warehouse is most commonly relational as SQL is generally a good fit for analytics queries. MS SQL Server supports transaction processing and data warehousing in the same product however they are becoming increasingly two separate storage and query engines served through the same SQL interface.

Open source SQL-on-Hadoop projects fall into the same data warehouse category: Apache Hive, Spark SQL, Cloudera Impala, Facebook Presto, Apache Tajo and Apache Drill. Some of them are based on Google Dremel.

Data warehouses don't see a variety of data models as transaction processing DBs do.
Most follow a star-schema where a giant central table (facts table) records events and foreign-key references to other tables (dimension tables) to e.g. normalize (dimension tables would record the who, what, where, how and why of an event).
This is like a star where facts table sits in the center and connects to peripheral dimension tables.
Snowflake schema is a variation of the star where dimensions are further broken down into subdimensions. They are more normalized but harder to work with.

##### Column-oriented storage

If you have trillion of rows in your facts table, storing and querying them efficiently becomes a problem.
Fact tables are always over 100 columns wide but one query rarely accesses more than 4 or 5 of them at the same time.

OLTP databases including document-based ones usually organize one row / document as a contiguous sequence of bytes, column-oriented storage instead stores all the values from each column together.
E.g. each column of a facts table gets stored in its own file, such that when only accessing a few columns in a query we only read those files as opposed to reading all the columns then filter.
Column-oriented storage layout requires each column file containing rows in the same order.

Column-oriented storage often offers great opportunities for compression.
**bitmap encoding** is often used. The cardinality of the set of distinct values in a column is often small compared with the number of rows.
We then use a bitmap to represent the set of distinct values in a column (e.g. 200 countries in the world, 25B to store), and each distinct column value X would then correspond with a series of bits where we have 1 bit for each column and we'd have 1 if that column is X and 0 if not.
For each column value X we'd then end up with a series of bits that are predominantly 0, and we can then apply run-length encoding (100 zeroes followed by 2 ones, ...) to further compress.
This storage schema also makes filtering by a few column values easier: e.g. we apply a bitwise-or over all the series of bits of those column values and return the selected columns.

Cassandra and HBase offer column-families which they inherited from BigTable. Those would still be row-based storage as within each column family they store all columns from a row together, along with the row key and they don't use column compression. Hence the BigTable model is still mostly row-oriented.

Column-oriented layouts also are good for making efficient uses of CPU cycles: CPU loads one L1-cache-ful of compressed column data, does some bitwise and/or without function calls (iterate through the data in a tight loop) on the compressed data directly. (single-instruction-multi-data/SIMD instructions on modern CPUs)
This leverages **vectorized processing**.

In a column store it might be easiest to store data in the order they come in, as then insertion becomes simple append; it's also possible to impose an order as in an SSTable, note that fields of the same record needs to remain in the same k-th record in every column data file. Sorting would also help with compression especially for the first sort key.
Vertica sorts a column-oriented storage in several ways: the data needs to have multiple copies anyway so why not sort them in different orders to answer different kinds of queries.

Writing to column-oriented storage can be tricky, as insertion in the middle requires rewriting all column data files.
LSM-trees don't have this constraint, column or row based, when enough writes have accumulated they are merged with the column files on disk and written to new files.

##### Data cubes and materialized views

Not every data warehouse is columnar, if queries often involve count, sum, avg, etc, we could cache some of the counts or sums that queries use most often.

A view is often defined as the resulting table of some query.
A **materialized view** is an actual copy of the query result written to disk (a denormalized copy of the data matching some conditions).
When the underlying data changes the materialized view needs to be updated as well.
They make writes more expensive which is why materialized view is not often seen in OLTP databases.

A materialized data cube is a (denormalized and high-dimensional) materialized view with some aggregation statistics such that particular queries on those statistics are faster.

### Summary

How databases handle storage and retrieval internally.
* OLTP / transaction processing workload: request volume is large, each touches few records, usually via some key / index, expect low-latency in response. Disk seek time is often the bottleneck here.
  * Storage
    * LSM-trees: append-only, immutable data files, SSTables, merge; systematically turn random-access writes to sequenetial writes, enabling higher write throughput
    * B-trees: overwrite fixed sized pages in-place
  * Indexing
    * multi-index
    * in-memory database
* OLAP / analytics workload / data warehouse: lower volume, queries needing to read a large number of records. Disk bandwidth is bottleneck. Column-oriented storage is increasingly popular for this.
  * Indexing are less relevant, instead compression becomes important

# Chap 4. Encoding and Evolution

Application change. Evolvability is important.
Server software usually goes through staged rollout, client software upgrade are at the mercy of clients.
Coexistence of old and new code makes backward and forward compatibility important. (new being able to work with old; old being able to work with new.)
Forward compatibility is usually trickier as it requires old code to ignore additions new code did.

Programs work with in-memory data structures as well as serialized / encoded data, when needing to transfer over the network (maybe, with the exception of memory mapped files).

* Language-specific serialization formats: `java.io.Serializable`, `pickle`, etc. These are usually easy to code but
  * they may be specific to that language
  * in order to restore data in the same object types the decoder needs to able to instantiate arbitrary classes, which is a source of security problems
  * versioning is often an after thought for these utilities
  * efficiency is often an after thought. These encodings can be very bloated / CPU-inefficient to use
For these reasons it's generally a bad idea to use the language's built-in serialization library for anything other than very transient purposes.

### JSON, XML and binary variants

JSON, XML, CSV are all textual formats, somewhat human-readable, and widely supported encodings.
Some subtle problems besides superficial syntactic issues:
* ambiguity around encoding of numbers. XML and CSV cannot differentiate a string of numbers and a number without referencing external schema. JSON can, but does not distinguish floats and integers and doesn't specify a floating point precision. An integer larger than 2\^53 cannot be exactly represented by IEEE 754 double precision float. Consequently JSON returned by Twitter's API includes tweet IDs twice, once as JSON number and once as decimal string, to work around the fact that number this large may not be correctly parsed by JS applications.
* JSON and XML have good support for unicode character strings, but not binary strings. People get around this limitation by using base64 to encode the binary data, and this increases size.
* JSON and XML both have optional schema support. XML schema is widely used, JSON not as much. In cases where a schema is not used, the decoding application potentially needs to hardcode the appropriate encoding / decoding logic.
* CSV does not have schema, each column is up for interpretation. It's also vague e.g. what happens if a value contains a comma. Escaping has been formally specified but all parsers support them correctly.

Despite these flaws these encodings will likely remain popular as data interchange formats.

Binary encoding has been developed for JSON and XML, since they don't prescribe a schema, they still need to include all the field names within encoded data.

### Thrift and protobuf

Apache Thrift and protobuf are binary encoding libraries that are based on the same principle.
Both require a schema in an interface definition language (IDL), and come with a code generation tool that takes the schema and produces code in various languages that implement it.
One big difference with binary encoding of JSON/XML is field names are not present in encoded data, instead field tags (numbers) are used, like normalization with a schema definition. 

Protobuf binary encoding uses variable length integers and encodes very similarly to Thrift CompactProtocol (Thrift also has a BinaryProtocol which does not use variable length integer).
Encoding is TLV.

##### Handling schema evolution

Add new fields to the schema provided that you give each field a new tag number.
Forward compatibility: old code not recognizing the tag number can just ignore it.
Backward compatibility: new code can still read old messages since tag number doesn't change, the only detail is that when adding a new field you cannot mark it required: they must be optional or have a default value.

Removing is like adding with backward and forward compatibility concerns reversed: you cannot remove a field that is required, and you can never use the same tag again.

Changing data types of fields may be possible, data can lose precision or become truncated.

One peculiarity in Protobuf: there is no explicit array but instead a repeated marker (a third option along with required and optional) meaning something can appear 0 to N times (exactly 1 time, or 0 or 1 times).

### Avro

Avro is another encoding format different from Protobuf and Thrift, it is developed since Thrift was deemed not a good fit for Hadoop's use cases.

Avro has an IDL to describe schema.
The peculiarity is in Avro having no field tags or type indication in the encoded data: a string or an integer is a length prefix + data (UTF-8 or variable length integer encoding.)
Being able to decode relies on going through the fields in the order that they appear in the schema, meaning the decoder can work only if using the exact same schema as encoder.

In order to support schema evolution, the writer's schema and reader's schema don't have to be the same.
When decoding, Avro resolves difference by looking at writer's schema and reader's schema side-by-side and translating the writer's schema into reader's schema.

Field reordering can be reconciled, fields in writer's schema but not reader's will be ignored by decoder, and field in reader's but not writer's will be filled with default values in reader's schema.

To maintain and backwards and forwards compatibility, you can then only add or remove fields with a default value.

Avro doesn't have optional / required marker as protobuf and thrift do, it has default values and union types instead, where allowing a field to be null requires including null in a union type.

In the context of Hadoop, Avro is often used for storing a large file of millions of records all encoded with the same schema. Hence the overhead of including that schema with the file is not huge.
In a database where records are written at different times with different schema, Avro keeps a versioned schema in a schema database.
When sending data over the network, Avro RPC protocol negotiates a shared schema between two parties.

Why might this be preferable to Protobuf and Thrift's schema? Not having tag numbers makes Avro friendlier to dynamically generated schemas. E.g. in a case where you want to encode a table in a relational database, exporting it to Avro / Protobuf / Thrift and then the table schema changes, when exporting the newer version Protobuf Thrift versions have to be careful about field tag, while Avro has no such concern.

Code generation is often useful for statically typed languages where the generated code allows type checking, while in dynamically typed language code generation is often frowned upon.

Protobuf, Thrift and Avro schemas are simpler than XML/JSON schemas as the latter support more detailed validation rule like regexp, integer ranger, etc.

### Pro of schemas

These encodings are based on the idea introduced in ASN.1, used to define various network protocols and its binary encoding (DER) is still used to encode SSL certificates (X.509).

Most relational database vendors also have their own proprietary binary encoding for their query protocol over the network, the database vendors usually then provides a driver (using the ODBC or JDBC APIs) that decodes data over the network.

Binary encodings based on a schemas are viable compared to textual formats like JSON/XML, in particular, binary encoding is more compact (omitting field names), schema is a good form of documentation, keeping a database of schemas allows checking backward and forward compatible changes, and in statically typed languages generating code from schema allows compile time type checking.

### Modes of dataflow

Usually data flows via databases, service calls, asynchronous message passing.

##### Via databases

Be mindful of forward and backward compatibility, forward compatibility, and when writer of an old version reads data written by a new version, writes on top of that, and being able to save that data back.

Data outlives code: upgrading server software is fast, but data written long ago will still be there.
Most databases avoid migration if possible due to its being expensive.

Archiving (snapshotting) data usually uses the latest schema, and Avro object container files are a good fit (or in column-oriented analytical format).

##### Via services

Server defines an API, clients request data from it. The API is called a service.
The approach is usually to decompose a larger application into smaller services by area of functionality, called a service-oriented architecture / microservices architecture.
Key design goal being to make the application easy to change and maintain by making services independently deployable and evolvable.

A **web service** is where HTTP is used as the underlying protocol of talking to the service.

REST and SOAP are two popular approaches to web services. They are diametrically opposed in terms of philosophy.
* REST is not a protocol but a design philosophy that builds upon the principles of HTTP, where it uses simple data formats, URLs for identifying resources, HTTP features for cache control, authentication and content type negotiation.
An API designed with such in mind is called RESTful.
* SOAP is an XML-based protocol for making network API requests. Although most commonly used over HTTP, it aims to be independent from HTTP and avoids using most HTTP features. API of a SOAP web service is described using Web Services Description Language, WSDL, an XML-based language. WSDL is not designed to be human-readable, users of SOAP rely heavily on tool support and code generation.

Web services are the latest incarnation of a long line of technologies for making API requests over a network, based off the ideas of **RPC**, whuch tries to make a request to a remote service look the same as calling a function or method in your programming language.
This may be fundamentally flawed in the sense that
* a local function call is predictable and either succeeds or fails depending only on the parameters that are under your control. A network request is unpredictable.
* a local function call either returns a result, or throws an exception, or never returns; a network request can have more outcome like returning without a result due to a timeout.
* retrying a call that didn't respond in time may actually cause the action to happen twice in the remote, in which case idempotence needs to be built in. Local calls don't have this problem.
* local function call times are more predictable while network delays can vary wildly.
* a local function call can take in references efficiently, network call requires all actual data to be sent over the network.
* the client and service may be implemented in different languages so RPC framework has to translate datatypes from one language into another, not a problem for local function calls.

REST does not try to hide the fact that it goes through network. RPC libraries can be built using REST.

With these flaws, RPC isn't going away in the short term.
gRPC is an RPC implementation using protobuf, Thrift and Avro come with RPC support.

The new generation of RPC framework is more explicit about a remote request being different from a local call: e.g. they may use futures / promises to encapsulate asynchronous actions that may fail.
gRPC also supports streams with a call consisting of more than one request and response.

Dataflow through services can be easier to involve than dataflow via database: you can assume servers are always upgraded first and clients after them. Hence responses need to be forward compatible and requests need to be backward compatible.

Forward and backward compatibility properties of an RPC scheme are usually inherited from the encoding format they use: Thrift, gRPC, Avro RPC; SOAP's XML schemas; RESTful API usually uses JSON without a formally specified schema where adding optional request param and adding to response are usually changes that maintain compatibility.

##### Message-passing dataflow

Asynchronous message-passing systems are somewhere between RPC and databases.
A client's request (message) is delivered to another process by going through an intermediary called a message broker (message queue, or message-oriented middleware).

Compared with RPC, a message broker
* can act as a buffer if the recipient is unavailable or overloaded, thus improving reliability
* can automatically redeliver messages to a process that has crashed preventing messages being lost
* avoids the sender needing to know the IP address and port number of the recipient
* allows one message to be sent to multiple recipients
* logically decouples the sender from the recipient

However, a difference compared with RPC is that message-passing is usually one-way: a sender does not expect to receive a reply.
A reply is possible via a different channel, and this communication pattern is asynchronous in that the sender doesn't wait for the message to be delivered but simply sends it and then forgets it.

Recently, RabbitMQ, Apache Kafka, etc, have become popular message brokers.
In general, message brokers are used as follows: one process sends a message to a named queue or topic, and the broker ensures the message is delivered to one or more consumers of or subscribers to the topic.
There can be many producers and many consumers on the same topic.

Message brokers typically don't enforce any particular data model and you can use encoding format.
If the encoding is backward and forward compatible, you have the flexibility to change producers and consumers independently and deploy them in any order.

The **actor model** is a programming model for concurrency in a single process.
Rather than dealing directly with threads (race conditions, locking, etc), logic is encapsulated in actors.
Each actor has some local states and communicates with other actors by sending and receiving asynchronous messages.
Message delivery is not guaranteed.

Distributed actor model uses this programming model on different nodes as there is less of a fundamental mismatch between local and remote communication when using the actor model.
This model essentially integrates a message broker and the actor programming model.

Some distributed actor model frameworks are Akka (Java's built-in serialization), Orleans, Erlang OTP.

### Summary

Encoding, their efficiency and evolvability impact.
* Programming specific-specific encodings
* Textual formats like JSON/XML/CSV. Optional schema. Can be vague about types.
* Binary schema-driven formats like Thrift, Protobuf, Avro

Modes of dataflow.
* Via databases. Writer process encodes, reader process decodes.
* Via RPC and RESTful APIs / SOAP. Client encodes request, decodes response, and the opposite for server.
* Via asynchronous message passing (using message brokers or actors).

Backward/forward compatibility and rolling upgrade are achievable with a bit care.

Deployment should be incremental and frequent.

# Part II. Distributed data

Part I deals with a single node, Part II deals with multiple where we may gain in scalability, fault tolerance / high availability and latency.

Problem with scaling vertically (communication via shared memory) using a more powerful machine is cost grows non-linearly.
Fault tolerance and latency are also issues with a single powerful node.
Another approach is a shared disk architecture but contention and overhead of locking limit its scalability.

Share nothing architecture / horizontally scaling are not necessarily the best solution for everything, while having advantages in cost of scaling, high availability and low latency (distributed to near where clients are), they usually add complexity for applications and sometimes limits the expressiveness of data model.

Two common ways of distributing data across nodes
* Replication. Same copy of data in different locations. Provides redundancy and helps improve performance.
* Partitioning / sharding. Split a big dataset into smaller ones to be stored separately.

# Chap 5. Replication

Reasons for replication:
* keep your data geographically close to your users,
* allow the system to continue working even part of it have failed,
* to scale out the number of machines that can serve read queries (thus increase read throughput)

This chapter assumes your dataset is small enough to live in one machine.
The complexity in replication lies in handling changes to replicated data.

Algorithms for replicating changes across nodes: single-leader, multi-leader and leaderless.

### Leaders and followers

To ensure change gets to all replicas, the most common solution is leader-based replication (active / passive, master / slave replication).
* One of the replicas is the leader (master / primary), all write requests must go through the leader, which first writes the new data to its local storage.
* Whenever leader writes to its local storage, it sends the change to all of its followers (read replicas, slaves, secondaries, hot standbys) as a part of replication log or change stream. Each follower updates accordingly by applying the changes in the same order as they were processed by the leader.
* a client read can be handled by the leader or any of the followers.

This mode of replication is built-in for many relational DBs, MySQL, PostgreSQL, etc, and some non-relational DBs, MongoDB, Espresso, etc.
This is not limited to distributed DBs, Kafka and RabbitMQ's high availability queues also use it.

### Synchronous and asynchronous replication

* Synchronous: the leader wait for a follower to confirm it received the write before reporting success to the user, and before making the write visible to other clients.
  * Advantage: the follower is guaranteed to have an up-to-date copy of the data that is consistent with leader. If leader suddenly fails data is still available on the follower.
  * Disadvantage: the write cannot be processed if the follower does not respond and the leader has to block all writes.
  * It's impractical for all followers to be synchronous.
* Asynchronous: leader does not wait for follower response before telling the user.

Usually if you enable synchronous replication on database it's **semi-synchronous** where one of the followers receive synchronous updates and all others async.
If the synchronous followers becomes slow, one of the async followers is made sync.
This guarantees up-to-date data on at least two nodes.

In practice leader-based replication is often configured to be full async where a write, even if confirmed by the leader, are not guaranteed to be durable.

### Setting up new followers

To spin up a new follower, full copy usually doesn't work as data is being written as we copy, while locking the DB means lowering availability.
Usually we take a snapshot of the leader's DB at some point, copy this snapshot over, and the new follower then requests all the changes that have happened since the snapshot's taken, until it fully catches up.
This requires the snapshot being assoicated with an exact location in leader's replication log, known as log sequence number, binlog coordinates.

### Handling node outages

* Follower-failure (catchup): each follower keeps a log of data changes it has received from leader. When a follower crashes, it picks up the last processed transaction from this log and request all the data changes since that.
* Leader-failure (**failover**): one of the followers needs to be promoted leader, and clients need to be reconfigured to send their writes to the new leader, and other followers need to start consuming data changes from the new leader.
  * Determining leader failure usually relies on heartbeat
  * Electing a new leader is a consensus problem. Usually the replica with most up-to-date data changes from the leader is chosen to minimize data loss
  * Reconfiguring the system to follow the new leader, and if old leader comes back, ensure it becomes a follower to the new leader.

Failover is fraught with things that can go wrong:
* With asynchronous replication, there can be writes from the leader before it failed that are not in the new leader. We could discard those unreplicated writes but this would violate client's durability assumption
* Discarding writes is especially dangerous if other storage systems outside of the DB need to be coordinated with database content.
* In some fault scenarios it could happen that two nodes believe they are the leader. As a safety catch some systems shut one down in this case, these need to be carefully designed as well so as to not shut both down.
* deciding the right heartbeat timeout for a leader to be considered dead.

### Implementation of replication logs

* Statement-based replication. In a relational system each INSERT, UPDATE or DELETE is forwarded to followers. This has problems with
  * calling nondeterministic function like NOW and RAND
  * if statements depend on existing data in the column, they must be executed in exactly the same order on each replica
  * statements with side effects (triggers, stored procedures, user-defined functions) may result in different side effects on different replica unless side effects are deterministic
  * there are workarounds but other replication methods are usually preferred.

* Write-ahead log shipping (in LSM trees this log is the main place for storage, in B-trees this is the write-ahead log for restoring to a consistent state after a crash)
  * we can use the same log for replication: leader appends to its own log and also sends it across the network to followers.
  * main disadvantage is log describes data on a very low level: like which bytes were changed in which disk blocks, this makes replication closely coupled to the storage engine. It's typically not possible to run different versions of the database software on the leader and followers.
  * this advantage has a big operational impact: upgrade requires downtime. If this allows followers to run a newer version than the leader, then zero downtime upgrade can be achieved by upgrading some followers then perform a failover.

* Logical (row-based) replication
  * uses a different log format for replication than the log of the storage engine. The former's a logical log while the latter's a physical log.
  * logical log for a relational DB is usually a sequence of records describing writes to tables at the granularity of a row: insert contains new values of all columns, delete cotntains the primary key or if no primary key old values of all columns, update contains the primary key (or enough info to uniquely identify a row) and the new values of all columns.
  * a transaction that modifies several rows generates several such records followed by a record to indicate a commit. MySQL binlog uses such.
  * this higher-level log allows leader and follower to run different storage engines.
  * this log is easier for external applications to parse, e.g. transcribing data to a warehouse or for building caches, custom indexes. This is called **change data capture**.

* Trigger-based replication
  * the above are replications implemented by the database system. Sometimes you want more flexibility as an application, to e.g. replicate a subset of the data. Then replication then may be moved up to application level, or use triggers and stored procedures available in many relational DBs, which lets you register custom application code executed automatically when data changes.
  * This usually has more overhead, is more error-prone but more flexible.

### Problems with replication lag

Replication helps with availability, read throughput and latency.

In a system with few writes and lots of reads one leader and many followers may seem ideal, though this system can only replicate asynchronously, meaning a client reading from an asynchronous follower may see out-of-date data: reading at the same time from leader and a follower may give different results, transiently.
Without writes eventually they should converge, and this is known as **eventual consistency** where eventual is vague and can be arbitrarily long.

##### Read your own writes

Read-your-write-consistency / read-after-write-consistency is a guarantee that user will always see any updates they submitted themselves.
To implement read-after-write-consistency, we could:
* when reading something that the user might have modified, read it from leader, otherwise read from follower. This requires your system to be able to identify what the user can modify, e.g. profile of that user on a social media page, or
* track the time of last update, and for a certain time (based on replication lag) after the last update make all reads from the leader, or
* client can remember the timestamp of its most recent write, then the system can ensure that the replica serving any reads for that user reflects updates at least until that timestamp, and switch to a different replica or wait if that replica does not have this timestamp. This can be a logical timestamp that indicates the ordering of writes, or system clock where clock synchronization becomes critical

Another complication arises when the same user is accessing your service from multiple devices.
* Timestamp remembering becomes more difficult: this metadata needs to be centralized and known across devices
* If your replicas are distributed across different data centers the user's multiple devices may connect to different data centers. If your approach requires reading from the leader you may first need to route requests from all of a user's devices to the same data center.

##### Monotonic reads

Another anomaly with asynchronous replication is user can see things moving back in time.
Monotonic reads is a guarantee this does not happen, a guarantee stronger than eventual and weaker than strong consistency.

One way to achieve this is make the user read always from the same replica, e.g. using a hash of the UserID, rather than randomly.
However this needs to handle rerouting when that replica fails.

##### Consistent prefix reads

A third anomaly with asynchronous replication is violation of causality.
Consistent prefix reads is a guarantee this does not happen, which says if a sequence of writes happens in a certain order, then anyone reading those writes will see them appear in the same order.

This is a particular problem in sharded databases as if the database always applies writes in the same order, reads always read a consistent prefix and this cannot happen.
But in many distributed databases different partitions operate independently so there is no global ordering of writes.
One solution is to make sure any writes causally related to each other are written to the same partition, but in some applications that cannot be done efficiently.


When working with an eventually consistent system it's important to consider what if replication lag gets long, and what kind of consistency guarantee you need to provide to users.
Application code can manually perform some operations on the leader to provide a stronger guarantee than the underlying DB, but this is error prone and complex, and it'd be ideal if application doesn't need to worry about consistency and can trust their DB to do the right thing.
This is why transactions exist: they provide stronger guarantees from the database such that applications can be simpler.

When moving away from a single node to multi node many dropped transactions, claiming they are too expensive or hurts availability too much, and eventual consistency is the only guarantee.
This is true to some extent, but an oversimplified statement.

### Multi-leader replication

A multi-leader configuration (master-master, active/active replication) allows more than one node to accept writes.
Each leader in this setup simultaneously acts as a follower to other leaders.

It rarely makes sense to use a multi-leader setup in a single datacenter as the benefits rarely outweighs the added complexity.
With multiple data centers this can serve as an alternative to having all your writes go through a leader in one data center.
* Perceived performance may be better in this case as user writes can be handled by the data center closest to them, and the inter-datacenter network delay is hidden from them. Having all writes go through one node may defeat the purpose of multi-data center.
* This can tolerate data center outage.
* This is more tolerant of network problems as inter-data center connections usually go through the Internet and is much more error prone than local network within a data center: a temporary network outage does not prevent writes from being made.

Some databases support multi-leader configuration by default, but it often implemented by external tools over commercials DBs.

Multi-leader comes with a big downside of the same data can be concurrently modified in two different data centers, and those writes have to have their conflicts resolved.

Multi-leader can be quite dangerous due to configuration pitfalls and surprising interaction with other DB features such as autoincrementing keys, triggers and integrity constraints.


Clients with offline operation support is essentially a multi-leader replication, as when offline the client's local DB acts as a leader that can accept writes.

Realtime collaborative editing such as Google Doc poses a similar problem, where the user edits can be written to the local web browser acting as a leader.
You could make a user wait until another has finished editing (holding a lock), which would be similar to single-leader replications with transactions on the leader.
For faster collaboration you'll want to make the unit of change very small and avoid locking, which brings the challenges of multi-leader replication including requiring conflict resolution.

