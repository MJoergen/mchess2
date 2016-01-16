#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "CBoard.h"

#ifdef DEBUG_HASH
#include "crc32.h"
#endif

// Create nice colours in the console output
#define TTY_YELLOW "[33m"
#define TTY_BLUE   "[34m"
#define TTY_NORM   "[0m"

static char pieces[] = "kqrbnp.PNBRQK";


/***************************************************************
 * operator <<
 * This displays the current board configuration on the stream.
 ***************************************************************/
std::ostream& operator <<(std::ostream &os, const CBoard &rhs)
{
    for (int row=8; row>=1; --row)
    {
        os << TTY_NORM << row << " ";
        for (int col=1; col<=8; ++col)
        {
            int number = (row+1)*10+col;
            int piece = rhs.m_board[number];
            if (piece != IV)
            {
                if (piece < 0)
                    os << TTY_BLUE;
                else if (piece > 0)
                    os << TTY_YELLOW;
                else
                    os << TTY_NORM;
                os << pieces[piece+6] << " ";
            }
        }
        os << std::endl;
    }
    os << TTY_NORM;
    os << "  a b c d e f g h" << std::endl;
    switch (rhs.m_side_to_move)
    {
        case  1 : os << "White to move" << std::endl; break;
        default : os << "Black to move" << std::endl; break;
    }
    if (rhs.m_castleRights & CASTLE_WHITE_SHORT)
        os << "K";
    if (rhs.m_castleRights & CASTLE_WHITE_LONG)
        os << "Q";
    if (rhs.m_castleRights & CASTLE_BLACK_SHORT)
        os << "k";
    if (rhs.m_castleRights & CASTLE_BLACK_LONG)
        os << "q";
    if (!rhs.m_castleRights)
        os << "-";

    os << " ";
    if (rhs.m_enPassantSquare)
        os << CSquare(rhs.m_enPassantSquare);
    else
        os << "-";
    os << std::endl;
    os << "Material : " << rhs.m_material << std::endl;
    if (rhs.isKingInCheck())
    {
        os << "You are in check!" << std::endl;
    }
    return os;
} // end of std::ostream& operator <<(std::ostream &os, const CBoard &rhs)


/**************************************************************
 * newGame
 * This resets the board configuration to the initial game setup
 ***************************************************************/
void CBoard::newGame()
{
    e_piece initial[120] = {
      IV, IV, IV, IV, IV, IV, IV, IV, IV, IV,
      IV, IV, IV, IV, IV, IV, IV, IV, IV, IV,
      IV, WR, WN, WB, WQ, WK, WB, WN, WR, IV,
      IV, WP, WP, WP, WP, WP, WP, WP, WP, IV,
      IV, EM, EM, EM, EM, EM, EM, EM, EM, IV,
      IV, EM, EM, EM, EM, EM, EM, EM, EM, IV,
      IV, EM, EM, EM, EM, EM, EM, EM, EM, IV,
      IV, EM, EM, EM, EM, EM, EM, EM, EM, IV,
      IV, BP, BP, BP, BP, BP, BP, BP, BP, IV,
      IV, BR, BN, BB, BQ, BK, BB, BN, BR, IV,
      IV, IV, IV, IV, IV, IV, IV, IV, IV, IV,
      IV, IV, IV, IV, IV, IV, IV, IV, IV, IV} ;

    m_board.reserve(120);
    for (int i=0; i<120; ++i)
    {
      m_board[i] = initial[i];
    }

    m_side_to_move = 1;

    m_castleRights = 
        CASTLE_WHITE_SHORT | 
        CASTLE_WHITE_LONG | 
        CASTLE_BLACK_SHORT | 
        CASTLE_BLACK_LONG;

    m_enPassantSquare = 0;
    m_material = 0;
    m_halfMoves = 0;
    m_fullMoves = 0;
    m_state.clear();
} // end of newGame


/**************************************************************
 * calcMaterial
 ***************************************************************/
void CBoard::calcMaterial()
{
    m_material = 0;
    for (int i=0; i<120; ++i)
    {
        switch (m_board[i])
        {
            case WP : m_material += 1; break;
            case WN : m_material += 3; break;
            case WB : m_material += 3; break;
            case WR : m_material += 5; break;
            case WQ : m_material += 9; break;
            case BP : m_material -= 1; break;
            case BN : m_material -= 3; break;
            case BB : m_material -= 3; break;
            case BR : m_material -= 5; break;
            case BQ : m_material -= 9; break;
            default : break;
        }
    }

    m_material *= m_side_to_move;

} // end of calcMaterial


