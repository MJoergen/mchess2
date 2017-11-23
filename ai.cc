#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "ai.h"
#include "trace.h"
#include "CTime.h"

const int INFINITY = 9999;

/***************************************************************
 * This is an implementation of
 * "NegaMax with Alpha Beta Pruning and Transposition Tables"
 * as seen on http://en.wikipedia.org/wiki/Negamax
 * 
 * See also http://frayn.net/beowulf/theory.html
 * and http://fierz.ch/strategy.htm
 *
 * Here alpha is a lower bound and beta is an upper bound,
 * both of which are non-inclusive.
 * In other words: Assume the result lies in the open interval
 * ]alpha, beta[
 * If the returned value is in this range, then it is exact.
 * If the returned value is beta or higher, then it is a new lower bound.
 * If the returned value is alpha or lower, then it is a new upper bound.
 *
 * The value returned is the value of the side to move.
 *
 ***************************************************************/
int AI::search(int alpha, int beta, int level, CMoveList& pv)
{
    // Check for illegal position (side NOT to move is in check).
    // In other words, the side to move can capture the opponents position.
    // This is an illegal position but corresponds to an immediate win.
    // Return a large positive value.
    if (m_board.isOtherKingInCheck())
    {
        TRACE("S : " << 9000 + level << std::endl);
        return 9000 + level;
    }

    TRACE("(" << alpha << "," << beta << ") " << m_moveList << ": ");

    // First we check if we are at leaf of tree.
    // If so, do a quiescence search.
    if (level == 0)
    {
        int val = quiescence(alpha, beta, pv);

        TRACE(pv << "->" << val << std::endl);

        // If a capture sequence was found, store the first move in the hash table.
        // This is an optimization that improves move ordering.
        if (pv.size())
        {
            CHashEntry hashEntry;
            if (val <= alpha)
            {
                hashEntry.m_nodeTypeAndLevel.nodeType = nodeUpper;
            }
            else if (val >= beta)
            {
                hashEntry.m_nodeTypeAndLevel.nodeType = nodeLower;
            }
            else
            {
                hashEntry.m_nodeTypeAndLevel.nodeType = nodeExact;
            }

            hashEntry.m_nodeTypeAndLevel.level = level;
            hashEntry.m_hashValue              = m_hashEntry.m_hashValue;
            hashEntry.m_searchValue            = val;
            hashEntry.m_bestMove               = pv[0];

            m_hashTable.insert(hashEntry);
            TRACE("Added quiet hashEntry : " << m_moveList << " " << hashEntry << std::endl);

            m_killerMove = pv[0];
        }

        return val;
    }

    m_nodes++;

    // Next, check if we have been at this position before (possibly with 
    // shallower search). This works extremely well together with iterative
    // deepening. Another possibility for position arising again is due to
    // transpositions, but that is less frequent.
    // The main benefit is that it gives a good move to search first.
    CHashEntry hashEntry;
    if (m_hashTable.find(m_hashEntry.m_hashValue, hashEntry))
    {
        TRACE("Found hashEntry : " << hashEntry);

        // Now we examine the search value stored.
        // This value is only to be trusted, if the
        // search level is sufficiently high.
        // Even though the level is not sufficient,
        // we may still use the stored move. This is done later.
        if (hashEntry.m_nodeTypeAndLevel.level >= level)
        {
            switch (hashEntry.m_nodeTypeAndLevel.nodeType)
            {
                case nodeLower :
                    // Lower bound. The true value may be more.
                    if (hashEntry.m_searchValue >= alpha)
                        alpha = hashEntry.m_searchValue;
                    break;

                case nodeUpper :
                    // Upper bound. The true value may be less.
                    if (hashEntry.m_searchValue <= beta)
                        beta = hashEntry.m_searchValue;
                    break;

                default : // case nodeExact :
                    // Exact value. We are done!
                    return hashEntry.m_searchValue;
            }

            // Check if window is closed.
            if (alpha >= beta)
            {
                return hashEntry.m_searchValue;
            }
        } // end of if level
    } // end of m_hashTable.find

    TRACE(std::endl);

    // Prepare to search through all legal moves.
    CMoveList moves;
    m_board.find_legal_moves(moves);

    // If we have been at this position before, which move was the best?
    // Search this move first, because it is likely to still be the best.
    // This often provides a quick refutation of the previous move, 
    // and therefore saves a lot of time.
    if (hashEntry.m_bestMove.Valid())
    {
        for (unsigned int i=0; i<moves.size(); ++i)
        {
            if (moves[i] == hashEntry.m_bestMove)
            {
                TRACE("Hash: Move reordering! Play " << hashEntry.m_bestMove
                        << " first" << std::endl);
                moves[i] = moves[0];
                moves[0] = hashEntry.m_bestMove;
                break;
            }
        }
    }
    else
    { // Search captures first.
        unsigned int j=0;
        for (unsigned int i=0; i<moves.size(); ++i)
        {
            if (moves[i].is_it_a_capture())
            {
                CMove tmpMove = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpMove;

                TRACE("Capture: Move reordering! Play " << tmpMove
                        << " first" << std::endl);
                j++;
            }
        }
    }

    int best_val = -INFINITY;
    int alpha_orig = alpha;

    // Loop through all legal moves.
    for (unsigned int i=0; i<moves.size(); ++i)
    {
        CMove move = moves[i];

#ifdef DEBUG_HASH
        uint32_t oldHash = m_board.calcHash();
        CHashEntry hashCopy(m_hashEntry);
#endif

        // Do a recursive search
        m_moveList.push_back(move);
        m_hashEntry.update(m_board, move);
        m_board.make_move(move);

        CMoveList pv_temp;
        int val = -search(-beta, -alpha, level-1, pv_temp);

        m_board.undo_move(move);
        m_hashEntry.update(m_board, move);
        m_moveList.pop_back();

#ifdef DEBUG_HASH
        uint32_t newHash = m_board.calcHash();
        if (oldHash != newHash)
        {
            TRACE("Hash failure" << std::endl);
            TRACE(m_board);
            exit(-1);
        }
        if (hashCopy != m_hashEntry)
        {
            TRACE("New hash failure" << std::endl);
            TRACE(m_board);
            exit(-1);
        }
#endif

        if (val > best_val)
        {
            // This is the best move so far.
            best_val = val;

            pv = move;
            pv += pv_temp;
        }

        // Now comes the part specific for alpha-beta pruning:
        // Since we are only interested, if another
        // move is better, we update our lower bound.
        if (val > alpha)
        {
            alpha = val;
        }
        // Now we check if the window has been closed.
        // If so, then stop the search.
        if (alpha >= beta)
        {
            // This is fail-soft, since we are returning the value best_val,
            // which might be outside the window.
            break;
        }

        if (m_pvSearch)
        {
            CTime now;
            if (m_timeEnd < now)
            {
                TRACE("Out of time. Stopping search." << std::endl);
                return alpha;
            }
        }

    } // end of for

    // If our king was captured, check for stalemate
    if (best_val < -8000)
    {
        if (!m_board.isKingInCheck())
            best_val = 0;
    }

    // Finally, store the result in the hash table.
    // We must be careful to determine whether the value is 
    // exact or a bound.
    if (best_val <= alpha_orig)
    {
        hashEntry.m_nodeTypeAndLevel.nodeType = nodeUpper;
    }
    else if (best_val >= beta)
    {
        hashEntry.m_nodeTypeAndLevel.nodeType = nodeLower;
    }
    else
    {
        hashEntry.m_nodeTypeAndLevel.nodeType = nodeExact;
    }

    hashEntry.m_nodeTypeAndLevel.level = level;
    hashEntry.m_hashValue              = m_hashEntry.m_hashValue;
    hashEntry.m_searchValue            = best_val;
    hashEntry.m_bestMove               = pv[0];

    m_hashTable.insert(hashEntry);
    TRACE("Added hashEntry : " << m_moveList << " " << hashEntry << std::endl);

    return best_val;
} // end of int search

