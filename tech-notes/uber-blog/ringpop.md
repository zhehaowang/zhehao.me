# Ringpop

### Membership management: SWIM gossip protocol

* Bootstrap

Each node is configured with a list of seed addresses, e.g. [172.0.16.2:3001, 172.0.16.14:3001].
When a node starts up, it tries to talk to a few seeds. If none is up, it thinks of itself as the only node alive.
If some are up, those would learn about the existence of this new node.

* Membership management

Nodes maintain a list of nodes known to itself in a binary search tree (presumably, \<consistent\_hash, address\>, such that key lookups are logN).
Nodes (randomly) pick those known to itself and heartbeats periodically (presumably, of its own status, its membership list and pinging status).
When a node joins, seeds who are aware of it will carry its information to those exchanging with the seeds, making the new node known.
Nodes detect failure (presumably not a binary concept as in Cassandra) using heartbeats and status it heard (transitively) from others.

Gossip communication is on top of Uber's transport layer protocol TChannel.

### Load distribution

* Consistent hashing
* Process or forward (one-hop at most)

### CAP

### References

* https://eng.uber.com/intro-to-ringpop/
* https://ringpop.readthedocs.io/en/latest/architecture_design.html


https://en.wikipedia.org/wiki/Riak
duplication of work?
Actor-like system

Route flap damping:
https://tools.ietf.org/html/rfc2439
Flap describes the fluctuating state of a route being up and down.
Damping describes mechanisms to mitigate this undesirable situation.

Geohashing