/**************************************************************
 * read_from_fen
 * Sets the board position from a FEN string
 * Returns true on error.
 ***************************************************************/
bool CBoard::read_from_fen(const char *fen, const char **endptr)
{
    unsigned int pos = 91;
    unsigned int strpos = 0;
    enum {st_board, st_sideToMove, st_castling, st_enpassant, st_halfmove, st_fullmove, st_finished}
        state = st_board;

    m_castleRights = 0;
    m_side_to_move = 0;
    m_enPassantSquare = 0;

    unsigned int len = strlen(fen);

    while (strpos < len and state != st_finished) 
    {
        switch (state)
        {
            case st_board :
                switch (fen[strpos])
                {
                    case 'k' : m_board[pos++] = BK; break;
                    case 'q' : m_board[pos++] = BQ; break;
                    case 'r' : m_board[pos++] = BR; break;
                    case 'b' : m_board[pos++] = BB; break;
                    case 'n' : m_board[pos++] = BN; break;
                    case 'p' : m_board[pos++] = BP; break;
                    case 'K' : m_board[pos++] = WK; break;
                    case 'Q' : m_board[pos++] = WQ; break;
                    case 'R' : m_board[pos++] = WR; break;
                    case 'B' : m_board[pos++] = WB; break;
                    case 'N' : m_board[pos++] = WN; break;
                    case 'P' : m_board[pos++] = WP; break;
                    case '/' :
                               if ((pos % 10) != 9)
                               {
                                   return true;
                               }
                               pos -= 18;
                               break;
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' : {
                                   int count = fen[strpos] - '0';
                                   while (count > 0)
                                   {
                                       if ((pos % 10) > 9) 
                                       {
                                           return true;
                                       }
                                       m_board[pos++] = EM;
                                       count--;
                                   }
                               }
                               break;
                    case ' ' :
                               if (pos != 29)
                               {
                                   return true;
                               }
                               state = st_sideToMove;
                               break;
                    default : return true;
                } // end of switch
                break;

            case st_sideToMove :
                switch (fen[strpos])
                {
                    case 'b' : m_side_to_move = -1; break;
                    case 'w' : m_side_to_move =  1; break;
                    case ' ' : if (m_side_to_move == 0)
                                   return true;
                               state = st_castling;
                               break;
                    default  : return true;
                }
                break;

            case st_castling :
                switch (fen[strpos])
                {
                    case 'K' : m_castleRights |= CASTLE_WHITE_SHORT; break;
                    case 'Q' : m_castleRights |= CASTLE_WHITE_LONG;  break;
                    case 'k' : m_castleRights |= CASTLE_BLACK_SHORT; break;
                    case 'q' : m_castleRights |= CASTLE_BLACK_LONG;  break;
                    case ' ' : state = st_enpassant; break;
                    case '-' : break;
                    default  : return true;
                }
                break;

            case st_enpassant :
                switch (fen[strpos])
                {
                    case 'a' :
                    case 'b' :
                    case 'c' :
                    case 'd' :
                    case 'e' :
                    case 'f' :
                    case 'g' :
                    case 'h' : m_enPassantSquare = (fen[strpos] - 'a') + 21; break;
                    case '1' :
                    case '2' :
                    case '3' :
                    case '4' :
                    case '5' :
                    case '6' :
                    case '7' :
                    case '8' : m_enPassantSquare += (fen[strpos] - '1')*10; break;
                    case ' ' : state = st_halfmove; break;
                    case '-' : break;
                    default  : return true;
                }
                break;

            case st_halfmove :
                {
                    char *endp;
                    switch (fen[strpos])
                    {
                        case '0' :
                        case '1' :
                        case '2' :
                        case '3' :
                        case '4' :
                        case '5' :
                        case '6' :
                        case '7' :
                        case '8' :
                        case '9' : m_halfMoves = strtol(&fen[strpos], &endp, 10); 
                                   strpos = endp-fen;
                                   break;
                        case ' ' : state = st_fullmove; break;
                        default  : state = st_finished; strpos--; break;
                    }

                }
                break;

            case st_fullmove :
                {
                    char *endp;
                    switch (fen[strpos])
                    {
                        case '0' :
                        case '1' :
                        case '2' :
                        case '3' :
                        case '4' :
                        case '5' :
                        case '6' :
                        case '7' :
                        case '8' :
                        case '9' : m_fullMoves = strtol(&fen[strpos], &endp, 10);
                                   strpos = endp-fen;
                                   break;
                        case ' ' : state = st_finished; break;
                        default  : state = st_finished;
                    }
                }
                break;

            default : // case st_finished :
                break;
        } // end of switch
        strpos++;
    } // end of while
    if (state == st_finished &&
            (m_enPassantSquare == 0 || CSquare(m_enPassantSquare).isValid()))
    {
        if (fen[strpos] >= '0' and fen[strpos] <= '9')
        {

        }

        calcMaterial();
        if (endptr)
            *endptr = &fen[strpos];
        return false;
    }
    return true;
} // end of read_from_fen


