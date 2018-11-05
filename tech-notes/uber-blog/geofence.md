# Geofence

Uber's highest QPS service.

At Uber, a **geofence** refers to a human-defined geographic area (or polygon in geometry terms) on the Earth’s surface.
Geofence is important for determining dynamic pricing, identifying key locations (such as an airport)

Basically a high-throughput, low-latency point-in-polygon () algorithm.

### Algorithm

Two-level O(n) lookup: cities, geofences within a city. (We are looking at approximately 100 + 100), as opposed to (100 * 100) total number of geofences in the world.

### Architecture

Stateless, each node has knowledge of geofences in the entire world.
For that they do a periodic poll.
Now that readers read from geofence index and polling writers write to it, they use a read-write lock as opposed to Go's channels.

### Why Go

* Coroutines, built-in support for multi-core concurrency
* CPU intensive (the claim is that Node works well in an IO intensive workload, while Go performs better in a CPU intensive workload)
* High-throughput low-latency