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

# Storage and retrieval

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

### Other indexes
