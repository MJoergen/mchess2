#ifndef _C_MOVE_H_
#define _C_MOVE_H_

#include "CSquare.h"

typedef enum {
    EM = 0,   // Empty
    WP = 1,   // White Pawn
    WN = 2,   // White Knight
    WB = 3,   // White Bishop
    WR = 4,   // White Rook
    WQ = 5,   // White Queen
    WK = 6,   // White King
    BP = -1,  // Black Pawn
    BN = -2,  // Black Knight
    BB = -3,  // Black Bishop
    BR = -4,  // Black Rook
    BQ = -5,  // Black Queen
    BK = -6,  // Black King
    IV = 99   // INVALID
} e_piece;

/***************************************************************
 * declaration of CMove
 ***************************************************************/

class CMove
{
    public:

        // Constructors
        CMove() : m_from(), m_to(), m_piece(IV), m_captured(IV), m_promoted() {}

        CMove(const char *str) : m_from(), m_to(), m_piece(IV), m_captured(IV), m_promoted()
            { FromString(str); }

        CMove(int8_t piece, const CSquare& from, const CSquare& to, int8_t captured = EM, int8_t promoted=EM) :
            m_from(from), m_to(to), m_piece(piece), m_captured(captured), m_promoted(promoted) {}


        // Modifiers
        const char * FromString(const char *s); // Returns NULL if error

        void SetCaptured(int8_t captured) { m_captured = captured; }
        void SetPiece(int8_t piece) { m_piece = piece; }


        // Accessor functions
        std::string ToShortString() const;
        std::string ToLongString() const;

        CSquare     From(void) const {return m_from;}
        CSquare     To(void) const {return m_to;}
        int8_t      GetPiece(void) const { return m_piece; }
        int8_t      GetCaptured(void) const { return m_captured; }
        int8_t      GetPromoted(void) const { return m_promoted; }

        // Utility functions
        bool        Valid(void) const { return GetCaptured() != IV; }
        bool        is_captured_piece_a_king(void) const { return (GetCaptured() == WK || GetCaptured() == BK); }
        bool        is_it_a_capture(void) const { return (GetCaptured() != EM); }
        bool        operator==(const CMove& rhs) const
        {
            if (rhs.From() != From())
                return false;
            if (rhs.To() != To())
                return false;
            return true;
        }

    private:
        CSquare     m_from;
        CSquare     m_to;
        int8_t      m_piece;
        int8_t      m_captured;
        int8_t      m_promoted;
}; /* end of CMove */

std::ostream& operator <<(std::ostream &os, const CMove &rhs);

#endif // _C_MOVE_H_

