### The mythical man month

Why is programming fun?
* Joy of making things
* Making things useful for other people
* The fascination of fashioning complex puzzle-like objects of interlocking moving parts and watching them work in subtle cycles
* Joy of always learning sprung from the non-repeating nature of the task
* The delight of working in such a tractable medium
* It’s fun because it gratifies creative longings built deep within us and delights sensibilities we have in common with all men.

The woes of programming
* One must perform perfectly
* Other people set one’s objectives, etc
* Dependence upon others
* Finding nitty little bugs is just work
* Work over which one has labored can be obsolete upon / before completion

Men-month as a metric: men and months are interchangeable commodities only when a task can be partitioned among many workers with no communication among them (e.g. reaping wheat or picking cotton), which is apparently not true for system programming.

Author’s usual estimation for software projects: ⅓ planning, ⅙ testing, ¼ unit testing and early integration test, ¼ integration and system test

Adding manpower to a late software project makes it later (Brooks’s law)

### Team division
Best performers and worst performers may have a 10:1 productive ratio.
Many would prefer a small team of elites. Though the cruel fact is for a system large enough, albeit more efficient, it would probably still take too long for the small team of elites, making the product obsolete by the time it’s complete.

One effective way, could be to organize the software team as a surgical team. (Mill’s concept)
The surgeon / chief programmer defines the specs, designs the system, and is able to do any part of the work. Needs great talent and experience.
The copilot is the alter ego of the surgeon, table to do any part of the job, but less experienced. His main function is to share in the design as a thinker, discussant, and evaluator. He serves as insurance of disaster to the surgeon.
The administrator, surgeon is in charge, but his effort should not be wasted on money, personnel, space, etc. The administrator takes care of that.
The editor, edits the documentation the surgeon writes.
An administrator secretary and an editor secretary.
The program clerk: maintaining technical records.
The toolsmith.
The tester.
The language lawyer. (master of the programming language in question)
The whole system is the product (design) of one mind, or at most two, acting uno animo. (As opposed to the conventional team, where each part of the system is the design and implementation of one person)
Specialization of function and a lack of the division of the problem (and a superior-subordinate relationship) are key in this concept.
In a larger team, the conceptual integrity of each piece should be maintained. (Meanwhile, a sharp distinction need to be made between architecture design and implementation)

### Aristocracy, democracy and system design
Conceptual integrity is the most important consideration in system design. (as determining factor of a system’s ease-to-useness)

Designing (or architecturing), especially setting the external specifications (the manual of the product) is not more creative than implementation (the design of implementation). The former process is an aristocracy which requires no apology.
Finishing architecturing does not necessarily block implementation design efforts.

### The second-system effect
What happens when architect proposes something more than the implementation can achieve: resolved by two parties’ communication.

An architect’s first work is apt to be spare and clean. He knows he doesn’t know what he’s doing, so he does it carefully and with great restraint.

Once done building the first system, this second system is the most dangerous a man ever designs: the general tendency is to overdesign the second system, using all the ideas and frills that the were cautiously sidetracked on the first one, and extra caution is usually required when designing the second system.

### Passing the word
The manual describes what the user sees / does, it should refrain from describing implementation details, whose freedom should be left to the implementers.

Pros and cons of formal definition (or implementation as definition) / prose definition in the manual; 

Having effective weekly architecture meetings, annual supreme courts for remaining minor decisions;

Telephone log Q&A of the architect.

### Why did the Tower of Babel fail
Communication is key. 
Communication happening informally, via meetings, and via a workbook.
Workbook keeps the external specs, the internal implementation design and documentations, etc.
Tree-like organization in large software systems. (Possible relationship between the producer and the technical director)
Thinkers are rare, doers are rarer, and thinker-doers are the rarest.

### Calling the shot
Effort required is roughly proportional to (size of program)^(1.5)

### Ten pounds in a five-pound sack
Size control: budget memory (resident) space, as well as hard-drive usage; define exactly what a module should do as you place limit on its size; the system architects must maintain continual vigilance to ensure continued system integrity. (Programming manager: total-system, user-oriented)
Representation of data is key to managing space-time tradeoffs (the essence to programming)

### The documentary hypothesis
Required documents: 
What: objectives, specification (first to come, last to finish)
When
How much
Where
Who

### Plan to throw one away
Prototype. Be prepared to redesign and reimplement.
Plan ahead to build a throwaway, you will, anyhow.
Design and be prepared for change.
Structure an organization for change.
Program maintenance is an entropy-increasing process. Hence at some point it’ll be a one step forward and one step back process.

### Sharp tools
Toolmaker of each surgical team (tools, documentation system, performance simulator)
Gradual adoption of high-level languages and interactive programming

### The Whole and the Parts
General paradigms to avoid bugs:
Testing the specification (externally)
Top-down design
Structured programming (avoid go-tos)

### Hatching a Catastrophe
Milestones need to be concrete
Fuzzy milestones boosts chronic schedule slippage, which is a morale killer and leads to catastrophe.
One characteristic of programming teams, “hustle”, running faster than necessary, moving sooner than necessary, trying harder than necessary. The other piece is late anyway is no excuse.
PERT chart (critical path scheduling): identifying scheduling dependencies
Plans and Controls group on a large software project: watchdog of the schedule
Boss (those higher than the first line managers) should encourage truthful status reports, and try to refrain from acting on them unless necessary.

### The Other Face
What documentation is required?
Overview: purpose, environment, domain and range (input and output), functions realized and algorithms (what it does), input and output formats, operating instruction (how to use and what to do if abnormal), options (what choices are given to user), running time, accuracy.
Every program shipped should have small test cases that serves as proof of working.
More thorough test cases should include: mainline cases, barely legitimate cases (edge cases within the domain of input data), barely illegitimate cases.
Flow charting every detail / statement is not useful; flowchart / system diagram on a higher level could be better for describing the internals of a system.
(To be able to reasonably document something, we should try to minimize the burden of documentation.)
Comment your code, name your variables, structure your code so that it’s easy to read. (self-documenting code)
Document your code as you are writing it.

### No silver bullets
Claim: no single improvement in technology, management would cause the software development productivity, simplicity, or reliability to increase by an order of magnitude.
The making of a great designer.
