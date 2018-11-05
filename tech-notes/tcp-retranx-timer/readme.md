### Note on TCP retranx timer and exponential backoff

Email from Zhehao to Ken to follow up on off-topic TCP discussion during dynamo meetup.

```
Basically TCP measures RTT, and computes a retransmission timeout (RTO) value based on historical measurements.
Details are in the quoted RFC below, but basically TCP keeps track of an estimated RTT and expected variance (not a simple average but configurable coefficients alpha and beta to tune how much weight the latest RTT sample should play vs current estimates), and the weighted sum of both is the RTO, with no exponential or randomization involved.
Rationale, I believe, is that TCP has positive ACKs thus it has enough information about how long a transmission is expected to take (RTT) on a running basis. With this knowledge it's easy to decide when to retransmit, namely when an ACK does not come back in time (that is, it exceeds RTT and an additional threshold that accounts for the variance).
On the other hand, I believe exponential backoff is often used when you've no knowledge of the other parties participating in, say, sharing the resource (e.g. how long they are going to keep using the resource for). They don't give you feedback and the best you can do is to back off.
In CSMA/CD, Since no one ends up being able to send anything if a collision occurs (really bad!), we want to back off fairly aggressively and we don't want to have a possibility of keeping the entire system in a collided state, thus the randomized exponential backoff. Collision detection / avoidance like this powers today's Ethernet.
```

```
P.S. related section from the TCP retrans RFC copied over:

   (2.1) Until a round-trip time (RTT) measurement has been made for a
         segment sent between the sender and receiver, the sender SHOULD
         set RTO <- 1 second, though the "backing off" on repeated
         retransmission discussed in (5.5) still applies.

         Note that the previous version of this document used an initial
         RTO of 3 seconds [PA00].  A TCP implementation MAY still use
         this value (or any other value > 1 second).  This change in the
         lower bound on the initial RTO is discussed in further detail
         in Appendix A.

   (2.2) When the first RTT measurement R is made, the host MUST set

            SRTT <- R
            RTTVAR <- R/2
            RTO <- SRTT + max (G, K*RTTVAR)

         where K = 4.

   (2.3) When a subsequent RTT measurement R' is made, a host MUST set

            RTTVAR <- (1 - beta) * RTTVAR + beta * |SRTT - R'|
            SRTT <- (1 - alpha) * SRTT + alpha * R'

         The value of SRTT used in the update to RTTVAR is its value
         before updating SRTT itself using the second assignment.  That
         is, updating RTTVAR and SRTT MUST be computed in the above
         order.

         The above SHOULD be computed using alpha=1/8 and beta=1/4 (as
         suggested in [JK88]).

         After the computation, a host MUST update
         RTO <- SRTT + max (G, K*RTTVAR)

   (2.4) Whenever RTO is computed, if it is less than 1 second, then the
         RTO SHOULD be rounded up to 1 second.

         Traditionally, TCP implementations use coarse grain clocks to
         measure the RTT and trigger the RTO, which imposes a large
         minimum value on the RTO.  Research suggests that a large
         minimum RTO is needed to keep TCP conservative and avoid
         spurious retransmissions [AP99].  Therefore, this specification
         requires a large minimum RTO as a conservative approach, while
         at the same time acknowledging that at some future point,
         research may show that a smaller minimum RTO is acceptable or
         superior.

   (2.5) A maximum value MAY be placed on RTO provided it is at least 60
         seconds.

...

   (5.5) The host MUST set RTO <- RTO * 2 ("back off the timer").  The
         maximum value discussed in (2.5) above may be used to provide
         an upper bound to this doubling operation.
```

```
A quick example of
 - First RTT sample being 100ms,
SRTT = 100ms
RTTVAR = 50ms
RTO = 300ms (assuming G being rather small), adjusted up to 1s

 - Second RTT sample being 200ms
SRTT = 112.5ms
RTTVAR = 62.5ms
RTO = 362.5ms (assuming G being rather small), adjusted up to 1s
```