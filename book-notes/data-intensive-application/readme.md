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

* Graph-like data models
