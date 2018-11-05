# Consistent Hashing

### Property

Given K number of keys and n number of slots, a change in the number of slots (hashtable resize) will cause on average K/n keys to be relocated, instead of in traditional hashing where almost all keys needs to be relocated.

### Usage

To reduce impact of partial system failures in large applications.

DHT uses consistent hashing to partition a keyspace.

### Technique

Map hashed values on a hash ring, instead of a linear space: each hash(key) corresponds with some values (angles) in the hash ring. Each server also has its hash(ID) mapped to some angles in the hash ring, and algorithm dictates that every key should fall into the first server in the ring that comes after it in a clockwise / counter-clockwise fashion.

In code, when looking for which server holds a particular key, could do a binary search on servers ordered by hash(ID).

To distribute keys evenly, each host corresponds with multiple tags (say, ID + number), and a hash(key) is hosted by the server that has the tag whose hash(tag) sits cloest to hash(key) in a clockwise / counter-clockwise fashion. How many tags a server corresponds to can be arbitrary, thus accounting for difference in server processing power.

With this, if a server fails, only keys that need to be relocated are those that were previously hosted by this server.

### Finding which node holds a key

Say in a distributed consistent hashing table a get request comes, and to serve this request it's possible to
* have a set of super nodes who know all the storage nodes in the network, and can find keys with only one hop (such lookup / category super nodes may become bottlenecks and the system is not fully peer-to-peer)
* set up a DNS-like hierarchical structure and have a series of directory nodes forward the request in a tree-like structure (is there a point for doing this?)
* do **fully distributed** lookups such as DHT Chord

[Chord](https://pdos.csail.mit.edu/papers/chord:sigcomm01/chord_sigcomm.pdf) proposes a mechanism in which, given 2^m discrete positions (possible keys) in the ring, a node at position k in the hash ring keeps a pointer (finger table) to which node holds position (k + 2^0, k + 2^1, ...k + 2^(m - 1)).
With the finger table, each node needs to know logN other nodes in adding or removing nodes, and any node can serve a key lookup request, with at most logN steps before finding the node that holds the key.
The way it works is, say, node i receives the request and if it doesn't have the key, it forwards the request to the node on its finger table who holds the last position before the given key.

In [Dynamo](https://www.allthingsdistributed.com/files/amazon-dynamo-sosp2007.pdf), the execution of get operation can be handled by a load balancer who then forwards the request to a random node. The random node has global knowledge of the preference list for all keys, and correctly forwards the request to the first node in the preference list that contains the data.
Alternatively the client could use a partition-aware client library that (has a global view of \<key, preference\_list\> and) routes directly to the first node on preference list (saving one extra hop).

(_Inferred from sec4.5, verify this "global knowledge": if this were true, updating global view either in the client library (through a directory service) or on each node could be non-trivial and could affect the consistency and availability of the system: in that while a node is being added or taken down (or other situations where the global view requires an update to be propagated), a get request may be routed to one who no longer has the information although such information is still present in the system, and the propagation itself (which could happen on each put) is expensive in terms of the number of nodes involved_)

### Trivia

Authors of the 1997 consistent hashing paper went on to found Akamai and build big CDNs with consistent hashing.

### Further reading

https://www.akamai.com/es/es/multimedia/documents/technical-publication/consistent-hashing-and-random-trees-distributed-caching-protocols-for-relieving-hot-spots-on-the-world-wide-web-technical-publication.pdf
