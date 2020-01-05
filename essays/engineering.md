# Recurring themes in software engineering

This year marks the end of a decade since I wrote my first line of code, and two years and a half since I became a professional software engineer.

Along the process some recurring themes are discovered, lessons learnt, realizations made, and this summary intends to be a discourse on these observations on a meta-engineering level (i.e. a collection of commonalities and general methodology).

The hope is that when faced with an engineering problem, one can see the alternatives easier, make better judgment calls, and think architecturally when it is needed.

### Tradeoffs

There is no magic.
It is all tradeoffs.

It is so between time-space in an algorithm, bias-variance in statistics, and noise-reduction vs details-sharpening when post processing a photo.

As is with most choices: a small company vs a big one, moving fast vs architecting something well, having an abstraction that covers all vs individual solutions tailored to each, even being compensated more or less (to a degree).

Anything reasonably sophisticated would likely involve making non-trivial tradeoffs, and arguably this decision-making element i.e. design / strategizing, is where the most dynamics and excitement lie.

Know the approach, identify its tradeoffs, map them to first principles, and be able to draw upon them when designing your own systems.

Prioritization is another form of tradeoff.
A practical system cannot be perfect in architecture.

Looking at MapReduce in 2019, perhaps few would think a fixed framework of mappers and reducers with a sorted shuffle step forced in between and materialization of transient states is anywhere near general enough for distributed processing workflow.

Yet it worked well and was general enough (at least for a time), for some particular workload (i.e. building search indexes).

Know your use case and its bottlenecks, and make tradeoffs from there.

### Simplicity

"A complex system that works is invariably found to have evolved from a simple system that works. The inverse proposition also appears to be true: a complex system designed from scratch never works and cannot be made to work".

Simplicity comes from knowing what things our system needs to do and not being over-ambitious: do one thing and do it well (single responsibility), plan for some changes but not all of them.

Unix programs / principle are good embodiments of such, as is a RISC instruction set.
GFS uses one in-memory master for directory storage, and failover is tackled by a different component.
Simple to reason about and simple to operate.

Why make something more complicated than it needs to be?
There may be a valid reason for that: to account for some possibility of future changes.

Yet such concerns often times are misguided into premature optimization:
* We thought we need multi-threading because we didn't understand concurrency well enough.
* We thought we need a high-availability distributed NoSQL database because that's what everyone talks about. 
* We thought we need blockchain because we never understood our problem well enough.

That said, there is a fine line here, which a later section will cover.

How can a system be simple if the business problem it models itself is complex to start with?

Well, we break it down into pipelines / compositions of components, and cheat by hiding the details of each behind abstractions.

### Abstraction, generalization and specification

Abstractions make complicated things seem simple.

It's like coding in assembly, C++ or Python, or having TCP on top of IP.

Abstractions take freedom away from the user: you are tied to the particular ways the abstraction approaches a problem, and as a result performance is often sacrificed: specific optimizations for some user's particular cases cannot be made.

Performance is overrated.

Well, shame on me for saying that as a person paid to build high-frequency trading platforms.
But this does seem the prime reason leading to premature optimization, i.e. the likes of "I thought this would run faster in hypothetical scenarios ABC".

Make data-driven decisions, and know that for most of our cases, something should be as fast they as they need to be, anything faster than that might be making sacrifices elsewhere (maintainability, operability, evolvability, cognitive overhead, etc), think twice if those are worthy sacrifices.

Yet often enough, they are worthy sacrifices. And specification comes in.

As the problem evolves, two uses cases thought to share the same logic start to diverge: the abstraction becomes over ambitious (a square is not a rectangle), or slow.

Holes are punched, encapsulation violated, patches that hurt maintainability goes in, and it becomes about time for specifications to be introduced, and abstractions refactored.

These are parts of the natural life cycle of engineering, and do not necessarily indicate a bad design / architecture, yet it is at times like these our barriers are best tested.

### Dependencies and coupling

Interface segregation. Dependency inversion.

### Plan for change

Problems evolve. Software evolves.

* When choosing how to store your data, did you consider how easy it'd be for the schema to evolve?
* When deploying your system, did you consider how easy it'd be to upgrade?
* When modularizing your code, did you consider what are the likely ways for it to evolve, and how well each are insulated from changes in the other on the likely paths of evolution? Are the ones likely to change at the same time for the same reason near each other?

Are the above considerations even important to your problem?

Do not consciously leave traps for the future person, nor should you prepare for armageddon in your code.

What's the criteria? Think hard and experience will tell.

### What experience brings

When someone amazes us with their ability to think outside the box and to come up with innovative solutions, it often times is their box being unfamiliar to us.

"Before we learn to think outside a box, learn to think inside one."

In some sense, experience may seem like the ability to simulate the situation accurately without needing to have actually experimented, or gauging the counterfactual.

Our repertoire of knowledge and toolkit is that box.

Knowledge comes in many levels.
* Knowing certain API and what happens underneath the hood give you an edge
* Knowing how to troubleshoot an in-house system gives you more
* Knowing their whys and why nots is even better, and
* Being able to create new ones, combining first principle and tried-and-trues of others makes you an architect.

There will always be the young, ambitious and eager person with just too much time on your team who will out-do you in the first few levels of knowing.

Yet the last ones come with experience: the tradeoffs are firmly implanted through your training that you will deduce, sometimes without having to experiment, what will work and what will not.

Think hard at the boxes we've built and at the ones we are given, dissect them, take them apart and rebuild them in different ways, and from such, gain experience.

### Devils in the detail

With experience we make empirical claims.

Know that any such can be dangerous, especially in that they often times come with assumptions we didn't fully realize, i.e. the details.

When not sure about something, experiment.
When sure about something, it does not hurt to also experiment.

Do the mental exercise, but always be prepared to get your hands dirty and write the code.

Undeterminism.

### One level of indirection

Lixia, a prominent researcher in Internet architecture and my advisor, once brought up the idea that one level of indirection is a powerful tool in engineering.

Over time, we saw many falling into this pattern.

* DNS offers a level of indirection for addressing, such that application level need not hard code IP addresses, which would be bound to a specific physical interface.
* Normalization / denormalization in database schema design is a level of indirection: when storing enumerables (e.g. neighborhood in someone's profile), do we store them as plain text in the user table, or an ID that can be cross referenced with a dedicated mapping to plain text.
* A message broker is a level of indirection: to handle producers coming and going, have them both talk to the broker as opposed to directly to each other.
* A pointer / reference is one level of indirection: with these the underlying object can now be mutated in-place, or shared by many.

There is, however, no universal answer to the question "should we or should we not apply a level of indirection".

Common sacrifices we make include efficiency due to needing to follow an extra link, for gains such as decoupling different layers, unifying different representations, and not needing to have changes propagate through the codebase.

Things to consider include if this should be modeled as a multi-layer situation, if there is one source of truth that everything else adheres to, etc.

(Anecdotally, another thing she brought up as an important tool in our engineering kit is randomization, example being TCP's random initial sequence number for crash recovery. I'm sure venturing more unto the undeterministic will yield more realizations for such.)

### Prototype and iteration

MVPs and convincing.

### Beyond coding

Testability.
Maintenance.
Operational.
Documentation.

### Routine and habits





