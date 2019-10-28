October 28, 2019
Trump Plaza, Jersey City

A recent conversation drew me to realize the surprising similarity between the way our memory works and that of a log structured merge tree.

In my pseudo-scientific language (idea credit Sophia),
* our short-term memory would be like a memtable
* long-term memory would be like an SSTable
* sleep would be the process of committing a memtable to an on-disk SSTable, it would also be the process where compaction happens
* our memory is keyed on some vague and flexible index, and some operations seem incredibly faulty in that indexes clash and the values become intertwined in unpredictable ways
* the query process would be similar in that memtable is queried first, and the latest SSTable, the next latest, etc
* memories, like memtables and SSTables, are append-only in that we cannot unsee something already seen. They are also immutable (in regular flows not including the random faults of the mind) once committed
* we may even also keep a Bloom-filter like structure as an optimization to the query process, to be able to quickly answer something with "that does not ring a bell at all"
* the notes we keep are like logs in such a system: to recover from e.g. a crash before commit from memtable, we'd need additional information like our physical notes
* other people's memory of the same event can also serve as backup mechanism / redundancy

* there definitely is also a layer of LRU cache that is associated to our short-term memory, too. Like, keeping hot content at the tip of tongue. what's a good way of formalizing this?

Takeaways,
* Sleep is important
* Consciously schedule / improve / review may help the compaction process
* We can probably create some interactive experience to model this process! (and bring in the idea of reference counting where something is forever lost in our memory / history when nothing remembers it.)