/***************************************************************
 * readMove
 ***************************************************************/
CMove CBoard::readMove(const char *p, const char **endptr) const
{
    CMove ret;

    CMoveList moves;
    find_legal_moves(moves);

    // Handle castling
    if (p[0] == 'O' and p[1] == '-' and p[2] == 'O')
    {
        if (m_side_to_move > 0)
        {
            ret = CMove(WK, E1, G1);
        }
        else
        {
            ret = CMove(BK, E8, G8);
        }

        p+=3;

        if (p[0] == '-' and p[1] == 'O')
        {
            if (m_side_to_move > 0)
            {
                ret = CMove(WK, E1, C1);
            }
            else
            {
                ret = CMove(BK, E8, C8);
            }

            p+=2;
        }
    }
    else
    {
        int piece = WP;
        switch (p[0])
        {
            case 'N' : piece = WN; ++p; break;
            case 'B' : piece = WB; ++p; break;
            case 'R' : piece = WR; ++p; break;
            case 'Q' : piece = WQ; ++p; break;
            case 'K' : piece = WK; ++p; break;
            default  : break;
        }
        if (m_side_to_move < 0)
            piece = -piece;

        int disambRow = 0;
        int disambCol = 0;
        if ((p[1] >= 'a' and p[1] <= 'h' and p[0] != 'x') or p[1] == 'x')
        {
            if (p[0] >= 'a' and p[0] <= 'h')
                disambCol = p[0] - 'a' + 1;
            else if (p[0] >= '1' and p[0] <= '8')
                disambRow = p[0] - '1' + 1;
            else return ret;
            ++p;
        }

        if (p[0] == 'x')
            ++p;

        CSquare targetSq;
        if (targetSq.FromString(p))
            return ret;

        p += 2;

        int promotion = 0;
        if (p[0] == '=')
        {
            ++p;
            switch (p[0])
            {
                case 'N' : promotion = WN; ++p; break;
                case 'B' : promotion = WB; ++p; break;
                case 'R' : promotion = WR; ++p; break;
                case 'Q' : promotion = WQ; ++p; break;
                default  : return ret;
            }
        }
        if (m_side_to_move < 0)
            promotion = -promotion;

        int numMoves = moves.size();
        for (int i=0; i<numMoves; ++i)
        {
            ret = moves[i];
            if (ret.GetPiece() != piece)
                continue;
            if (ret.To() != targetSq)
                continue;
            if (ret.GetPromoted() != promotion)
                continue;
            if (disambCol && disambCol != ret.From().col())
                continue;
            if (disambRow && disambRow != ret.From().row())
                continue;
            break;
        }

    } // end of if else

    if (p[0] == '+' or p[0] == '#')
        ++p;

    if (endptr)
        *endptr = p;

    return ret;
} // end of readMove


/***************************************************************
 * isSquareThreatened
 * Returns true if OTHER side to move threatens this square
 ***************************************************************/
