Testsuites
==========

There are several different test suites in this directory.

- perftsuite.epd  : Testing move generation
- searchsuite.epd : Testing the search engine


Move generation
===============

This test suite should be run whenever changes are made
to the board representation or the move generating function.

Use the command 

    mchess -p perftsuite.epd

to verify the generation of legal moves.

This takes approx two minutes on an average PC to complete.


Search test suite
=================

This test suite contains over five hundred "interesting" positions.

Use the command

    mchess -s searchsuite.epd

to do a test of the search function.

This takes approx ten hours on an average PC to complete, because
each position has a time limit of one minute.

After completion, the file searchsuite.epd.log contains the results of each
test.


Other tests
===========

Additionally, there are a number of .fen files with interesting positions.

