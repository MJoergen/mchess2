mchess2
=======

This is a fully functioning chess engine.
It is meant as a starting point for writing your own chess engine. Use it for
inspiration :-)

This is actually a part of the answer:
http://www.quora.com/How-do-I-get-started-with-my-own-chess-engine/answer/Michael-J%C3%B8rgensen-2


Features
========

- Supports all legal chess moves, incl. castling and en-passant
- Alpha-beta search strategy, with quiescence and iterative deepening.
- Transposition tables.
- A simple console (ASCII) user interface.
- UCI interface (for GUI)
- Time control
- Test suites
- It searches around 200k nodes per second on an average computer.


Documentation
=============

The source code has comments in most places.  Otherwise, the best place for a high level description is the
above link to my answer on Quora.
Additionally, the directory doc contains some additional information on writing chess engines.


TODO
====

The following is a list of features to be implemented.
They are roughly ordered by most needed first.

- Improve evaluation function. The current two calls to findLegalMoves is very slow.
- Opening book
- Check for draw by repetition.
- Adapt search depth depending on time.
- Improve negamax: E.g. NegaScout, PV-search, Aspiration Windows, MTD(f).
- Add statistics.
- Get it to play on FICS.
- In search(), consider captures first. And of them, consider first those that capture a greater piece with a lesser piece.
- Keep a list of killer moves, those that cause the window to close.