bool CBoard::isSquareThreatened(const CSquare& sq) const
{
    if (m_side_to_move > 0)
    {
        // Check for pawns
        if (m_board[sq + NW] == BP || m_board[sq + NE] == BP)
            return true;

        // Check for knigts
        {
            int dirs[8] = {NNW, NNE, NWW, NEE, SSW, SSE, SWW, SEE};

            for (int k=0; k<8; ++k)
            {
                int dir = dirs[k];
                if (m_board[sq + dir] == BN)
                    return true;
            }
        }

        // Check for diagonal (bishop and queen)
        {
            int dirs[4] = {NW, NE, SW, SE};

            for (int k=0; k<4; ++k)
            {
                int dir = dirs[k];
                int sq2 = sq;
                while (1)
                {
                    sq2 += dir;
                    if (m_board[sq2] == BB || m_board[sq2] == BQ)
                        return true;
                    if (m_board[sq2] != EM)
                        break;
                }

            }
        }

        // Check for horizontal/vertical (rook and queen)
        {
            int dirs[4] = {N, S, W, E};

            for (int k=0; k<4; ++k)
            {
                int dir = dirs[k];
                int sq2 = sq;
                while (1)
                {
                    sq2 += dir;
                    if (m_board[sq2] == BR || m_board[sq2] == BQ)
                        return true;
                    if (m_board[sq2] != EM)
                        break;
                }

            }
        }

        // Check for king
        {
            int dirs[8] = {NW, NE, SW, SE, N, S, W, E};

            for (int k=0; k<8; ++k)
            {
                int dir = dirs[k];
                if (m_board[sq + dir] == BK)
                    return true;
            }
        }
    }
    else
    {
        // Check for pawns
        if (m_board[sq + SW] == WP || m_board[sq + SE] == WP)
            return true;

        // Check for knigts
        {
            int dirs[8] = {NNW, NNE, NWW, NEE, SSW, SSE, SWW, SEE};

            for (int k=0; k<8; ++k)
            {
                int dir = dirs[k];
                if (m_board[sq + dir] == WN)
                    return true;
            }
        }

        // Check for diagonal (bishop and queen)
        {
            int dirs[4] = {NW, NE, SW, SE};

            for (int k=0; k<4; ++k)
            {
                int dir = dirs[k];
                int sq2 = sq;
                while (1)
                {
                    sq2 += dir;
                    if (m_board[sq2] == WB || m_board[sq2] == WQ)
                        return true;
                    if (m_board[sq2] != EM)
                        break;
                }

            }
        }

        // Check for horizontal/vertical (rook and queen)
        {
            int dirs[4] = {N, S, W, E};

            for (int k=0; k<4; ++k)
            {
                int dir = dirs[k];
                int sq2 = sq;
                while (1)
                {
                    sq2 += dir;
                    if (m_board[sq2] == WR || m_board[sq2] == WQ)
                        return true;
                    if (m_board[sq2] != EM)
                        break;
                }

            }
        }

        // Check for king
        {
            int dirs[8] = {NW, NE, SW, SE, N, S, W, E};

            for (int k=0; k<8; ++k)
            {
                int dir = dirs[k];
                if (m_board[sq + dir] == WK)
                    return true;
            }
        }
    }

    return false;
} // end of isSquareThreatened


/***************************************************************
 * find_legal_moves
 * This generates a complete list of all pseudo-legal moves.
 * A pseudo-legal move does not consider, whether the king will
 * be in check. This is handled else-where.
 ***************************************************************/
