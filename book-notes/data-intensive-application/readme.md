# Designing Data Intensive Applications

# Chap 1. Reliability, scalability and maintainability

Data intensive vs compute intensive

### Reliability

* Fault and failure
* Hardware error, usually uncorrelated and handled by redundancy
* Software error, usually happens when certain assumptions about the system don't hold any more. Correlated and cascading and harder to debug
* Human error

### Scalability

* Defining load

##### The Twitter example: to handle large fanout large read workload problem

* Relational query approach
* Fanout queue approach

##### Measuring performance

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

### Handling write conflicts

Imagine two users modifying the same thing at the same time, in a single-leader system, the second write (ordering is deterministic) can be either blocked or rejected while the first is ongoing.

You could make the replication synchronous to handle this, but this breaks the main advantage of multi-leader replication: allowing each replica to accept writes independently.

##### Conflict avoidance

Best way to deal with conflicts is to avoid them: if the application can ensure all writes for a particular record go through the same leader (the social media profile example), then conflicts cannot occur.

In some cases you have to change the designated leader for a record, due to datacenter failure or user having moved, in which case concurrent writes on different leaders needs to be dealt with again.

##### Converging towards a consistent state

In a multi-leader setup if each leader were to just apply writes in the order they receive them, eventual consistency cannot be guaranteed in that writes can have different orders getting to different leaders.
We'll want a convergent way which can be achieved with:
* give each write a unique ID (random number, timestamp, UUID, hash, etc) and pick the write with the highest ID as the winner. If timestamp is used this is known as last write wins, a popular approach but prone to data loss.
* give each replica a unique ID and let writes originated at a higher-numbered replica always take precedence. This also implies data loss.
* somehow merge them, e.g. order them alphabetically then concatenate.
* record the conflict in an explicit data structure that preserves all information and write application code that resolves the conflict at some later time (perhaps by prompting the user).

##### Custom conflict resolution logic

Conflict resolution in multi-leader systems are often application-specific, and these systems would let application supply their custom logic for conflict resolution, which gets executed either on-write or on-read. In latter's case all conflicting versions are written and given to the reader the next time they are read, the reader resolves automatically / manually, and the result gets written back.

Note that conflict resolution usually applies at the level of an individual row or document, not for an entire transaction.
A transaction with several writes usually have each of these writes considered separately for conflict resolution.

Conflict resolution is error-prone, Amazon is often cited as having surprising effects due to this.
* This inspired research in **CRDT** (conflict-free replicated datatypes), a family of data structures for sets, maps, ordered lists, counters, etc that can be updated concurrently by multiple users and automatically resolve conflicts in reasonable ways.
* Mergeable persistent data structure tracks history explicitly, similarly to git, and uses a three-way merge function. (whereas CRDT uses two way merge)
* Operational transformation is the algorithm behind Google docs, designed particularly for concurrent editing of an ordered list of items, such as the list of characters that constitute a document.

##### Multi-leader replication topologies

With more than two leaders, various replication topologies are possible, circular, star, all-to-all, etc.

In circular and star a replication passes through several nodes before reaching all replicas.
Nodes forward data and to prevent infinite replication loops each node is given a unique identifier and in the replication log each write is tagged with the identifiers of all nodes it passed through, and a node won't apply changes that are already tagged with its own tag.

Node failure in a circular or star topology interrupting replication flow is also a bigger issue than in a more densely connected topology.

All-to-all topologies may have issues with some replication messages overtaking others, like causality being violated because different paths gets causally related messages over at different times.
Simply adding a timestamp won't fix it as timestamp cannot be assumed to be in sync all the time.

To order these correctly version-vector can be used.
Many multi-leader replication aren't implemented carefully and it's worth checking your DB's docs and test to ensure it actually provides the guarantees you believe it to have.

### Leaderless replication

Dynamo is an example of this where a client can write to any nodes.
Riak, Cassandra are open-source leaderless replication systems inspired by Dynamo.

There is no concept of failover in a leaderless replication system.
The client writes to multiple replicas and as long as enough number of replicas returned success the write is considered successful by the client.
The client also reads from multiple replicas and in parallel and version numbers are used to decide which value is newer.

When an unavailable node comes back online, to get up-to-date data to it we could use read repair, where a client makes a read from several nodes in parallel and detect stale response, in which case they write the newer value back to that relica. This works well for values that are frequently read.

Many data stores also have an anti-entropy process that constantly looks for differences in data between replicas and copies over missing data from one replica to another. This may have signifcant delays.

##### Quorums for reading and writing

If there are n replicas, every write must be confirmed by w to be considered successful, and we must query at least r nodes for each read. 
As long as w + r > n we expect to get up-to-date value when reading, because at least one we read from must be up to date.
Reads and writes that obey this are called quorum reads and writes.
A typical setup is to let n be an odd number and r, w be n/2 rounded up.
When fewer nodes returned success the read or write operation returns error.

Lowering w + r below n will make you likely to read stale values, but allows higher availability and lower latency.

Although quorums appear to guarantee that a read returns the latest written value, in practice it's not so simple due to edge cases.
Dynamo-sstyle databases are generally optimized for use cases that can tolerate eventual consistency, and r + w shouldn't be taken as guarantees.
In particular you usually don't get read-your-writes, monotonic-reads, consistent-prefix-reads, as they require transactions or consensus.

##### Monitoring staleness

For leader-based replication because leaders and followers apply the write in the same order, you can typically monitor the amount of replication lag.
In leaderless replication there is no fixed order in which writes are applied, making monitoring more difficult.

Eventual consistency is a deliberately vague guarantee, but for operability it's important to be able to quantify eventual.

##### Sloppy quorums and hinted handoff

Leaderless databases with appropriately configured quoroms can tolerate failures without failover as well as slowness, since as long as w / r nodes returned the operation succeeds.
This makes them appealing for high-availability low-latency workload, and one that can occasionally tolerate stale reads.

This scheme as described is not tolerant to network partition, during which it's likely fewer than w or r reachable nodes remain and a quorum cannot be reached.

Designers face a tradeoff in this case: block off all reads / writes, or let writes proceed anyway to nodes that are reachable but aren't among the n nodes on which they value usually lives.

The latter is **sloppy quorum**: reads and writes still require r and w successful responses, but those may include nodes that aren't among the designated n home nodes for a value.

Once the partition is fixed, any writes that one node temporarily accepted on behalf of another are sent to the appropriate home nodes, this is called **hinted handoff**.

Sloppy quorum is particularly useful for increasing write availability, the database can accept writes as long as any w nodes are available.
This also means even when w + r > n, you cannot be sure to read the latest value for a key, as the value may have been temporarily written to some nodes outside of n.

Hence sloppy quorum isn't a quorum but a durability assurance.
Sloppy quorum are optional in all common Dynamo implementations (Riak, Cassandra, Voldemort).

##### Multi-data center operation

Leaderless replication is also useful for multi-datacenter operation, since it is designed to handle conflicting concurrent writes, network interruptions, and latency spikes.

Cassandra usually have the number of replicas n in all data centers, each write is sent to all replica but only w needs to acknowledge, and these are usually all from local data centers so the client is unaffected by inter-data center links.

### Detecting concurrent writes

Dynamo-style DBs allows several clients to write to the same key, and conflicts can occur during writes, read repair or hinted handoff.

If each node simply overwrote the value for a key whenever it received a write request from a client, we could end up in an eventually inconsistent state.
The replicas should converge towards some value, and if you as the application developer want to avoid losing data, you usually need to know a lot about the internals of your database's conflict handling.

##### Last write wins

Concurrent writes don't have a natural ordering so we force arbitrary orders on them, e.g. attach a timestamp to each write and pick the biggest timestamp as the most recent. This conflict resolution LWW is the only support resolution in Cassandra.

LWW achieves eventual convergence but at the cost of durability. It may even drop writes that are not concurrent due to time drift.

If losing data is unacceptable LWW is a poor mechanism for conflict resolution, the only safe way to use LWW is to ensure a key is written only once and immutable thereafter. E.g. a recommended way of using Cassandra is to use a UUID as the key, thus giving each write operation a unique key.

##### "Happens-before" relationship and concurrency

How to decide if two writes are concurrent: an operation A happens before another operation B if B knows about A or depends on A or builds upon A in some way.
Whether one operation happens before another is the key to defining what concurrency means.
Two can be said to be concurrent if neither knows about the other.

Note that concurrent here does not necessarily mean happening at the same physical time as with clocks in distributed system it's usually quite difficult to tell two things happening at the same time.

(From a physics perspective, if information cannot travel faster than the speed of light, then two events sufficiently far away from each other cannot possibly affect each other if the time delta of them happening is lower than the time light would take to propagate from one location to another.)

##### Capturing "happens-before"

