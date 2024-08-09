#include <string>

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