void CBoard::find_legal_moves(CMoveList &moves) const
{
    moves.clear();

    if (m_side_to_move > 0)
        for (int i=A1; i<=H8; ++i)
        {
            int8_t piece = m_board[i];
            int j;

            switch (piece)
            {
                case WP : 
                    j = i + 10;
                    if (m_board[j] == EM)
                    {
                        if (i > 80)
                        {
                            {
                                CMove move(piece, i, j, EM, WQ);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, EM, WR);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, EM, WB);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, EM, WN);
                                moves.push_back(move);
                            }
                        }
                        else
                        {
                            CMove move(piece, i, j, EM);
                            moves.push_back(move);
                        }

                        j = i + 20;
                        if (m_board[j] == EM)
                        {
                            if (i < 40)
                            {
                                CMove move(piece, i, j, EM);
                                moves.push_back(move);
                            }
                        }
                    }

                    j = i + 9;
                    if ((m_board[j] != IV && m_board[j] < 0) || j == m_enPassantSquare)
                    {
                        if (i > 80)
                        {
                            {
                                CMove move(piece, i, j, m_board[j], WQ);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], WR);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], WB);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], WN);
                                moves.push_back(move);
                            }
                        }
                        else
                        {
                            CMove move(piece, i, j, m_board[j]);
                            moves.push_back(move);
                        }
                    }

                    j = i + 11;
                    if ((m_board[j] != IV && m_board[j] < 0) || j == m_enPassantSquare)
                    {
                        if (i > 80)
                        {
                            {
                                CMove move(piece, i, j, m_board[j], WQ);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], WR);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], WB);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], WN);
                                moves.push_back(move);
                            }
                        }
                        else
                        {
                            CMove move(piece, i, j, m_board[j]);
                            moves.push_back(move);
                        }
                    }
                    break;

                case WN : 
                    {
                        int dirs[8] = {NNW, NNE, NWW, NEE, SSW, SSE, SWW, SEE};

                        for (int k=0; k<8; ++k)
                        {
                            int dir = dirs[k];

                            j = i+dir;
                            if (m_board[j] != IV) 
                                if (m_board[j] <= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                }
                        }
                    }

                    break;

                case WB : 
                    {
                        int dirs[4] = {NW, NE, SW, SE};

                        for (int k=0; k<4; ++k)
                        {
                            int dir = dirs[k];
                            j = i;
                            while (1)
                            {
                                j += dir;
                                if (m_board[j] == IV) break;
                                if (m_board[j] <= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                    if (m_board[j] < 0)
                                        break;
                                }
                                else
                                    break;
                            }
                        }
                    }

                    break;

                case WR : 
                    {
                        int dirs[4] = {N, W, S, E};

                        for (int k=0; k<4; ++k)
                        {
                            int dir = dirs[k];
                            j = i;
                            while (1)
                            {
                                j += dir;
                                if (m_board[j] == IV) break;
                                if (m_board[j] <= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                    if (m_board[j] < 0)
                                        break;
                                }
                                else
                                    break;
                            }
                        }
                    }

                    break;

                case WQ : 
                    {
                        int dirs[8] = {N, W, S, E, NW, NE, SW, SE};

                        for (int k=0; k<8; ++k)
                        {
                            int dir = dirs[k];
                            j = i;
                            while (1)
                            {
                                j += dir;
                                if (m_board[j] == IV) break;
                                if (m_board[j] <= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                    if (m_board[j] < 0)
                                        break;
                                }
                                else
                                    break;
                            }
                        }
                    }

                    break;

                case WK : 
                    {
                        int dirs[8] = {N, W, S, E, NW, NE, SW, SE};

                        for (int k=0; k<8; ++k)
                        {
                            int dir = dirs[k];

                            j = i+dir;
                            if (m_board[j] != IV) 
                                if (m_board[j] <= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                }
                        }

                        if (m_castleRights & CASTLE_WHITE_SHORT)
                        { // Then King and Rook must be in place
                            if (m_board[F1] == EM && m_board[G1] == EM)
                            {
                                if (!isSquareThreatened(E1) && !isSquareThreatened(F1))
                                {
                                    CMove move(WK, E1, G1);
                                    moves.push_back(move);
                                }
                            }
                        }
                        if (m_castleRights & CASTLE_WHITE_LONG)
                        { // Then King and Rook must be in place
                            if (m_board[B1] == EM && m_board[C1] == EM && m_board[D1] == EM)
                            {
                                if (!isSquareThreatened(E1) && !isSquareThreatened(D1))
                                {
                                    CMove move(WK, E1, C1);
                                    moves.push_back(move);
                                }
                            }
                        }
                    }

                    break;

                default : // Invalid, wrong color, or empty
                    continue;
            }

        }

    else
        for (int i=A1; i<=H8; ++i)
        {
            int8_t piece = m_board[i];
            int j;

            switch (piece)
            {
                case BP : 
                    j = i - 10;
                    if (m_board[j] == EM)
                    {
                        if (i < 40)
                        {
                            {
                                CMove move(piece, i, j, EM, BQ);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, EM, BR);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, EM, BB);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, EM, BN);
                                moves.push_back(move);
                            }
                        }
                        else
                        {
                            CMove move(piece, i, j, EM);
                            moves.push_back(move);
                        }

                        j = i - 20;
                        if (m_board[j] == EM)
                        {
                            if (i > 80)
                            {
                                CMove move(piece, i, j, EM);
                                moves.push_back(move);
                            }
                        }
                    }

                    j = i - 9;
                    if ((m_board[j] != IV && m_board[j] > 0) || j == m_enPassantSquare)
                    {
                        if (i < 40)
                        {
                            {
                                CMove move(piece, i, j, m_board[j], BQ);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], BR);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], BB);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], BN);
                                moves.push_back(move);
                            }
                        }
                        else
                        {
                            CMove move(piece, i, j, m_board[j]);
                            moves.push_back(move);
                        }
                    }

                    j = i - 11;
                    if ((m_board[j] != IV && m_board[j] > 0) || j == m_enPassantSquare)
                    {
                        if (i < 40)
                        {
                            {
                                CMove move(piece, i, j, m_board[j], BQ);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], BR);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], BB);
                                moves.push_back(move);
                            }
                            {
                                CMove move(piece, i, j, m_board[j], BN);
                                moves.push_back(move);
                            }
                        }
                        else
                        {
                            CMove move(piece, i, j, m_board[j]);
                            moves.push_back(move);
                        }
                    }
                    break;

                case BN : 
                    {
                        int dirs[8] = {NNW, NNE, NWW, NEE, SSW, SSE, SWW, SEE};

                        for (int k=0; k<8; ++k)
                        {
                            int dir = dirs[k];

                            j = i+dir;
                            if (m_board[j] != IV) 
                                if (m_board[j] >= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                }
                        }
                    }

                    break;

                case BB : 
                    {
                        int dirs[4] = {NE, NW, SE, SW};

                        for (int k=0; k<4; ++k)
                        {
                            int dir = dirs[k];
                            j = i;
                            while (1)
                            {
                                j += dir;
                                if (m_board[j] == IV) break;
                                if (m_board[j] >= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                    if (m_board[j] > 0)
                                        break;
                                }
                                else
                                    break;
                            }
                        }
                    }

                    break;

                case BR : 
                    {
                        int dirs[4] = {N, W, S, E};

                        for (int k=0; k<4; ++k)
                        {
                            int dir = dirs[k];
                            j = i;
                            while (1)
                            {
                                j += dir;
                                if (m_board[j] == IV) break;
                                if (m_board[j] >= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                    if (m_board[j] > 0)
                                        break;
                                }
                                else
                                    break;
                            }
                        }
                    }

                    break;

                case BQ : 
                    {
                        int dirs[8] = {N, W, S, E, NW, SW, NE, SE};

                        for (int k=0; k<8; ++k)
                        {
                            int dir = dirs[k];
                            j = i;
                            while (1)
                            {
                                j += dir;
                                if (m_board[j] == IV) break;
                                if (m_board[j] >= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                    if (m_board[j] > 0)
                                        break;
                                }
                                else
                                    break;
                            }
                        }
                    }

                    break;

                case BK : 
                    {
                        int dirs[8] = {N, W, S, E, NW, SW, NE, SE};

                        for (int k=0; k<8; ++k)
                        {
                            int dir = dirs[k];

                            j = i+dir;
                            if (m_board[j] != IV) 
                                if (m_board[j] >= 0)
                                {
                                    CMove move(piece, i, j, m_board[j]);
                                    moves.push_back(move);
                                }
                        }

                        if (m_castleRights & CASTLE_BLACK_SHORT)
                        { // Then King and Rook must be in place
                            if (m_board[F8] == EM && m_board[G8] == EM)
                            {
                                if (!isSquareThreatened(E8) && !isSquareThreatened(F8))
                                {
                                    CMove move(BK, E8, G8);
                                    moves.push_back(move);
                                }
                            }
                        }
                        if (m_castleRights & CASTLE_BLACK_LONG)
                        { // Then King and Rook must be in place
                            if (m_board[B8] == EM && m_board[C8] == EM && m_board[D8] == EM)
                            {
                                if (!isSquareThreatened(E8) && !isSquareThreatened(D8))
                                {
                                    CMove move(BK, E8, C8);
                                    moves.push_back(move);
                                }
                            }
                        }
                    }

                    break;

                default : // Invalid, wrong color, or empty
                    continue;
            }

        }
} // end of void CBoard::find_legal_moves(CMoveList &moves) const;