/***************************************************************
 * This performs a search of capture moves only.
 * The static evaluation is used as a lower bound on the score.
 * This assumes that there always exists a move that will
 * improve the score. This is safe, unless we are in zug-zwang.
 ***************************************************************/
int AI::quiescence(int alpha, int beta, CMoveList& pv)
{
    pv.clear();

    // Check for illegal position (side NOT to move is in check).
    // In other words, the side to move can capture the opponents position.
    // This is an illegal position but corresponds to an immediate win.
    // Return a large positive value.
    if (m_board.isOtherKingInCheck())
    {
        TRACE("Q : 9000" << std::endl);
        return 9000;
    }

    m_nodes++;

    int best_val = m_board.get_value();

    TRACE("  (" << alpha << "," << beta << ") " << m_moveList << "* ");
    TRACE(best_val << std::endl);

    if (best_val >= beta)
        return best_val;

    if (best_val > alpha)
        alpha = best_val;

    CMoveList moves;
    m_board.find_legal_moves(moves);

    // Search killer move first
    unsigned int j=0;
    if (m_killerMove.Valid())
    {
        for (unsigned int i=0; i<moves.size(); ++i)
        {
            if (moves[i] == m_killerMove && moves[i].is_it_a_capture())
            {
                CMove tmpMove = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpMove;

                TRACE("Killer move reordering! Play " << tmpMove
                        << " first" << std::endl);
                j++;
            }
        }
    } // end of if lastMove.Valid()

    // Search recaptures to same square first.
    CMove lastMove = m_moveList.last();
    if (lastMove.Valid())
    {
        for (unsigned int i=j; i<moves.size(); ++i)
        {
            if (moves[i].To() == lastMove.To())
            {
                CMove tmpMove = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpMove;

                TRACE("Recapture move reordering! Play " << tmpMove
                        << " first" << std::endl);
                j++;
            }
        }
    } // end of if lastMove.Valid()

    // Search piece captures first.
    for (unsigned int i=j; i<moves.size(); ++i)
    {
        if (moves[i].GetCaptured() != WP && 
                moves[i].GetCaptured() != BP && 
                moves[i].GetCaptured() != EM)
        {
            CMove tmpMove = moves[i];
            moves[i] = moves[j];
            moves[j] = tmpMove;

            TRACE("Piece capture move reordering! Play " << tmpMove
                    << " first" << std::endl);
            j++;
        }
    }

    for (unsigned int i=0; i<moves.size(); ++i)
    {
        CMove move = moves[i];

        if (!move.is_it_a_capture())
            continue; // Skip non-captures

        // Do a recursive search
        m_moveList.push_back(move);
        m_hashEntry.update(m_board, move);
        m_board.make_move(move);

        CMoveList pv_temp;
        int val = -quiescence(-beta, -alpha, pv_temp);

        m_board.undo_move(move);
        m_hashEntry.update(m_board, move);
        m_moveList.pop_back();

        if (val > best_val)
        {
            // This is the best move so far.
            best_val = val;

            pv = move;
            pv += pv_temp;
        }

        // Update lower bound
        if (val > alpha)
        {
            alpha = val;
        }

        // Stop if window is closed.
        if (alpha >= beta)
        {
            break; // fail-soft
        }
    } // end of for

    return best_val;
} // end of quiescence

