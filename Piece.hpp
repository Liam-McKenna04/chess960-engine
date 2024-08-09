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

    static char toEPDChar(int piece) {
        char pieceChars[] = " KQBNRP";
        char c = pieceChars[abs(piece)];
        return (piece > 0) ? c : std::tolower(c);
    }

    // New function to convert EPD character to piece
    static int fromEPDChar(char c) {
        std::string pieceChars = " KQBNRP";
        int color = std::isupper(c) ? White : Black;
        int type = pieceChars.find(std::toupper(c));
        return create(type, color);
    }

    static std::string boardToEPD(const int board[64]) {
        std::ostringstream epd;
        int emptyCount = 0;

        for (int rank = 0; rank < 8; ++rank) {
            for (int file = 0; file < 8; ++file) {
                int piece = board[rank * 8 + file];
                if (piece == Empty) {
                    emptyCount++;
                } else {
                    if (emptyCount > 0) {
                        epd << emptyCount;
                        emptyCount = 0;
                    }
                    epd << toEPDChar(piece);
                }
            }
            if (emptyCount > 0) {
                epd << emptyCount;
                emptyCount = 0;
            }
            if (rank < 7) epd << '/';
        }

        return epd.str();
    }

    static void EPDToBoard(const std::string& epd, int board[64]) {
        std::istringstream iss(epd);
        std::string token;
        int index = 0;

        while (std::getline(iss, token, '/')) {
            for (char c : token) {
                if (std::isdigit(c)) {
                    int emptyCount = c - '0';
                    for (int i = 0; i < emptyCount; ++i) {
                        board[index++] = Empty;
                    }
                } else {
                    board[index++] = fromEPDChar(c);
                }
            }
        }
    }
};