/***************************************************************
 * make_move
 * This updates the board according to the move
 ***************************************************************/
void CBoard::make_move(const CMove &move)
{
    m_state.push_back((m_enPassantSquare << 8) | m_castleRights);
    m_enPassantSquare = 0;
    switch (move.GetCaptured())
    {
        case WP : case BP : m_material += 1; break;
        case WN : case BN : m_material += 3; break;
        case WB : case BB : m_material += 3; break;
        case WR : case BR : m_material += 5; break;
        case WQ : case BQ : m_material += 9; break;
        default : break;
    }

    switch (move.GetPromoted())
    {
        case WN : case BN : m_material += 3-1; break;
        case WB : case BB : m_material += 3-1; break;
        case WR : case BR : m_material += 5-1; break;
        case WQ : case BQ : m_material += 9-1; break;
        default : break;
    }

    // If rook is captured, then update castling rights.
    switch (move.GetCaptured())
    {
        case WR :
            if (move.To() == H1)
                m_castleRights &= ~CASTLE_WHITE_SHORT;
            if (move.To() == A1)
                m_castleRights &= ~CASTLE_WHITE_LONG;
            break;
        case BR :
            if (move.To() == H8)
                m_castleRights &= ~CASTLE_BLACK_SHORT;
            if (move.To() == A8)
                m_castleRights &= ~CASTLE_BLACK_LONG;
            break;
        default : break;
    }

    switch (m_board[move.From()])
    {
        case WK :
            if (move.From() == E1 &&
                    move.To() == G1) // White castling short
            {
                m_board[H1] = EM;
                m_board[F1] = WR;
            }
            else if (move.From() == E1 &&
                    move.To() == C1 ) // White castling long
            {
                m_board[A1] = EM;
                m_board[D1] = WR;
            }

            m_castleRights &= ~(CASTLE_WHITE_SHORT | CASTLE_WHITE_LONG);
            break;

        case BK :
            if (move.From() == E8 &&
                    move.To() == G8) // Black castling short
            {
                m_board[H8] = EM;
                m_board[F8] = BR;
            }
            else if (move.From() == E8 &&
                    move.To() == C8) // Black castling long
            {
                m_board[A8] = EM;
                m_board[D8] = BR;
            }

            m_castleRights &= ~(CASTLE_BLACK_SHORT | CASTLE_BLACK_LONG);
            break;

        case WR :
            if (move.From() == H1) // White moves king rook
            {
                m_castleRights &= ~(CASTLE_WHITE_SHORT);
            }
            else if (move.From() == A1) // White moves queen rook
            {
                m_castleRights &= ~(CASTLE_WHITE_LONG);
            }
            break;

        case BR :
            if (move.From() == H8) // Black moves king rook
            {
                m_castleRights &= ~(CASTLE_BLACK_SHORT);
            }
            else if (move.From() == A8) // Black moves queen rook
            {
                m_castleRights &= ~(CASTLE_BLACK_LONG);
            }
            break;

        case WP :
            if (move.To() - move.From() == 20) // White double pawn move
            {
                m_enPassantSquare = move.From() + 10;
            }
            else if (move.To() - move.From() != 10) // White pawn capture
            {
                if (m_board[move.To()] == EM) { // En-passant capture
                    m_board[move.To() - 10] = EM;
                }
            }
            break;

        case BP :
            if (move.From() - move.To() == 20) // Black double pawn move
            {
                m_enPassantSquare = move.To() + 10;
            }
            else if (move.From() - move.To() != 10) // Black pawn capture
            {
                if (m_board[move.To()] == EM) { // En-passant capture
                    m_board[move.To() + 10] = EM;
                }
            }
            break;

        default :
            break;
    } // end of switch

    m_board[move.To()] = m_board[move.From()];
    if (move.GetPromoted() != EM)
        m_board[move.To()] = move.GetPromoted();
    m_board[move.From()] = EM;
    m_side_to_move = -m_side_to_move;
    m_material = -m_material;
} // end of void CBoard::make_move(const CMove &move)


