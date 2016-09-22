# bloom
Bloom filter in C

A rather straightforward implementation of a Bloom filter in C.
Create a new filter by setting the number of items you plan to store and a
probability of false positives p between 0 and 1.

Add all your data items once, then submit candidates. If a candidate had never
been seen before the filter will tell you with absolute certainty. If the
filter tells you the candidate may have been seen before it has probability
p of being wrong (false positive).

This is convenient to have when your lookup operation can be prohibitively
expensive. Pass your candidate data through a Bloom filter first to learn
if it is worth performing the lookup at all.

Nicolas -- Sep 2016

