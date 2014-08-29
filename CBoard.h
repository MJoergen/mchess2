#include <vector>
#include <string>

#include "CMove.h"
#include "CMoveList.h"

#ifndef _C_BOARD_H_
#define _C_BOARD_H_

// The board is represented as an array of 120 elements.
// arranged in 12 rows and 10 columns.
// The following diagram shows the numbering of the elements.
// Note: The square A1 is element number 21
//
// 111 118 112 113 114 115 116 117 118 119
// 100 108 102 103 104 105 106 107 108 109
//  90  A8  92  93  94  95  96  97  H8  99
//  80  88  82  83  84  85  86  87  88  89
//  70  71  72  73  74  75  76  77  78  79
//  60  61  62  63  64  65  66  67  68  69
//  50  51  52  53  54  55  56  57  58  59
//  40  41  42  43  44  45  46  47  48  49
//  30  31  32  33  34  35  36  37  H2  39
//  20  A1  22  23  24  25  26  27  H1  29
//  10  11  12  13  14  15  16  17  18  19
//   0   1   2   3   4   5   6   7   8   9
//
// Each element contains one of the following values:
//   0  :  Empty
//   1  :  White Pawn
//   2  :  White Knight
//   3  :  White Bishop
//   4  :  White Rook
//   5  :  White Queen
//   6  :  White King
//   -1 :  Black Pawn
//   -2 :  Black Knight
//   -3 :  Black Bishop
//   -4 :  Black Rook
//   -5 :  Black Queen
//   -6 :  Black King
//   99 :  INVALID

enum // Directions
{
    N = 10, S = -10, E = -1, W = 1,
    NW = 11, SW = -9, NE = 9, SE = -11,
    NNW = 21, NNE = 19, NWW = 12, NEE = 8,
    SSW = -19, SSE = -21, SWW = -8, SEE = -12
};

/***************************************************************
 * declaration of CBoard
 ***************************************************************/
class CBoard
{
    public:
        CBoard() : m_board(), m_state(), m_side_to_move(), m_castleRights(),
            m_enPassantSquare(), m_material(), m_halfMoves(), m_fullMoves()
            { newGame(); }

        // Copy constructor
        CBoard(const CBoard& rhs);

        void newGame();
        bool read_from_fen(const char *fen, const char **endptr = NULL);
        CMove readMove(const char *fen, const char **endptr) const;
        void find_legal_moves(CMoveList &moves) const;
        void make_move(const CMove &move);
        void undo_move(const CMove &move);
        int  get_value();
        bool IsMoveValid(CMove &move) const;
#ifdef DEBUG_HASH
        uint32_t calcHash() const;
#endif
        bool isKingInCheck() const;
        bool isOtherKingInCheck() const;
        bool whiteToMove() const {return m_side_to_move > 0;}

        friend std::ostream& operator <<(std::ostream &os, const CBoard &rhs);
        friend class CHashEntry;

    private:
        void calcMaterial();
        bool isSquareThreatened(const CSquare& sq) const;
        void swap_sides() {m_side_to_move = -m_side_to_move;}

        std::vector<int8_t>   m_board;
        std::vector<uint32_t> m_state; // Contains a log of m_enPassantSquare and m_castleRights
        int m_side_to_move;
        int m_castleRights;
#define CASTLE_WHITE_SHORT (1<<0)
#define CASTLE_WHITE_LONG  (1<<1)
#define CASTLE_BLACK_SHORT (1<<2)
#define CASTLE_BLACK_LONG  (1<<3)
        int m_enPassantSquare;
        int m_material;
        int m_halfMoves;
        int m_fullMoves;

}; // end of class CBoard

#endif // _C_BOARD_H_