/***************************************************************
 * undo_move
 * This reverses the effect of make_move
 ***************************************************************/
void CBoard::undo_move(const CMove &move)
{
    m_material = -m_material;
    switch (move.GetCaptured())
    {
        case WP : case BP : m_material -= 1; break;
        case WN : case BN : m_material -= 3; break;
        case WB : case BB : m_material -= 3; break;
        case WR : case BR : m_material -= 5; break;
        case WQ : case BQ : m_material -= 9; break;
        default : break;
    }

    switch (move.GetPromoted())
    {
        case WN : case BN : m_material -= 3-1; break;
        case WB : case BB : m_material -= 3-1; break;
        case WR : case BR : m_material -= 5-1; break;
        case WQ : case BQ : m_material -= 9-1; break;
        default : break;
    }

    m_enPassantSquare = 0;
    switch (m_board[move.To()])
    {
        case WP :
            if ((move.To() - move.From())%10 != 0 && move.GetCaptured() == EM)
            {
                m_enPassantSquare = move.To();
                m_board[m_enPassantSquare - 10] = BP;
            }
            break;

        case BP :
            if ((move.From() - move.To())%10 != 0 && move.GetCaptured() == EM)
            {
                m_enPassantSquare = move.To();
                m_board[m_enPassantSquare + 10] = WP;
            }
            break;

        case WK :
            if (move.From() == E1 &&
                    move.To() == G1) // White castling short
            {
                m_board[F1] = EM;
                m_board[H1] = WR;
            }
            else if (move.From() == E1 &&
                    move.To() == C1) // White castling long
            {
                m_board[D1] = EM;
                m_board[A1] = WR;
            }
            break;

        case BK :
            if (move.From() == E8 &&
                    move.To() == G8) // Black castling short
            {
                m_board[F8] = EM;
                m_board[H8] = BR;
            }
            else if (move.From() == E8 &&
                    move.To() == C8) // Black castling long
            {
                m_board[D8] = EM;
                m_board[A8] = BR;
            }
            break;

        default :
            break;
    } // end of switch

    m_board[move.From()] = move.GetPiece();
    m_board[move.To()] = move.GetCaptured();
    m_side_to_move = -m_side_to_move;

    if (!m_state.empty())
    {
        uint32_t state = m_state.back();
        m_enPassantSquare = state >> 8;
        m_castleRights = state & 0xFF;
        m_state.pop_back();
    }
    else
    {
        m_enPassantSquare = 0;
    }
} // end of void CBoard::undo_move(const CMove &move)


