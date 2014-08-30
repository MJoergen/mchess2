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

This test suite contains over five hundred "interesting" positions, taken from 
http://chessprogramming.wikispaces.com/Test-Positions

Use the command

    mchess -s searchsuite.epd

to do a test of the search function.

This takes approx ten hours on an average PC to complete, because
each position has a time limit of one minute.

After completion, the file searchsuite.epd.log contains the results of each
test.

On an average machine, I get the following results:
- Bratko-Kopec Test        :   8 out of 24
- CCR One Hour Test        :   5 out of 25
- Kaufman Test             :   3 out of 25
- Longuet Chess Test II    :   2 out of 35
- Null Move Test-Positions :   3 out of 5
- Silent but deadly        :  35 out of 134
- Win at Chess             : 166 out of 300

Total : 222 out of 548, i.e. 40.5 %



Other tests
===========

Additionally, there are a number of .fen files with interesting positions.