/***************************************************************
 * find_best_move
 *
 * This is the main AI.
 * It returns what it considers to be the best legal move in the
 * current position.
 ***************************************************************/
CMove AI::find_best_move(int wTime, int bTime, int movesToGo)
{
    m_nodes = 0;
    m_hashEntry.set(m_board);
    m_moveList.clear();

    CTime timeStart;
    m_timeEnd = timeStart;
    if (wTime && bTime && movesToGo)
    {
        if (m_board.whiteToMove())
            m_timeEnd += wTime / movesToGo;
        else
            m_timeEnd += bTime / movesToGo;
    }
    else
    {
        m_timeEnd += 1000*60*60*24; // 24 hours
    }

    CMoveList moves;
    m_board.find_legal_moves(moves);

    CMoveList best_moves;

    int best_val;
    CMoveList pv;
    int num_good;

    int level = 0;
    while (level <= 2)
    {
        TRACE("level: " << level << std::endl);
        TRACE("moves: ");
        TRACE(moves.ToShortString());
        TRACE(std::endl);

        CMove best_move;

        best_moves.clear();
        best_val = -INFINITY;
        num_good = 0;

        m_pvSearch = true;

        for (unsigned int i=0; i<moves.size(); ++i)
        {
            // We are looking for values in the range [best_val, INFINITY[, 
            // which is the same as ]best_val-1, INFINITY[
            int alpha = best_val-1;
            int beta = INFINITY;

            CMove move = moves[i];

            m_moveList.push_back(move);
            m_hashEntry.update(m_board, move);
            m_board.make_move(move);

            CMoveList pv_temp;
            int val = -search(-beta, -alpha, level, pv_temp);

            m_board.undo_move(move);
            m_hashEntry.update(m_board, move);
            m_moveList.pop_back();

            TRACE("]" << alpha << ", " << beta << "[ "
                    << move << " " << pv_temp << " -> " << val);

            if (val > best_val)
            {
                num_good = 0;
            }

            if (val >= best_val)
            {
                num_good++;

                pv = move;
                pv += pv_temp;

                best_val = val;
                best_move = move;

                CTimeDiff timeDiff(timeStart);

                unsigned long millisecs = timeDiff.millisecs();

                unsigned long nps = 0;
                if (millisecs)
                    nps = (m_nodes*1000)/millisecs;

                std::cout << "info depth " << level << " score cp " << best_val;
                std::cout << " time " << millisecs << " nodes " << m_nodes << " nps " << nps;
                std::cout << " pv " << pv << std::endl;

                // This is the move reordering. Good moves are searched first on next iteration.
                best_moves.insert_front(move);
                TRACE(" front" << std::endl);
            }
            else
            {
                // This is the move reordering. Bad moves are searched last on next iteration.
                best_moves.push_back(move);
                TRACE(" back" << std::endl);
            }

            m_pvSearch = false;

            CTime now;
            if (m_timeEnd < now)
            {
                TRACE("Out of time. Stopping search." << std::endl);
                break;
            }
        } // end of for

        moves = best_moves;

        CTimeDiff timeDiff(timeStart);

        unsigned long millisecs = timeDiff.millisecs();

        unsigned long nps = 0;
        if (millisecs)
            nps = (m_nodes*1000)/millisecs;

        std::cout << "info depth " << level << " score cp " << best_val;
        std::cout << " time " << millisecs << " nodes " << m_nodes << " nps " << nps;
        std::cout << " pv " << pv << std::endl;

        CTime now;
        if (m_timeEnd < now)
        {
            TRACE("Out of time. Stopping search." << std::endl);
            break;
        }

        level += 2;
    }

    CMove move = best_moves[rand()%num_good];

    TRACE(num_good << " moves to choose from" << std::endl);
    TRACE("Playing " << move << std::endl);

    return move;
} // end of CMove find_best_move(CBoard &board)