/***************************************************************
 * IsMoveValid
 * This returns true, if the move is legal.
 ***************************************************************/
bool CBoard::IsMoveValid(CMove &move) const
{
    CMoveList moves;
    find_legal_moves(moves);
    for (unsigned int i=0; i<moves.size(); ++i)
    {
        if (moves[i] == move)
        {
            move.SetPiece(m_board[move.From()]);
            move.SetCaptured(m_board[move.To()]);
            return true;
        }
    }
    return false;
} // end of bool CBoard::IsMoveValid(CMove &move)


/***************************************************************
 * get_value
 *
 * It returns an integer value showing how good the position
 * is for the side to move.
 *
 * This is a very simple evaluation function. Only two factors are
 * considered:
 * 1. The material balance:
 *      * Pawn   100
 *      * Knight 300
 *      * Bishop 300
 *      * Rook   500
 *      * Queen  900
 * 2. The difference in number of legal moves of both players.
 *
 * The latter tends to favor positions, where the computer
 * has many legal moves. This implies centralization and development.
 ***************************************************************/
int CBoard::get_value()
{
    CMoveList moves;
    find_legal_moves(moves);
    int my_moves = moves.size();
    swap_sides();

    find_legal_moves(moves);
    int his_moves = moves.size();
    swap_sides();

    return (my_moves-his_moves) + 100*m_material;
} // end of int CBoard::get_value()


/***************************************************************
 * Returns true if player to move is in check.
 ***************************************************************/
bool CBoard::isKingInCheck() const
{
    e_piece king;
    CSquare kingSquare = 0;

    if (m_side_to_move == 1)
        king = WK;
    else
        king = BK;

    // Look for our king
    for (int i=A1; i<=H8; ++i)
    {
        if (m_board[i] == king)
        {
            kingSquare = i;
            break;
        }
    }

    assert (kingSquare != 0); // The king MUST be somewhere

    return isSquareThreatened(kingSquare);

} // end of isKingInCheck


/***************************************************************
 * Returns true if player NOT to move is in check.
 ***************************************************************/
bool CBoard::isOtherKingInCheck() const
{
    ((CBoard *)this)->m_side_to_move = -m_side_to_move;
    bool retval = isKingInCheck();
    ((CBoard *)this)->m_side_to_move = -m_side_to_move;
    return retval;
} // end of isOtherKingInCheck


#ifdef DEBUG_HASH
/***************************************************************
 * calcHash
 ***************************************************************/
uint32_t CBoard::calcHash() const
{
    CRC32 crc;

    for (int i=0; i<120; ++i)
    {
        crc.update(m_board[i]);
    }
    crc.update(m_side_to_move);
    crc.update(m_castleRights);
    crc.update(m_enPassantSquare);
    crc.update(m_material);

    return crc.get();
} // end of calcHash
#endif


/***************************************************************
 * copy constructor
 ***************************************************************/
CBoard::CBoard(const CBoard& rhs)
    : m_board(), m_state(), m_side_to_move(), m_castleRights(),
    m_enPassantSquare(), m_material(), m_halfMoves(), m_fullMoves()
{
    m_side_to_move    = rhs.m_side_to_move;
    m_castleRights    = rhs.m_castleRights;
    m_enPassantSquare = rhs.m_enPassantSquare;
    m_material        = rhs.m_material;

    m_board.clear();
    m_board.reserve(120);
    for (int i=0; i<120; ++i)
    {
        m_board[i] = rhs.m_board[i];
    }

    m_state.clear();
    for (std::vector<uint32_t>::const_iterator it = rhs.m_state.begin();
            it != rhs.m_state.end(); ++it)
    {
        m_state.push_back(*it);
    }
} // end of copy constructor