Imagine a single server with multiple writers,
* Server maintains a version number for every key, increments the version every time that key is written, and stores the new version number along with the value written.
* When a client reads a key the server returns all values that have not been overwritten as well as the lastest version number. A client must read a key before writing.
* When a client writes a key it must include the version number from the previous read (as an indication of what I've already seen), and it must merge together all the values it received in the prior read.
* When the server receives a write with a particular version number, it can overwrite all values with that version number or below (since it knows that they have been merged into the new value), but it must keep all values with a higher version number (because those values are concurrent with the incoming write)

Essentially, when a write includes the version number from a prior read, that tells us which previous state the write is based on.

##### Merging concurrently written values

The above algorithm ensures nothing is silently dropped, but it unfortunately requires the clients to do extra work to merge the concurrently written values (siblings) before writing another.

Merging siblings is essentially the same problem as conflict resolution in multi-leader replication as discussed before.

You could merge by taking one value based on a timestamp (losing data), take a union of all siblings, or if you allow clients to also remove, union won't do the right thing and the system must leave a marker (tombstone) with the appropriate version number to indicate that the item has been removed when merging siblings.

As merging siblings in application code is complex and error-prone, some data structures try to perform this automatically, e.g. CRDTs.

##### Version vectors

Scaling the algorithm to multiple replicas, we need to use a version per key as well as per replica.
Each replica increments its own version number when processing a write, while also keeping track of version numbers it saw from other replicas. This indicates what to overwrite and what to keep as siblings.

The collection of version numbers from all the replicas is called a **version vector**, they are sent from the replica to the client when read, and sent back to the database when a value is subsequently written.
Version vector allows the DB to tell which writes are causal and which are concurrent.

Riak's dotted version vector is probably the most used, which it calls a causal context.

Similar to the single-replica example, the application may need to merge siblings.

### Summary

Replication can serve these goals: high availability, disconnected operations, lower latency, scale better.

Three major approaches to replication:
* single-leader, write all goes to leader, read can be served from any. Reads might be stale. Easy to understand and implement.
* multi-leader, write sent to one of several leaders, leaders propagate changes to each other and followers. More robust but harder to reason and provides only weak consistency guarantees.
* leaderless, read and write both from several nodes, to detect and correct nodes with stale data.

The effects of replication lag and different consistency models: eventual, read-your-write, read-monotonic, consistent-prefix-reads.

In multi-leader and leaderless system, ways to reconcile write conflicts. LWW, version-vectors based merge.

# Chap 6. Partitioning

A partition in this chapter is called a shard in MongoDB, Elasticsearch, SolrCloud, a region in HBase, a tablet in Bigtable, a vnode in Cassandra and Riak, and a vBucket in Couchbase.

In effect each partition is a small database of its own, although the database may support operations that touch multiple partitions at the same time.

The main reason for wanting partitioning is scalability. Large complex queries can potentially be parallelized across many nodes.
Different partitions can be placed on different nodes in a shared nothing cluster.

The fundamentals of partitioning apply to both kinds of workloads.

### Partitioning and replication

Partitioning is usually combined with replication so that copies of each partition are stored on multiple nodes.
Even though each record belongs to exactly one partition, it may still be stored on multiple different nodes for fault tolerance.

Each partition's leader is assigned to one node, and each node can be the leader for some partitions and a follower for other partitions.

The choice of partitioning scheme is mostly independent from the choice of replication scheme, so this chapter ignores replication.

### Partitioning of Key-Value data

Goal: spread the data and the query load evenly across nodes.

A skewed partitioning makes partitioning less effective in that some partitions serve more than others (hot spot).

We can randomly assign, which is unideal in that we don't know which node a particular queried item is on and we have to query all nodes in parallel.

##### Partitioning by key range

Partitioning by a sorted key is used by Bigtable, its open source equivalent HBase, and earlier MongoDB.

With each partition we keep keys in sorted orders,
* advantage being range scans are easy,
* downside being certain access patterns can lead to hot spots (think a process keeps writing real world clock time sensor readings to a timestamp partitioned database. In this case you need something other than timestamp as the first element of the key, e.g. the sensor name. Now if you want all sensor levels over a time range, multiple queries are needed)

##### Partitioning by key hash

A good hash function takes skewed data and makes it uniformly distributed.
For partitioning purposes they need not be cryptographically strong, MD5, murmur3, Fowler-Noll-Vo functions are all used.
Each partition would now serve a range of hashes as opposed to keys.
This technique is good at distributing keys fairly among partitions.
Boundaries are evenly spaced, or chosen pseudo-randomly.

**Consistent hashing** uses randomly chosen partition doundaries to avoid the need for central control or distributed consensus.
This approach actually doesn't work very well for databases so it's rarely used in practice.

Using hash means the sorted order of keys is lost.
Range queries on primary keys are not supported by Riak, Couchbase or Voldemort, and in MongoDB enabling hash-based partitioning means range queries will be sent to all partitions.

Cassandra does a compromise between the two partitioning strategies: a table can be declared with a compound primary key of several columns, the first part of that key is hashed to determine the partition, and the others are used as a concatenated index for sorting data in Cassandra's SSTable.
A query can therefore specify the partitioning key and do range queries on the other columns with a fixed partitioning key.

This enables an elegant data model for one-to-many relationships, like a social media site where a user has many updates, the partitioning key is the user ID, and we could then query the user's feed within a time range.

##### Skewed workloads and relieving hot spots

Think of unusual cases of a celebrity on social media, with user ID hash-based key all their query would still fall onto the same partition.

Today's DB typically doesn't handle this automatically, application can reduce the skew if it knows one key to be very hot, by appending a random number to the key (1 digit gives you 10 partitions), with downsides being read now needs to do additional work to read from all partitions.
This also requires additional bookkeeping.

### Partitioning and secondary indexes

If data is over accessed by primary key we can determine the partition from that key.

The problem is more complicated with secondary indexes involved.
A secondary index usually doesn't identify a record uniquely but rather is a way of searching for occurrences of a particular value.

Secondary indexes are the bread and butter of relational DBS and they are common in document databases, too.
Many key-value stores such as HBase and Voldemort have completely avoided them, some started adding them, and they are the raison d'etre of search servers such as Solr and Elasticsearch.

The problem is they don't map neatly to partitions, and two main approaches are document-based partitioning and term-based partitioning.

##### Document-based partitioning

Each partition maintains its own secondary indexes covering only documents in that partition (i.e. a local index).
(e.g. map each secondary key's value to the primary keys of records living in this partition)

So querying by secondary index would mean sending a query to all partitions, and combine the results you get back.

This approach to querying a partitioned database is sometimes known as **scatter/gather**, and it can make read queries on secondary indexes quite expensive.

Despite this it is widely used in Cassandra, MongoDB, Riak, Elasticsearch and SolrCloud.

##### Term-based partitioning

Rather than having local indexes as above, we can a global index that covers all data in all partitions and partition that global index to different nodes (by key range, or hash).

The term (full-text indexes) we are looking for decides which partition the global index is on, we read the index from that partition and figure out the primary keys of records we need to query.

Reads now only needs to request from partitions containing the term it wants (as well as the global index), but writes are slower as a writing to a single document may now affect multiple partitions of the index (every term in the document might be on a different partition).

This would also require a distributed transaction across all partitions affected by a write (to keep data and indexes on different partitions in sync), which many don't support.
In practice updates to a global secondary indexes are often asynchronous, as in Dynamo.

### Rebalancing partitions

Data size change, machine failure, etc, all calls for moving data from one node to another, a process called **rebalancing**.

Requirements for rebalancing: load should be shared fairly between nodes, while rebalancing the DB should continue accepting reads and writes, no more data than necessary should be moved when rebalancing.

##### Hash mod n

This makes rebalancing expensive in terms of data that has to be moved around, hence earlier the hash-based partition lets each node store a hash range.

##### Fixed number of partitions

Create much more partitions than there are nodes, assign multiple partitions to the same node.

If a new node is added, the node can steal a few partitions from every existing node until partitions are fairly distributed again.

The number of partitions or the mapping from keys to partitions don't change, the only thing that changes is the mapping of partitions to nodes.
While a rebalancing is ongoing, the old node that this partitions is on continues serving read and write requests.

This approach is used by Riak, Elasticsearch, etc.

For simplicity, some of these DBs don't implement splitting a partition, so the number configured initially is the max number of partitions you are going to have, which should be larger than your number of nodes.
You should then choose a number high enough to account for future growth but not too high as each partition has management overhead.

##### Dynamic partitioning

For key-range partitioning DBs a fixed number of partitions with fixed boundaries can be very inconvenient.

For this reason HBase creates partitions dynamically: it splits and merges in process similar to B-tree nodes.
After a split one half is transferred to another nodes, and in case of HBase, the transfer happens through HDFS the underlying distributed file system.

Advantage is the number of partitions adapts to the volume of data.

To not start out from one single partition (empty DB), HBase and MongoDB allow an initial set of partitions to be configured on an empty DB.
This requires you to know what the key distribution is going to look like.

Dynamic partitioning can be applied for key range partitioned data as well as hash-partitioned data.

##### Partitioning proportionally to nodes

With dynamic partitioning the number of partitions is proportional to the size of the dataset, as split and merge keep the size of each partition between some fixed min and max.

With fixed number of partitions the size of partitions is proportional to the size of the dataset.

In both these cases the number of partitions is independent of the number of nodes.

Cassandra makes the number of partitions proportional to the number of nodes, i.e. to have a fixed number of partitions per node.
In this case the size of each partition grows proportionally to the dataset size while the number of nodes remains unchanged, but when increasing the number of nodes each partition becomes smaller.
Since a larger data volume generally requires a larger number of nodes to store, this also keeps the size of each partition fairly stable.

When a new node joins the cluster it randomly chooses a fixed number of existing partitions to split and takes ownership of one half of each of those split partitions while leaving the other half in place.
This introduces unfair splits but averaged over a large number of partitions the new node ends up taking a fair share of load.

Picking partition boundaries randomly requires hash-based partitioning so the boundaries can be picked from the range of numbers produced by the hash function.

##### Manual or automatic rebalancing

Fully automated rebalancing can be convenient due to less operational work, but can be unpredictable in that rebalancing is expensive and if not done carefully this can overload the network and harm the performance while rebalancing is in progress.

This could create cascading failure when used in combination with automatic failure detection. (Detect an overloaded node to be slow, decides to move data away from it and further overloading that node)

### Request routing

How does a node know which partition to request from?

This is an instance of a more general problem called **service discovery**.

Several high-level approaches:
* Allow clients to contact any node (via a round-robin load balancer, e.g.), if that node does not own the partition it forwards the request to the appropriate node, receives a reply and passes that on.
* Send requests from clients to a routing tier first which determines the node to handle the request and forwards it. The routing tier acts as a partition-aware load balancer.
* Require clients be aware of the partitioning and the assignment of partitioning to nodes, a client can connect directly to the appropriate node without any intermediary.

In all cases the key problem is how does the routing decision component learn about changes in partition assignment?

Many distributed data systems rely on a separate coordination service such as ZooKeeper to keep track of this cluster metadata.

Each node registers itself with ZooKeeper which maintains the authoritative mapping of partitions to nodes.
The routing component subscribes to ZooKeeper and gets a notification when a change in partition happens.

HBase, SolrCloud and Kafka use ZooKeeper to track partition assignment.
MongoDB uses similar architecture but relies on its own config server implementation.

Cassandra and Riak take a different approach which uses a gossip protocol to disseminate any changes in cluster state.
Requests can be sent to any node and that node forwards them to the appropriate node for the requested partition.
This model puts more complexity in the database nodes but avoids the dependency on an external coordination service such as ZooKeeper.

Couchbase does not rebalance automatically which simplifies the design.

When using a routing tier or sending requests to a random node clients still need to find the IP addresses to connect to, this usually isn't fast changing and DNS works just fine.

### Parallel query execution

The above focused on very simple queries that reads or writes a single key (or scatter/gather in the case of working with document-partitioned secondary indexes).
This is about the level of access supported by most NoSQL distributed data stores.

Massively parallel processing, often used for analytical workload, is much more sophisticated in the types of queries they support.
The query optimizer breaks a complex query into parallel stages.

### Summary

Goal of partitioning: scalability: spread the load evenly, avoid hotspots.

Two main approaches:
* Key range partitioning. Efficient range queries, risk of hot spots.
* Hash partitioning. Each partition owns a range of hashes. Inefficient range queries, more even data distribution. This is often used in combination with fixed number of partitions although dynamic partitioning can also be used.
* Or a hybrid of the two like the compound key in Cassandra.

To support secondary indexes:
* Document-partitioned indexes (local indexes). Secondary index stored in the same partition as the primary key and value. A single partition to update on write, but a read of the secondary index requires a scatter/gather across over all partitions
* Term-based indexes (global indexes). When write several partitions will need to be updated, however a read can be served from a single partition.

Rebalancing strategies.
Routing techniques.

By design each partition operates mostly independently which allows a partitioned database to scale to multiple machines.

# Chap 7. Transactions

Many things can go wrong in a distributed data system, failing in the middle of a write, may crash at any time, network can partition, several clients can overwrite each others' changes, may read data that's partially written, race conditions, etc.

Transactions have been the mechanism of choice for simplifying these issues: they simplify the programming model for applications accessing a DB.
Transactions group a series of reads and writes into one logical unit to be executed as one: the entire batch either succeeds or fails.

Transactions make application not need to worry about partial failures.
Not every application needs transactions.

This chapter discusses read committed, snapshot isolation, and serializability.
These concepts apply to distributed storage as well as single node.

Transactions have been the main casualty of the no-SQL movement.
There emerged a popular belief that transactions are the antihesis of scalability. This is not necessarily true.

### ACID

The safety guarantees provided by transactions are described by **ACID** (atomicity, consistency, isolation, durability.)
In practice one database's implementation of ACID does not equal another's. E.g. isolation can be quite ambiguous.

Systems not meeting ACID (vague) are sometimes called BASE (basically available, soft state, eventual consistency), an even vaguer term.

Atomicity (atomic operation), in the context of multi-threaded program, means another thread cannot see the the half-finished operation of another.
In ACID atomicity does not have to do with multiple processes trying to access data at the same time (that is isolation).

In ACID **atomicity** means if among several writes one fails at some point, the database must discard or undo any writes it has made so far in that transaction.

**Consistency** is a terribly overloaded term.
* Replica consistency refers to eventual consistency, read-after-write, monotonic-read, consistent-prefix-read.
* Consistent hashing is an approach to partitioning some systems use for rebalancing.
* CAP theorem consistency means linearizability
* ACID consistency means an application-specific notion of database being in a good-state.

ACID consistency means some invariants about your data must always be true. (like in an accounting system credits and debits must balance.)
This then becomes an application-specific definition.
The application may rely on the DB's atomicity and isolation properties to achieve consistency, this is not up to the DB alone and C from ACID should be tossed.

ACID **isolation** means concurrently executing transactions are isolated from each other.
The classic database textbooks formalize isolation as serializability meaning each transaction can pretend it's the only transaction running on the DB, though in reality there might be several running at the same time.

ACID **durability** is a promise that data a transaction wrote successfully will not be forgotten.
It usually involves writing to hard drive as well as a write-ahead log for recovery and a DB reporting a transaction as successful only after successful log write.
Perfect durability (reliability) does not exist, there are risk-reduction techniques but take any theoretical guarantee with a grain of salt.

Durability historically meant writing to a disk, but now has been adapted to mean replication.

### Single and multi-object operations

Atomicity and isolation describe what should happen if clients make transactions: all-or-nothing, concurrent transactions shouldn't interfere with each other (another transaction should not see half written results of this transaction)

This usually requires some way to tell which reads and writes are in the same transaction, relational database uses `begin transaction` and `commit`.
Many non-relational DBs don't have a way of grouping operations together, even with a multi-object API in the same statement, it doesn't necessarily mean that statement guarantees all-or-nothing.

Atomicity and isolation apply to single-object writes as well, imagine a disk failure when halfway through writing a large object.
What does the storage engine guarantee in this case?
It'd be very confusing if no guarantees are provided, so storage engines typically provide atomicity and isolation on the level of a single object on one node, which can be implemented with a write-ahead log (atomicity) and a lock on each object (isolation).

Some databases also provide an atomic increment operation, and a compare-and-set operation.

These single-object atomicity and isolation guarantees aren't the usual sense of ACID A and I: they usually refer to grouping multi-object modifications into one unit of execution.

In many cases multiple objects need to be coordinated, updates with foreign keys in relational model, updating denormalized fields in document model, and updating secondary indexes.
Implementing these without transactions is possible, but error handling is made difficult.

A key feature of a transaction is that it can be aborted and safely retried if an error occurred.

Although retrying an aborted transaction is a simple and effective error handling mechanism, it isn't perfect in that
* transaction may have actually succeeded but network failed to deliver the success message. You'll be redoing the transaction in this case which would require an application-level deduplication mechanism
* if error is due to overload retrying is going to make matters worse without reasonable exponential backoff / number limits.
* it's only worth retrying after transient errors, retrying after permanent error is pointless.
* if the transaction is not side effect free the side effect could happen twice.
* if a client fails while retrying what it's trying to write is lost.

### Weak isolation levels

Race conditions come into play when one transaction reads / writes data that is concurrently modified by another transaction.
Concurrency can be difficult to reason about and debug.
For this reason databases have long tried to hide concurrency issues from application developer by providing isolation.

**Serializable isolation** is a guarantee that transactions have the same effect as if run one at a time.
This comes at a performance price, one which many DBs don't want to pay.

##### Read committed

**Read committed** is the most basic level of transaction isolation. It makes two guarantees: when reading from DB, you will only see data that have been committed (no dirty reads), when writing to the DB you will only overwrite data that has been committed (no dirty writes).

Read committed does not prevent e.g. two transactions incrementing the same counter but the counter ends up being incremented only once. (first read, second read, both get the same value and did not dirty read, first write incremented, commit, then second write incremented, commit, the later commit did not dirty write)

Most commonly databases prevent dirty writes by using row-level locks: modifying a row / document requires holding a lock on it, and it must then hold the lock until the transaction is committed or aborted.

No dirty read can be enforced by having readers acquire the same lock while they read, this affects performance.
And instead most databases prevent dirty reads by remembering the old committed value and the new value set by the transaction currently holding the write lock, and while write transaction is ongoing it returns the old committed value to readers.

##### Snapshot isolation and repeatable reads

Imagine a user transferring money between her 2 bank accounts, one transaction does two writes: acct1 += 100 (1), acct2 -= 100 (2), commit, another transaction does two reads of acct1 (3) and acct2 (4), commit, with read committed isolation if we have sequence 3 1 2 4 the read is going to see acct1 without the +100 and acct2 with the -100.
Such a read is **unrepeatable** (or **read skew**) in that doing the read transaction again after the write is committed will give the expected result.

**Snapshot isolation** is the most common solution, idea being each transaction reads from a consistent snapshot of the database, meaning the transaction sees all the data that was committed in the DB at the start of the transaction, even if the data is subsequently changed by another transaction, each transaction sees only the old data from the particular point in time.

This makes integrity checks possible which would otherwise be difficult with just read committed isolation.

Implementing snapshot isolation also uses a writer lock but read does not require locks.
Read is built upon a generalized approach in read committed: as the DB must potentially keep several different committed versions of an object, as various in-progress transactions may need to see the state of the DB at different points in time.
This is known as **multi-version concurrency control (MVCC)**

Still, readers don't block writers and writers don't block readers.

Each transaction is usually given a unique increasing ID, and each write is tagged with transaction ID (created by transaction X, deleted by transaction Y. An update is delete + creates).
Deleted aren't immediately gone but garbage collected later.

When a transaction reads, the transactio IDs are used decide which objects are visible (essentially a long enough history of writes to the object, and find the latest committed point in history that was before your read transaction).
* At the start of each transaction, DB makes a list of all other transactions in progress at the time. Any writes those transactions have made are ignored (even if they become committed later).
* Any writes made by aborted transactions are ignored.
* Any writes made by transactions with a later transaction ID (started after the current transaction) are ignored.
* All other writes are visible to the application's queries.

How do indexes work in a multi-version DB?
We could have it point to all versions of an object and require an index query to filter out versions not visible to the current transaction. 

With a B-tree implementation, indexing multiple versions could look like a append-only/copy-on-write B-tree that does not overwrite pages of the tree when updated, but creates a copy of each modified page.
Parent pages up to the root of the tree are copied to point to the new versions of child pages. Pages not affected by a write need not be copied.
Every write then creates a new B-tree root and a particular root is a consistent snapshot of the database at the point in time when it was created.
This approach requires a background process for compaction and garbage collection.

Repeatable read can be a confusing term in SQL standards. Some use repeatable reads to refer serializability.

##### Preventing lost updates

Dirty write is only one type of write conflicts that can happen.
Another is the lost update problem, as illustrated in the incrementing two counters case. (an read-update-write cycle, parse-change-write-back a json object, two users editting the same wiki at the same time, etc)

**Atomic write operation** is one solution, which removes the need of read-update-write cycles in application code, which are usually the best if your code can be expressed in such.

Mongo supports atomic operations for making local modifications of a json document, and redis provides atomic operation to update a data structure like priority queue.
They are usually implemented with an exclusive lock on the object such that when read, other reads are also blocked.
Another option is to force all atomic operations on a single thread.

**Explicit locking** is another approach if the DB's built-in atomic operations don't provide the needed functionality.
This works but can be hard to get right.

**Automatically detecting lost updates**, as opposed to forcing serial like in previous approaches, this allows parallel and tries to detect lost update and when detected, forces one transmission to abort and retry.
This check can be performed efficiently in conjunction with snapshot isolation.

Some DBs instead provide a **compare-and-set** operation: this avoids lost updates by allowing an update to happen only if the value has not changed since you last read it.
If current value does not match what you previously read, this forces an abort and the read-modify-write cycle has to be retried.

In a replicated scenario, lost updates can happen on different nodes.
Locks and compare-and-set assume there is a single up-to-date copy of the data, which cannot be guaranteed in a multi-leader / leaderless replication.

Instead they allow concurrent writes to create siblings, and use application code or special data structures to resolve and merge them.

Atomic operations can work well in a replicated context, especially if they are commutative (they can be applied in different orders and get the same result).

##### Write skew and phantoms

Imagine you have a hospital where at least one person has to be present oncall, and a person can give up oncall if at least there is another oncall.

Now the only two persons oncall update their individual records to give up oncall at the same time (check if there is another person on-call using a count(select) on all, if so, give up its own on-call), and the system could end up with 0 persons oncall.

This is a **write skew**, not a dirty write or lost update, since the two transactions are updating two different objects.
This can be thought of a generalization of the lost update problem: two transactions read the same objects, then update some objects (different in this case, same  in the case of dirty writes or lost updates).

Automatically preventing write skew requires true serializable isolation.

Some DB provides constraints (foreign keys constraints, or restrictions on a particular value).
Most don't have support for constraint involving multiple objects but one may be implemented with materialized views or triggers.

Without true serializable isolation level your best option is to explicitly lock all the rows the transaction depends on.

Enforcing two users cannot claim the same username in a snapshot isolation DB has the same problem, fortunately unique constraint is a simple solution here where the DB will reject the second transaction.

All these examples follows a similar pattern: read-check condition-write, where the write could change the condition checked in step 2.
In the first example we can lock everything read in step 1, and in the second we are checking for absence and we can't attach a lock to anything.
An approach called **materializing conflicts** would have us create locks in advance for non-existent objects.
This is error-prone and leaks a concurrency mechanism into application model.

The effect where a write in one transaction changes the result of a search query in another transaction is called a **phantom**.
Snapshot isolation prevents phantoms in read-only queries, but read-write transactions can still have phantoms that led to write skew.

### Serializability

Looking at some application code it's hard to tell if it's safe to run at a particular isolation level, and there are no tools to help detect race conditions.

The simple answer has been use serializable isolation, which was usually regarded as the highest isolation level.

Historically on a single node serializable isolation was implemented with actual serial execution, two phase locking, or optimistic concurrency control techniques such as serializable snapshot isolation.

##### Actual serial execution

Remove concurrency entirely.
Only recently have designers decided a single-threaded loop for executing transactions was feasible.
Two developments caused this: RAM has become cheap enough that it's often feasible to keep the entire dataset in memory, executions become much faster due to no disk involvement. DB designers realized OLTP reads and writes are usually small. By contrast analytical workloads are usually large and read-only, they can run a consistent snapshot outside of the serial execution loop. 

Redis, Datomic has support for this. In order to make the most of the single thread, transactions need to be structured differently from their traditional form.
Systems with single-threaded serial transaction processing don't allow interactive multi-statement transactions, instead the application must submit the entire transaction code to the DB ahead of time, as a **stored procedure**, to prevent the single thread waiting on the back and forth network transmission cost of interactive queries in the same transaction. (every transaction has to be small and fast)

Stored procedure has existed for some time in relational databases, and they've been part of SQL for long. They've a bad reputaion for different vendors using their own languages (Oracle PL, SQL server T-SQL, Redis Lua, Datomic Java), code running in a DB being difficult to manage / test / deploy / integrate with a monitor system, and a DB is often much more performance sensitive than an application server and a badly written stored procedure can mean more trouble.

Executing all transactions serially made concurrency control simpler, but limits the transaction throughput to the speed of a single CPU core on a single machine, this can be a bottleneck for a high write throughput system.

To scale to multiple CPU cores, you can partition your data such that each transaction only needs to read and write data in one partition served by one CPU core.
For any transaction needing to access multiple partitions the database must coordinate the transaction across all the partitions it touches, and the stored procedure needs to be performed in lock step across all partitions to ensure serializability across the system.
Data with multiple secondary indexes is particularly difficult.

##### Two phase locking

For around 30 years there was only one algorithm widely used for serializability: 2PL.
Note that 2PL is completely different from 2 phase commit, 2PC.

To prevent dirty writes, we have when two concurrent transactions trying to write the same object, the lock ensures the second writer wait till the first one has finished or aborted before it may continue.

Two phase locking's lock requirement is much stronger.
Several transactions are allowed to read the same object concurrently, but as soon as anyone wants to write an object, exclusive access is required.
* If transaction A has read an object and transaction B wants to write that object, B has to wait till A commits or aborts before it can continue.
* If transaction A has written an object and transaction B wants to read that object, B has to wait until A commits or aborts before it can continue.

In 2PL writers don't just block other writers, they also block readers and vice-versa.
This captures the key difference with snapshot isolation where readers never block writers and writers never block readers.

MySQL and SQL server use 2PL to implement serializable isolation level.

Blocking of readers and writers is implemented by having a lock on each object in the DB, the lock can be either in exclusive mode or shared mode.
* To read an object, the transaction has to acquire the lock in shared mode (not owned by any exclusive)
* To write to an object, the transaction has to acquire the lock in exclusive mode (not owned by any exclusive or shared)
* If a transaction first reads then writes, it may upgrade its shared lock to an exclusive lock, a process that works the same way as getting an exclusive lock directly.
* After a transaction has acquired the lock it must continue to hold the lock until the end of the transaction. This is where the name two-phase came from, first phase is when the locks are acquired, second phase where all locks are released at the end of a transaction.

Deadlock can happen with 2PL (they can also happen in lock-based read committed isolation level, but much more rarely), in which case the DB automatically detects and abort one of the transactions.
The aborted is later retried by the DB.

Big downside of 2PL is performance, due to locking overhead and reduced concurrency.

To prevent phantom writes, we may need **predicate locks** which rather than belonging to a particular row in a table, it belongs to all objects that match some search conditions.
If a transaction wants to read / write an object matching the predicate, it must acquire the shared / exclusive predicate lock as well. Same goes for transactions trying to insert / delete.
Predicate locks apply even to objects that don't yet exist in the database.

If two-phase locking includes predicate locks, the DB prevents all forms of write skews and other race conditions, making it serializable.

Predicate locks don't perform well, instead most DB with 2PL implement **index-range locking**, a simplified approximation of predicate locks (they lock a bigger range of objects than necessary by locking all objects associated with an index range, but they have lower overheads).

If there are no suitable index where a range lock can be attached, then the DB can fall back on entire table locking, it's safe but bad for performance.

##### Serializable Snapshot Isolation

Serializable isolation and good performance seem at odds with each other.
SSI may be able to change that.

2PL is a **pessimistic concurrency control** mechanism, based on the principle that if anything might possibly go wrong, it's better to wait until the situation is safe again before doing anything.

Serial execution in a sense is pessimistic to the extreme.

By contrast, SSI is an **optimistic concurrency control** technique.
Optimistic in that it lets transactions continue and hope everything will turn out alright, and when a transaction wants to commit, the DB checks whether anything bad happened.
If so, one has to be aborted and retried.

This is an old idea and performs badly if there is high contention.
However if there is enough spare capacity and contention is not too high, optimistic might be able to outperform pessimistic ones.

Contention can be reduced with commutative atomic operations. (when it doesn't matter which one is committed first.)

Earlier the write skew happened due to trasaction having acted on an outdated premise (query result might have changed), we could detect these transactions and abort them.
There are two cases:
* detecting reads of a stale MVCC object version (uncommitted write occurred before the read)
To prevent this anomaly, the database needs to track when a transaction ignores another transactions writes due to MVCC visibility rules.
When the transaction wants to commit, the database checks whether any of the ignored writes have now been committed. If so the transaction must be aborted.
* detecting writes that affect prior reads (the write occurs after the read)
An index keeps track of ongoing transactions that have read it, and when a transaction writes to the database it must look in the indexes for any other ongoing transactions that have read the affected data.
It notifies those transactions the data they read may not be up-to-date (consequently they may or may not need to be aborted and retried).

To decide the granularity at which transactions' reads and and writes are tracked, there is the tradeoff between bookkeeping overhead and aborting more transactions than necessary.

The big advantage over 2PL is one transaction does not need to block waiting for locks held by another transaction.
In particular, read-only queries can run on a consistent snapshot without requiring any locks, which is appealing for read-heavy loads.

Compared to serial execution SSI is not limited to the throughput of a single CPU core.
Serialization conflict detection can be distributed.
Transactions can read and write data in multiple partitioning while ensuring serializable isolation.

Performance SSI is affected by the rate of aborts, and SSI requires read-write transactions to be fairly short.
SSI is probably less sensitive to slow transactions than 2PL or serial execution.

### Summary

Transactions are an abstraction layer that allows an application to pretend that certain concurrent problems and faults don't exist: a large class of errors is reduced down to a simple transaction abort and the application just needs to retry.

Isolation level, read committed, snapshot isolation / repeatable read, serializable.
* Dirty reads, dirty writes, guaranteed by >= read committed
* Read skew, guaranteed by >= snapshot isolation, usually implemented with MVCC
* Lost updates, some snapshot isolation implementation prevent this, others require a manual lock (`SELECT FOR UPDATE`)
* Write skew (read-check premise-write), only serializable isolation prevents this anomaly
* Phantom read (one transaction reads results matching a condition, another writes that affects the results of the query). Snapshot isolation can prevent straightforward phantom reads, but phantoms in the context of write skew needs the likes of index-range locks.

Only serializable prevents all these issues, when using a weaker isolation level application needs additional logic (e.g. explicit locking) to protect against these.

Three ways to implement serializable.
* actual serial execution
* 2PL (pessimistic)
* SSI (optimistic)

# Chap 8. The Trouble with Distributed Systems

This chapter turns our pessimism to maximum, and assume everything that can go wrong will go wrong, except Byzantine failures.

### Faults and partial failures

An individual computer with good software is usually fully deterministic: either fully functional or completely broken, not something in between.

This is a deliberate choice in the design of computers, if an internal fault happens, we prefer it to crash completely rather than returning a wrong results, because the latter are hard to deal with: it hides away the physical reality on which they are implemented.

Distributed systems are completely different, nondeterministic partial failures are possible, which makes distributed systems hard to work with.

Two extremes of building large-scale computing systems, high-performance computer with thousands of cores, or cloud computing usually with multi-tenant datacenters, commodity computers connected with IP network and elastic resource allocation.

The first approach deals with errors usually with regular snapshotting.
If a node fails the entire cluster halts and recovers from a snapshot, this is more like a single-node approach of error handling.
This chapter focuses on the failure handling of the second type.
Difference being
the second type of applications is often expected to be **online**, in that they need to be available to serve users with low latency at any time.
Unlike a supercomputer where each node is rather reliable, the commercial hardware has much higher error rates. It becomes reasonable to assume at any time in point something is always broken, and the system needs to be able to tolerate failed nodes (useful also for rolling upgrade, restarts, uninterrupted services). 
Network is often IP and Ethernet based arranged in **Clos topologies** to provide high bisection bandwidth.
Geographically distributed deployment where inter-datacenter is usually slow and unreliable.

We build a (reasonably) reliable system from unreliable components. (think error-correcting codes, reliability in TCP, etc)

In distributed systems, suspicion, pessimism, and paranoia pay off.

### Unreliable networks

Share-nothing architecture (not the only way but by far predominant).

Loss packet switched network. Delay, loss, one end (temporarily) stop responding, etc are all possible.
The sender can't tell whether the packet was delivered (with a possibly dropped ack then yes).

The usual way to handle this is timeout. Wait some time until you give up and try again.

Handling network faults doesn't necessarily mean tolerating them: you could just deliver an error message, but you do need to know how your software reacts to network problems and ensure they can recover.
It may make sense ti deliberately trigger network problems to test (Chaos monkey)

### Detecting faults. Timeouts. Undetected delays

Many systems need to automatically detect faulty nodes.
It's sometimes hard to do. TCP ports refusing connection can get you back a RST or FIN, but the node can crash mid request. If application process fails a script can detect and tell other nodes if the host OS is still working. The router may give you back an ICMP destination unreachable if the node you are trying to get to is unreachable. Hardware failure can also be detected at switch hardware level if you cam query the management interface of the switches.

If you want to be sure a request was successful, you need a positive ACK from the application itself.
NACK is useful for speedy detection but they cannot be relied upon.
You can wait for timeout, retry a few times, and declare dead if no response.

There is no simple answer to how to set the timeout.

Premature declaration of a node being down is problematic (places additional load (cascading failure possibility), potentially duplicated operation).

Imagine you have a network transmission time upperbound of d and processing time upperbound of r, then 2d + r seems a reasonable timeout, but most systems in reality don't have these guarantees.
If your timeout is low, it only takes a transient spike in round-trip time to throw the system off balance.

Many can queue a network packet: switch queueing, receiver OS queueing, sender queueing (e.g. due to TCP flow control).

TCP packet loss can cause further delay in OS waiting for retransmission and retransmitted packet to be acknowledged.

UDP is a good choice in situations where delayed data is worthless, e.g. videoconferencing and VoIP.

Better than a a configured constant timeouts, systems can continuously measure response times and their variability (jitter), and automatically adjust timeouts according to observed response time distribution.
TCP retransmission timeouts works this way, as does Phi Accrual failure detector in Cassandra.

### Synchronous network vs asynchronous

Telephone networks (synchronous) are circuit switched, a circuit (a fixed route) is established between the two parties throughout the call, much more reliable and does not suffer from queueing, as the 16B space for the call have already been reserved in the next hop of the network (during a call each side is guaranteed to be able to send exactly 16B of audio every 250ms).
Because the network has no queueing, the maximum end-to-end delay is fixed: a bounded delay.

TCP/IP network is packet switched as they are optimized for bursty traffic.
Audio/video call has a stable data rate, while web browsing can have a variable data rate: TCP/IP tries to deliver as fast as possible.

There has been attempts to build hybrid of packet switching and circuit switching such as Asynchronous Transfer Mode (ATM network), a competitor of Ethernet's.
It implements end-to-end flow control at link layer which reduces the need for queueing in the network, though it can still suffer from congestion and cause delays.
With careful use of quality of service (QoS, prioritization and scheduling of packets) and admission control (rate-limiting senders), it's possible to emulate circuit switching on packet switching networks or provide statistically bounded delay.

More generally, latency guarantees are achievable in certain environments, if resources are statically partitioned: dividing network link statically as described above, or allocating a static number of CPU cycles to each thread.
These come at the cost of reducing utilization. Multi-tenancy with dynamic resource partitioning provides better utilization, making it cheaper but with the downside of variable delay.

Variable delays in networks are not a law of nature, but simply the result of a cost/benefit tradeoff.
Currently deployed technology does not allow us to make any guarantees about delays or reliability of the network.

### Unreliable clocks

Time is tricky business in a distributed system: communication is not instantaneous, each machine has its own clock, usually a quatz crystal oscillator.
These are not perfectly accurate and each machine may have its own notion of time.

It is possible to sync time to some degree, most commonly with NTP, which allows the computer clock to be adjusted according to the time reported by a group of servers.
The servers in turn get their time from a more accurate time source, such as a GPS receiver.

##### Monotonic clock and time-of-day clock

Modern computer has at least these two kinds and they serve different purposes.
Time-of-day clock / wall clock time gets you time according to some calendar, `clock_gettime(CLOCK_REALTIME)` call on Linux, gets you number of seconds since the epoch UTC 1970 Jan 1 midnight according to Gregorian calendar, not counting leap seconds (a day may not have exactly 86400 seconds).

Time-of-day clocks are usually sync'ed with NTP, some oddities include, e.g. when a local clock is too ahead it may jump back in time to a previous point.
These jumps make time-of-day clock unsuitable for measuring elapsed time, historically they are also very coarse grained.

Monotonic clock is suitable for measuring time interval, such as a timeout or a service's response time.
`clock_gettime(CLOCK_MONOTONIC)` on Linux is monotonic clock. These clocks are guaranteed to move forward.
The absolute value of monotonic clock is meaningless, and monotonic clocks are not synchronized across machines.
It also makes no sense to compare the monotonic clock time from two different computers.

On a multi-CPU system, there may be a separate timer per CPU which is not synchronized with other CPUs.
OS tries to compensate for this discrepancy but one should take this guarantee of monotonicity with a grain of salt.

NTP may adjust the frequency at which the monotonic clock moves forward (by 0.05%) if it detects that the computer's local quartz is moving faster or slower than the NTP server, but it cannot cause monotonic clock to jump forwards or backwards.

Clocks drift (run faster or slower than it should) depending on the temperature.
Google assumes a clock drift of 200 parts per million for its servers, an equivalent of 6ms drift for a clock that is resync'ed with a server every 30s. This limits the best possible accuracy you can have, for wall clocks.
If a computer's clock differs too much from an NTP server, it may refuse to synchronize, or the local clock will be forcibly reset (consequently any applications will see a jump forward / backward in time).
NTP synchronization can also only be as good as the network delay.
NTP clients are robust enough to query a number of configured servers and discard outliers.
Leap seconds result in a minute being 59s or 61s long, which could mess up systems not designed with leap seconds in mind.

It is possible to achieve very good accuracy if you care about it sufficiently to invest significant resources, e.g. mifid ii draft requires all HFT to synchronize their clocks to within 100ms of UTC to help detect market manipulation.
Such precision can be achieved using GPS receivers, precision time protocol, and careful deployment.

If you use software that requires synchronized clocks it is essential that you also carefully monitor the clock offsets between all the machines.
Any node whose clock drifts too far from the others should be declared dead and removed from the cluster.

##### Timestamp for ordering events

Using synchronized wall clock to order events in distributed systems is not advisable.
In a multi-leader system drift between nodes can cause the replicas to not be eventually consistent without further intervention. (no matter if using leader timestamp or client timestamp, if multi-clients)
Use logical clocks for this purpose, which are based on incrementing counters rather than an oscillating quartz crystal.

##### Clock reading as confidence interval

Clock readings (over the network / compared with a server whose time this syncs to) is more like a range of times within a confidence interval.
`clock_gettime()` return value doesn't tell you the expected error of a timestamp, and you don't know the confidence interval, Google's TrueTime API in spanner explicitly reports the confidence interval on the local clock.
It returns two values `[earliest, latest]`, whose width depends on how long it has been since the local quartz clock was last sync'ed with a more accurate clock source.

##### Synchronized clocks for global snapshots transaction ID

Recall that in snapshot isolation each transaction has a motonically increasing ID and if B reads a value written by A, B should have a transaction ID higher than that of A's.
Generating a monotonically increasing ID in a distributed system with lots of small rapid transactions can be challenging.
Synchronized clock with good enough accuracy can be used to generate this ID, and Spanner implements snapshot isolation across data centers this way: with time returning a confidence interval, if two intervals don't overlap then we know in which order those two times are.
In order to ensure transaction timestamp reflects causality, Spanner deliberately waits for the length of confidence interval before committing a read-write transaction, so any transaction that can read this data happens at a sufficiently later time so that their confidence intervals don't overlap.
Hence Spanner needs to keep the interval as small as possible to minimize wait time, and for this reason Google deploys a GPS receiver or atomic clock in each data center, allowing clocks to be synchronized within 7ms.

##### Process pauses

Assume we've a single leader system, how does a leader know it's still leader and not proclaimed dead by others?
We can let the leader hold a lease, a lock with timeout.
To be leader the node has to renew lease before it expires.

Imagine a process renews its lease 10s before expiry (which should not be the wall clock time of a different node who set it), but the last request processing took longer than that, then by the time the request finishes this node would no longer hold the lease.
A pause like this can happen if mark-and-sweep GC runs long enough, virtual machine suspension and resume, context switches, slow disk access, OS swapping in-memory and on-disk vram pages frequently (thrashing), a process receiving a SIGSTOP followed by a late SIGCONT.

A node in a distributed system must assume that its execution can be paused for a significant length of time at any time even in the middle of execution.

##### Response time guarantees, limiting the impact of GC

Processes can pause for unbounded time as shown before.
On the other hand hard real time systems have a specified deadline by which the software must respond.
Providing real-time guarantees in a system requires support from all levels of software stack: a real time operating system (**RTOS**) that allows processes to be scheduled with a guaranteed allocation of CPU time in specified interval is needed. Library functions need to document worst-case execution time. Dynamic memory allocation may be restricted or disallowed algother. An enormous of testing is required to guarantee the requirements being met.

These places a lot of constraints on programming languages, libraries and tooling. Real-time systems often have lower throughput as they have to prioritize timely response above all else.

For most server-side data systems, real-time guarantee is not economical.
Consequently they must suffer pauses and clock instability.

To limit the pause from mark-and-sweep GC, an emerging idea is to treat GC pause like brief outage of the node and let other nodes handle requests.
If a node needs to GC soon it stops taking in requests and GCs after finishing up.
Some trading systems do this.
A variant of the idea is to only GC short lived objects (cheap to GC) and restart periodically.

### Knowledge, truth, and lies

Reasoning about distributed system can be hard as you don't know the state of other nodes for sure: the only way is to ask them via a not always reliable network.

In a distributed system we can state the assumptions we are making about the behavior (the system model) and design the actual system in such a way that it meets those assumptions.

A node cannot necessarily trust its own judgment of a situation: it may think itself alive as it can hear from other nodes, but other nodes cannot hear from it and declare it dead. (similarly, think itself a leader, a holder of a lease, etc)
Instead the truth is defined by the majority, a quorum that requires a minimum number of votes.
Most quorums require a majority number of votes as there cannot be a differing quorum at the same time.

Frequently a system requires there be only one of something. E.g. a single leader, only one node holding a lock, globally unique username.
Say a node grabs a lease to write something, then GCs itself, lease expires during GC and is granted to another node, the GC'ed node coming back may resume its write operation thinking itself still holding the lease. This is problematic and happened for HBase. 

**Fencing token** is a technique that can address this: each time the lock server grants a lease it also returns a fencing token, a number incremented by the lock service, we then require every client's write request to the storage service to include the current fencing token, and the storage service will reject old fencing tokens if it has already seen a newer one.

ZooKeeper can be used a lock service, with the transaction ID or node version as monotonically increasing candidates for fencing token.

##### Byzantine faults

This book assumes nodes are unreliable but honest.
Distributed system problems become much harder if there is a risk of nodes lying.

A system is Byzantine fault-tolerant, if it continues to operate correctly even if some of nodes are not obeying the protocol.

Flight control systems typically need to be Byzantine fault tolerant due to radiation corrupting physical hardware.
With multiple participating organization a system may need to be Byzantine fault tolerant, blockchain tries to address such.

In most server-side data systems the cost of deploying Byzantine fault tolerant solutions make them impracticable.
In a client/server architecture if we assume untrustworthy clients the servers can usually perform validation to decide what's allowed.

Most Byzantine fault tolerant algorithms require a super majority of more than two-thirds of the nodes to be functioning correctly.
In scenarios if a malicious attacker can compromise software running on other nodes then Byzantine fault tolerance won't help, and traditional mechanisms (authentication, access control, encryption, firewalls) continue to be the main protection against attackers.

Weak forms of lying / unreliability handling is pragmatic and doesn't require full-blown Byzantine fault tolerant solutions.
TCP checksums, user input sanitization, setting up redundant NTP servers are good examples.

##### System model and reality. Algorithm correctness. Safety and liveness

A **system model** is an abstraction that describes what things an algorithm may assume (formalizes the kinds of faults that we expect to happen in a system.)

Timing-assumptions-wise 3 system models are in common use: synchronous (bounded network delay, processes pauses, and clock error), partially synchronous (most of the time bounded, realistic for many systems), asynchronous (no timing assumptions, in fact we don't even have a clock and cannot use timeouts).

Node-failure-wise 3 system models are in common use: crash-stop faults (node fails in one way -- crashing, and does not come back), crash-recovery faults (crash may happen at any time, a node may also come back after some time. Nodes are presumed to have persistent storage so pre-crash state can be captured but in-memory states are lost), Byzantine faults (nodes can do anything).

For modeling real systems partially synchronous model with crash-recovery is most common.

Correctness of an algorithm under these models are described by its properties: e.g. sorting algorithm should have all elements sorted.
Fencing tokens generation should have uniqueness, monotonic sequence, and availability (node who requests a fencing token and does not crash should eventually get a token)

An algorithm is correct if in some system model it always satisfies its properties in all situations that we assume may occur in that system model.

Two kinds of properties: in the fencing token example unique and monotonic are safety, and availability is liveness.

**Safety** means nothing bad happens (if violated we can point at a particular point in time at which it's broken, and a violation cannot be undone), and **liveness** mean something good happens eventually (may not hold at some point in time, but there is always hope that it may be satisfied in the future).

Distinguishing between safety and liveness helps with dealing with difficult system models.

Theoretical, abstract system models are quite useful, even though in practice a real system can violate the assumptions of an abstract model, making empirical testing equally important.

# Summary

This chapter covers what could go wrong in a distributed system: lossy network, clock out of sync, process pause.
Alternatives that guarantee these don't happen throughout the stack do exist, but are usually costly.

Partial failure can occur is the defining characteristic of distributed systems.

If you can simply keep things on a single machine, it is generally worth doing so.
However, scalability, fault tolerance and low latency can make distributed systems desirable.

# Chap 9. Consistency and consensus

Given the problems in distributed systems introduced in chap 8, one good way of tackling them is to find general purpose abstractions with useful guarantees, implement them once, and let application run under those guarantees.

Transaction is one such guarantee that hides underlying concurrency and crashes, and provides acid to the application.

**Consensus** is another such guarantee: getting nodes to agree on something.

### Consistency models, linearizability

Recall **linearizability** (atomic consistency, strong consistency, immediate consistency, external consistency), the strongest form of consistency where we make the system appear as if there is only one copy of the data and all operations on it are atomic.

Linearizability requires the data written by a completed write call to be immediately available for all subsequent read calls.
Read after write is done must then reflect the written result, read concurrent with write can return the result before or after the write, but once result after write is returned, subsequent reads must return the result after write.

Linearizability vs serializability:
* the former is a recency guarantee on reads and writes of a register (one individual object), so it does not prevent problems like write skew.
* the latter is an isolation property of transactions where each transaction may read and write multiple objects. It guarantees that transactions behave the same as if they had executed in some serial order. It is Ok for that serial order to be different from the order in which transactions were actually run.

A database providing both serializability and linearizability is known as strict serializability or strong one-copy serializability. 2PL and actual serial execution are typically linearizable. Serializable snapshot isolation is not linearizable by design.

Linearizability is useful in the following scenarios:
* locking and leader election. In a single leader system one way to elect a leader is to use a lock. Every node that starts up tries to acquire a lock, and the one that succeeds becomes the leader. No matter how this is implemented it must be linearizable: all nodes must agree which node owns the lock otherwise the lock is useless. Coordination service like ZooKeeper use consensus algorithms to implement linearizability in a fault-tolerant way, and are often used for locking and leader election implementation.
* uniqueness guarantee. The situation is similar to a lock: when a user registers for a service (with a unique username) you can think of them acquiring a lock on their chosen username. A hard uniqueness constraint typically requires linearizability. Foreign key or attribute constraints can be implemented without requiring linearizability.
* cross-channel timing dependencies. 

##### Implementing linearizability

To implement linearizability one way is to just have one copy of the data, which is not fault tolerant.
We need replications and revisiting different replication mechanisms:
* single-leader replication is linearizable if reading from leader or synchronously updated followers.
* consensus algorithms bear a resemblance to single-leader replication. They also implement linearizable storage safely.
* multi-leader replication systems are generally not linearizable: write conflicts resolution are typically an artifact of lacking a single copy of data.
* leaderless replication systems like Dynamo claim strong consistency by requiring w + r > n. This is not quite true. LWW conflict resolution based on time-of-day clock are not linearizable as clock timestamps cannot be guaranteed to consistent with actual event timing due to clock skews. Sloppy quorum also ruins linearizability. Even with strict quorum this is not necessarily true. To make strict quorum linearizable, a reader must perform read repair synchronously before returning results to the application and a writer must read the latest state of a quorum of nodes before sending its writes.

##### The cost of linearizability, CAP theorem

When partition happens, pick one out of consistency (linearizability) or availability.
* If your application requires linearizability and some replicas are disconnected from the other replicas due to a network problem, then some replicas cannot process requests while they are disconnected: they must either wait until the network problem is fixed, or return error (become unavailable)
* If your application does not require linearizability then it can be written in a way that each replica can process requests independently, even if it is disconnected from other replicas (e.g. multi-leader). In this case the application can remain available in the face of a network problem, but its behavior is not linearizable.

All in all, there is a lot of misunderstanding about CAP and many so-called highly available systems actually don't meet CAP's idiosyncratic definition of availability.

CAP as formally defined is of very narrow scope: it only considers one consistency model (linearizability) and one kind of fault (network partition).
It doesn't say anything about network delays, dead nodes, or other tradeoffs, thus although historically influential, CAP has little practical value for designing systems.
CAP has also been superseded by more impossibility and more precise results in distributed systems.

Although linearizability is a useful guarantee, surprisingly few systems are linearizable: RAM on a modern CPU is not in the face of multi-threading race conditions (unless a memory barrier or fence is used).

Reason for this is each CPU core having its own cache, and memory access first goes to the cache, then changes are asynchronously written to memory. This creates multiple copies and with asynchronous updates, linearizability is lost.
The reason to drop linearizability in this case has nothing to do with CAP, but for performance.
The same is true for many distributed databases that don't provide linearizability: they sacrifice it for performance, not so much for fault tolerance.

Linearizability is slow.
Can it be made fast? The answer is perhaps no. Research has shown if you want linearizability, response time of reads and writes is least proportional to the uncertainty of delays in the network.
In a network with highly variable delays, the response time for linearizability is inevitably going to be high.

Weaker consistency systems, however, can be made faster.

### Ordering guarantees

The definition of linearizability (behaves as if there is only a single copy of the data and every operation takes effect atomically) implies that operations are executed in some well-defined order. 
Ordering has been an important theme, single leader addresses this; serializability is about ensuring transactions are as if they are executed in some sequential order, timestamps introduced during clock synchronization is another attempt at determining which happened first.

There is deep theoretical connection between ordering, linearizability and consensus.

##### Ordering and causality

Ordering helps preserve causality.
Consistency level consistent prefix reads is about causality.
One read/write knowing about another is another expression of causality.
Read skew is a violation of causality (the answer can be seen but not the question).
A consistent snapshot in Snapshot Isolation means consistent with causality: if it contains an answer it must contain a question.

Causality imposes on an ordering of events: cause comes before effect.

A system is **causally consistent** if it obeys the ordering imposed by causality, e.g. snapshot isolation provides causal consistency.

A **total order** allows any two elements to be compared, a causal order is not a total order. (integer space has a total order, sets don't. Sets are partially ordered, by subset / superset.)

Linearizability has a total order of operations (one copy, all atomic).

Causality in distributed systems may have two operations being concurrent: two events can be ordered if they are causally related, otherwise they are incomparable, this means causality defines a partial order.

Git history is very much like a graph of causal dependencies.

Linearizability is thus stronger than causal consistency (linearizability implies causality)
Causal consistency is the strongest possible consistency model that does not slow down due to network delays and remains available in the face of network failure.

The technique for determining which operation happened before which is similar to earlier discussion in detecting concurrent writes in a leaderless datastore.
In order to determine causal ordering, the database needs to know which version of the data was read by the application, this is why earlier discussion had the version number from the prior operation being passed back to the database on a write.
SSI conflict detection uses a similar idea: when a transaction wants to commit, the DB checks whether the version of the data that it read is still up to date, to this end, the DB keeps track of which data has been read by which transaction.

##### Sequence number ordering

If there is a logical clock that generates a unique sequence number for each operation, then the sequence numbers define a total ordering.
In particular we can create sequence numbers in a total order that is consistent with causality.

In single-leader replication, the replication log defines a total order of write operations that is consistent with causality. The leader can generate a sequence number for each write event in the log.

Without a single-leader, various methods are used in practice: each node generates its independent set (one odd, another even); sufficiently high resolution time-of-day clock can be used; preallocate a block of sequence numbers for each node.
These methods all have the problem of the sequence numbers they generate are not consistent with causality: one node being faster than another breaks the first, clock skew breaks the second, and preallocating block breaks causality in that a later block is always ranked later.

The above are inconsistent with causality but **Lamport timestamp**, a pair (counter, nodeID) is.
Uniqueness is obvious, total ordering is defined first order by counter value then node ID, and the key idea is every node keeps track of the maximum counter value it has seen so far, and includes that maximum on every request. When a node receives a request or response with the maximum value greater than its own counter value, it immediately increases its own counter to that maximum.

As long as the maximum counter value is carried along with every operation, this scheme ensures that the ordering from the Lamport timestamps is consistent with causality, because every causal dependency results in an increased timestamp.

Version vectors are different from Lamport timestamps: version vectors can distinguish whether two operations are concurrent or causally dependent, whereas Lamport timestamp always enforce a total ordering. You cannot tell two operations are concurrent or causally dependent from Lamport timestamp, but they are more compact than version vectors.

Total ordering as enforced by Lamport timestamp can be not sufficient: when enforcing unique username, you can use Lamport timestamp to decide which operation to register the same username came later and reject that, but this happens after the fact. (you have to check with every other node to find out which timestamps it has generated, and cannot guarantee if one node is unreachable. This hurts availability.)

The problem is total ordering emerges only after you have collected all the operations.
The idea of knowing when your total order is finalized is captured in the section below.

##### Total order broadcast

Total order broadcast / atomic broadcast is usually described as a protocol for exchanging messages between nodes where two safety properties need to be satisfied:
* reliable delivery: if a message is delivered to one node, it is delivered to all nodes
* totally ordered delivery: messages are delivered to every node in the same order

ZooKeeper implements total order broadcast.
Total order broadcast is exactly what one needs for DB replication: if every message represents a write to the DB, and every replica processes the same writes in the same order, then the replicas will remain consistent with each other. This principle is known as **state machine replication**.

Total order broadcast can be used to implement serializable transactions, if every message represents a deterministic transaction to be executed as a stored procedure ad if every node processes those messages in the same order then partitions and replicas of the DB are kept consistent with each other.

An important aspect of total order broadcast is that order is fixed at the time the messages are delivered: a node cannot retroactively insert a message into earlier position in the order if subsequent messages have already been delivered. This fact makes total order broadcast stronger than timestamp ordering.

Total order broadcast is also useful for implementing a lock service that provides fencing tokens, request to acquire a lock is appended as a message to the log and all messages are sequentially ordered in the order they appear in the log, and the sequence number can then serve as a fencing token as it's monotonically increasing.
ZooKeeper zxid is one such sequence number.

(Partitioned databases with a single leader per partition often maintain ordering only per partition, and they cannot consistency guarantees across partitions e.g. consistent snapshots, foreign key references. Total ordering across all partitions is possible but requires additional coordination)

##### Linearizability and total order broadcast

Linearizability is not quite the same as total order broadcast.

If you have total order broadcast, you can build linearizable storage on top of it.
Imagine we have a total order broadcast log, when writing to a key, we first append the desire to write to the log, and then when we commit the write we only do so if we see our attempt to write happens first since the last committed write happened in log. If so we commit, otherwise we abort. (a linearizable compare-and-set)

This ensures linearizable writes and a similar approach can be used to implement serializable multi-object transactions on top of a log.

This (asynchronous update) procedure does not guarantee linearizable reads, one can read a stale value. (This write linearizability provides sequential consistency / timeline consistency, slightly weaker than linearizability.)

To make reads linearizable we can do: sequencing reads through the log by appending a message reading the log, and performing actual read when the message is delivered back to you; if the log allows you to fetch the position of the last log message in a linearizable way, you can query that position wait for all entries up to that point to be delivered, then perform the read; you can make your read from a replica that is synchronously updated on writes and is thus sure to be up to date.

We can also build total order broadcast from linearizable storage.

Assume you have a lineariable register that stores an int and has an atomic increment-and-get operation, for every message you want to send through total order broadcast, you increment-and-get the linearizable integer, and then attach the value you got from the register as a sequence number to the message. Resend lost messages and let the recipients apply the messages consecutively by sequence number.

Unlike Lamport clocks, the numbers you get from incrementing the linearizable register form a sequence without gaps.
Seeing a gap in the sequence number means the recipient need to wait, and this is the key difference between total order broadcast and timestamp ordering.

If things never fail, building a linearizable increment-and-get is easy: you could keep it in a variable on one node, when dealing with failure, in general you end up with a consensus algorithm to generate linearizable sequence number generator.

It can be proved that a linearizable compare-and-set (or increment-and-get) register and total order broadcast are both equivalent to consensus, and the solution for one can be transformed into that for another.

### Consensus

Getting nodes to agree is a subtle but important problem in leader election, atomic commits, etc.

**FLP result** claims no algorithm is always able to reach consensus if risking a node crash, in the _asynchronous system model_ where a deterministic algorithm cannot use clocks or timeout (e.g. to detect crashes).

**2-Phase Commit** (2PC) is a simple form of consensus, which can be used for atomicity in multiple-object transactions, e.g. when updating a secondary index.

In a single-node write scenario, atomicity can be achieved by first making the written data durable in a write-ahead log, then write the commit record.
Crash-recovery would consider the write committed / aborted if seeing / not seeing the commit record at the end.

In case of multi-nodes being involved, e.g. term-partitioned secondary index (where the secondary index can live on a different node from where the data is), we can't just do write data - commit record sequence for each node.
(Note that most NoSQL DBs don't support distributed transactions, but clustered relational DBs do.)

Commits are irrevocable (as implied by read-committed consistency).
A node must commit when it is certain all other nodes involved in the transaction are going to commit.
This is where 2PC comes in.

Differentiate 2PC with 2PL: latter is for achieving serializable isolation, and former is for atomic commit to distributed nodes.

2PC uses an extra component: coordinator  / transaction manager.

The workflow is
* Write: Coordinator write to individual nodes
* Prepare: Coordinator asks each node to be ready for commit (after which node promises to commit without actually committing)
  * If a node replies with "yes ready to commit", there's no turning back from this decision: it must commit if later the Coordinator tells it to
* Commit: Upon hearing back from all nodes that they are ready for commit, the Coordinator tells all to commit
  * If failure to commit happens at this stage, the coordinator must retry until succeeds, there's no turning back, wait for a participant recovery if needed

Two points of no-return in the workflow: a node cannot refuse to commit later on if it has replied to Coordinator that it will commit if told, and once Coordinator makes the decision to commit the decision is irrevocable.

In case of a Coordinator crash,
* if it happens before "prepare", participant can abort.
* if it happens after participant replying yes, then the participant cannot abort unilaterally: it has to wait for Coordinator recovery. This is why the Coordinator must write its commit / abort decision to its write-ahead log so that when recovering it knows what its decision was.
In other words, commit point in 2PC comes down to a single node (Coordinator) deciding to commit or abort.

2PC is blocking atomic commit as nodes potentially have to wait for coordinator recovery.

3PC can make this process asynchronous but 3PC assumes bounded network response time and bounded node response time: in general non blocking commit requires a perfect failure detector, a reliable mechanism to detect crashes.

### Distributed transaction

2PC provides an important atomicity guarantee but cause operational problems, kills performance, and promises more than it can deliver.

Distributed transactions comes in
* Database internal distributed transaction: all nodes running the same software
* Heterogeneous distributed transaction: nodes run different software or even some running DB others running message brokers. A lot more challenging.

**Exactly once message processing** between heterogeneous systems allows systems to be integrated in powerful ways, e.g. message from a message queue can be acknowledged as processed if and only if the database transaction for processing the message was successfully committed.
This can be implemented by atomically committing the message acknowledgement and the database writes in a single transaction. With distributed transaction support this can be achieved when the two are not on the same machine.
Such a distributed transaction is only possible if all systems affected by the transaction are able to use the same atomic commit protocol.

XA is a standard for implementing 2PC across heterogeneous technologies. A series of language API bindings interacting with a Coordinator (and a Coordinator library implementation).

In case of a Coordinator crash, participants are stuck in their transaction, they cannot move on as database transaction usually take a row-level exclusive lock on any row they modify to prevent dirty writes and DB cannot release those locks until the transaction commits or aborts.

To counter potentially waiting for Coordinator forever, many XA implementation allows a participant to unilaterally decide to abort, which can break atomicity.

XA either has Coordinator being a single point of failure, or Coordinator faces the same distributed transaction problem: its distributed log becomes a database requiring replica consistency etc.

XA works across systems, and is necessarily a lowest common denominator, and cannot detect deadlocks, or implement Serializable Snapshot Isolation.

### Fault tolerant consensus

The consensus problem is normally formalized as: one or more nodes may propose values, and the consensus algorithm decides on one of those values.
It must satisfy the properties:
* uniform agreement (no two nodes decide differently; safety),
* integrity (no nodes decide twice; safety),
* validity (if a node decides some value v, then v was proposed by some node; safety),
* termination (every node that does not crash has to eventually decide some value; liveness)

If you don't care about fault tolerance, then satisfying the first three principles is easy: you can hardcode one node to be the dictator and let that node make all of the decisions. Should it fail then the system is stuck (fourth property is violated).
2PC with its Coordinator does just the above, and violated termination property.
Termination property formalizes the idea of fault tolerance.

If all nodes crash and none are running then it is not possible for an algorithm to decide anything.
It can be proved that any consensus algorithm requires at least a majority of nodes to be functioning correctly in order to assure termination.

Many consensus algorithm assume that there no Byzantine faults.
(It is possible to make consensus robust against Byzantine faults as long as fewer than one-third of the nodes are Byzantine faulty.)

Best known consensus algorithms are Viewstamped Replication, Paxos, Raft and Zab.
Most of these actually don't use the formal definition here of agreeing on one value while satisfying the properties, instaed they decide on a sequence of values, which makes them also total order broadcast algorithms.

Total order broadcast requires messages to be delivered exactly once in the same order to all nodes. This is equivalent to performing several rounds of consensus: each round nodes first propose what message they to send next and then decide on the next message to be delivered (same message, same order - agreement, no duplicate - integrity, message not corrupted - validity, messages are not lost - termination).

Viewstamped Replication, Raft, Multi-Paxos and Zab implement total order broadcast directly, Paxos implements one-value-at-a-time consensus.

##### Single leader replication and consensus

Isn't Chap 5's single leader replication essentially total order broadcast? The answer comes down to how the leader is chosen. If manually chosen, then you have a total order broadcast of the not-fault-tolerant way: termination is violated as without manual intervention progress isn't made in case of leader failure.

Automatic leader election + failover and promoting a new leader brings us closer to total order broadcast / consensus.
There's a problem however: the split brain issue in which two nodes think themselves the leader at the same time.
We then have to have all nodes agree on who the leader is to achieve consensus, and to have all nodes agree is itself a consensus problem.

All of the consensus protocols discussed so far internally use a leader in some form, but they don't guarantee the leader being unique.
Instead they make a weaker guarantee: the protocols define an epoch number (ballot number - Paxos, view number - Viewstamped Replication, term number - Raft) and guarantee within each epoch the leader is unique.

Every time current leader is thought to be dead, a vote is started to elect a new leader.
This election is given an incremented epoch number (totally ordered and monotonically increasing). If leader from the last epoch wasn't dead after all then leader with the higher epoch number prevails.

Before a leader is allowed to decide anything, it must first check there isn't another leader with a higher epoch number.
To do so it must collect votes from a quorum of nodes: for every decision a leader wants to make, it must send the proposed value to the other nodes and wait for a quorum of nodes to respond in favor of the proposal.
The quorum typically consists of a majority of nodes. A node votes in favor of a leader's proposal only if it is not aware of any other leader with a higher epoch.

Two rounds of voting: choosing a leader, then on its proposal.
Key insight is quorum for these two votes must overlap. Thus if the vote on a proposal does not reveal any higher-numbered epoch.

Difference with 2PC is that Coordinator is not previously selected, and going ahead requires votes from a majority but not every node.

##### Limitations

Consensus provides useful properties to distributed where everything is uncertain, can be used to implement linearizable atomic operations / total order broadcast in a fault tolerant way.
However they are not used everywhere because of the benefits coming at a cost.
* The process by which nodes votes on proposals before they are decided is a kind of synchronous replication.
* Consensus systems always require a majority to operate. 3 to tolerate 1 failure, 5 to tolerate 2.
* Most consensus algorithms assume a fixed set of nodes that participate in voting, meaning you can't add or remove nodes in the cluster. Dynamic membership extensions would allow the above but they are much less well understood.
* Consensus systems generally rely on timeouts to detect failed nodes and can be hard to apply in environments with highly variable network delays such as geographically distributed systems. Frequent reelection in such cases could cause the system to end up spending more time choosing a leader than doing useful work.
* Sometimes consensus algorithms are particularly sensitive to network problems. Raft has been shown to bounce leaders often if one particular network link is consistently unreliable.

### Membership and coordination services

ZooKeeper, etcd are often described as described as distributed key-value stores or coordination and configuration services.

They offer APIs looking like reading / writing value for a given key and iterating over keys.

As an application developer it is rare for one to directly interact with ZooKeeper, instead one would rely on it via other projects: HBase, Kafka, Hadoop YARN all rely on ZooKeeper running in the background.

ZooKeeper and etcd are designed to hold small amounts of data that can fit entirely in memory.
You wouldn't want to store all your application's data here, and this small amount of data is replicated across all nodes using a fault-tolerant total order broadcast algorithm. (each message broadcasted is a write to DB and applying writes in the same order provides consistency across replicas)

ZooKeeper is modeled after Google Chubby, implementing total order broadcast and other features like
* Linearizable atomic operations. A lock can be implemented using an atomic compare-and-set. The consensus protocol guarantees the operation is atomic and linearizable even upon node failure. A distributed lock is usually implemented as a lease which has an expiry so that it's eventually released in case of client failure.
* Total ordering of operations. ZooKeeper can provide fencing token as a monotonically increasing number (increases every time a lock is acquired). ZooKeeper provides this by total ordering all operations and giving each a monotonically increasing transaction ID and version number.
* Failure detection. Clients maintain a long-lived session on ZooKeeper servers, and the client and server periodically exchange heartbeats to check the other node is still alive. If heartbeats cease for a duration longer than the session timeout, ZooKeeper declares the session dead. Any locks held by a session can be configured to be automatically released when session times out.
* Change notification. Clients can read locks and values created by another and also watch for changes. E.g. it can find out when a client joins / fails via notifications.

Out of these only linearizable atomic operations requires consensus, but the rest makes ZooKeeper useful for distributed coordination.

One example where ZooKeeper / Chubby model works well is when you want a leader elected from several instances of a process or service. Useful for single-leader DB and also job scheduler, etc.

Another example is when you have partitioned resource and need to decide which partition to assign to which node and rebalance load when new nodes join / leave (can be implemented with ZooKeeper atomic operations and notifications).

Libraries like Apache Curator provide higher-level tools on top of ZooKeeper client API.

An application may grow from running on a single node to thousands of nodes.
Majority vote over this many would be inefficient so instead ZooKeeper is configured to run on a fixed number of nodes (3, 5).
Normally the kind of data managed by ZooKeeper is quite slow changing (on the timescale of minutes / hours. Tools like Apache Bookkeeper can be used for faster changing state of the application)

Another use case is service discovery (find the IP address to talk to for a service).
It is less clear whether service discovery actually requires consensus. DNS is the traditional way of looking up IP address for a service name, it uses multiple layers of caching and DNS reads are absolutely not linearizable.

Leader election does require consensus. Some consensus systems support read-only caching replicas to log the decision of consensus but not actively participate in voting, to help other nodes.

ZooKeeper etc can be seen as part of research into membership services: deciding which nodes are currently active. With unbounded network delay it's not possible to reliably detect whether another node has failed. But if you decide failure with consensus nodes can come to an agreement about which nodes should be considered alive.
With membership determined and agreed upon, choosing a leader can be simply choosing the lowest numbered among current members.

### Summary

The consistency model linearizability, where replicated data appears as though there is only one copy, and all actions act on it atomically.
Causality is a weaker consistency model where not everything has to be in a single, totally ordered timeline, version history can be a timeline with branching and merging.

With causal ordering things like no two users can claim the same username still cannot be implemented distributedly, which led to consensus.

A wide range of problem are equivalent to consensus: linearizable compare-and-set (register decides to set or abort based on comparison of given value and current value), atomic transaction commit (db deciding whether to commit or abort), total order broadcast, locks and leases (deciding which client holds it), membership / coordination, uniqueness constraint.

These are straightforward with a single leader, but if the leader fails the system stops making progress.
To handle the situation we can wait for leader to recover (2PC), manual failover, choose a new leader automatically (consensus problem).

If you find yourself wanting one of the above reducible to consensus and you want it to be fault tolerant, try tools like ZooKeeper.

Not every system requires consensus: leaderless and multi-leader replication systems typically don't use global consensus: maybe it's Ok when multiple leaders don't agree, we may be able to merge branching version histories.

# Part 3 Derived data

* Systems of records: source of truth, if there is any discrepancy between another system and the system of record, system of record's data is by definition the correct one. Written once and typically normalized.
* Derived data systems: process / transform existing data in some way. If you lose it you can always regenerate it. E.g. cache. Technically redundant but good for read performance.
Being clear about which is which helps bring clarity on a confusing system.

Whether the DB stores systems of records or derived data is up to your application and how you use such data.

# Chap 10 Batch Processing

The first two parts talk about request / response, in an online system triggered by user (**services**). Response time is important.

A different system is an offline system / **batch processing system**, taking in a large amount of input data, runs a job to process it, and may take a while. Usually scheduled periodically. Throughput is important.

**Stream processing system** is something in between, a near-real-time processing system. Operates on events shortly after they happen.

MapReduce is a batch processing algorithm, and was subsequently implemented in Hadoop, CouchDB, MongoDB.

Batch processing is a very old form of computing, and MapReduce bears an uncanny resemblance to the electromechanical IBM card-sorting machines.

### Batch processing with Unix tools

`awk '{print $x}'`, `uniq -c`, `sort -r -n`, `sed`, `grep`, `xargs`, etc. Powerful and performs well, worth learning.

Linux `sort` automatically handles larger than memory workload by spilling to disk, and automatically parallelizes sorting across multiple CPU cores.
A chain of Unix commands easily scales to a large dataset without running out of memory.

Note that mergesort has sequential access patterns that perform well on disks (remember that optimizing for sequential IO was a recurring theme earlier.)

Unix pipes reflects well the design philosophy of Unix:
* make each program do one thing well (to do a new job, build afresh rather than complicating the old with features),
* expect the output of every program to be the input to another, yet unknown, program (don't clutter output with extraneous information, don't insist on interactive input, avoid stringently columnar or binary formats),
* design and build to be tried early, ideally within weeks, don't hesitate to throw away the clumsy part and rebuild,
* use tools in preference to unskilled help to lighten a programming task, even if you have to detour to build the tools and expect to throw some of them out after using them

(automation, rapid prototyping, incremental iteration, friendly to experimentation, breaking down large projects into manageable chunks; very much like Agile of today's.)

Many Unix programs can be joined together in flexible ways, Unix expects output of one to be input of another i.e. exposing a uniform interface, to achieve this composability.
On Unix that interface is a file descriptor, can be a device driver, network socket, communication channel to another program (unix sockets, stdin, etc), having all these share an interface is quite remarkable.
By convention, many of Unix program will treat this sequence of bytes from a file descriptor as ascii.

Another characteristic of Unix tools is their use of stdin and stdout: a program can read / write files if it needs to, but the Unix approach works best if the tool doesn't worry about file paths and instead interact with stdin / stdout, so that logic and writing are separated: the program doesn't care about where the input is coming and where the output goes to. (somewhat similar to the ideas loose coupling, late binding, and inversion of control)
stdin and stdout have limitations, e.g. program with multiple inputs / outputs will be tricky, and you can't pipe your output to a network connection, etc.

Part of what makes Unix tools so successful is they make it easy to see what's going on: input is immutable, output can be piped to less for inspection, you can write the output of pipeline stages to a file and use that as the starting point for the next stage, allowing you to restart the later stage without rerunning the entire pipeline.

The biggest limitation of Unix tools is they only run on a single machine, and this is where the likes of Hadoop come in.

### MapReduce

Similar to Unix tools but distributed, MapReduce usually does not modify input, and produces output as sequential file writes (no random access and modification).

Reads and writes from distributed file system like Hadoop's HDFS, based on its proprietary counterparty GFS.
HDFS also follows a shared-nothing architecture, and a NameNode is in charge of keeping track of directory and block mapping, like the single master in GFS.

HDFS scaled well to over tens of thousands machines storing over PBs of data.

##### Workflow

* Input is split into records. For each record, a Mapper handles each record independently, and can generate any number of (key, value) pairs from the record, e.g. extracting some field from a record.
* The MapReduce framework takes key value pairs generated by the Mapper, collects all values belonging to the same key, and calls the Reducer with an iterator over that collection of values, the Reducer then produce output records.

Each input to a Mapper is typically hundreds of MBs in size, the scheduler tries to run each mapper on a machine that stores a replica of the input file, provided that the machine has enough spare RAM and CPU (putting the computation near the data, less copying over the network and better locality).
The number of Mappers is typically decided by the number of input blocks, and the number of reducers is typically user specified.

To ensure same keys end up on the same Reducer, the framework maps a hash of the key to the reducer task.

The key value pairs must be sorted, since the dataset is likely too large to be sorted in memory, instead each Map partitions its output by Reducer based on the hash of key, then each of these partitions is written to a local sorted file using SSTable / LSM trees.

Whenever a Mapper finishes writing its sorted output files, the scheduler notifies the Reducer it can start fetching output sorted file from the Mapper.

The process of partitioning by reducer, sorting and copying data from mappers to reducers is known as **shuffle**.

Reducer takes files from mappers and merge them together preserving the sorted order (even if different Mappers produced the same keys for this Reducer)

Reducer is called with a key and an iterator that sequentially scans over records with the same key, uses arbitrary logic to process these records and generate any number of output records which are written to a file on a distributed file system.

It is very common for MapReduce jobs to be chained into workflows. Hadoop MapReduce does not have particular support for workflows (chaining done implicitly e.g. by different directory names used by different MapReduce jobs).
This is slightly different from Unix pipes where output of one is immediately fed as input to the next over a small memory buffer. This materialization of intermediate state has pros and cons.

MapReduce discards partial output of a failed job, and dependent jobs can only start when their dependencies finish successfully.
A number of workflow schedulers are introduced for Hadoop to handle this dependency diagram.
These need good tooling support to manage complex dependency diagrams / dataflows.

##### Reduce-side joins and grouping

It's common for a data record to have an association with another: via foreign key in a relational model, document reference in a document model, or an edge in a graph model.
Join happens when you need both sides of the reference.
Denormalization can reduce the need for joins.

In a DB with index joins typically involve multiple index-based lookups.
MapReduce jobs has no indexes conventionally, it scans the entirety of input files.

Joins in the context of batch processing means resolving all occurrences of some association within a dataset.

An example problem for a MapReduce job is given user click-stream records identified by an user ID, associate each record with user details stored in a remote users DB (think denormalization, stars schema is also related).
A common approach is for the job to take a copy of the user database and put it on the same distributed file system the map job is running on.

One set of mappers would go through partitioned click-stream records, extracting the user ID as key and relevant info, another set of mappers would go through the user DB also extracting the user ID as key and other relevant info.
The MapReduce framework would then partition mapper output by key in sorted order, and user record with the same ID ends up on the same reducer adjacent to each other, and the reducer can perform join logic easily. (Secondary sort can even make these key value pairs appear in a certain order.)
This algorithm is known as a **sort-merge-join**.

The key emitted by mapper in this case is almost like an address designating which reducer this goes to.

MapReduce framework separates the physical communication and failure handling aspect from application logic.

Besides join, another common pattern is group-by.
The simplest way to set this up is to let mappers use grouping key as key. Grouping and joining look similar in MapReduce.

The pattern of bringing all records with the same key to the same place breaks down if very large amount of data are associated with the same key (linchpin objects, hot keys), and the process of collecting all data related with that key leads to skew / hot spot.

If join input has hot keys, Pig's skewed join algorithm first runs a sampling to job to decide which keys are hot, and mapper sends a record associated with a hot key to a randomly chosen reducer instead of a deterministic one (records relating to the hot key would also be replicated over all reducers).
And you can add another MapReduce job to aggregate the (more compact) results that the randomly chosen reducers produced.

Hive takes a different approach to handling hot keys, it requires hot keys to specified explicitly in table schema, and it uses a map-side join for that key when joining.

##### Map-side joins

The above performs join logic in the reducers, and mappers prepare the input data.
This has the advantage of not needing to make any assumptions about the input data's properties / structure.
Downside is sorting, copying to reducers and merging reducer results can be expensive.

You can perform faster map-side joins if you know certain things about the data.
No reducers would be needed.

The simplest way, **broadcast-hash-join** is when a large dataset is joined with a dataset small enough to be loaded entirely into memory of each mapper.
The mapper can then load the join-dataset into memory, and as it goes through its chunk of records it can perform join and produce joined output.
Pig, Hive, Impala, etc all support this.

If the inputs to map-side joins are partitioned in the same way as user DB is, then hash join can be applied to each partition independently.
Each mapper only needs to load the partition of DB that would contain records relevant to its input records. This is a **partitioned map join**.

If input is not only partitioned the same way but also sorted the same way, then a **map-side merge join** is possible, and user DB partition does not have to fit entirely into mapper's memory.

In the Hadoop ecosystem, this kind of metadata about the partitioning of datasets is often maintained in HCatalog and the Hive metastore.

Note map-side join outputs chunks of output files sorted in the same order as the chunks of input, and reduce-side join is chunks of records partitioned and sorted by the join key.

##### Output of batch workflows

Recall transaction processing workload with analytics workload have different characteristics.

Batch processing fits more in analytics, but not quite analytics.

MapReduce was originally introduced to build indexes for Google's search engine (and remains a good way to build indexes for Lucene / Solr), Google has moved away from this.

Building document-partitioned indexes (and building classifier systems and recommendation systems) parallelizes very well. Since querying a search index is a read-only operation, these index files are immutable once created unless source documents change.

The output of such jobs is usually some database where a web interface would query and separate from the Hadoop infrastructure.

Writing directly to the DB from your Mapper or Reducer job may be a bad idea since
* Making a network request per record is not performant, even if the client library supports batching.
* MapReduce jobs often run in parallel and all mappers and reducers writing to the same DB concurrently may overwhelm it.
* Finally MapReduce provides a clean all-or-nothing guarantee for job output, however, writing to an external system from inside a job produces visible side-effect, and you have to worry about results of partial execution being available to the rest of the system.

MapReduce jobs output handling follows Unix philosophy by treating input as immutable and avoiding side effects such as writing to a DB.
* This minimizes irreversibility, makes rollback easier (only code rollback is needed, not database, too).
* Makes retrying a partial failed job easier.
* The same set of files can be used as input for various jobs.
* And like Unix tools, this separate logic from wiring.

### Comparing Hadoop to Distributed Databases

Hadoop is like a distributed version of Unix where HDFS is the file system and MapReduce a (sorted-shuffling) distributed implementation of Unix process.

The ideas of MapReduce has been present in so-called massively parallel processing (MPP) databases for a while.
But MapReduce + distributed file system provides something much more like a general purpose OS.

##### Diversity of storage

Some difference between MapReduce and MPP DBs include DBs require you to structure data according to a particular model, Hadoop opens up possibility of indiscriminately dumping data into HDFS and only later figure out how to process it further.

The idea is similar to a **data warehouse**: simply bringing data from various parts of a large organization together in one place is valuable, and careful schema design slows this process down.
This shifts the burden of interpretting data to the consumer / schema-on-read. (the sushi principle: raw data is better)

##### Diversity of processing models

MPP databases are monolithic, with query planning, scheduling and execution optimized for specific needs of the DB (e.g. supporting SQL queries).
If you are building recommendation systems and full text search indexes then merely SQL is usually not enough.
MapReduce provided the ability to easily run custom code over large dataset. You can build SQL with MapReduce (Hive did this), and much more.

Subsequently people found MapReduce performed too badly for some types of processing, so various other processing models have been developed over Hadoop.

The Hadoop ecosystem includes both random-access OLTP databases such as HBase (open source BigTable with SSTable and LSM trees), as well as MPP-style analytic DBs like Impala. Neither uses MapReduce, but both use HDFS.

##### Designing for frequent faults

MPP databases usually let user resubmit the entire query upon failure (this is fine for typically seconds / minutes long analytic jobs), while MapReduce allows retrying at the granularity of a job.
MPP DBs also prefer keeping as much data in memory as possible, while MapReduce is more eager to write to disk.

This is related with Google's environment of mixed-use datacenters in which online production services and offline batch jobs run on the same machines and every task has a resource allocation enforced using containers.
This architecture allows non-production / low-priority jobs to overclaim resources to improve the utilization of machines. As MapReduce runs at low priority they are prone to being preempted by higher priority processes needing their resource, relying less on in-memory states and more on writing to disk is preferable.
Current open source schedulers uses preemption less.

### Beyond MapReduce

MapReduce is only one programming model for distributed systems.
MapReduce provides a simple abstraction over a distributed filesystem, but is laborious to use.
Pig, Hive, Cascading, Crunch are abstractions over MapReduce to address the hard-to-useness.

MapReduce is general and robust, but other tools can be magnitudes faster for certain kinds of processing.

##### Materialization of intermediate state

MapReduce chain jobs by writing the first to file and having the second pick up from where the file is written.

This makes sense if the result of first should be made widely available and reused as input to different jobs, but not when output of one job is only ever used as input to one other job (intermediate state).
This materialization of internal state differs from piping Unix commands. This has following downsides:
* A MapReduce job can start when all tasks in the preceding jobs have completed, whereas processes connected by a Unix pipe can start at the same time, with output consumed as soon as it's produced. This having to wait contributes to having more stragglers in execution and slows down the pipeline.
* Mappers are often redundant: they read back the same file that was just written by a reducer and prepare it for the next stage of partitioning and sorting: if the reducer output was partitioned and sorted in the same way as mapper output then reducers could be chained together directly without interleaving with mapper stages.
* Storing intermediate state in a distributed file system (who replicates them) is often overkill for temporary data.

##### Dataflow engines

To address the above issues, several execution engines for distributed batch computations were developed, including Spark, Tez and Flink.

They handle an entire workflow as one job, rather than breaking it up into independent subjobs.

They explicitly model the flow of data through several processing stages, hence they are known as dataflow engines.
Like MapReduce, they work by repeatedly calling a user-defined function to process one record at a time on a single thread.
They parallelize work by partitioning inputs, and they copy the output of one function over the network to become the input to another function.

Unlike MapReduce these functions (operators) don't have the strict roles of Map and Reduce, but instead can be assembled in more flexible ways.
* One option is to repartition and sort records by key, like in shuffle stage of MapReduce. This enables sort-merge-join and grouping as MapReduce would.
* Another possibility is to take several inputs and to partition them in the same way but skip the sorting. This saves effort on partitioned hash joins where partitioning is important but the order is not as hash randomizes it anyway
* For broadcast hash joins the same output from one operator can be sent to all partitions of the join operator.

This processing engine style offers advantages over MapReduce:
* Expensive work like sorting only need to be performed where it is required.
* No unnecessary Map tasks.
* Because all joins and data dependencies in a workflow are explicitly declared, the scheduler has an overview of what data is required where, so it can make locality optimizations.
* It is usually sufficient for intermediate state between operators to be kept in memory or written to a local disk, which requires less IO than writing to HDFS. MapReduce uses this optimization for Mapper output but dataflow engines generalize it to all intermediate state.
* Operators can start as soon as input is ready, no need to wait for for the entire preceding stage to finish before the next one starts.
* Existing JVM processes can be reused to run new operators, reducing startup overheads compared to MapReduce which launches a new JVM for each task.

They can implement the same thing as MapReduce and usually significantly faster.
Workflows implemented in Pig, Hive or Cascading can switch from MapReduce to Tez or Spark with simple configuration changes.

Tez is a fairly thin library relying on YARN shuffle service for copying data between nodes whereas Spark and Flink are big frameworks with their own network communication layer, scheduler and user-facing API.


Fault-tolerance is easy in MapReduce due to full materialization of intermediate states.
The dataflow engines avoid writing intermediate states to HDFS, so they retry from an earlier latest stage (possibly from original input on HDFS) where intermediate data is still available.

To enable this recomputation, the framework must keep track of how a given piece of data is computed: inputs and operators applied to it.
Spark uses **Resilient Distributed Dataset** (RDD) to keep track of ancestry of data, while Flink checkpoints operator state. 

When recomputing data it's important to know whether computation is deterministic. (Partial result was computed and delivered to downstream, then upstream fails and if the delivered results can vary between retries then the downstream needs to be killed and restarted as well).

It's better to deterministic but nondeterministic behavior creeps in via unordered container iteration, probablistic algorithms, system clock usage, etc.


Returning to the Unix analogy, MapReduce is like writing the result of each step to a temporary file, dataflow engines look more like Unix pipes.

A sorting operation inevitably needs to consume the entire input before producing any output, any operator that requires sorting will thus need to accumulate state.

In terms of job output storage, like MapReduce, HDFS is still usually the destination instead of building DB writes into user defined functions.

##### Graph and iterative processing

Think graphs in analytics workload. E.g. PageRank.



(_is no dirty writes an atomicity, consistency (linearizability) and isolation guarantee?_)
(_is linearizability the strongest form for replica consistency? The furthest in line in eventual, read-your-write, consistency-prefix-read, causal?_)
(_does linearizability imply atomicity? how does it not imply serializable isolation? the weakest form of isolation, read committed, also implies no dirty writes. Is atomicity any more than no dirty reads, no dirty writes / read committed?_)
(_are distributed transactions and replica consistency connected problems?_)
(_does full write broadcast and r+w>total_nodes achieve linearizability?_)
(_does 2PC give you anything on top of synchronous single-leader replication?_)