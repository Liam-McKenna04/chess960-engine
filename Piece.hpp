#ifndef PIECE_HPP
#define PIECE_HPP

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

class Piece {
   public:
    // Piece types
    static const int Empty = 0;
    static const int King = 1;
    static const int Queen = 2;
    static const int Bishop = 3;
    static const int Knight = 4;
    static const int Rook = 5;
    static const int Pawn = 6;

    // Colors
    static const int White = 1;
    static const int Black = -1;

    // Utility functions
    static int getType(int piece) { return abs(piece); }

    static int getColor(int piece) { return (piece > 0) ? White : Black; }

    static bool matchesColor(int piece1, int piece2) {
        // checks if 2 ints are both positive or negative, returns false if
        // either is 0
        return ((piece1 != 0 && piece2 != 0 && ((piece1 < 0) == (piece2 < 0))));
    }

    static bool isPiece(int piece, int pieceType) {
        return abs(piece) == pieceType;
    }

    static std::string getName(int piece) {
        switch (abs(piece)) {
            case King:
                return "King";
            case Queen:
                return "Queen";
            case Bishop:
                return "Bishop";
            case Knight:
                return "Knight";
            case Rook:
                return "Rook";
            case Pawn:
                return "Pawn";
            default:
                return "Empty";
        }
    }

    static int create(int type, int color) { return type * color; }
};
#endif